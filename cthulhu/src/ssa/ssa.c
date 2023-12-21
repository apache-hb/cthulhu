#include "common/common.h"

#include "cthulhu/tree/query.h"

#include "std/str.h"
#include "std/map.h"
#include "std/set.h"
#include "std/vector.h"

#include "std/typed/vector.h"

#include "memory/memory.h"
#include "base/panic.h"
#include "core/macros.h"

#include <string.h>

/// @brief the ssa compilation context
typedef struct ssa_compile_t
{
    /// result data

    /// @brief all modules in the program
    /// vector<ssa_module>
    vector_t *modules;

    /// @brief direct dependencies between symbols
    /// dependecies are a cyclic graph, this is a map of the edges
    /// map<ssa_symbol, set<ssa_symbol>>
    map_t *symbol_deps;

    /// internal data

    arena_t *arena;

    /// @brief all globals in the program
    /// map<tree, ssa_symbol>
    map_t *globals;

    /// @brief all functions in the program
    /// map<tree, ssa_symbol>
    map_t *functions;

    /// @brief all types in the program
    /// map<tree, ssa_type>
    map_t *types;

    /// @brief all locals in the current symbol
    /// map<tree, size_t>
    map_t *symbol_locals;

    /// @brief all loops in the current symbol
    /// map<tree, ssa_loop>
    map_t *symbol_loops;

    /// @brief the current block being compiled
    ssa_block_t *current_block;

    /// @brief the current symbol being compiled
    /// can be a function or a global
    ssa_symbol_t *current_symbol;

    /// @brief the current module being compiled
    ssa_module_t *current_sodule;

    /// @brief the path to the current module
    /// used for name mangling. vector<const char *>
    vector_t *path;
} ssa_compile_t;

/// @brief loop jump context
typedef struct ssa_loop_t
{
    /// @brief the block to jump to when continuing the loop
    ssa_block_t *enter_loop;

    /// @brief the block to jump to when exiting the loop
    ssa_block_t *exit_loop;
} ssa_loop_t;

static void add_dep(ssa_compile_t *ssa, const ssa_symbol_t *symbol, const ssa_symbol_t *dep)
{
    set_t *set = map_get_ptr(ssa->symbol_deps, symbol);
    if (set == NULL)
    {
        set = set_new(8);
        map_set_ptr(ssa->symbol_deps, symbol, set);
    }

    set_add_ptr(set, dep);
}

