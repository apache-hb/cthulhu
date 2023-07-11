#include "common/common.h"

#include "cthulhu/hlir/query.h"

#include "std/map.h"
#include "std/set.h"
#include "std/vector.h"

#include "base/memory.h"

typedef struct ssa_compile_t {
    map_t *globals; ///< map<h2, ssa_symbol_t>
    map_t *locals; ///< map<h2, ssa_symbol_t>

    ssa_block_t *currentBlock;
    ssa_symbol_t *currentSymbol;

    map_t *deps;
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

static ssa_symbol_t *global_create(ssa_compile_t *ssa, const h2_t *tree)
{

}

ssa_result_t ssa_compile(map_t *mods)
{
    ssa_compile_t ssa = {
        .globals = map_optimal(32),
        .locals = map_optimal(32),
        .deps = map_optimal(64)
    };

    ssa_result_t result = {
        .deps = ssa.deps
    };

    return result;
}
