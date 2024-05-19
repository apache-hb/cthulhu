// SPDX-License-Identifier: LGPL-3.0-only

#include "common.h"

#include "std/str.h"
#include "std/map.h"
#include "std/vector.h"
#include "std/typed/vector.h"

#include "fs/fs.h"

#include "base/panic.h"

#include <stdarg.h>
#include <stdint.h>

char *begin_module(emit_t *emit, fs_t *fs, const ssa_module_t *mod)
{
    // if the last element of the path and the module name are the same then remove the last element
    // this isnt required to be semanticly correct but it makes the output look nicer

    char *path = str_replace(mod->name, ".", "/", emit->arena);

    // create a folder if we need one for this module
    if (str_contains(mod->name, "."))
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

names_t names_new(size_t size, arena_t *arena)
{
    names_t names = {
        .counter = 0,
        .names = map_optimal(size, kTypeInfoPtr, arena)
    };

    return names;
}

void counter_reset(emit_t *emit)
{
    names_reset(&emit->vreg_names);
    names_reset(&emit->block_names);
}

static char *name_increment(names_t *names, const void *obj, char *existing, arena_t *arena)
{
    char *name = map_get(names->names, obj);
    if (name != NULL) { return name; }

    if (existing != NULL)
    {
        map_set(names->names, obj, existing);
        return existing;
    }

    char *id = str_format(arena, "%zu", names->counter++);
    map_set(names->names, obj, id);
    return id;
}

char *get_step_name(emit_t *emit, const ssa_step_t *step)
{
    return name_increment(&emit->vreg_names, step, NULL, emit->arena);
}

char *get_block_name(emit_t *emit, const ssa_block_t *block)
{
    return name_increment(&emit->block_names, block, (char*)block->name, emit->arena);
}

char *get_anon_name(emit_t *emit, const ssa_symbol_t *symbol, const char *prefix)
{
    names_t *names = &emit->anon_names;
    char *name = map_get(names->names, symbol);
    if (name != NULL) { return name; }

    char *id = str_format(emit->arena, "%s%zu", prefix, names->counter++);
    map_set(names->names, symbol, id);
    return id;
}

char *get_step_from_block(emit_t *emit, const ssa_block_t *block, size_t index)
{
    ssa_step_t *step = typevec_offset(block->steps, index);
    return get_step_name(emit, step);
}

static char *digit_to_string(ssa_type_digit_t digit, arena_t *arena)
{
    return str_format(arena, "digit(%s.%s)", sign_name(digit.sign), digit_name(digit.digit));
}

static char *params_to_string(typevec_t *params, arena_t *arena)
{
    size_t len = typevec_len(params);
    vector_t *vec = vector_of(len, arena);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_param_t *param = typevec_offset(params, i);
        const char *ty = type_to_string(param->type, arena);
        vector_set(vec, i, str_format(arena, "%s: %s", param->name, ty));
    }

    return str_join(", ", vec, arena);
}

static char *closure_to_string(ssa_type_closure_t closure, arena_t *arena)
{
    const char *result = type_to_string(closure.result, arena);
    char *params = params_to_string(closure.params, arena);

    return str_format(arena, "closure(result: %s, params: [%s], variadic: %s)", result, params, closure.variadic ? "true" : "false");
}

static char *pointer_to_string(ssa_type_pointer_t pointer, arena_t *arena)
{
    const char *pointee = type_to_string(pointer.pointer, arena);
    switch (pointer.length)
    {
    case 0: return str_format(arena, "ptr(%s)", pointee);
    case SIZE_MAX: return str_format(arena, "unbounded-ptr(%s)", pointee);
    default: return str_format(arena, "ptr(%s of %zu)", pointee, pointer.length);
    }
}

static char *record_to_string(ssa_type_record_t record, arena_t *arena)
{
    size_t len = typevec_len(record.fields);
    vector_t *fields = vector_of(len, arena);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_field_t *field = typevec_offset(record.fields, i);
        vector_set(fields, i, (char*)field->name);
    }

    char *joined = str_join(", ", fields, arena);
    return str_format(arena, "record(fields: [%s])", joined);
}

static char *enum_to_string(ssa_type_enum_t sum, arena_t *arena)
{
    size_t len = typevec_len(sum.cases);
    vector_t *variants = vector_of(len, arena);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_case_t *field = typevec_offset(sum.cases, i);
        char *segment = str_format(arena, "%s: %s", field->name, mpz_get_str(NULL, 10, field->value));
        vector_set(variants, i, segment);
    }

    char *joined = str_join(", ", variants, arena);
    return str_format(arena, "enum(variants: [%s])", joined);
}

const char *type_to_string(const ssa_type_t *type, arena_t *arena)
{
    switch (type->kind)
    {
    case eTypeEmpty: return "empty";
    case eTypeUnit: return "unit";
    case eTypeBool: return "bool";
    case eTypeOpaque: return "opaque";
    case eTypeEnum: return enum_to_string(type->sum, arena);
    case eTypeDigit: return digit_to_string(type->digit, arena);
    case eTypeClosure: return closure_to_string(type->closure, arena);
    case eTypePointer: return pointer_to_string(type->pointer, arena);
    case eTypeStruct: return record_to_string(type->record, arena);
    default: CT_NEVER("unknown type kind %d", type->kind);
    }
}
