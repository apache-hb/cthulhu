#include "base/log.h"
#include "cthulhu/events/events.h"
#include "cthulhu/mediator/check.h"
#include "cthulhu/mediator/interface.h"

#include "io/console.h"
#include "memory/memory.h"
#include "notify/colour.h"
#include "notify/notify.h"
#include "notify/text.h"
#include "scan/node.h"
#include "support/langs.h"

#include "cthulhu/check/check.h"

#include "cthulhu/emit/emit.h"
#include "cthulhu/ssa/ssa.h"

#include "base/panic.h"
#include "core/macros.h"

#include "std/map.h"
#include "std/str.h"
#include "std/vector.h"

#include "fs/fs.h"
#include "io/io.h"

#include "argparse/argparse.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK_REPORTS(reports, msg)                                                                \
    do                                                                                             \
    {                                                                                              \
        int err = end_reports(reports, msg, kReportConfig);                                        \
        if (err != 0)                                                                              \
        {                                                                                          \
            return err;                                                                            \
        }                                                                                          \
    } while (0)

static const version_info_t kVersion = {
    .license = "GPLv3",
    .desc = "Test harness",
    .author = "Elliot Haisley",
    .version = NEW_VERSION(0, 0, 1),
};

static io_t *make_file(const char *path, os_access_t flags, arena_t *arena)
{
    io_t *io = io_file(path, flags, arena);
    CTASSERT(io_error(io) == 0);
    return io;
}

typedef struct user_ptr_t
{
    uint32_t size;
    FIELD_SIZE(size) char data[];
} user_ptr_t;

typedef struct user_arena_t
{
    char *memory_start;
    char *memory_cursor;
    char *memory_end;

    size_t alloc_count;
    size_t realloc_count;
    size_t free_count;
} user_arena_t;

static user_ptr_t *get_memory(user_arena_t *arena, size_t size)
{
    CTASSERTF(arena->memory_cursor + size + sizeof(user_ptr_t) < arena->memory_end,
              "out of memory");

    // TODO: align all allocations to 16 bytes

    user_ptr_t *ptr = (user_ptr_t *)arena->memory_cursor;
    ptr->size = (uint32_t)size;
    arena->memory_cursor += size + sizeof(user_ptr_t);

    arena->alloc_count++;

    return ptr;
}

static user_ptr_t *get_ptr(void *ptr)
{
    char *data = (char *)ptr;
    return (user_ptr_t *)(data - sizeof(user_ptr_t));
}

static void *user_malloc(const mem_t *mem, size_t size)
{
    arena_t *alloc = mem_arena(mem);
    user_arena_t *user = (user_arena_t *)alloc->user;

    user_ptr_t *ptr = get_memory(user, size);
    return ptr->data;
}

static void *user_realloc(const mem_t *mem, void *ptr, size_t new_size, size_t old_size)
{
    CTU_UNUSED(old_size);

    arena_t *alloc = mem_arena(mem);
    user_arena_t *user = (user_arena_t *)alloc->user;

    user_ptr_t *old = get_ptr(ptr);

    if (old->size >= new_size) return old->data;

    user_ptr_t *new = get_memory(user, new_size);
    memcpy(new->data, old->data, old->size);

    user->realloc_count++;

    return new->data;
}

static void user_free(const mem_t *mem, void *ptr, size_t size)
{
    CTU_UNUSED(ptr);
    CTU_UNUSED(size);

    arena_t *alloc = mem_arena(mem);

    user_arena_t *user = (user_arena_t *)alloc->user;

    user->free_count++;
}

static user_arena_t new_user_arena(size_t size)
{
    char *memory = malloc(size);
    CTASSERT(memory != NULL);

    user_arena_t arena = {
        .memory_start = memory,
        .memory_cursor = memory,
        .memory_end = memory + size,

        .alloc_count = 0,
        .realloc_count = 0,
        .free_count = 0,
    };

    return arena;
}

static arena_t new_alloc(user_arena_t *arena)
{
    arena_t alloc = {
        .name = "user",
        .fn_malloc = user_malloc,
        .fn_realloc = user_realloc,
        .fn_free = user_free,
        .user = arena,
    };

    return alloc;
}

static int check_reports(logger_t *logger, report_config_t config, const char *title)
{
    int err = text_report(logger_get_events(logger), config, title);
    return err;
}

#define CHECK_LOG(logger, fmt)                               \
    do                                                       \
    {                                                        \
        int err = check_reports(logger, report_config, fmt); \
        if (err != EXIT_OK)                                  \
        {                                                    \
            return err;                                      \
        }                                                    \
    } while (0)

