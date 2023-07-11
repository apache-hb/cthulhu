#include "common/common.h"

#include "cthulhu/hlir/query.h"

#include "std/str.h"
#include "std/map.h"
#include "std/set.h"
#include "std/vector.h"

#include "base/memory.h"

typedef struct ssa_compile_t {
    /// result data

    vector_t *modules; ///< vector<ssa_module>

    map_t *deps;

    /// internal data

    map_t *globals; ///< map<h2, ssa_symbol>
    map_t *locals; ///< map<h2, ssa_symbol>

    ssa_block_t *currentBlock;
    ssa_symbol_t *currentSymbol;

    vector_t *path;
} ssa_compile_t;

static void add_dep(ssa_compile_t *ssa, const ssa_symbol_t *symbol, const ssa_symbol_t *dep)
{
    set_t *set = map_get_ptr(ssa->deps, symbol);
    if (set == NULL)
    {
        set = set_new(8);
        map_set_ptr(ssa->deps, symbol, set);
    }

    set_add_ptr(set, dep);
}

static ssa_symbol_t *symbol_create(ssa_compile_t *ssa, const h2_t *tree)
{
    const char *name = h2_get_name(tree);
    ssa_type_t *type = ssa_type_from(h2_get_type(tree));
    const h2_attrib_t *attrib = h2_get_attrib(tree);

    ssa_symbol_t *symbol = ctu_malloc(sizeof(ssa_symbol_t));
    symbol->linkage = attrib->link;
    symbol->visibility = attrib->visibility;
    symbol->linkName = attrib->mangle;

    symbol->locals = NULL;

    symbol->name = name;
    symbol->type = type;
    symbol->value = NULL;
    symbol->entry = NULL;

    symbol->blocks = vector_new(4);

    return symbol;
}

static ssa_module_t *module_create(ssa_compile_t *ssa, const char *name)
{
    vector_t *path = vector_clone(ssa->path);

    ssa_module_t *mod = ctu_malloc(sizeof(ssa_module_t));
    mod->name = name;
    mod->path = path;

    mod->globals = map_optimal(32);
    mod->functions = map_optimal(32);

    return mod;
}

static void compile_module(ssa_compile_t *ssa, const h2_t *tree)
{
    const char *id = h2_get_name(tree);
    ssa_module_t *mod = module_create(ssa, id);

    vector_push(&ssa->modules, mod);
    vector_push(&ssa->path, (char*)id);

    map_t *children = h2_module_tag(tree, eSema2Modules);
    map_iter_t iter = map_iter(children);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);

        compile_module(ssa, entry.value);
    }

    vector_drop(ssa->path);
}

ssa_result_t ssa_compile(map_t *mods)
{
    ssa_compile_t ssa = {
        .modules = vector_new(4),
        .deps = map_optimal(64),

        .globals = map_optimal(32),
        .locals = map_optimal(32)
    };

    map_iter_t iter = map_iter(mods);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);

        ssa.path = str_split(entry.key, ".");
        compile_module(&ssa, entry.value);
    }

    ssa_result_t result = {
        .modules = ssa.modules,
        .deps = ssa.deps
    };

    return result;
}
