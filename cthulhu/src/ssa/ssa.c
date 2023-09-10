#include "common/common.h"

#include "cthulhu/tree/query.h"

#include "report/report.h"

#include "std/str.h"
#include "std/map.h"
#include "std/set.h"
#include "std/vector.h"

#include "std/typed/vector.h"

#include "base/memory.h"
#include "base/panic.h"

#include <string.h>

typedef struct ssa_compile_t {
    /// result data

    vector_t *modules; ///< vector<ssa_module>

    map_t *deps;

    /// internal data

    map_t *globals; ///< map<tree, ssa_symbol>
    map_t *functions; ///< map<tree, ssa_symbol>

    map_t *locals; ///< map<tree, ssa_symbol>
    map_t *loops; ///< map<tree, ssa_loop>

    ssa_block_t *currentBlock;
    ssa_symbol_t *currentSymbol;
    ssa_module_t *currentModule;

    vector_t *path;
} ssa_compile_t;

typedef struct ssa_loop_t {
    ssa_block_t *enterLoop;
    ssa_block_t *exitLoop;
} ssa_loop_t;

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

static ssa_symbol_t *symbol_create(ssa_compile_t *ssa, const tree_t *tree, ssa_storage_t storage)
{
    CTU_UNUSED(ssa);

    const char *name = tree_get_name(tree);
    const ssa_type_t *type = ssa_type_from(tree_get_type(tree));
    const attribs_t *attrib = tree_get_attrib(tree);

    ssa_symbol_t *symbol = ctu_malloc(sizeof(ssa_symbol_t));
    symbol->linkage = attrib->link;
    symbol->visibility = attrib->visibility;
    symbol->linkName = attrib->mangle;
    symbol->storage = storage;

    symbol->locals = NULL;
    symbol->params = NULL;
    symbol->consts = vector_new(4);

    symbol->name = name;
    symbol->type = type;
    symbol->value = NULL;
    symbol->entry = NULL;

    symbol->blocks = vector_new(4);

    return symbol;
}

static ssa_storage_t create_storage_type(const tree_t *decl)
{
    ssa_storage_t storage = {
        .type = ssa_type_from(tree_get_storage_type(decl)),
        .size = tree_get_storage_size(decl),
        .quals = tree_get_storage_quals(decl)
    };

    return storage;
}

static ssa_symbol_t *function_create(ssa_compile_t *ssa, const tree_t *tree)
{
    CTASSERTF(tree_is(tree, eTreeDeclFunction), "expected function, got %s", tree_to_string(tree));
    ssa_storage_t storage = { .type = NULL, .size = 0, .quals = eQualUnknown };
    ssa_symbol_t *self = symbol_create(ssa, tree, storage);

    size_t locals = vector_len(tree->locals);
    self->locals = typevec_of(sizeof(ssa_local_t), locals);
    for (size_t i = 0; i < locals; i++)
    {
        const tree_t *local = vector_get(tree->locals, i);
        ssa_storage_t storage = create_storage_type(local);
        const ssa_type_t *type = ssa_type_from(tree_get_type(local));
        const char *name = tree_get_name(local);

        ssa_local_t it = {
            .storage = storage,
            .name = name,
            .type = type,
        };

        typevec_set(self->locals, i, &it);
        map_set_ptr(ssa->locals, local, (void*)(uintptr_t)i);
    }

    size_t params = vector_len(tree->params);
    self->params = typevec_of(sizeof(ssa_param_t), params);
    for (size_t i = 0; i < params; i++)
    {
        const tree_t *param = vector_get(tree->params, i);
        ssa_type_t *ty = ssa_type_from(tree_get_type(param));
        const char *name = tree_get_name(param);

        ssa_param_t it = {
            .name = name,
            .type = ty,
        };

        typevec_set(self->params, i, &it);
        map_set_ptr(ssa->locals, param, (void*)(uintptr_t)i);
    }

    return self;
}

static ssa_module_t *module_create(ssa_compile_t *ssa, const char *name)
{
    vector_t *path = vector_clone(ssa->path);

    ssa_module_t *mod = ctu_malloc(sizeof(ssa_module_t));
    mod->name = name;
    mod->path = path;

    mod->globals = vector_new(32);
    mod->functions = vector_new(32);

    return mod;
}