static ssa_symbol_t *symbol_create(ssa_compile_t *ssa, const tree_t *tree, ssa_storage_t storage)
{
    CTU_UNUSED(ssa);

    const char *name = tree_get_name(tree);
    const ssa_type_t *type = ssa_type_create_cached(ssa->types, tree_get_type(tree));
    const tree_attribs_t *attrib = tree_get_attrib(tree);

    ssa_symbol_t *symbol = ARENA_MALLOC(ssa->arena, sizeof(ssa_symbol_t), name, ssa);
    symbol->linkage = attrib->link;
    symbol->visibility = attrib->visibility;
    symbol->link_name = attrib->mangle;
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

static ssa_storage_t create_storage_type(map_t *types, const tree_t *decl)
{
    ssa_storage_t storage = {
        .type = ssa_type_create_cached(types, tree_get_storage_type(decl)),
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
        ssa_storage_t type_storage = create_storage_type(ssa->types, local);
        const ssa_type_t *type = ssa_type_create_cached(ssa->types, tree_get_type(local));
        const char *name = tree_get_name(local);

        ssa_local_t it = {
            .storage = type_storage,
            .name = name,
            .type = type,
        };

        typevec_set(self->locals, i, &it);
        map_set_ptr(ssa->symbol_locals, local, (void*)(uintptr_t)i);
    }

    size_t params = vector_len(tree->params);
    self->params = typevec_of(sizeof(ssa_param_t), params);
    for (size_t i = 0; i < params; i++)
    {
        const tree_t *param = vector_get(tree->params, i);
        ssa_type_t *ty = ssa_type_create_cached(ssa->types, tree_get_type(param));
        const char *name = tree_get_name(param);

        ssa_param_t it = {
            .name = name,
            .type = ty,
        };

        typevec_set(self->params, i, &it);
        map_set_ptr(ssa->symbol_locals, param, (void*)(uintptr_t)i);
    }

    return self;
}

static ssa_module_t *module_create(ssa_compile_t *ssa, const char *name)
{
    vector_t *path = vector_clone(ssa->path);

    ssa_module_t *mod = ARENA_MALLOC(ssa->arena, sizeof(ssa_module_t), name, ssa);
    mod->name = name;
    mod->path = path;

    mod->globals = vector_new(32);
    mod->functions = vector_new(32);
    mod->types = vector_new(32);

    return mod;
}

static ssa_operand_t add_const(ssa_compile_t *ssa, ssa_value_t *value)
{
    ssa_symbol_t *symbol = ssa->current_symbol;
    size_t index = vector_len(ssa->current_symbol->consts);
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
        .vreg_context = bb,
        .vreg_index = index
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
    return bb_add_step(ssa->current_block, step);
}

static ssa_block_t *ssa_block_create(ssa_symbol_t *symbol, const char *name, size_t size, arena_t *arena)
{
    ssa_block_t *bb = ARENA_MALLOC(arena, sizeof(ssa_block_t), name, symbol);
    bb->name = name;
    bb->steps = typevec_new(sizeof(ssa_step_t), size, arena);
    vector_push(&symbol->blocks, bb);

    return bb;
}

static ssa_operand_t compile_tree(ssa_compile_t *ssa, const tree_t *tree);

static ssa_operand_t compile_branch(ssa_compile_t *ssa, const tree_t *branch)
{
    ssa_operand_t cond = compile_tree(ssa, branch->cond);
    ssa_block_t *current = ssa->current_block;

    ssa_block_t *tail_block = ssa_block_create(ssa->current_symbol, "tail", 0, ssa->arena);
    ssa_block_t *then_block = ssa_block_create(ssa->current_symbol, "then", 0, ssa->arena);
    ssa_block_t *else_block = branch->other != NULL ? ssa_block_create(ssa->current_symbol, "other", 0, ssa->arena) : NULL;

    ssa_step_t step = {
        .opcode = eOpBranch,
        .branch = {
            .cond = cond,
            .then = operand_bb(then_block),
            .other = else_block ? operand_bb(else_block) : operand_bb(tail_block)
        }
    };

    ssa_operand_t tail = {
        .kind = eOperandBlock,
        .bb = tail_block
    };
    ssa_step_t jump_to_tail = {
        .opcode = eOpJump,
        .jump = {
            .target = tail
        }
    };

    ssa->current_block = then_block;
    compile_tree(ssa, branch->then);
    bb_add_step(then_block, jump_to_tail);

    if (branch->other != NULL)
    {
        ssa->current_block = else_block;
        compile_tree(ssa, branch->other);
        bb_add_step(else_block, jump_to_tail);
    }

    ssa->current_block = current;
    add_step(ssa, step);

    ssa->current_block = tail_block;

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
    ssa_block_t *loop_block = ssa_block_create(ssa->current_symbol, NULL, 0, ssa->arena);
    ssa_block_t *body_block = ssa_block_create(ssa->current_symbol, NULL, 0, ssa->arena);
    ssa_block_t *tail_block = ssa_block_create(ssa->current_symbol, NULL, 0, ssa->arena);

    ssa_loop_t *save = ARENA_MALLOC(ssa->arena, sizeof(ssa_loop_t), "ssa_loop", ssa);
    save->enter_loop = body_block;
    save->exit_loop = tail_block;
    map_set_ptr(ssa->symbol_loops, tree, save);

    ssa_operand_t loop = {
        .kind = eOperandBlock,
        .bb = loop_block
    };

    ssa_operand_t tail = {
        .kind = eOperandBlock,
        .bb = tail_block
    };

    ssa_step_t enter_loop = {
        .opcode = eOpJump,
        .jump = {
            .target = loop
        }
    };
    add_step(ssa, enter_loop);

    ssa->current_block = loop_block;
    ssa_operand_t cond = compile_tree(ssa, tree->cond);
    ssa_step_t cmp = {
        .opcode = eOpBranch,
        .branch = {
            .cond = cond,
            .then = operand_bb(body_block),
            .other = tail
        }
    };
    add_step(ssa, cmp);

    ssa->current_block = body_block;
    compile_tree(ssa, tree->then);

    ssa_step_t repeat_loop = {
        .opcode = eOpJump,
        .jump = {
            .target = loop
        }
    };
    add_step(ssa, repeat_loop);

    ssa->current_block = tail_block;
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
                .target = operand_bb(loop->exit_loop)
            }
        };
        return add_step(ssa, step);
    }
    case eJumpContinue: {
        ssa_step_t step = {
            .opcode = eOpJump,
            .jump = {
                .target = operand_bb(loop->enter_loop)
            }
        };
        return add_step(ssa, step);
    }

    default: NEVER("unhandled jump %d", jump);
    }
}

static size_t get_field_index(const tree_t *ty, const tree_t *field)
{
    CTASSERTF(tree_is(ty, eTreeTypeStruct), "expected struct, got %s", tree_to_string(ty));

    size_t result = vector_find(ty->fields, field);
    CTASSERTF(result != SIZE_MAX, "field `%s` not found in `%s`", tree_get_name(field), tree_to_string(ty));
    return result;
}

