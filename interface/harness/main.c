#include "cthulhu/mediator/interface.h"
#include "cthulhu/mediator/check.h"

#include "memory/memory.h"
#include "support/langs.h"

#include "cthulhu/check/check.h"

#include "cthulhu/ssa/ssa.h"
#include "cthulhu/emit/emit.h"

#include "report/report.h"

#include "base/panic.h"

#include "std/str.h"
#include "std/vector.h"
#include "std/map.h"

#include "io/io.h"
#include "fs/fs.h"

#include "argparse/argparse.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK_REPORTS(reports, msg) \
    do { \
        int err = end_reports(reports, msg, kReportConfig); \
        if (err != 0) { \
            return err; \
        } \
    } while (0)

static const report_config_t kReportConfig = {
    .limit = SIZE_MAX,
    .warningsAreErrors = false
};

static const version_info_t kVersion = {
    .license = "GPLv3",
    .desc = "Test harness",
    .author = "Elliot Haisley",
    .version = NEW_VERSION(0, 0, 1)
};

static io_t *make_file(const char *path, os_access_t flags)
{
    io_t *io = io_file(path, flags);
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

static user_ptr_t *get_memory(user_arena_t *arena, size_t size, const char *name)
{
    CTASSERTF(arena->memory_cursor + size + sizeof(user_ptr_t) < arena->memory_end, "out of memory %s", name);

    user_ptr_t *ptr = (user_ptr_t*)arena->memory_cursor;
    ptr->size = (uint32_t)(size);
    arena->memory_cursor += size + sizeof(user_ptr_t);

    arena->alloc_count++;

    return ptr;
}

static user_ptr_t *get_ptr(void *ptr)
{
    char *data = (char*)ptr;
    return (user_ptr_t*)(data - sizeof(user_ptr_t));
}

static void *user_malloc(alloc_t *alloc, size_t size, const char *name)
{
    user_arena_t *user = (user_arena_t*)alloc->user;

    user_ptr_t *ptr = get_memory(user, size, name);
    return ptr->data;
}

static void *user_realloc(alloc_t *alloc, void *ptr, size_t new_size, size_t old_size)
{
    CTU_UNUSED(old_size);

    user_arena_t *user = (user_arena_t*)alloc->user;

    user_ptr_t *old = get_ptr(ptr);

    if (old->size >= new_size)
        return old->data;

    user_ptr_t *new = get_memory(user, new_size, "realloc");
    memcpy(new->data, old->data, old->size);

    user->realloc_count++;

    return new->data;
}

static void user_free(alloc_t *alloc, void *ptr, size_t size)
{
    CTU_UNUSED(size);
    CTU_UNUSED(ptr);

    user_arena_t *user = (user_arena_t*)alloc->user;

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

static alloc_t new_alloc(user_arena_t *arena)
{
    alloc_t alloc = {
        .name = "user",
        .arena_malloc = user_malloc,
        .arena_realloc = user_realloc,
        .arena_free = user_free,
        .user = arena,
    };

    return alloc;
}

int run_test_harness(int argc, const char **argv)
{
    mediator_t *mediator = mediator_new("example", kVersion);
    lifetime_t *lifetime = lifetime_new(mediator);
    ap_t *ap = ap_new("example", NEW_VERSION(1, 0, 0));

    langs_t langs = get_langs();

    reports_t *reports = lifetime_get_reports(lifetime);

    for (size_t i = 0; i < langs.size; i++)
    {
        const language_t *lang = langs.langs + i;
        lifetime_config_language(lifetime, ap, lang);
    }

    for (size_t i = 0; i < langs.size; i++)
    {
        const language_t *lang = langs.langs + i;
        lifetime_add_language(lifetime, lang);
    }

    CHECK_REPORTS(reports, "adding languages");

    // harness.exe <name> [files...]
    CTASSERT(argc > 2);

    for (int i = 2; i < argc; i++)
    {
        const char *path = argv[i];
        const char *ext = str_ext(path);
        const language_t *lang = lifetime_get_language(lifetime, ext);

        io_t *io = make_file(path, eAccessRead | eAccessText);

        lifetime_parse(lifetime, lang, io);

        CHECK_REPORTS(reports, "parsing source");
    }

    for (size_t stage = 0; stage < eStageTotal; stage++)
    {
        lifetime_run_stage(lifetime, stage);

        char *msg = format("running stage %s", stage_to_string(stage));
        CHECK_REPORTS(reports, msg);
    }

    lifetime_resolve(lifetime);
    CHECK_REPORTS(reports, "resolving symbols");

    map_t *modmap = lifetime_get_modules(lifetime);

    check_tree(reports, modmap);
    CHECK_REPORTS(reports, "validations failed");

    ssa_result_t ssa = ssa_compile(modmap);
    CHECK_REPORTS(reports, "generating ssa");

    ssa_opt(reports, ssa);
    CHECK_REPORTS(reports, "optimizing ssa");

    fs_t *fs = fs_virtual(reports, "out");

    emit_options_t baseOpts = {
        .reports = reports,
        .fs = fs,

        .modules = ssa.modules,
        .deps = ssa.deps,
    };

    ssa_emit_options_t emitOpts = {
        .opts = baseOpts
    };

    ssa_emit_result_t ssaResult = emit_ssa(&emitOpts);
    CHECK_REPORTS(reports, "emitting ssa");
    CTU_UNUSED(ssaResult); // TODO: check for errors

    c89_emit_options_t c89Opts = {
        .opts = baseOpts
    };

    c89_emit_result_t c89Result = emit_c89(&c89Opts);
    CHECK_REPORTS(reports, "emitting c89");

    OS_RESULT(const char *) cwd = os_dir_current();
    CTASSERTF(os_error(cwd) == 0, "failed to get cwd %s", os_error_string(os_error(cwd)));

    const char *testDir = format("%s" NATIVE_PATH_SEPARATOR "test-out", OS_VALUE(const char*, cwd));
    const char *runDir = format("%s" NATIVE_PATH_SEPARATOR "%s", testDir, argv[1]);

    logverbose("creating output directory %s", runDir);
    fs_t *out = fs_physical(reports, runDir);
    CHECK_REPORTS(reports, "creating output directory");

    fs_sync(out, fs);
    CHECK_REPORTS(reports, "syncing output directory");

    size_t len = vector_len(c89Result.sources);
    vector_t *sources = vector_of(len);
    for (size_t i = 0; i < len; i++)
    {
        const char *part = vector_get(c89Result.sources, i);
        char *path = format("%s" NATIVE_PATH_SEPARATOR "%s", runDir, part);
        vector_set(sources, i, path);
    }

    logverbose("compiling: %s", str_join(" ", sources));
    logverbose("include: %s", runDir);

#if OS_WINDOWS
    const char *libDir = format("%s" NATIVE_PATH_SEPARATOR "lib", runDir);

    OS_RESULT(bool) create = os_dir_create(libDir);
    CTASSERTF(os_error(create) == 0, "failed to create dir `%s` %s", libDir, os_error_string(os_error(create)));

    int status = system(format("cl /nologo /c %s /I%s\\include /Fo%s\\", str_join(" ", sources), runDir, libDir));
    if (status != 0)
    {
        report(reports, eFatal, NULL, "compilation failed `%d`", status);
    }
#else
    int status = system(format("cd %s && cc %s -c -Iinclude", runDir, str_join(" ", sources)));
    if (WEXITSTATUS(status) != EXIT_OK)
    {
        report(reports, eFatal, NULL, "compilation failed %d", WEXITSTATUS(status));
    }
#endif

    CHECK_REPORTS(reports, "compiling");

    return 0;
}

int main(int argc, const char **argv)
{
    size_t size = 1024 * 1024 * 64;
    user_arena_t arena = new_user_arena(size);
    alloc_t alloc = new_alloc(&arena);
    gDefaultAlloc = alloc;

    verbose = true;

    int result = run_test_harness(argc, argv);

    logverbose("done");

    free(arena.memory_start);

    return result;
}
