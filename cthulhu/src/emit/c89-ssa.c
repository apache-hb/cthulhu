#include "cthulhu/emit/c89.h"

#include "cthulhu/hlir/attribs.h"
#include "cthulhu/hlir/digit.h"
#include "cthulhu/ssa/ssa.h"

#include "std/vector.h"
#include "std/str.h"
#include "std/map.h"
#include "std/set.h"

#include "report/report.h"

#include "base/panic.h"

#include "common.h"

#include <stdio.h>

typedef struct c89_ssa_emit_t
{
    emit_t emit;

    map_t *stepCache; // map_t<ssa_step_t*, const char*>

    set_t *completeEdges; // set_t<ssa_block_t*>
    set_t *pendingEdges; // set_t<ssa_block_t*>
} c89_ssa_emit_t;

#define REPORTS(emit) ((emit)->emit.reports)

static const char *get_type_name(c89_ssa_emit_t *emit, const ssa_type_t *type, const char *name);

static const char *get_digit_name(c89_ssa_emit_t *emit, digit_t digit, sign_t sign)
{
    switch (digit)
    {
    case eDigitChar:
        return sign == eSigned ? "signed char" : "unsigned char";
    case eDigitShort:
        return sign == eSigned ? "signed short" : "unsigned short";
    case eDigitInt:
        return sign == eSigned ? "signed int" : "unsigned int";
    case eDigitLong:
        return sign == eSigned ? "signed long long" : "unsigned long long";
    case eDigitSize:
        return sign == eSigned ? "ssize_t" : "size_t";
    case eDigitPtr:
        return sign == eSigned ? "intptr_t" : "uintptr_t";
    default:
        report(REPORTS(emit), eInternal, NULL, "unhandled digit: %d", digit);
        return "";
    }
}

static const char *get_signature_type(c89_ssa_emit_t *emit, const ssa_type_t *type, const char *name)
{
    size_t len = vector_len(type->args);
    vector_t *params = vector_of(len);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_param_t *arg = vector_get(type->args, i);
        const char *param = get_type_name(emit, arg->type, NULL);
        vector_set(params, i, (char*)param);
    }

    const char *result = get_type_name(emit, type->result, NULL);
    const char *args = str_join(", ", params);

    return name == NULL 
        ? format("%s(*)(%s)", result, args) 
        : format("%s(*%s)(%s)", result, name, args);
}

static const char *get_type_name(c89_ssa_emit_t *emit, const ssa_type_t *type, const char *name)
{
    CTASSERT(type != NULL);

    ssa_kind_t kind = type->kind;

    switch (kind)
    {
    case eTypeDigit: {
        const char *digit = get_digit_name(emit, type->digit, type->sign);
        return name == NULL ? digit : format("%s %s", digit, name);
    }

    case eTypeBool:
        return name == NULL ? "bool" : format("bool %s", name);

    case eTypeUnit:
    case eTypeEmpty:
        return name == NULL ? "void" : format("void %s", name);

    case eTypeString:
        return name == NULL ? "const char *" : format("const char *%s", name);

    case eTypePointer:
        return name == NULL 
            ? format("%s *", get_type_name(emit, type->ptr, NULL)) 
            : format("%s *%s", get_type_name(emit, type->ptr, NULL), name);

    case eTypeSignature:
        return get_signature_type(emit, type, name);

    case eTypeStruct:
        return name == NULL 
            ? format("struct %s", type->name) 
            : format("struct %s %s", type->name, name);

    default:
        report(REPORTS(emit), eInternal, NULL, "unhandled type kind: %d", kind);
        return "";
    }
}

static const char *get_function_name(c89_ssa_emit_t *emit, const ssa_flow_t *function)
{
    switch (function->linkage)
    {
    case eLinkEntryCli:
        return "main";

    default:
        return function->name;
    }
}

static const char *emit_digit(c89_ssa_emit_t *emit, const ssa_type_t *type, const mpz_t digit)
{
    const char *prefix = get_digit_name(emit, type->digit, type->sign);

    char *str = mpz_get_str(NULL, 10, digit);

    return format("((%s)(%s))", prefix, str);
}