int run_test_harness(int argc, const char **argv, arena_t *alloc)
{
    mediator_t *mediator = mediator_new("example", kVersion);
    lifetime_t *lifetime = lifetime_new(mediator, alloc);

    langs_t langs = get_langs(alloc);

    logger_t *reports = lifetime_get_logger(lifetime);
    for (size_t i = 0; i < langs.size; i++)
    {
        const language_t *lang = langs.langs + i;
        lifetime_add_language(lifetime, lang);
    }

    io_t *msg_buffer = io_stdout(alloc);

    text_config_t text_config = {
        .config = {
            .zeroth_line = false,
            .print_source = true,
            .print_header = true,
            .max_columns = 80
        },
        .colours = colour_get_disabled(),
        .io = msg_buffer,
    };

    report_config_t report_config = {
        .report_format = eTextSimple,
        .text_config = text_config,
    };

    CHECK_LOG(reports, "adding languages");

    // harness.exe <name> [files...]
    CTASSERT(argc > 2);

    for (int i = 2; i < argc; i++)
    {
        const char *path = argv[i];
        const char *ext = str_ext(path);
        const language_t *lang = lifetime_get_language(lifetime, ext);

        io_t *io = make_file(path, eAccessRead | eAccessText, alloc);

        lifetime_parse(lifetime, lang, io);

        CHECK_LOG(reports, "parsing source");
    }

    for (size_t stage = 0; stage < eStageTotal; stage++)
    {
        lifetime_run_stage(lifetime, stage);

        char *msg = format("running stage %s", stage_to_string(stage));
        CHECK_LOG(reports, msg);
    }

    lifetime_resolve(lifetime);
    CHECK_LOG(reports, "resolving symbols");

    map_t *modmap = lifetime_get_modules(lifetime);

    check_tree(reports, modmap);
    CHECK_LOG(reports, "validations failed");

    ssa_result_t ssa = ssa_compile(modmap);
    CHECK_LOG(reports, "generating ssa");

    ssa_opt(reports, ssa);
    CHECK_LOG(reports, "optimizing ssa");

    fs_t *fs = fs_virtual("out", alloc);

    emit_options_t base_options = {
        .arena = alloc,
        .reports = reports,
        .fs = fs,

        .modules = ssa.modules,
        .deps = ssa.deps,
    };

    ssa_emit_options_t emit_options = {.opts = base_options};

    ssa_emit_result_t ssa_emit_result = emit_ssa(&emit_options);
    CHECK_LOG(reports, "emitting ssa");
    CTU_UNUSED(ssa_emit_result); // TODO: check for errors

    c89_emit_options_t c89_emit_options = {.opts = base_options};

    c89_emit_result_t c89_emit_result = emit_c89(&c89_emit_options);
    CHECK_LOG(reports, "emitting c89");

    OS_RESULT(const char *) cwd = os_dir_current();
    CTASSERTF(os_error(cwd) == 0, "failed to get cwd %s", os_error_string(os_error(cwd)));

    const char *test_dir = format("%s" NATIVE_PATH_SEPARATOR "test-out",
                                 OS_VALUE(const char *, cwd));
    const char *run_dir = format("%s" NATIVE_PATH_SEPARATOR "%s", test_dir, argv[1]);

    fs_t *out = fs_physical(run_dir, alloc);
    if (out == NULL)
    {
        msg_notify(reports, &kEvent_FailedToCreateOutputDirectory, node_builtin(), "failed to create output directory");
    }
    CHECK_LOG(reports, "creating output directory");

    sync_result_t result = fs_sync(out, fs);
    if (result.path != NULL)
    {
        msg_notify(reports, &kEvent_FailedToWriteOutputFile, node_builtin(), "failed to sync %s", result.path);
    }
    CHECK_LOG(reports, "syncing output directory");

    size_t len = vector_len(c89_emit_result.sources);
    vector_t *sources = vector_of(len);
    for (size_t i = 0; i < len; i++)
    {
        const char *part = vector_get(c89_emit_result.sources, i);
        char *path = format("%s" NATIVE_PATH_SEPARATOR "%s", run_dir, part);
        vector_set(sources, i, path);
    }

#if OS_WINDOWS
    const char *lib_dir = format("%s" NATIVE_PATH_SEPARATOR "lib", run_dir);

    OS_RESULT(bool) create = os_dir_create(lib_dir);
    CTASSERTF(os_error(create) == 0, "failed to create dir `%s` %s", lib_dir,
              os_error_string(os_error(create)));

    int status = system(
        format("cl /nologo /c %s /I%s\\include /Fo%s\\", str_join(" ", sources), run_dir, lib_dir));
    if (status != 0)
    {
        msg_notify(reports, &kEvent_FailedToWriteOutputFile, node_builtin(), "compilation failed `%d`", status);
    }
#else
    int status = system(format("cd %s && cc %s -c -Iinclude", runDir, str_join(" ", sources)));
    if (WEXITSTATUS(status) != EXIT_OK)
    {
        msg_notify(reports, &kEvent_FailedToWriteOutputFile, node_builtin(), "compilation failed %d", WEXITSTATUS(status));
    }
#endif

    CHECK_LOG(reports, "compiling");

    return 0;
}

int main(int argc, const char **argv)
{
    size_t size = (size_t)(1024U * 1024U * 64U);
    user_arena_t arena = new_user_arena(size);
    arena_t alloc = new_alloc(&arena);
    init_gmp_alloc(&alloc);

    ctu_log_control(eLogEnable);

    int result = run_test_harness(argc, argv, &alloc);

    free(arena.memory_start);

    return result;
}