static ssa_operand_t add_const(ssa_compile_t *ssa, ssa_value_t *value)
{
    ssa_symbol_t *symbol = ssa->currentSymbol;
    size_t index = vector_len(ssa->currentSymbol->consts);
    vector_push(&symbol->consts, value);

    ssa_operand_t operand = {
        .kind = eOperandConst,
        .constant = index
    };

    return operand;
}

static ssa_operand_t bb_add_step(ssa_block_t *bb, ssa_step_t step)
{
    size_t index = typevec_len(bb->steps);

    typevec_push(bb->steps, &step);

    ssa_operand_t operand = {
        .kind = eOperandReg,
        .vregContext = bb,
        .vregIndex = index
    };

    return operand;
}

static ssa_operand_t operand_empty(void)
{
    ssa_operand_t operand = {
        .kind = eOperandEmpty
    };
    return operand;
}

static ssa_operand_t operand_bb(ssa_block_t *bb)
{
    ssa_operand_t operand = {
        .kind = eOperandBlock,
        .bb = bb
    };
    return operand;
}

static ssa_operand_t add_step(ssa_compile_t *ssa, ssa_step_t step)
{
    return bb_add_step(ssa->currentBlock, step);
}

static ssa_block_t *ssa_block_create(ssa_symbol_t *symbol, const char *name, size_t size)
{
    ssa_block_t *bb = ctu_malloc(sizeof(ssa_block_t));
    bb->name = name;
    bb->steps = typevec_new(sizeof(ssa_step_t), size);
    vector_push(&symbol->blocks, bb);

    return bb;
}

static ssa_operand_t compile_tree(ssa_compile_t *ssa, const tree_t *tree);

static ssa_operand_t compile_branch(ssa_compile_t *ssa, const tree_t *branch)
{
    ssa_operand_t cond = compile_tree(ssa, branch->cond);
    ssa_block_t *current = ssa->currentBlock;

    ssa_block_t *tailBlock = ssa_block_create(ssa->currentSymbol, "tail", 0);
    ssa_block_t *thenBlock = ssa_block_create(ssa->currentSymbol, "then", 0);
    ssa_block_t *elseBlock = branch->other ? ssa_block_create(ssa->currentSymbol, "else", 0) : NULL;

    ssa_step_t step = {
        .opcode = eOpBranch,
        .branch = {
            .cond = cond,
            .then = operand_bb(thenBlock),
            .other = elseBlock ? operand_bb(elseBlock) : operand_bb(tailBlock)
        }
    };

    ssa_operand_t tail = {
        .kind = eOperandBlock,
        .bb = tailBlock
    };
    ssa_step_t jumpToTail = {
        .opcode = eOpJump,
        .jump = {
            .target = tail
        }
    };

    ssa->currentBlock = thenBlock;
    compile_tree(ssa, branch->then);
    bb_add_step(thenBlock, jumpToTail);

    if (branch->other != NULL)
    {
        ssa->currentBlock = elseBlock;
        compile_tree(ssa, branch->other);
        bb_add_step(elseBlock, jumpToTail);
    }

    ssa->currentBlock = current;
    add_step(ssa, step);

    ssa->currentBlock = tailBlock;

    return tail;
}

static ssa_operand_t compile_loop(ssa_compile_t *ssa, const tree_t *tree)
{
    /**
    * turns `while (cond) { body }` into:
    *
    * .loop:
    *   %c = cond...
    *   branch %c .body .tail
    * .body:
    *   body...
    *   jmp .loop
    * .tail:
    *
    */
    ssa_block_t *loopBlock = ssa_block_create(ssa->currentSymbol, NULL, 0);
    ssa_block_t *bodyBlock = ssa_block_create(ssa->currentSymbol, NULL, 0);
    ssa_block_t *tailBlock = ssa_block_create(ssa->currentSymbol, NULL, 0);

    ssa_loop_t *save = ctu_malloc(sizeof(ssa_loop_t));
    save->enterLoop = bodyBlock;
    save->exitLoop = tailBlock;
    map_set_ptr(ssa->loops, tree, save);

    ssa_operand_t loop = {
        .kind = eOperandBlock,
        .bb = loopBlock
    };

    ssa_operand_t tail = {
        .kind = eOperandBlock,
        .bb = tailBlock
    };

    ssa_step_t enterLoop = {
        .opcode = eOpJump,
        .jump = {
            .target = loop
        }
    };
    add_step(ssa, enterLoop);

    ssa->currentBlock = loopBlock;
    ssa_operand_t cond = compile_tree(ssa, tree->cond);
    ssa_step_t cmp = {
        .opcode = eOpBranch,
        .branch = {
            .cond = cond,
            .then = operand_bb(bodyBlock),
            .other = tail
        }
    };
    add_step(ssa, cmp);

    ssa->currentBlock = bodyBlock;
    compile_tree(ssa, tree->then);

    ssa_step_t repeatLoop = {
        .opcode = eOpJump,
        .jump = {
            .target = loop
        }
    };
    add_step(ssa, repeatLoop);

    ssa->currentBlock = tailBlock;
    return loop;
}

