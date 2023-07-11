#include "common.h"

#include "std/str.h"
#include "std/map.h"

#include "std/typed/vector.h"

#include <stdarg.h>
#include <string.h>

#include "report/report.h"

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
