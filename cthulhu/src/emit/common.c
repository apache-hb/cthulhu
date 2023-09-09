#include "common.h"

#include "report/report.h"

#include "std/str.h"
#include "std/map.h"
#include "std/vector.h"
#include "std/typed/vector.h"

#include "io/fs.h"

#include "base/panic.h"

#include <stdarg.h>
#include <string.h>

static bool check_root_mod(vector_t *path, const char *id)
{
    const char *tail = vector_tail(path);
    return str_equal(tail, id);
}

char *begin_module(emit_t *emit, fs_t *fs, const ssa_module_t *mod)
{
    CTU_UNUSED(emit);

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

static char *digit_to_string(ssa_type_digit_t digit)
{
    return format("digit(%s.%s)", sign_name(digit.sign), digit_name(digit.digit));
}

static char *params_to_string(typevec_t *params)
{
    size_t len = typevec_len(params);
    vector_t *vec = vector_of(len);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_param_t *param = typevec_offset(params, i);
        const char *ty = type_to_string(param->type);
        vector_set(vec, i, format("%s: %s", param->name, ty));
    }

    return str_join(", ", vec);
}
static char *closure_to_string(ssa_type_closure_t closure)
{
    const char *result = type_to_string(closure.result);
    char *params = params_to_string(closure.params);

    return format("closure(result: %s, params: [%s], variadic: %s)", result, params, closure.variadic ? "true" : "false");
}

static char *pointer_to_string(ssa_type_pointer_t pointer)
{
    const char *pointee = type_to_string(pointer.pointer);
    switch (pointer.length)
    {
    case 0: return format("ptr(%s)", pointee);
    case SIZE_MAX: return format("unbounded-ptr(%s)", pointee);
    default: return format("ptr(%s of %zu)", pointee, pointer.length);
    }
}

static char *record_to_string(ssa_type_record_t record)
{
    size_t len = typevec_len(record.fields);
    vector_t *fields = vector_of(len);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_field_t *field = typevec_offset(record.fields, i);
        const char *ty = type_to_string(field->type);
        vector_set(fields, i, format("%s: %s", field->name, ty));
    }

    return format("record(fields: [%s])", str_join(", ", fields));
}

const char *type_to_string(const ssa_type_t *type)
{
    switch (type->kind)
    {
    case eTypeEmpty: return "empty";
    case eTypeUnit: return "unit";
    case eTypeBool: return "bool";
    case eTypeDigit: return digit_to_string(type->digit);
    case eTypeClosure: return closure_to_string(type->closure);
    case eTypePointer: return pointer_to_string(type->pointer);
    case eTypeRecord: return record_to_string(type->record);
    default: NEVER("unknown type kind %d", type->kind);
    }
}