static ssa_operand_t add_jump(ssa_compile_t *ssa, ssa_loop_t *loop, tree_jump_t jump)
{
    switch (jump)
    {
    case eJumpBreak: {
        ssa_step_t step = {
            .opcode = eOpJump,
            .jump = {
                .target = operand_bb(loop->exitLoop)
            }
        };
        return add_step(ssa, step);
    }
    case eJumpContinue: {
        ssa_step_t step = {
            .opcode = eOpJump,
            .jump = {
                .target = operand_bb(loop->enterLoop)
            }
        };
        return add_step(ssa, step);
    }

    default: NEVER("unhandled jump %d", jump);
    }
}

static size_t get_field_index(const tree_t *ty, const tree_t *field)
{
    size_t result = vector_find(ty->fields, field);
    CTASSERTF(result != SIZE_MAX, "field `%s` not found in `%s`", tree_get_name(field), tree_to_string(ty));
    return result;
}

static ssa_operand_t get_field(ssa_compile_t *ssa, const tree_t *tree)
{
    const tree_t *ty = tree_get_type(tree->object);
    ssa_operand_t object = compile_tree(ssa, tree->object);
    size_t index = get_field_index(ty, tree->field);

    ssa_step_t step = {
        .opcode = eOpMember,
        .member = {
            .object = object,
            .index = index
        }
    };
    return add_step(ssa, step);
}

