#include "common.h"

#include "report/report.h"

#include "io/fs.h"

#include "std/str.h"
#include "std/map.h"

#include "base/panic.h"

#include <string.h>

typedef struct c89_t {
    reports_t *reports;
    fs_t *fs;

    map_t *includes; // map<ssa_module_t*, const char*>
    map_t *sources;
} c89_t;

static void create_module_file(c89_t *emit, const char *root, ssa_module_t *mod)
{
    char *sourceFile = format("src/%s.c", root);
    char *includeFile = format("include/%s.h", root);

    fs_file_create(emit->fs, sourceFile);
    fs_file_create(emit->fs, includeFile);

    io_t *src = fs_open(emit->fs, sourceFile, eAccessWrite | eAccessText);
    io_t *inc = fs_open(emit->fs, includeFile, eAccessWrite | eAccessText);

    write_string(inc, "#pragma once\n");
    write_string(src, "#include \"%s.h\"\n", root);

    map_set_ptr(emit->includes, mod, includeFile);
    map_set_ptr(emit->sources, mod, sourceFile);

    io_close(inc);
    io_close(src);
}

static void create_module_dir(c89_t *emit, const char *root, ssa_module_t *mod)
{
    const char *name = mod->name;
    CTASSERT(name != NULL);

    const char *path = (root == NULL) ? name : format("%s/%s", root, name);

    if (!map_empty(mod->globals) || !map_empty(mod->functions))
    {
        create_module_file(emit, path, mod);
    }

    if (map_empty(mod->modules))
    {
        return;
    }

    char *includeDir = format("include/%s", path);
    char *sourceDir = format("src/%s", path);

    fs_dir_create(emit->fs, includeDir);
    fs_dir_create(emit->fs, sourceDir);

    map_iter_t iter = map_iter(mod->modules);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        create_module_dir(emit, path, entry.value);
    }
}

static void create_root_dir(c89_t *emit, ssa_module_t *mod)
{
    map_iter_t iter = map_iter(mod->modules);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        ssa_module_t *child = entry.value;
        create_module_dir(emit, NULL, child);
    }
}

c89_emit_result_t emit_c89(const c89_emit_options_t *options)
{
    emit_options_t opts = options->opts;
    map_t *sourceMap = map_optimal(64);
    c89_t c89 = {
        .reports = opts.reports,
        .fs = opts.fs,
        .includes = map_optimal(64),
        .sources = sourceMap
    };

    fs_dir_create(c89.fs, "include");
    fs_dir_create(c89.fs, "src");

    create_root_dir(&c89, opts.mod);

    c89_emit_result_t result = {
        .sources = map_values(sourceMap)
    };

    return result;
}
