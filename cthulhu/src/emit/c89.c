#include "cthulhu/emit/emit.h"
#include "cthulhu/ssa/ssa.h"

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

    const char pragma[] = "#pragma once\n";
    io_write(inc, pragma, sizeof(pragma) - 1);

    char *include = format("#include \"%s.h\"\n", root);
    io_write(src, include, strlen(include));

    map_set_ptr(emit->includes, mod, includeFile);
    map_set_ptr(emit->sources, mod, sourceFile);

    io_close(inc);
    io_close(src);
}

static void create_module_dir(c89_t *emit, const char *root, ssa_module_t *mod)
{
    const char *name = mod->name;
    CTASSERT(name != NULL);

    if (!map_empty(mod->globals) || !map_empty(mod->functions))
    {
        create_module_file(emit, root, mod);
    }

    if (map_empty(mod->modules))
    {
        return;
    }
    
    char *path = format("%s/%s", root, name);
    logverbose("mod: (root=%s, path=%s, name=%s)", root, path, name);
    
    char *includeDir = format("include/%s", root);
    char *sourceDir = format("src/%s", root);

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
    logverbose("mod: root");
    map_iter_t iter = map_iter(mod->modules);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        ssa_module_t *child = entry.value;
        const char *path = entry.key;
        create_module_dir(emit, path, child);
    }
}

void emit_c89(const emit_options_t *options)
{
    c89_t c89 = { 
        .reports = options->reports, 
        .fs = options->fs,
        .includes = map_optimal(64),
        .sources = map_optimal(64)
    };

    fs_dir_create(c89.fs, "include");
    fs_dir_create(c89.fs, "src");

    create_root_dir(&c89, options->mod);
}