static ssa_operand_t get_field(ssa_compile_t *ssa, const tree_t *tree)
{
    const tree_t *ty = tree_get_type(tree->object);
    CTASSERTF(tree_ty_is_address(ty), "expected address, got %s", tree_to_string(ty));

    ssa_operand_t object = compile_tree(ssa, tree->object);
    size_t index = get_field_index(ty->ptr, tree->field);

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
            .value = ssa_value_from(ssa->types, tree)
        };
        return operand;
    }

    case eTreeExprString: {
        return add_const(ssa, ssa_value_from(ssa->types, tree));
    }

    case eTreeExprCast: {
        ssa_operand_t expr = compile_tree(ssa, tree->cast);
        ssa_step_t step = {
            .opcode = eOpCast,
            .cast = {
                .operand = expr,
                .type = ssa_type_create_cached(ssa->types, tree_get_type(tree))
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
        CTASSERTF(symbol != NULL, "symbol table missing `%s` (%p)", tree_to_string(tree), (void*)tree);

        add_dep(ssa, ssa->current_symbol, symbol);

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
        ssa_loop_t *target = map_get_ptr(ssa->symbol_loops, tree->label);
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
        size_t idx = (uintptr_t)map_get_default_ptr(ssa->symbol_locals, tree, (void*)UINTPTR_MAX);
        CTASSERTF(idx != UINTPTR_MAX, "local `%s` not found", tree_get_name(tree));

        ssa_operand_t local = {
            .kind = eOperandLocal,
            .local = idx
        };
        return local;
    }

    case eTreeDeclParam: {
        size_t idx = (uintptr_t)map_get_default_ptr(ssa->symbol_locals, tree, (void*)UINTPTR_MAX);
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

        add_dep(ssa, ssa->current_symbol, fn);
        ssa_operand_t operand = {
            .kind = eOperandFunction,
            .function = fn
        };

        return operand;
    }

    case eTreeStmtBranch:
        return compile_branch(ssa, tree);

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

        ssa_symbol_t *global = symbol_create(ssa, tree, create_storage_type(ssa->types, tree));

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

static void add_module_types(ssa_compile_t *ssa, ssa_module_t *mod, map_t *types)
{
    map_iter_t iter = map_iter(types);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);

        const tree_t *tree = entry.value;
        ssa_type_t *type = ssa_type_create_cached(ssa->types, tree);

        vector_push(&mod->types, type);
        map_set_ptr(ssa->types, tree, type);
    }
}

static void compile_module(ssa_compile_t *ssa, const tree_t *tree)
{
    const char *id = tree_get_name(tree);
    ssa_module_t *mod = module_create(ssa, id);

    add_module_globals(ssa, mod, tree_module_tag(tree, eSemaValues));
    add_module_functions(ssa, mod, tree_module_tag(tree, eSemaProcs));
    add_module_types(ssa, mod, tree_module_tag(tree, eSemaTypes));

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
    ssa_block_t *bb = ssa_block_create(symbol, "entry", 4, ssa->arena);

    symbol->entry = bb;
    ssa->current_block = bb;
    ssa->current_symbol = symbol;
}

// static void reset_local_maps(ssa_compile_t *ssa)
// {
//     map_reset(ssa->locals);
//     map_reset(ssa->loops);
// }

/// @brief a prediction of how many items will be in each map
/// this is not a hard limit, but a hint to the allocator
typedef struct ssa_map_sizes_t
{
    /// @brief the number of modules in the program
    size_t modules;

    /// @brief the number of dependencies between symbols
    size_t deps;

    /// @brief the number of globals in the program
    size_t globals;

    /// @brief the number of functions in the program
    size_t functions;

    /// @brief the number of types in the program
    size_t types;
} ssa_map_sizes_t;

void count_modules(ssa_map_sizes_t *sizes, const tree_t *tree)
{
    CTASSERT(tree_is(tree, eTreeDeclModule));

    sizes->modules += 1;

    sizes->globals += map_count(tree_module_tag(tree, eSemaValues));
    sizes->functions += map_count(tree_module_tag(tree, eSemaProcs));
    sizes->types += map_count(tree_module_tag(tree, eSemaTypes));

    // count all child modules
    map_iter_t iter = map_iter(tree_module_tag(tree, eSemaModules));
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        count_modules(sizes, entry.value);
    }
}

ssa_map_sizes_t predict_maps(map_t *mods)
{
    // initialize will small sizes just in case something
    // returns 0
    ssa_map_sizes_t sizes = {
        .modules = 4,
        .deps = 4,

        .globals = 32,
        .functions = 32,
        .types = 32,
    };

    map_iter_t iter = map_iter(mods);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        count_modules(&sizes, entry.value);
    }

    sizes.deps = sizes.functions + sizes.globals;

    return sizes;
}

ssa_result_t ssa_compile(map_t *mods)
{
    ssa_map_sizes_t sizes = predict_maps(mods);

    ssa_compile_t ssa = {
        .arena = get_global_arena(),

        .modules = vector_new(sizes.modules),
        .symbol_deps = map_optimal(sizes.deps),

        .globals = map_optimal(sizes.globals),
        .functions = map_optimal(sizes.functions),
        .types = map_optimal(sizes.types),

        // TODO: these should be per symbol rather than persistent global
        .symbol_locals = map_optimal(sizes.deps * 4),
        .symbol_loops = map_optimal(32),
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

        map_reset(ssa.symbol_loops);
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

        map_reset(ssa.symbol_loops);
    }

    ssa_result_t result = {
        .modules = ssa.modules,
        .deps = ssa.symbol_deps
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
