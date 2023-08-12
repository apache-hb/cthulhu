#include "common.h"

#include "std/str.h"
#include "std/map.h"
#include "std/vector.h"

#include "io/fs.h"

#include "std/typed/vector.h"

#include <stdarg.h>
#include <string.h>

#include "report/report.h"

static bool check_root_mod(vector_t *path, const char *id)
{
    const char *tail = vector_tail(path);
    return str_equal(tail, id);
}

char *begin_module(emit_t *emit, fs_t *fs, const ssa_module_t *mod)
{
    // if the last element of the path and the module name are the same then remove the last element
    // this isnt required to be semanticly correct but it makes the output look nicer

    bool isRoot = check_root_mod(mod->path, mod->name);
    vector_t *vec = vector_clone(mod->path);

    if (isRoot) { vector_drop(vec); }

    char *path = str_join("/", vec);
    if (vector_len(vec) > 0)
    {
        fs_dir_create(fs, path);
    }

    return path;
}

static void names_reset(names_t *names)
{
    map_reset(names->names);
    names->counter = 0;
}

names_t names_new(size_t size)
{
    names_t names = {
        .counter = 0,
        .names = map_optimal(size)
    };

    return names;
}

void counter_reset(emit_t *emit)
{
    names_reset(&emit->vregNames);
    names_reset(&emit->blockNames);
}

static char *name_increment(names_t *names, const void *obj, char *existing)
{
    char *name = map_get_ptr(names->names, obj);
    if (name != NULL) { return name; }

    if (existing != NULL)
    {
        map_set_ptr(names->names, obj, existing);
        return existing;
    }

    char *id = format("%zu", names->counter++);
    map_set_ptr(names->names, obj, id);
    return id;
}

char *get_step_name(emit_t *emit, const ssa_step_t *step)
{
    return name_increment(&emit->vregNames, step, NULL);
}

char *get_block_name(emit_t *emit, const ssa_block_t *block)
{
    return name_increment(&emit->blockNames, block, (char*)block->name);
}

char *get_step_from_block(emit_t *emit, const ssa_block_t *block, size_t index)
{
    ssa_step_t *step = typevec_offset(block->steps, index);
    return get_step_name(emit, step);
}

void write_string(io_t *io, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char *msg = formatv(fmt, args);
    va_end(args);

    io_write(io, msg, strlen(msg));
}