static const char *emit_value(c89_ssa_emit_t *emit, const ssa_value_t *value)
{
    ssa_kind_t kind = value->type->kind;

    if (!value->initialized)
    {
        report(REPORTS(emit), eInternal, NULL, "attempting to emit empty value");
        return "";
    }

    switch (kind)
    {
    case eTypeDigit:
        return emit_digit(emit, value->type, value->digit);

    case eTypeBool:
        return value->boolean ? "true" : "false";

    case eTypeString:
        return format("\"%s\"", str_normalizen(value->string.data, value->string.size));

    default:
        report(REPORTS(emit), eInternal, NULL, "unhandled value kind: %d", kind);
        return "";
    }
}

static bool value_exists(const ssa_value_t *value)
{
    if (value == NULL) { return false; }
    if (!value->initialized) { return false; }
    if (value->type->kind == eTypeEmpty) { return false; }

    return true;
}

static void c89_emit_ssa_global(c89_ssa_emit_t *emit, const ssa_flow_t *global)
{
    CTASSERT(global != NULL);
    
    const char *name = global->name;
    const char *type = get_type_name(emit, global->type, name);

    if (value_exists(global->value))
    {
        const char *value = emit_value(emit, global->value);
        WRITE_STRINGF(&emit->emit, "%s = %s;\n", type, value);
    }
    else
    {
        WRITE_STRINGF(&emit->emit, "%s;\n", type);
    }
}

static bool is_extern(const ssa_flow_t *flow)
{
    return flow->linkage == eLinkImported;
}

static const char *get_function_return_type(c89_ssa_emit_t *emit, const ssa_flow_t *flow)
{
    if (is_entry_point(flow->linkage))
    {
        return "int main";
    }
    else
    {
        const char *name = get_function_name(emit, flow);
        return get_type_name(emit, flow->type->result, name);
    }
}

static const char *get_param_name(const ssa_param_t *param, size_t idx, bool fwd)
{
    return fwd ? param->name : format("param%zu", idx);
}

static void c89_emit_function_decl(c89_ssa_emit_t *emit, const ssa_flow_t *flow, bool fwd)
{
    CTASSERT(flow->type->kind == eTypeSignature);
    const ssa_type_t *type = flow->type;

    switch (flow->linkage)
    {
    case eLinkImported:
        WRITE_STRING(&emit->emit, "extern ");
        break;
    case eLinkInternal:
        WRITE_STRING(&emit->emit, "static ");
        break;
    default:
        break;
    }

    const char *result = get_function_return_type(emit, flow);

    WRITE_STRINGF(&emit->emit, "%s(", result);
    
    size_t totalParams = vector_len(type->args);
    if (totalParams == 0) 
    {
        WRITE_STRING(&emit->emit, "void");
    }
    else
    {
        for (size_t i = 0; i < totalParams; i++)
        {
            const ssa_param_t *param = vector_get(type->args, i);
            const char *paramType = get_type_name(emit, param->type, get_param_name(param, i, fwd));

            WRITE_STRINGF(&emit->emit, "%s", paramType);

            if (i < totalParams - 1)
            {
                WRITE_STRING(&emit->emit, ", ");
            }
        }
    }

    if (type->variadic)
    {
        WRITE_STRING(&emit->emit, ", ...");
    }

    WRITE_STRING(&emit->emit, ")");
}

static bool operand_is_empty(ssa_operand_t op)
{
    return op.kind == eOperandEmpty;
}

static void add_edge(c89_ssa_emit_t *emit, const ssa_block_t *block)
{
    bool complete = set_contains_ptr(emit->completeEdges, block);
    if (complete) { return; }

    set_add_ptr(emit->pendingEdges, block);
}

static bool mark_edge_complete(c89_ssa_emit_t *emit, const ssa_block_t *block)
{
    if (set_contains_ptr(emit->completeEdges, block)) { return false; }

    set_add_ptr(emit->completeEdges, block);
    return true;
}

static const char *emit_operand(c89_ssa_emit_t *emit, ssa_operand_t operand)
{
    switch (operand.kind)
    {
    case eOperandImm:
        return emit_value(emit, operand.value);
    case eOperandReg:
        return map_get_ptr(emit->stepCache, operand.vreg);
    case eOperandBlock:
        add_edge(emit, operand.bb);
        return operand.bb->id;

    case eOperandGlobal:
        return operand.global->name;

    case eOperandFunction:
        return get_function_name(emit, operand.function);

    case eOperandLocal:
        return format("local%zu", operand.local);

    case eOperandParam:
        return format("param%zu", operand.param);

    case eOperandEmpty:
        return "";

    default:
        ctu_assert(emit->emit.reports, "unhandled operand kind: %d", operand.kind);
        return "";
    }
}

