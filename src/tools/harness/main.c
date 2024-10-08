// SPDX-License-Identifier: GPL-3.0-only

#include "base/util.h"
#include "setup/setup.h"
#include "format/colour.h"
#include "base/log.h"
#include "cthulhu/events/events.h"
#include "cthulhu/broker/broker.h"

#include "arena/arena.h"
#include "io/console.h"
#include "memory/memory.h"
#include "notify/notify.h"
#include "format/notify.h"
#include "scan/node.h"
#include "support/loader.h"

#include "cthulhu/check/check.h"

#include "cthulhu/ssa/ssa.h"

#include "base/panic.h"
#include "core/macros.h"

#include "std/map.h"
#include "std/str.h"
#include "std/vector.h"

#include "fs/fs.h"
#include "io/io.h"
#include "os/os.h"

#include "argparse/argparse.h"
#include "support/support.h"

#include <stddef.h>
#include <stdlib.h> // for malloc, free, system

#define CHECK_REPORTS(reports, msg)                         \
    do                                                      \
    {                                                       \
        int err = end_reports(reports, msg, kReportConfig); \
        if (err != 0)                                       \
        {                                                   \
            return err;                                     \
        }                                                   \
    } while (0)

static const frontend_t kFrontendHarness = {
    .info = {
        .id = "frontend-harness",
        .name = "Test Harness",
        .version = {
            .license = "GPLv3",
            .desc = "End to end test harness",
            .author = "Elliot Haisley",
            .version = CT_NEW_VERSION(0, 0, 2),
        },
    }
};

static io_t *make_file(const char *path, os_access_t flags, arena_t *arena)
{
    io_t *io = io_file(path, flags, arena);
    os_error_t err = io_error(io);
    CTASSERTF(err == 0, "failed to open file `%s` (%s)", path, os_error_string(err, arena));
    return io;
}

typedef struct user_ptr_t
{
    uint32_t size;
    uint32_t pad0;
    uint32_t pad1;
    uint32_t pad2;

    STA_FIELD_SIZE(size) char data[];
} user_ptr_t;

CT_STATIC_ASSERT(sizeof(user_ptr_t) == 16, "user_ptr_t must be 16 byte aligned");

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
    size_t aligned = CT_ALIGN_POW2(size, 16);
    size_t space = aligned + sizeof(user_ptr_t); // required space

    CTASSERTF(arena->memory_cursor + space < arena->memory_end,
              "out of memory (size: %zu)", size);

    user_ptr_t *ptr = (user_ptr_t *)arena->memory_cursor;

    // align the pointer itself
    ptr = (user_ptr_t *)CT_ALIGN_POW2((uintptr_t)ptr, 16);
    ptr->size = (uint32_t)size;
    arena->memory_cursor = (char *)ptr + aligned + sizeof(user_ptr_t);

    arena->alloc_count++;

    return ptr;
}

static user_ptr_t *get_ptr(void *ptr)
{
    char *data = (char *)ptr;
    return (user_ptr_t *)(data - sizeof(user_ptr_t));
}

static void *user_malloc(size_t size, void *user)
{
    user_arena_t *data = (user_arena_t *)user;

    user_ptr_t *ptr = get_memory(data, size);
    return ptr->data;
}

static void *user_realloc(void *ptr, size_t new_size, size_t old_size, void *user)
{
    CT_UNUSED(old_size);

    user_arena_t *data = (user_arena_t *)user;

    user_ptr_t *old = get_ptr(ptr);

    if (old->size >= new_size) return old->data;

    user_ptr_t *new = get_memory(data, new_size);
    ctu_memcpy(new->data, old->data, old->size);

    data->realloc_count++;

    return new->data;
}

