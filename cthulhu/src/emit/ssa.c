#include "common.h"

#include "std/map.h"
#include "std/str.h"
#include "std/vector.h"
#include "std/set.h"

#include "std/typed/vector.h"

#include "base/panic.h"

#include "report/report.h"

#include "io/fs.h"

#include <string.h>

static bool check_root_mod(vector_t *path, const char *id)
{
    const char *tail = vector_tail(path);
    return str_equal(tail, id);
}

static const char *type_to_string(const ssa_type_t *type);

static char *digit_to_string(ssa_type_digit_t digit)
{
    return format("digit(%s.%s)", sign_name(digit.sign), digit_name(digit.digit));
}

static char *qualify_to_string(ssa_type_qualify_t qualify)
{
    return format("qualify(quals: %s, type: %s)", quals_name(qualify.quals), type_to_string(qualify.type));
}

static char *closure_to_string(ssa_type_closure_t closure)
{
    size_t len = typevec_len(closure.params);
    vector_t *vec = vector_of(len);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_param_t *param = typevec_offset(closure.params, i);
        const char *ty = type_to_string(param->type);
        vector_set(vec, i, format("%s: %s", param->name, ty));
    }

    const char *result = type_to_string(closure.result);

    char *params = str_join(", ", vec);

    return format("closure(result: %s, params: %s, variadic: %s)", result, params, closure.variadic ? "true" : "false");
}

static const char *type_to_string(const ssa_type_t *type)
{
    switch (type->kind)
    {
    case eTypeEmpty: return "empty";
    case eTypeUnit: return "unit";
    case eTypeBool: return "bool";
    case eTypeString: return "string";
    case eTypeDigit: return digit_to_string(type->digit);
    case eTypeQualify: return qualify_to_string(type->qualify);
    case eTypeClosure: return closure_to_string(type->closure);
    default: NEVER("unknown type kind %d", type->kind);
    }
}

static void emit_ssa_module(fs_t *fs, const ssa_module_t *mod)
{
    // this is really badly named :p
    // whats really going on is that we're checking if the module has the same name.
    // as the last element in the path, in these cases we dont want to emit the last element of the path.
    // this is not required for correctness but makes the output nicer to grok.
    bool isRootMod = check_root_mod(mod->path, mod->name);

    vector_t *vec = vector_clone(mod->path); // lets not scuff the original path
    if (isRootMod) { vector_drop(vec); }

    // TODO: this may start failing if the api for fs_dir_create changes
    char *path = str_join("/", vec);
    char *file = format("%s/%s.ssa", path, mod->name);
    fs_dir_create(fs, path);
    fs_file_create(fs, file);

    io_t *io = fs_open(fs, file, eAccessWrite | eAccessText);
    write_string(io, "module {name=%s", mod->name);
    if (vector_len(vec) > 0) { write_string(io, ", path=%s", path); }
    write_string(io, "}\n");

    write_string(io, "\n");

    size_t len = vector_len(mod->globals);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_symbol_t *global = vector_get(mod->globals, i);
        write_string(io, "%s global %s: %s\n", vis_name(global->visibility), global->name, type_to_string(global->type));
        write_string(io, "\t[extern = %s, linkage = %s]\n",
            global->linkName == NULL ? "null" : global->linkName,
            link_name(global->linkage));

        if (i != len - 1) { write_string(io, "\n"); }
    }
}

ssa_emit_result_t emit_ssa(const ssa_emit_options_t *options)
{
    const emit_options_t opts = options->opts;
    size_t len = vector_len(opts.modules);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_module_t *mod = vector_get(opts.modules, i);
        emit_ssa_module(opts.fs, mod);
    }

    ssa_emit_result_t result = { NULL };
    return result;
}