static ssa_operand_t compile_tree(ssa_compile_t *ssa, const tree_t *tree)
{
    CTASSERT(ssa != NULL);
    CTASSERT(tree != NULL);

    switch (tree->kind)
    {
    case eTreeExprEmpty: {
        ssa_operand_t operand = {
            .kind = eOperandEmpty
        };
        return operand;
    }

    case eTreeExprDigit:
    case eTreeExprBool:
    case eTreeExprUnit: {
        ssa_operand_t operand = {
            .kind = eOperandImm,
            .value = ssa_value_from(tree)
        };
        return operand;
    }

    case eTreeExprString: {
        return add_const(ssa, ssa_value_from(tree));
    }

    case eTreeExprCast: {
        ssa_operand_t expr = compile_tree(ssa, tree->cast);
        ssa_step_t step = {
            .opcode = eOpCast,
            .cast = {
                .operand = expr,
                .type = ssa_type_from(tree_get_type(tree))
            }
        };
        return add_step(ssa, step);
    }

    case eTreeExprOffset: {
        ssa_operand_t expr = compile_tree(ssa, tree->expr);
        ssa_operand_t offset = compile_tree(ssa, tree->offset);

        ssa_step_t step = {
            .opcode = eOpOffset,
            .offset = {
                .array = expr,
                .offset = offset
            }
        };
        return add_step(ssa, step);
    }

    case eTreeExprField: {
        return get_field(ssa, tree);
    }

    case eTreeExprUnary: {
        ssa_operand_t expr = compile_tree(ssa, tree->operand);
        ssa_step_t step = {
            .opcode = eOpUnary,
            .unary = {
                .operand = expr,
                .unary = tree->unary
            }
        };
        return add_step(ssa, step);
    }

    case eTreeExprBinary: {
        ssa_operand_t lhs = compile_tree(ssa, tree->lhs);
        ssa_operand_t rhs = compile_tree(ssa, tree->rhs);
        ssa_step_t step = {
            .opcode = eOpBinary,
            .binary = {
                .lhs = lhs,
                .rhs = rhs,
                .binary = tree->binary
            }
        };
        return add_step(ssa, step);
    }

    case eTreeDeclGlobal: {
        ssa_symbol_t *symbol = map_get_ptr(ssa->globals, tree);
        CTASSERT(symbol != NULL);

        add_dep(ssa, ssa->currentSymbol, symbol);

        ssa_operand_t operand = {
            .kind = eOperandGlobal,
            .global = symbol
        };

        return operand;
    }

    case eTreeExprLoad: {
        ssa_operand_t operand = compile_tree(ssa, tree->load);
        ssa_step_t step = {
            .opcode = eOpLoad,
            .load = {
                .src = operand
            }
        };
        return add_step(ssa, step);
    }

    case eTreeStmtJump: {
        ssa_loop_t *target = map_get_ptr(ssa->loops, tree->label);
        CTASSERTF(target != NULL, "loop not found");

        return add_jump(ssa, target, tree->jump);
    }

    case eTreeExprReference:
    case eTreeExprAddress: {
        ssa_operand_t operand = compile_tree(ssa, tree->expr);
        ssa_step_t step = {
            .opcode = eOpAddress,
            .addr = {
                .symbol = operand
            }
        };
        return add_step(ssa, step);
    }

    case eTreeStmtBlock: {
        size_t len = vector_len(tree->stmts);
        for (size_t i = 0; i < len; i++)
        {
            const tree_t *stmt = vector_get(tree->stmts, i);
            compile_tree(ssa, stmt);
        }

        return operand_empty();
    }

    case eTreeStmtAssign: {
        ssa_operand_t dst = compile_tree(ssa, tree->dst);
        ssa_operand_t src = compile_tree(ssa, tree->src);

        ssa_step_t step = {
            .opcode = eOpStore,
            .store = {
                .dst = dst,
                .src = src
            }
        };
        return add_step(ssa, step);
    }

    case eTreeDeclLocal: {
        size_t idx = (uintptr_t)map_get_default_ptr(ssa->locals, tree, (void*)UINTPTR_MAX);
        CTASSERTF(idx != UINTPTR_MAX, "local `%s` not found", tree_get_name(tree));

        ssa_operand_t local = {
            .kind = eOperandLocal,
            .local = idx
        };
        return local;
    }

    case eTreeDeclParam: {
        size_t idx = (uintptr_t)map_get_default_ptr(ssa->locals, tree, (void*)UINTPTR_MAX);
        CTASSERTF(idx != UINTPTR_MAX, "param `%s` not found", tree_get_name(tree));

        ssa_operand_t param = {
            .kind = eOperandParam,
            .param = idx
        };
        return param;
    }

    case eTreeStmtReturn: {
        ssa_operand_t value = compile_tree(ssa, tree->value);
        ssa_step_t step = {
            .opcode = eOpReturn,
            .ret = {
                .value = value
            }
        };
        return add_step(ssa, step);
    }

    case eTreeExprCall: {
        ssa_operand_t callee = compile_tree(ssa, tree->callee);
        size_t len = vector_len(tree->args);
        typevec_t *args = typevec_of(sizeof(ssa_operand_t), len);
        for (size_t i = 0; i < len; i++)
        {
            const tree_t *arg = vector_get(tree->args, i);
            ssa_operand_t operand = compile_tree(ssa, arg);
            typevec_set(args, i, &operand);
        }

        ssa_step_t step = {
            .opcode = eOpCall,
            .call = {
                .function = callee,
                .args = args
            }
        };

        return add_step(ssa, step);
    }

    case eTreeDeclFunction: {
        ssa_symbol_t *fn = map_get_ptr(ssa->functions, tree);
        CTASSERT(fn != NULL);

        add_dep(ssa, ssa->currentSymbol, fn);
        ssa_operand_t operand = {
            .kind = eOperandFunction,
            .function = fn
        };

        return operand;
    }

    case eTreeStmtBranch: return compile_branch(ssa, tree);
    case eTreeExprCompare: {
        ssa_operand_t lhs = compile_tree(ssa, tree->lhs);
        ssa_operand_t rhs = compile_tree(ssa, tree->rhs);

        ssa_step_t step = {
            .opcode = eOpCompare,
            .compare = {
                .lhs = lhs,
                .rhs = rhs,
                .compare = tree->compare
            }
        };

        return add_step(ssa, step);
    }

    case eTreeStmtLoop:
        return compile_loop(ssa, tree);

    default: NEVER("unhandled tree %s", tree_to_string(tree));
    }
}

static void add_module_globals(ssa_compile_t *ssa, ssa_module_t *mod, map_t *globals)
{
    map_iter_t iter = map_iter(globals);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);

        const tree_t *tree = entry.value;
        CTASSERTF(tree_is(tree, eTreeDeclGlobal), "expected global, got %s", tree_to_string(tree));

        ssa_symbol_t *global = symbol_create(ssa, tree, create_storage_type(tree));

        vector_push(&mod->globals, global);
        map_set_ptr(ssa->globals, tree, global);
    }
}

