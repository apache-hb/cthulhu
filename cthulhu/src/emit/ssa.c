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

typedef struct ssa_emit_t {
    emit_t emit;

    fs_t *fs;
    map_t *deps;
} ssa_emit_t;

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

    return format("closure(result: %s, params: [%s], variadic: %s)", result, params, closure.variadic ? "true" : "false");
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
    case eTypeClosure: return closure_to_string(type->closure);
    default: NEVER("unknown type kind %d", type->kind);
    }
}

static void emit_ssa_attribs(io_t *io, const ssa_symbol_t *symbol)
{
    write_string(io, "\t[extern = %s, linkage = %s]\n",
        symbol->linkName == NULL ? "null" : symbol->linkName,
        link_name(symbol->linkage)
    );
}

static const char *value_to_string(const ssa_value_t *value)
{
    const ssa_type_t *type = value->type;
    switch (type->kind)
    {
    case eTypeDigit: return mpz_get_str(NULL, 10, value->digitValue);
    case eTypeBool: return value->boolValue ? "true" : "false";
    case eTypeUnit: return "unit";
    case eTypeEmpty: return "empty";
    case eTypeString: return format("\"%s\"", str_normalizen(value->stringValue, value->stringLength));

    default: NEVER("unknown type kind %d", type->kind);
    }
}

static const char *operand_to_string(ssa_emit_t *emit, ssa_operand_t operand)
{
    switch (operand.kind)
    {
    case eOperandBlock:
        return format(".%s", get_block_name(&emit->emit, operand.bb));
    case eOperandImm:
        return format("$%s", value_to_string(operand.value));
    case eOperandReg:
        return format("%%%s", get_step_from_block(&emit->emit, operand.vregContext, operand.vregIndex));
    case eOperandGlobal: {
        const ssa_symbol_t *symbol = operand.global;
        return format("@%s", symbol->name);
    }
    case eOperandFunction: {
        const ssa_symbol_t *symbol = operand.function;
        return format("::%s", symbol->name);
    }
    case eOperandLocal: {
        size_t index = operand.local;
        return format("local(%zu)", index);
    }
    default: NEVER("unknown operand kind %d", operand.kind);
    }
}

static void emit_ssa_block(ssa_emit_t *emit, io_t *io, const ssa_block_t *bb)
{
    size_t len = typevec_len(bb->steps);
    write_string(io, ".%s: [len=%zu]\n", get_block_name(&emit->emit, bb), len);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_step_t *step = typevec_offset(bb->steps, i);
        switch (step->opcode)
        {
        case eOpUnary:
            ssa_unary_t unary = step->unary;
            write_string(io, "\t%%%s = unary %s %s\n",
                get_step_name(&emit->emit, step),
                unary_name(unary.unary),
                operand_to_string(emit, unary.operand)
            );
            break;
        case eOpBinary:
            ssa_binary_t binary = step->binary;
            write_string(io, "\t%%%s = binary %s %s %s\n",
                get_step_name(&emit->emit, step),
                binary_name(binary.binary),
                operand_to_string(emit, binary.lhs),
                operand_to_string(emit, binary.rhs)
            );
            break;
        case eOpLoad:
            ssa_load_t load = step->load;
            write_string(io, "\t%%%s = load %s\n",
                get_step_name(&emit->emit, step),
                operand_to_string(emit, load.src)
            );
            break;
        case eOpReturn:
            ssa_return_t ret = step->ret;
            write_string(io, "\tret %s\n", operand_to_string(emit, ret.value));
            break;
        case eOpJump:
            ssa_jump_t jmp = step->jump;
            write_string(io, "\tjump %s\n", operand_to_string(emit, jmp.target));
            break;
        case eOpStore:
            ssa_store_t store = step->store;
            write_string(io, "\tstore %s %s\n",
                operand_to_string(emit, store.dst),
                operand_to_string(emit, store.src)
            );
            break;
        case eOpCall:
            ssa_call_t call = step->call;
            size_t len = typevec_len(call.args);
            vector_t *args = vector_of(len);
            for (size_t i = 0; i < len; i++)
            {
                const ssa_operand_t *arg = typevec_offset(call.args, i);
                vector_set(args, i, (char*)operand_to_string(emit, *arg));
            }
            write_string(io, "\t%%%s = call %s (%s)\n",
                get_step_name(&emit->emit, step),
                operand_to_string(emit, call.function),
                str_join(", ", args)
            );
            break;
        case eOpBranch:
            ssa_branch_t branch = step->branch;
            write_string(io, "\tbranch %s %s %s\n",
                operand_to_string(emit, branch.cond),
                operand_to_string(emit, branch.then),
                operand_to_string(emit, branch.other)
            );
            break;
        case eOpCompare:
            ssa_compare_t compare = step->compare;
            write_string(io, "\t%%%s = compare %s %s %s\n",
                get_step_name(&emit->emit, step),
                compare_name(compare.compare),
                operand_to_string(emit, compare.lhs),
                operand_to_string(emit, compare.rhs)
            );
            break;
        default: NEVER("unknown opcode %d", step->opcode);
        }
    }
}

