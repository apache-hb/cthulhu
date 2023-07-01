#include "common.h"

#include "std/map.h"
#include "std/str.h"
#include "std/vector.h"
#include "std/typevec.h"

#include "base/panic.h"

#include "io/fs.h"

#include <string.h>

typedef struct ssa_t {
    reports_t *reports;
    fs_t *fs;

    map_t *modules;
} ssa_t;

static const char *ssa_type_to_string(const ssa_type_t *type);

static const char *ssa_digit_to_string(ssa_type_digit_t digit)
{
    return format("digit(digit=%s, sign=%s)", digit_name(digit.digit), sign_name(digit.sign));
}

static const char *ssa_closure_to_string(ssa_type_closure_t closure)
{
    const char *result = ssa_type_to_string(closure.result);
    size_t len = typevec_len(closure.params);
    vector_t *params = vector_of(len);

    ssa_param_t param = { NULL, NULL };
    for (size_t i = 0; i < len; i++)
    {
        typevec_get(closure.params, i, &param);
        const char *type = ssa_type_to_string(param.type);
        vector_set(params, i, format("%s: %s", param.name, type));
    }

    char *args = str_join(", ", params);
    return format("closure(result: %s, args: [%s])", result, args);
}

static const char *ssa_qualify_to_string(ssa_type_qualify_t qualify)
{
    const char *type = ssa_type_to_string(qualify.type);
    const char *quals = quals_name(qualify.quals);
    return format("qualify(type: %s, quals: %s)", type, quals);
}

static const char *ssa_type_to_string(const ssa_type_t *type)
{
    switch (type->kind)
    {
    case eTypeEmpty: return "empty";
    case eTypeUnit: return "unit";
    case eTypeBool: return "bool";
    case eTypeDigit: return ssa_digit_to_string(type->digit);
    case eTypeString: return "string";
    case eTypeClosure: return ssa_closure_to_string(type->closure);
    case eTypeQualify: return ssa_qualify_to_string(type->qualify);
    default: NEVER("Invalid type kind: %d", type->kind);
    }
}

static void create_module_file(ssa_t *emit, const char *root, ssa_module_t *mod)
{
    char *sourceFile = format("ssa/%s.ssa", root);

    fs_file_create(emit->fs, sourceFile);

    io_t *src = fs_open(emit->fs, sourceFile, eAccessWrite | eAccessText);

    char *name = str_replace(root, "/", ".");
    write_string(src, "module = %s\n", name);

    map_iter_t globals = map_iter(mod->globals);
    while (map_has_next(&globals))
    {
        map_entry_t entry = map_next(&globals);
        ssa_symbol_t *global = entry.value;
        write_string(src, "global %s: %s = noinit\n", global->name, ssa_type_to_string(global->type));
    }

    map_set_ptr(emit->modules, mod, sourceFile);

    io_close(src);
}

static void create_module_dir(ssa_t *emit, const char *root, ssa_module_t *mod)
{
    const char *name = mod->name;
    CTASSERT(name != NULL);

    const char *path = (root == NULL) ? name : format("%s/%s", root, name);

    create_module_file(emit, path, mod);

    if (map_empty(mod->modules))
    {
        return;
    }

    char *sourceDir = format("ssa/%s", path);

    fs_dir_create(emit->fs, sourceDir);

    map_iter_t iter = map_iter(mod->modules);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        create_module_dir(emit, path, entry.value);
    }
}

static void create_root_dir(ssa_t *emit, ssa_module_t *mod)
{
    map_iter_t iter = map_iter(mod->modules);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        ssa_module_t *child = entry.value;
        create_module_dir(emit, NULL, child);
    }
}

void emit_ssa(const emit_options_t *options)
{
    ssa_t ssa = {
        .reports = options->reports,
        .fs = options->fs,
        .modules = map_optimal(4),
    };

    fs_dir_create(ssa.fs, "ssa");

    create_root_dir(&ssa, options->mod);
}
