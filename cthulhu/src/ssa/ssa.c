#include "common.h"

#include "cthulhu/hlir/query.h"

#include "report/report.h"

#include "std/str.h"
#include "std/map.h"
#include "std/vector.h"
#include "std/typevec.h"

#include "base/memory.h"
#include "base/panic.h"

typedef struct ssa_t {
    map_t *globals;
    map_t *functions;

    ssa_block_t *current;
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

static void add_global(ssa_t *ssa, ssa_module_t *mod, const h2_t *h2, ssa_symbol_t *sym)
{
    map_set_ptr(ssa->globals, h2, sym);
    map_set_ptr(mod->globals, h2, sym);
}

static void add_function(ssa_t *ssa, ssa_module_t *mod, const h2_t *h2, ssa_symbol_t *func)
{
    map_set_ptr(ssa->functions, h2, func);
    map_set_ptr(mod->functions, h2, func);
}

static ssa_symbol_t *get_global(ssa_t *ssa, const h2_t *h2)
{
    ssa_symbol_t *sym = map_get_ptr(ssa->globals, h2);
    CTASSERTF(sym != NULL, "global not found: %s", h2->name);
    return sym;
}

static ssa_block_t *ssa_block_new(const char *name, size_t len)
{
    ssa_block_t *self = ctu_malloc(sizeof(ssa_block_t));
    self->name = name;
    self->steps = typevec_new(sizeof(ssa_step_t), len);
    return self;
}

static ssa_symbol_t *ssa_symbol_new(const char *name, ssa_type_t *type, const h2_attrib_t *attribs)
{
    CTASSERT(name != NULL);
    CTASSERTF(!str_contains(name, "."), "invalid symbol name: %s", name);

    ssa_symbol_t *sym = ctu_malloc(sizeof(ssa_symbol_t));
    sym->linkage = attribs->link;
    sym->visible = attribs->visible;
    sym->mangle = attribs->mangle;

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

    ssa_symbol_t *sym = ssa_symbol_new(name, type, h2_get_attrib(global));
    sym->entry = ssa_block_new("entry", 32);
    return sym;
}

static ssa_symbol_t *ssa_function_new(const h2_t *function)
{
    const char *name = h2_get_name(function);
    ssa_type_t *type = ssa_type_from(h2_get_type(function));

    return ssa_symbol_new(name, type, h2_get_attrib(function));
}

static ssa_operand_t ssa_add_step(ssa_t *ssa, ssa_step_t step)
{
    ssa_block_t *block = ssa->current;
    size_t index = typevec_len(block->steps);

    typevec_push(block->steps, &step);

    ssa_operand_t operand = {
        .kind = eOperandReg,
        .vregContext = block,
        .vregIndex = index,
    };

    return operand;
}

static ssa_operand_t ssa_compile_step(ssa_t *ssa, const h2_t *tree)
{
    UNUSED(ssa);
    UNUSED(tree);

    switch (tree->kind)
    {
    case eHlir2ExprEmpty: {
        ssa_operand_t operand = {
            .kind = eOperandEmpty
        };
        return operand;
    }
    case eHlir2ExprDigit: {
        ssa_operand_t operand = {
            .kind = eOperandImm,
            .value = ssa_value_from(tree),
        };

        return operand;
    }

    case eHlir2DeclGlobal: {
        ssa_operand_t operand = {
            .kind = eOperandGlobal,
            .global = get_global(ssa, tree),
        };

        return operand;
    }
    default: NEVER("invalid expression %d", tree->kind);
    }
}

static void ssa_add_globals(ssa_t *ssa, ssa_module_t *mod, h2_t *tree)
{
    map_t *globals = h2_module_tag(tree, eSema2Values);
    map_iter_t iter = map_iter(globals);

    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        ssa_symbol_t *symbol = ssa_global_new(entry.value);

        add_global(ssa, mod, entry.value, symbol);
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

        add_function(ssa, mod, entry.value, fn);
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
    ssa_module_t *mod = root;

    size_t len = vector_len(parts);
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
        .globals = map_new(64),
        .functions = map_new(64),
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

    map_iter_t globals = map_iter(ssa.globals);
    while (map_has_next(&globals))
    {
        map_entry_t entry = map_next(&globals);
        const h2_t *tree = entry.key;
        ssa_symbol_t *symbol = entry.value;

        ssa.current = symbol->entry;
        ssa_operand_t op = ssa_compile_step(&ssa, tree->global);
        ssa_step_t step = {
            .opcode = eOpReturn,
            .ret = {
                .value = op
            }
        };

        ssa_add_step(&ssa, step);
    }

    map_iter_t functions = map_iter(ssa.functions);
    while (map_has_next(&functions))
    {
        map_entry_t entry = map_next(&functions);
        const h2_t *tree = entry.key;
        ssa_symbol_t *symbol = entry.value;

        logverbose("compiling function %s", symbol->name);

        if (tree->body == NULL) { continue; }

        logverbose("has body %s", symbol->name);

        symbol->entry = ssa_block_new(symbol->name, 32);
    }

    return root;
}
