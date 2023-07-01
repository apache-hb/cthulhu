#include "common.h"

#include "cthulhu/hlir/query.h"

#include "report/report.h"

#include "std/str.h"
#include "std/map.h"
#include "std/vector.h"

#include "base/memory.h"
#include "base/panic.h"

typedef struct ssa_t {
    map_t *globals;

    map_t *modules;
} ssa_t;

static ssa_module_t *ssa_compile_module(ssa_t *ssa, const char *path, h2_t *tree);

static ssa_module_t *ssa_module_new(const char *name)
{
    ssa_module_t *mod = ctu_malloc(sizeof(ssa_module_t));
    mod->name = name;
    mod->globals = map_optimal(32);
    mod->functions = map_optimal(32);
    mod->modules = map_optimal(4);
    return mod;
}

static void add_module(ssa_module_t *root, ssa_module_t *other)
{
    CTASSERT(other->name != NULL);
    CTASSERTF(!str_contains(other->name, "."), "invalid module name: %s", other->name);

    map_set(root->modules, other->name, other);
}

static void add_global(ssa_module_t *mod, ssa_symbol_t *sym)
{
    map_set(mod->globals, sym->name, sym);
}

static void add_function(ssa_module_t *mod, ssa_symbol_t *func)
{
    map_set(mod->functions, func->name, func);
}

static ssa_symbol_t *ssa_symbol_new(const char *name, ssa_type_t *type)
{
    CTASSERT(name != NULL);
    CTASSERTF(!str_contains(name, "."), "invalid symbol name: %s", name);

    ssa_symbol_t *sym = ctu_malloc(sizeof(ssa_symbol_t));
    sym->name = name;
    sym->type = type;
    sym->value = NULL;
    sym->entry = NULL;
    return sym;
}

static ssa_symbol_t *ssa_global_new(const h2_t *global)
{
    const char *name = h2_get_name(global);
    ssa_type_t *type = ssa_type_from(h2_get_type(global));

    return ssa_symbol_new(name, type);
}

static ssa_symbol_t *ssa_function_new(const h2_t *function)
{
    const char *name = h2_get_name(function);
    ssa_type_t *type = ssa_type_from(h2_get_type(function));

    return ssa_symbol_new(name, type);
}

static void ssa_add_globals(ssa_t *ssa, ssa_module_t *mod, h2_t *tree)
{
    map_t *globals = h2_module_tag(tree, eSema2Values);
    map_iter_t iter = map_iter(globals);

    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        ssa_symbol_t *symbol = ssa_global_new(entry.value);

        add_global(mod, symbol);
    }
}

static void ssa_add_functions(ssa_t *ssa, ssa_module_t *mod, h2_t *tree)
{
    map_t *functions = h2_module_tag(tree, eSema2Procs);
    map_iter_t iter = map_iter(functions);

    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        ssa_symbol_t *fn = ssa_function_new(entry.value);

        add_function(mod, fn);
    }
}


static void ssa_add_modules(ssa_t *ssa, ssa_module_t *mod, h2_t *tree)
{
    map_t *modules = h2_module_tag(tree, eSema2Modules);
    map_iter_t iter = map_iter(modules);

    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        ssa_module_t *sub = ssa_compile_module(ssa, entry.key, entry.value);
        add_module(mod, sub);
    }
}

static ssa_module_t *ssa_compile_module(ssa_t *ssa, const char *path, h2_t *tree)
{
    ssa_module_t *mod = ssa_module_new(path);
    ssa_add_globals(ssa, mod, tree);
    ssa_add_functions(ssa, mod, tree);
    ssa_add_modules(ssa, mod, tree);
    return mod;
}

static void ssa_compile_path(ssa_t *ssa, ssa_module_t *root, const char *path, h2_t *tree)
{
    vector_t *parts = str_split(path, ".");
    size_t len = vector_len(parts);
    ssa_module_t *mod = root;
    for (size_t i = 0; i < len - 1; i++)
    {
        const char *part = vector_get(parts, i);
        ssa_module_t *next = ssa_module_new(part);
        add_module(mod, next);
        mod = next;
    }

    const char *name = vector_tail(parts);
    add_module(mod, ssa_compile_module(ssa, name, tree));
}

ssa_module_t *ssa_compile(map_t *mods)
{
    ssa_t ssa = {
        .globals = map_new(64)
    };

    ssa_module_t *root = ssa_module_new(NULL);

    map_iter_t iter = map_iter(mods);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        const char *path = entry.key;
        h2_t *mod = entry.value;

        ssa_compile_path(&ssa, root, path, mod);
    }

    return root;
}