static void add_module_functions(ssa_compile_t *ssa, ssa_module_t *mod, map_t *functions)
{
    map_iter_t iter = map_iter(functions);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);

        const tree_t *tree = entry.value;
        ssa_symbol_t *symbol = function_create(ssa, tree);

        vector_push(&mod->functions, symbol);
        map_set_ptr(ssa->functions, tree, symbol);
    }
}

static void compile_module(ssa_compile_t *ssa, const tree_t *tree)
{
    const char *id = tree_get_name(tree);
    ssa_module_t *mod = module_create(ssa, id);

    add_module_globals(ssa, mod, tree_module_tag(tree, eSemaValues));
    add_module_functions(ssa, mod, tree_module_tag(tree, eSemaProcs));

    vector_push(&ssa->modules, mod);
    vector_push(&ssa->path, (char*)id);

    map_t *children = tree_module_tag(tree, eSemaModules);
    map_iter_t iter = map_iter(children);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);

        compile_module(ssa, entry.value);
    }

    vector_drop(ssa->path);
}

static void begin_compile(ssa_compile_t *ssa, ssa_symbol_t *symbol)
{
    ssa_block_t *bb = ssa_block_create(symbol, "entry", 4);

    symbol->entry = bb;
    ssa->currentBlock = bb;
    ssa->currentSymbol = symbol;
}

ssa_result_t ssa_compile(map_t *mods)
{
    ssa_compile_t ssa = {
        .modules = vector_new(4),
        .deps = map_optimal(64),

        .globals = map_optimal(32),
        .functions = map_optimal(32),

        .locals = map_optimal(32),
        .loops = map_optimal(32),
    };

    map_iter_t iter = map_iter(mods);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);

        ssa.path = str_split(entry.key, ".");
        compile_module(&ssa, entry.value);
    }

    map_iter_t globals = map_iter(ssa.globals);
    while (map_has_next(&globals))
    {
        map_entry_t entry = map_next(&globals);

        const tree_t *tree = entry.key;
        ssa_symbol_t *global = entry.value;

        begin_compile(&ssa, global);

        if (tree->initial != NULL)
        {
            ssa_operand_t value = compile_tree(&ssa, tree->initial);
            ssa_step_t ret = {
                .opcode = eOpReturn,
                .ret = {
                    .value = value
                }
            };
            add_step(&ssa, ret);
        }
        else
        {
            ssa_value_t *noinit = ssa_value_noinit(global->type);
            ssa_operand_t value = operand_value(noinit);
            ssa_step_t ret = {
                .opcode = eOpReturn,
                .ret = {
                    .value = value
                }
            };
            add_step(&ssa, ret);
        }
    }

    map_iter_t functions = map_iter(ssa.functions);
    while (map_has_next(&functions))
    {
        map_entry_t entry = map_next(&functions);

        const tree_t *tree = entry.key;
        ssa_symbol_t *symbol = entry.value;

        begin_compile(&ssa, symbol);

        const tree_t *body = tree->body;
        if (body != NULL)
        {
            // TODO: should extern functions be put somewhere else
            compile_tree(&ssa, body);
        }
        else
        {
            CTASSERTF(symbol->linkage == eLinkImport,
                "function `%s` has no implementation, but is not an imported symbol (linkage=%s)",
                symbol->name, link_name(symbol->linkage)
            );
        }
    }

    ssa_result_t result = {
        .modules = ssa.modules,
        .deps = ssa.deps
    };

    return result;
}

const char *ssa_type_name(ssa_kind_t kind)
{
    switch (kind)
    {
#define SSA_KIND(ID, NAME) case ID: return NAME;
#include "cthulhu/ssa/ssa.inc"
    default: NEVER("unhandled ssa kind %d", kind);
    }
}

const char *ssa_opkind_name(ssa_opkind_t kind)
{
    switch (kind)
    {
#define SSA_OPKIND(ID, NAME) case ID: return NAME;
#include "cthulhu/ssa/ssa.inc"
    default: NEVER("unhandled ssa opkind %d", kind);
    }
}

const char *ssa_opcode_name(ssa_opcode_t opcode)
{
    switch (opcode)
    {
#define SSA_OPCODE(ID, NAME) case ID: return NAME;
#include "cthulhu/ssa/ssa.inc"
    default: NEVER("unhandled ssa opcode %d", opcode);
    }
}
