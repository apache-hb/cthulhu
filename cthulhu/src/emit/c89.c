#include "common.h"

#include "report/report.h"

#include "io/fs.h"

#include "std/str.h"
#include "std/map.h"
#include "std/vector.h"

#include "base/panic.h"
#include "base/memory.h"

#include <string.h>

typedef struct c89_t {
    reports_t *reports;
    fs_t *fs;
    c89_flags_t flags;

    map_t *modules; // map<ssa_module_t*, c89_file_t*>

    vector_t *path;
} c89_t;

typedef struct c89_file_t {
    const char *path;
    io_t *io;
} c89_file_t;

typedef struct c89_module_t {
    c89_file_t source;
    c89_file_t include;
} c89_module_t;

static c89_file_t create_new_file(const char *path, io_t *io)
{
    c89_file_t result = {
        .path = path,
        .io = io,
    };

    return result;
}

static c89_module_t *create_new_module(c89_file_t src, c89_file_t inc)
{
    c89_module_t *result = ctu_malloc(sizeof(c89_module_t));
    result->source = src;
    result->include = inc;
    return result;
}

static const char *join_path(c89_t *emit, vector_t *path, const char *name)
{
    const char *join = (emit->flags & eEmitFlat) ? "." : "/";

    return (vector_len(path) > 0)
        ? format("%s%s%s", str_join(join, path), join, name)
        : name;
}

static c89_module_t *create_module_file(c89_t *emit, const char *root, vector_t *path, ssa_module_t *mod)
{
    char *sourceFile = format("src/%s.c", root);
    char *includeFile = format("include/%s.h", root);

    fs_file_create(emit->fs, sourceFile);
    fs_file_create(emit->fs, includeFile);

    io_t *src = fs_open(emit->fs, sourceFile, eAccessWrite | eAccessText);
    io_t *inc = fs_open(emit->fs, includeFile, eAccessWrite | eAccessText);

    write_string(inc, "#pragma once\n");
    write_string(src, "#include \"%s.h\"\n", root);

    const char *id = (vector_len(path) > 0)
        ? format("%s.%s", str_join(".", path), mod->name)
        : mod->name;

    write_string(inc, "\n/**\n * module %s\n */\n", id);

    c89_file_t incFd = create_new_file(includeFile, inc);
    c89_file_t srcFd = create_new_file(sourceFile, src);

    c89_module_t *self = create_new_module(srcFd, incFd);

    map_set_ptr(emit->modules, mod, self);

    return self;
}

static void create_module_dir(c89_t *emit, ssa_module_t *mod)
{
    const char *name = mod->name;
    CTASSERT(name != NULL);

    const char *path = join_path(emit, emit->path, name);

    create_module_file(emit, path, emit->path, mod);

    if (map_empty(mod->modules))
    {
        return;
    }

    char *includeDir = format("include/%s", path);
    fs_dir_create(emit->fs, includeDir);

    char *sourceDir = format("src/%s", path);
    fs_dir_create(emit->fs, sourceDir);

    vector_push(&emit->path, (char*)name);

    map_iter_t iter = map_iter(mod->modules);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        create_module_dir(emit, entry.value);
    }

    vector_drop(emit->path);
}

static void create_root_dir(c89_t *emit, ssa_module_t *mod)
{
    map_iter_t iter = map_iter(mod->modules);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        ssa_module_t *child = entry.value;
        create_module_dir(emit, child);
    }
}

static void close_all(map_t *modMap)
{
    map_iter_t iter = map_iter(modMap);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        c89_module_t *mod = entry.value;
        io_close(mod->source.io);
        io_close(mod->include.io);
    }
}

c89_emit_result_t emit_c89(const c89_emit_options_t *options)
{
    emit_options_t opts = options->opts;
    c89_t c89 = {
        .reports = opts.reports,
        .fs = opts.fs,
        .flags = options->flags,
        .modules = map_optimal(64),

        .path = vector_new(4)
    };

    if (c89.flags & eEmitFlat)
    {
        CTASSERTF(c89.flags & eEmitHeaders, "eEmitFlat requires eEmitHeaders");
    }

    fs_dir_create(c89.fs, "include");
    fs_dir_create(c89.fs, "src");

    create_root_dir(&c89, opts.mod);

    vector_t *sources = map_values(c89.modules);
    size_t len = vector_len(sources);
    vector_t *paths = vector_of(len);
    for (size_t i = 0; i < len; i++)
    {
        c89_module_t *mod = vector_get(sources, i);
        c89_file_t src = mod->source;
        vector_set(paths, i, (char*)src.path);
    }

    close_all(c89.modules);

    c89_emit_result_t result = {
        .sources = paths
    };

    return result;
}