static const char *c89_emit_return(c89_ssa_emit_t *emit, ssa_return_t ret)
{
    if (operand_is_empty(ret.value))
    {
        return "return;";
    }

    return format("return %s;", emit_operand(emit, ret.value));
}

static const char *c89_emit_jmp(c89_ssa_emit_t *emit, ssa_jmp_t jmp)
{
    return format("goto %s;", emit_operand(emit, jmp.label));
}

static const char *c89_emit_branch(c89_ssa_emit_t *emit, ssa_branch_t branch)
{
    const char *cond = emit_operand(emit, branch.cond);
    const char *trueLabel = emit_operand(emit, branch.truthy);
    const char *falseLabel = emit_operand(emit, branch.falsey);

    return format("if (%s) { goto %s; } else { goto %s; }", cond, trueLabel, falseLabel);
}

static char *c89_gen_reg(c89_ssa_emit_t *emit, const ssa_step_t *step)
{
    return format("reg%s", step->id);
}

static const char *c89_emit_compare(c89_ssa_emit_t *emit, const ssa_step_t *step)
{
    const ssa_compare_t compare = step->compare;
    const char *lhs = emit_operand(emit, compare.lhs);
    const char *rhs = emit_operand(emit, compare.rhs);

    const char *op = compare_symbol(compare.op);

    char *name = c89_gen_reg(emit, step);
    const char *type = get_type_name(emit, step->type, name);
    map_set_ptr(emit->stepCache, step, name);
    
    return format("%s = (%s %s %s);", type, lhs, op, rhs);;
}

static const char *c89_emit_load(c89_ssa_emit_t *emit, const ssa_step_t *step)
{
    const ssa_load_t load = step->load;
    const char *src = emit_operand(emit, load.src);

    char *name = c89_gen_reg(emit, step);
    const char *type = get_type_name(emit, step->type, name);
    map_set_ptr(emit->stepCache, step, name);

    return format("%s = %s;", type, src);
}

static const char *c89_emit_store(c89_ssa_emit_t *emit, const ssa_step_t *step)
{
    const ssa_store_t store = step->store;
    const char *src = emit_operand(emit, store.src);
    const char *dst = emit_operand(emit, store.dst);

    return format("%s = %s;", dst, src);
}

static const char *c89_emit_unary(c89_ssa_emit_t *emit, const ssa_step_t *step)
{
    const ssa_unary_t unary = step->unary;

    const char *operand = emit_operand(emit, unary.operand);
    const char *op = unary_symbol(unary.op);

    char *name = c89_gen_reg(emit, step);
    const char *type = get_type_name(emit, step->type, name);
    map_set_ptr(emit->stepCache, step, name);

    return format("%s = (%s%s);", type, op, operand);
}

static const char *c89_emit_binary(c89_ssa_emit_t *emit, const ssa_step_t *step)
{
    const ssa_binary_t binary = step->binary;

    const char *lhs = emit_operand(emit, binary.lhs);
    const char *rhs = emit_operand(emit, binary.rhs);
    const char *op = binary_symbol(binary.op);

    char *name = c89_gen_reg(emit, step);
    const char *type = get_type_name(emit, step->type, name);
    map_set_ptr(emit->stepCache, step, name);

    return format("%s = (%s %s %s);", type, lhs, op, rhs);
}

static const char *c89_emit_imm(c89_ssa_emit_t *emit, const ssa_step_t *step)
{
    const ssa_imm_t imm = step->imm;

    const char *value = emit_operand(emit, imm.value);

    char *name = c89_gen_reg(emit, step);
    const char *type = get_type_name(emit, step->type, name);
    map_set_ptr(emit->stepCache, step, name);

    return format("%s = %s;", type, value);
}

static bool should_store_result(const ssa_type_t *type)
{
    return !(type->kind == eTypeEmpty || type->kind == eTypeUnit);
}

static const char *c89_emit_call(c89_ssa_emit_t *emit, const ssa_step_t *step)
{
    const ssa_call_t call = step->call;

    const char *symbol = emit_operand(emit, call.symbol);

    vector_t *params = vector_of(call.len);
    for (size_t i = 0; i < call.len; i++)
    {
        const char *arg = emit_operand(emit, call.args[i]);
        vector_set(params, i, (char*)arg);
    }

    const char *args = str_join(", ", params);

    if (should_store_result(step->type))
    {
        char *name = c89_gen_reg(emit, step);
        const char *type = get_type_name(emit, step->type, name);
        map_set_ptr(emit->stepCache, step, name);

        return format("%s = %s(%s);", type, symbol, args);
    }
    else
    {
        return format("%s(%s);", symbol, args);
    }
}