static void emit_ssa_blocks(ssa_emit_t *emit, io_t *io, vector_t *bbs)
{
    size_t len = vector_len(bbs);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_block_t *bb = vector_get(bbs, i);
        emit_ssa_block(emit, io, bb);
    }
}

static void emit_symbol_deps(io_t *io, const ssa_symbol_t *symbol, map_t *deps)
{
    set_t *all = map_get_ptr(deps, symbol);
    if (all != NULL)
    {
        write_string(io, "deps: (");
        set_iter_t iter = set_iter(all);
        while (set_has_next(&iter))
        {
            const ssa_symbol_t *dep = set_next(&iter);
            write_string(io, "%s", dep->name);

            if (set_has_next(&iter)) { write_string(io, ", "); }
        }
        write_string(io, ")\n");
    }
}

static void emit_ssa_module(ssa_emit_t *emit, const ssa_module_t *mod)
{
    fs_t *fs = emit->fs;
    // TODO: dedup this

    // this is really badly named :p
    // whats really going on is that we're checking if the module has the same name.
    // as the last element in the path, in these cases we dont want to emit the last element of the path.
    // this is not required for correctness but makes the output nicer to grok.
    bool isRootMod = check_root_mod(mod->path, mod->name);

    vector_t *vec = vector_clone(mod->path); // lets not scuff the original path
    if (isRootMod) { vector_drop(vec); }

    // TODO: this may start failing if the api for fs_dir_create changes
    char *path = str_join("/", vec);
    if (vector_len(vec) > 0)
    {
        fs_dir_create(fs, path);
    }

    char *file = format("%s/%s.ssa", path, mod->name);
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
        emit_symbol_deps(io, global, emit->deps);

        write_string(io, "%s global %s: %s\n", vis_name(global->visibility), global->name, type_to_string(global->type));
        emit_ssa_attribs(io, global);

        emit_ssa_blocks(emit, io, global->blocks);

        if (i != len - 1) { write_string(io, "\n"); }
    }

    size_t fns = vector_len(mod->functions);
    for (size_t i = 0; i < fns; i++)
    {
        const ssa_symbol_t *fn = vector_get(mod->functions, i);
        emit_symbol_deps(io, fn, emit->deps);

        write_string(io, "%s fn %s: %s\n", vis_name(fn->visibility), fn->name, type_to_string(fn->type));
        emit_ssa_attribs(io, fn);

        emit_ssa_blocks(emit, io, fn->blocks);

        if (i != fns - 1) { write_string(io, "\n"); }
    }
}

ssa_emit_result_t emit_ssa(const ssa_emit_options_t *options)
{
    const emit_options_t opts = options->opts;
    ssa_emit_t emit = {
        .emit = {
            .reports = opts.reports,
            .blockNames = names_new(64),
            .vregNames = names_new(64),
        },
        .fs = opts.fs,
        .deps = opts.deps
    };

    size_t len = vector_len(opts.modules);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_module_t *mod = vector_get(opts.modules, i);
        emit_ssa_module(&emit, mod);
    }

    ssa_emit_result_t result = { NULL };
    return result;
}
