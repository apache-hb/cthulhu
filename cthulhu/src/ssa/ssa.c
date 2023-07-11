#include "common/common.h"

#include "cthulhu/hlir/query.h"

#include "report/report.h"

#include "std/str.h"
#include "std/map.h"
#include "std/set.h"
#include "std/vector.h"

#include "std/typed/vector.h"
#include "std/typed/set.h"

#include "base/memory.h"
#include "base/panic.h"

#if 0
static size_t ssa_type_hash(const void *ptr)
{
    const ssa_type_t *type = ptr;
    return type->kind;
}

static bool ssa_type_equal(const void *lhs, const void *rhs)
{
    const ssa_type_t *lhsType = lhs;
    const ssa_type_t *rhsType = rhs;

    if (lhsType->kind != rhsType->kind)
    {
        return false;
    }

    return false;
}

static const typeset_info_t kTypeSetSSAInfo = {
    .typeSize = sizeof(ssa_type_t),
    .fnHash = ssa_type_hash,
    .fnEqual = ssa_type_equal
};
#endif

typedef struct ssa_t {
    map_t *globals;
    map_t *functions;

    map_t *locals;

    ssa_block_t *currentBlock;
    ssa_symbol_t *currentSymbol;

    typeset_t *types;

    // dependency graph
    map_t *deps; // map<ssa_symbol_t*, typeset<ssa_symbol_t*>>
} ssa_t;

#if 0
static void add_graph_dep(ssa_t *ssa, const ssa_symbol_t *symbol, const ssa_symbol_t *dep)
{
    set_t *set = map_get_ptr(ssa->deps, symbol);

    if (set == NULL)
    {
        set = set_new(4);
        map_set_ptr(ssa->deps, symbol, set);
    }

    set_add_ptr(set, dep);
}
#endif

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

static void add_local(ssa_t *ssa, const h2_t *h2, size_t local)
{
    map_set_ptr(ssa->locals, h2, (void*)local);
}

static size_t get_local(ssa_t *ssa, const h2_t *h2)
{
    size_t value = (size_t)map_get_default_ptr(ssa->locals, h2, (void*)SIZE_MAX);
    CTASSERTF(value != SIZE_MAX, "local not found: %s", h2->name);
    return value;
}

static ssa_symbol_t *get_global(ssa_t *ssa, const h2_t *h2)
{
    ssa_symbol_t *sym = map_get_ptr(ssa->globals, h2);
    CTASSERTF(sym != NULL, "global not found: %s", h2->name);
    return sym;
}

static ssa_symbol_t *get_function(ssa_t *ssa, const h2_t *h2)
{
    ssa_symbol_t *sym = map_get_ptr(ssa->functions, h2);
    CTASSERTF(sym != NULL, "function not found: %s", h2->name);
    return sym;
}

static ssa_block_t *ssa_block_new(ssa_symbol_t *symbol, const char *name, size_t len)
{
    ssa_block_t *self = ctu_malloc(sizeof(ssa_block_t));
    self->name = name;
    self->steps = typevec_new(sizeof(ssa_step_t), len);

    vector_push(&symbol->blocks, self);
    return self;
}

static ssa_symbol_t *ssa_symbol_new(const char *name, ssa_type_t *type, const h2_attrib_t *attribs)
{
    CTASSERT(name != NULL);
    CTASSERTF(!str_contains(name, "."), "invalid symbol name: %s", name);

    ssa_symbol_t *sym = ctu_malloc(sizeof(ssa_symbol_t));
    sym->linkage = attribs->link;
    sym->visibility = attribs->visibility;
    sym->linkName = attribs->mangle;

    sym->locals = NULL;

    sym->name = name;
    sym->type = type;
    sym->value = NULL;
    sym->entry = NULL;

    sym->blocks = vector_new(4);
    return sym;
}

static ssa_symbol_t *ssa_global_new(const h2_t *global)
{
    const char *name = h2_get_name(global);
    ssa_type_t *type = ssa_type_from(h2_get_type(global));

    ssa_symbol_t *sym = ssa_symbol_new(name, type, h2_get_attrib(global));
    sym->entry = ssa_block_new(sym, "entry", 32);
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
    switch (step.opcode)
    {
    case eOpReturn:
    case eOpBranch:
    case eOpJump:
    case eOpStore:
        break;
    default:
        CTASSERT(step.type != NULL);
    }

    CTASSERT(ssa->currentBlock != NULL);

    ssa_block_t *block = ssa->currentBlock;
    size_t index = typevec_len(block->steps);

    typevec_push(block->steps, &step);

    ssa_operand_t operand = {
        .kind = eOperandReg,
        .vregContext = block,
        .vregIndex = index,
    };

    return operand;
}

static ssa_operand_t ssa_compile_step(ssa_t *ssa, const h2_t *tree);