static const char *c89_emit_step(c89_ssa_emit_t *emit, const ssa_step_t *step)
{
    switch (step->opcode)
    {
    case eOpReturn:
        return c89_emit_return(emit, step->ret);

    case eOpJmp:
        return c89_emit_jmp(emit, step->jmp);

    case eOpBranch:
        return c89_emit_branch(emit, step->branch);

    case eOpNop:
        return "/* nop */";

    case eOpCompare:
        return c89_emit_compare(emit, step);

    case eOpLoad:
        return c89_emit_load(emit, step);

    case eOpStore:
        return c89_emit_store(emit, step);

    case eOpUnary:
        return c89_emit_unary(emit, step);

    case eOpBinary:
        return c89_emit_binary(emit, step);

    case eOpImm:
        return c89_emit_imm(emit, step);

    case eOpCall:
        return c89_emit_call(emit, step);

    default:
        ctu_assert(emit->emit.reports, "unhandled opcode: %d", step->opcode);
        return "";
    }
}

static void c89_emit_block(c89_ssa_emit_t *emit, const ssa_block_t *block)
{
    if (!mark_edge_complete(emit, block))
    {
        return;
    }

    WRITE_STRINGF(&emit->emit, "%s: {\n", block->id);
    emit_indent(&emit->emit);

    size_t len = vector_len(block->steps);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_step_t *step = vector_get(block->steps, i);
        const char *it = c89_emit_step(emit, step);
        WRITE_STRINGF(&emit->emit, "%s\n", it);
    }

    emit_dedent(&emit->emit);
    WRITE_STRING(&emit->emit, "}\n");

    set_iter_t iter = set_iter(emit->pendingEdges);
    while (set_has_next(&iter))
    {
        const ssa_block_t *next = set_next(&iter);
        c89_emit_block(emit, next);
    }
}

static void c89_emit_function(c89_ssa_emit_t *emit, const ssa_flow_t *function)
{
    if (is_extern(function)) { return; }

    map_reset(emit->stepCache);
    set_reset(emit->completeEdges);
    set_reset(emit->pendingEdges);

    c89_emit_function_decl(emit, function, false);
    WRITE_STRING(&emit->emit, " {\n");
    emit_indent(&emit->emit);

    size_t locals = vector_len(function->locals);
    for (size_t i = 0; i < locals; i++)
    {
        const ssa_type_t *local = vector_get(function->locals, i);
        const char *type = get_type_name(emit, local, format("local%zu", i));
        WRITE_STRINGF(&emit->emit, "%s;\n", type);
    }

    c89_emit_block(emit, function->entry);

    emit_dedent(&emit->emit);
    WRITE_STRING(&emit->emit, "}\n");
}

static void c89_fwd_function(c89_ssa_emit_t *emit, const ssa_flow_t *function)
{
    if (is_entry_point(function->linkage)) { return; }

    c89_emit_function_decl(emit, function, true);
    WRITE_STRING(&emit->emit, ";\n");
}

void c89_emit_ssa_modules(reports_t *reports, ssa_module_t *module, io_t *dst)
{
    c89_ssa_emit_t emit = {
        .emit = {
            .reports = reports,
            .io = dst,
            .indent = 0,
        },
        .stepCache = map_new(0x1000),
        .completeEdges = set_new(64),
        .pendingEdges = set_new(64),
    };

    section_t symbols = module->symbols;

    size_t totalGlobals = vector_len(symbols.globals);
    size_t totalFunctions = vector_len(symbols.functions);

    WRITE_STRING(&emit.emit, "#include <stdbool.h>\n");

    for (size_t i = 0; i < totalGlobals; i++)
    {
        c89_emit_ssa_global(&emit, vector_get(symbols.globals, i));
    }

    for (size_t i = 0; i < totalFunctions; i++)
    {
        c89_fwd_function(&emit, vector_get(symbols.functions, i));
    }

    for (size_t i = 0; i < totalFunctions; i++)
    {
        c89_emit_function(&emit, vector_get(symbols.functions, i));
    }
}