static void user_free(void *ptr, size_t size, void *user)
{
    CT_UNUSED(ptr);
    CT_UNUSED(size);

    user_arena_t *data = (user_arena_t *)user;

    data->free_count++;
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

typedef struct arena_user_wrap_t
{
    arena_t arena;
    user_arena_t user;
} arena_user_wrap_t;

static arena_t new_alloc(user_arena_t *user)
{
    arena_t arena = {
        .name = "user",
        .fn_malloc = user_malloc,
        .fn_realloc = user_realloc,
        .fn_free = user_free,
        .user = user,
    };

    return arena;
}

static int check_reports(logger_t *logger, report_config_t config, const char *title)
{
    int err = text_report(logger_get_events(logger), config, title);
    logger_reset(logger);
    return err;
}

#define CHECK_LOG(logger, fmt)                                  \
    do                                                          \
    {                                                           \
        int log_ok = check_reports(logger, report_config, fmt); \
        if (log_ok != CT_EXIT_OK)                                  \
        {                                                       \
            return log_ok;                                      \
        }                                                       \
    } while (0)

int run_test_harness(int argc, const char **argv, arena_t *arena)
{
    // harness.exe <name> [files...]
    CTASSERT(argc > 2);

    broker_t *broker = broker_new(&kFrontendHarness, arena);
    loader_t *loader = loader_new(arena);
    support_t *support = support_new(broker, loader, arena);

    char *cwd = os_cwd_string(arena);
    CTASSERTF(ctu_strlen(cwd), "failed to get cwd");

    // test name
    const char *name = argv[1];
    int start = 2;

#if CT_BUILD_SHARED
    start = 3;
    loaded_module_t mod = {0};
    CTASSERTF(support_load_module(support, eModLanguage, argv[2], &mod), "failed to load module `%s` (%s: %s)", argv[2], load_error_string(mod.error), os_error_string(mod.os, arena));
#else
    support_load_default_modules(support);
#endif

    logger_t *logger = broker_get_logger(broker);
    const node_t *node = broker_get_node(broker);

    io_t *msg_buffer = io_stdout();

    text_config_t text_config = {
        .config = {
            .zeroth_line = false,
            .max_columns = 80,
        },
        .colours = &kColourNone,
        .io = msg_buffer,
    };

    report_config_t report_config = {
        .report_format = eTextSimple,
        .text_config = text_config,
    };

    CHECK_LOG(logger, "adding languages");

    broker_init(broker);

    CTASSERTF(start < argc, "no files to parse");

    for (int i = start; i < argc; i++)
    {
        const char *path = argv[i];
        const char *ext = str_ext(path, arena);
        CTASSERTF(ext != NULL, "no extension for file `%s`", path);
        language_runtime_t *lang = support_get_lang(support, ext);
        CTASSERTF(lang != NULL, "no language for extension `%s`", ext);

        io_t *io = make_file(path, eOsAccessRead, arena);

        broker_parse(lang, io);

        CHECK_LOG(logger, "parsing source");
    }

    for (size_t stage = 0; stage < ePassCount; stage++)
    {
        broker_run_pass(broker, stage);

        char *msg = str_format(arena, "running stage %s", broker_pass_name(stage));
        CHECK_LOG(logger, msg);
    }

    broker_resolve(broker);
    CHECK_LOG(logger, "resolving symbols");

    vector_t *mods = broker_get_modules(broker);

    check_tree(logger, mods, arena);
    CHECK_LOG(logger, "validation");

    ssa_result_t ssa = ssa_compile(mods, arena);
    CHECK_LOG(logger, "generating ssa");

    ssa_opt(logger, ssa, arena);
    CHECK_LOG(logger, "optimizing ssa");

    fs_t *fs = fs_virtual("out", arena);

    target_runtime_t *debug = support_get_target(support, "debug");
    target_runtime_t *cfamily = support_get_target(support, "cfamily");

    CTASSERT(debug != NULL);
    CTASSERT(cfamily != NULL);

    target_emit_t emit = {
        .layout = eFileLayoutFlat,
        .fs = fs,
    };

    target_emit_ssa(debug, &ssa, &emit);
    CHECK_LOG(logger, "emitting debug ssa");

    emit_result_t cfamily_result = target_emit_ssa(cfamily, &ssa, &emit);
    CHECK_LOG(logger, "emitting cfamily ssa");

#if 0
    emit_options_t base_options = {
        .arena = arena,
        .reports = logger,
        .fs = fs,

        .modules = ssa.modules,
        .deps = ssa.deps,
    };

    ssa_emit_options_t emit_options = {.opts = base_options};

    ssa_emit_result_t ssa_emit_result = emit_ssa(&emit_options);
    CHECK_LOG(logger, "emitting ssa");
    CT_UNUSED(ssa_emit_result); // TODO: check for errors


    c89_emit_options_t c89_emit_options = {.opts = base_options};

    c89_emit_result_t c89_emit_result = emit_c89(&c89_emit_options);
    CHECK_LOG(logger, "emitting c89");
#endif
    const char *test_dir = str_format(arena, "%s" CT_NATIVE_PATH_SEPARATOR "test-out", cwd);
    const char *run_dir = str_format(arena, "%s" CT_NATIVE_PATH_SEPARATOR "%s", test_dir, name);

    fs_t *out = fs_physical(run_dir, arena);
    if (out == NULL)
    {
        msg_notify(logger, &kEvent_FailedToCreateOutputDirectory, node,
                   "failed to create output directory");
    }
    CHECK_LOG(logger, "creating output directory");

    sync_result_t result = fs_sync(out, fs);
    if (result.path != NULL)
    {
        msg_notify(logger, &kEvent_FailedToWriteOutputFile, node, "failed to sync %s",
                   result.path);
    }
    CHECK_LOG(logger, "syncing output directory");

    size_t len = vector_len(cfamily_result.files);
    vector_t *sources = vector_of(len, arena);
    for (size_t i = 0; i < len; i++)
    {
        const char *part = vector_get(cfamily_result.files, i);
        char *path = str_format(arena, "%s" CT_NATIVE_PATH_SEPARATOR "%s", run_dir, part);
        vector_set(sources, i, path);
    }

#if CT_OS_WINDOWS
    const char *lib_dir = str_format(arena, "%s" CT_NATIVE_PATH_SEPARATOR "lib", run_dir);

    os_error_t cwd_err = os_dir_create(lib_dir);
    CTASSERTF(cwd_err == eOsExists || cwd_err == eOsSuccess, "failed to create dir `%s` %s", lib_dir, os_error_string(cwd_err, arena));

    char *cmd = str_format(arena, "cl /nologo /WX /W2 /c %s /I%s\\include /Fo%s\\", str_join(" ", sources, arena), run_dir, lib_dir);
    int status = system(cmd); // NOLINT
    if (status != 0)
    {
        msg_notify(logger, &kEvent_FailedToWriteOutputFile, node,
                   "compilation failed `%d`", status);
    }
#else
#   define CC_FLAGS "-Werror -Wno-format-contains-nul -Wno-unused-variable -Wno-unused-function"
    char *cmd = str_format(arena, "cd %s && cc %s -c -Iinclude " CC_FLAGS, run_dir, str_join(" ", sources, arena));
    int status = system(cmd); // NOLINT
    if (WEXITSTATUS(status) != CT_EXIT_OK)
    {
        msg_notify(logger, &kEvent_FailedToWriteOutputFile, node,
                   "compilation failed %d", WEXITSTATUS(status));
    }
#endif

    broker_deinit(broker);

    CHECK_LOG(logger, "compiling");

    return 0;
}

int main(int argc, const char **argv)
{
    setup_default(NULL);

    size_t size = (size_t)(1024U * 1024U * 64U);
    user_arena_t arena = new_user_arena(size);
    arena_t user = new_alloc(&arena);
    init_global_arena(&user);
    init_gmp_arena(&user);

    ctu_log_update(true);

    int result = run_test_harness(argc, argv, &user);

    free(arena.memory_start);

    return result;
}