static void add_stmt_block(ssa_t *ssa, vector_t *stmts)
{
    size_t len = vector_len(stmts);
    for (size_t i = 0; i < len; i++)
    {
        const h2_t *stmt = vector_get(stmts, i);
        ssa_compile_step(ssa, stmt);
    }
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
    case eHlir2ExprDigit:
    case eHlir2ExprBool:
    case eHlir2ExprString:
    case eHlir2ExprUnit: {
        ssa_operand_t operand = {
            .kind = eOperandImm,
            .value = ssa_value_from(tree),
        };
        return operand;
    }
    case eHlir2ExprBinary: {
        ssa_operand_t lhs = ssa_compile_step(ssa, tree->lhs);
        ssa_operand_t rhs = ssa_compile_step(ssa, tree->rhs);

        ssa_step_t step = {
            .opcode = eOpBinary,
            .type = ssa_type_from(tree->type),
            .binary = {
                .lhs = lhs,
                .rhs = rhs,
                .binary = tree->binary,
            }
        };
        return ssa_add_step(ssa, step);
    }
    case eHlir2ExprLoad: {
        ssa_operand_t operand = ssa_compile_step(ssa, tree->load);
        ssa_step_t step = {
            .opcode = eOpLoad,
            .type = ssa_type_from(tree->type),
            .load = {
                .src = operand,
            }
        };
        return ssa_add_step(ssa, step);
    }
    case eHlir2ExprCall: {
        size_t len = vector_len(tree->args);
        typevec_t *args = typevec_of(sizeof(ssa_operand_t), len);
        for (size_t i = 0; i < len; i++)
        {
            const h2_t *arg = vector_get(tree->args, i);
            ssa_operand_t operand = ssa_compile_step(ssa, arg);
            typevec_set(args, i, &operand);
        }

        ssa_operand_t callee = ssa_compile_step(ssa, tree->callee);

        ssa_step_t step = {
            .opcode = eOpCall,
            .type = ssa_type_from(tree->type),
            .call = {
                .function = callee,
                .args = args,
            }
        };
        return ssa_add_step(ssa, step);
    }
    case eHlir2DeclGlobal: {
        ssa_operand_t operand = {
            .kind = eOperandGlobal,
            .global = get_global(ssa, tree),
        };
        return operand;
    }
    case eHlir2DeclFunction: {
        ssa_operand_t operand = {
            .kind = eOperandFunction,
            .function = get_function(ssa, tree),
        };
        return operand;
    }

    case eHlir2DeclLocal: {
        size_t idx = get_local(ssa, tree);

        ssa_operand_t operand = {
            .kind = eOperandLocal,
            .local = idx,
        };
        return operand;
    }

    case eHlir2StmtBlock: {
        ssa_block_t *block = ssa_block_new(ssa->currentSymbol, NULL, 32);
        ssa_block_t *next = ssa_block_new(ssa->currentSymbol, NULL, 32);

        ssa_operand_t operand = {
            .kind = eOperandBlock,
            .bb = block,
        };

        ssa_step_t into = {
            .opcode = eOpJump,
            .jump = {
                .target = operand
            }
        };

        ssa_step_t step = {
            .opcode = eOpJump,
            .jump = {
                .target = {
                    .kind = eOperandBlock,
                    .bb = next,
                },
            }
        };

        ssa_add_step(ssa, into);

        ssa->currentBlock = block;

        add_stmt_block(ssa, tree->stmts);

        ssa_add_step(ssa, step);

        ssa->currentBlock = next;

        return operand;
    }

    case eHlir2StmtAssign: {
        ssa_operand_t dst = ssa_compile_step(ssa, tree->dst);
        ssa_operand_t src = ssa_compile_step(ssa, tree->src);

        ssa_step_t step = {
            .opcode = eOpStore,
            .store = {
                .dst = dst,
                .src = src,
            }
        };
        return ssa_add_step(ssa, step);
    }

    case eHlir2StmtReturn: {
        ssa_operand_t operand = ssa_compile_step(ssa, tree->value);

        ssa_step_t step = {
            .opcode = eOpReturn,
            .ret = {
                .value = operand,
            }
        };
        return ssa_add_step(ssa, step);
    }

    case eHlir2Resolve:
        NEVER("resolve %s should be resolved by now", tree->name);

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

static typevec_t *ssa_gen_locals(ssa_t *ssa, const h2_t *tree)
{
    CTASSERT(h2_is(tree, eHlir2DeclFunction));

    map_reset(ssa->locals);

    size_t len = vector_len(tree->locals);
    typevec_t *dst = typevec_of(sizeof(ssa_local_t), len);

    for (size_t i = 0; i < len; i++)
    {
        const h2_t *local = vector_get(tree->locals, i);
        ssa_local_t sym = {
            .name = h2_get_name(local),
            .type = ssa_type_from(h2_get_type(local))
        };

        typevec_set(dst, i, &sym);
        add_local(ssa, local, i);
    }

    return dst;
}

static void begin_pass(ssa_t *ssa, ssa_symbol_t *sym, ssa_block_t *block)
{
    ssa->currentSymbol = sym;
    ssa->currentBlock = block;
}

ssa_module_t *ssa_compile(map_t *mods)
{
    ssa_t ssa = {
        .globals = map_new(64),
        .functions = map_new(64),

        .locals = map_new(64),

        .deps = map_new(128)
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

        begin_pass(&ssa, symbol, symbol->entry);

        ssa_operand_t op = ssa_compile_step(&ssa, tree->global);
        ssa_step_t step = {
            .opcode = eOpReturn,
            .type = symbol->type,
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

        if (tree->body == NULL) { continue; }

        symbol->locals = ssa_gen_locals(&ssa, tree);
        symbol->entry = ssa_block_new(symbol, "entry", 32);
        begin_pass(&ssa, symbol, symbol->entry);

        CTASSERT(h2_is(tree->body, eHlir2StmtBlock));
        add_stmt_block(&ssa, tree->body->stmts);
    }

    return root;
}
