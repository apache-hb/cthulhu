#include "cthulhu/emit/c89.h"

#include "cthulhu/hlir/attribs.h"
#include "cthulhu/hlir/digit.h"
#include "cthulhu/ssa/ssa.h"

#include "std/vector.h"
#include "std/str.h"
#include "std/map.h"
#include "std/set.h"
#include "io/io.h"

#include "report/report.h"

#include "base/panic.h"

#include "common.h"

#include <stdio.h>

typedef struct c89_ssa_emit_t
{
    reports_t *reports;
    
    emit_t emit;
    emit_t header;

    emit_t *dst;

    map_t *stepCache; // map_t<ssa_step_t*, const char*>

    set_t *completeEdges; // set_t<ssa_block_t*>
    set_t *pendingEdges; // set_t<ssa_block_t*>

    set_t *publicSymbols; // set_t<void*>

    const ssa_flow_t *currentFlow;
} c89_ssa_emit_t;

static bool should_emit_to_header(c89_ssa_emit_t *emit, const void *symbol)
{
    return set_contains_ptr(emit->publicSymbols, symbol);
}

static void add_public_symbol(c89_ssa_emit_t *emit, const void *symbol)
{
    set_add_ptr(emit->publicSymbols, symbol);
}

static bool has_header(c89_ssa_emit_t *emit)
{
    return emit->header.io != NULL;
}

static void pick_dst(c89_ssa_emit_t *emit, const void *symbol)
{
    if (!has_header(emit))
    {
        emit->dst = &emit->emit;
        return;
    }

    emit->dst = should_emit_to_header(emit, symbol) 
        ? &emit->header 
        : &emit->emit;
}

#define REPORTS(emit) ((emit)->reports)

#define WRITE_TEXT(it, str) WRITE_STRING((it)->dst, str)
#define WRITE_TEXTF(it, fmt, ...) WRITE_STRINGF((it)->dst, fmt, __VA_ARGS__)

#define WRITE_SOURCE(it, str) WRITE_STRING(&(it)->emit, str)
#define WRITE_SOURCEF(it, fmt, ...) WRITE_STRINGF(&(it)->emit, fmt, __VA_ARGS__)

#define WRITE_HEADER(it, str) WRITE_STRING(&(it)->header, str)
#define WRITE_HEADERF(it, fmt, ...) WRITE_STRINGF(&(it)->header, fmt, __VA_ARGS__)

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

    case eTypeDecimal:
        return name == NULL ? "float" : format("float %s", name);

    case eTypeBool:
        return name == NULL ? "bool" : format("bool %s", name);

    case eTypeUnit:
    case eTypeEmpty:
        return name == NULL ? "void" : format("void %s", name);

    case eTypeString:
        return name == NULL ? "const char *" : format("const char *%s", name);

    case eTypePointer:
        return name == NULL 
            ? format("%s*", get_type_name(emit, type->ptr, NULL))
            : get_type_name(emit, type->ptr, format("*%s", name));

    case eTypeSignature:
        return get_signature_type(emit, type, name);

    case eTypeStruct:
        return name == NULL 
            ? format("struct %s", type->name) 
            : format("struct %s %s", type->name, name);

    case eTypeUnion:
        return name == NULL 
            ? format("union %s", type->name) 
            : format("union %s %s", type->name, name);

    case eTypeOpaque:
        return name == NULL ? "void *" : format("void *%s", name);

    case eTypeArray:
        return name == NULL 
            ? format("%s[%zu]", get_type_name(emit, type->ptr, NULL), type->size) 
            : get_type_name(emit, type->ptr, format("%s[%zu]", name, type->size));

    default:
        report(REPORTS(emit), eInternal, NULL, "unhandled type kind: %d", kind);
        return "";
    }
}

static const char *get_flow_name(const ssa_flow_t *function)
{
    switch (function->linkage)
    {
    case eLinkEntryCli:
        return "main";
    case eLinkImported:
        return function->symbol == NULL ? function->name : function->symbol;

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

static char *emit_decimal(const mpq_t decimal)
{
    mpz_t num;
    mpz_t den;
    mpz_init(num);
    mpz_init(den);

    mpq_get_num(num, decimal);
    mpq_get_den(den, decimal);

    return format("(%s.%sf)", mpz_get_str(NULL, 10, num), mpz_get_str(NULL, 10, den));
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

    case eTypeDecimal:
        return emit_decimal(value->decimal);

    case eTypeOpaque:
        return format("(void*)(0x%s)", mpz_get_str(NULL, 16, value->digit));

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
    
    pick_dst(emit, global);

    const char *name = get_flow_name(global);
    const char *type = get_type_name(emit, global->type, name);

    if (global->linkage == eLinkImported || !value_exists(global->value))
    {
        WRITE_TEXTF(emit, "extern %s;\n", type);
    }
    else
    {
        const char *value = emit_value(emit, global->value);
        WRITE_SOURCEF(emit, "%s = %s;\n", type, value);
    }
}

static bool is_extern(const ssa_flow_t *flow)
{
    return flow->linkage == eLinkImported;
}

static const char *get_function_return_type(c89_ssa_emit_t *emit, const ssa_flow_t *flow)
{
    switch (flow->linkage)
    {
    case eLinkEntryCli:
        return "int main";
    case eLinkEntryGui:
        return "int wWinMain";
    default:
        break;
    }

    const char *name = get_flow_name(flow);
    return get_type_name(emit, flow->type->result, name);
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
        WRITE_TEXT(emit, "extern ");
        break;
    case eLinkInternal:
        WRITE_TEXT(emit, "static ");
        break;
    
    default:
        break;
    }

    const char *result = get_function_return_type(emit, flow);

    WRITE_TEXTF(emit, "%s(", result);
    
    size_t totalParams = vector_len(type->args);
    if (totalParams == 0) 
    {
        WRITE_TEXT(emit, "void");
    }
    else
    {
        for (size_t i = 0; i < totalParams; i++)
        {
            const ssa_param_t *param = vector_get(type->args, i);
            const char *paramType = get_type_name(emit, param->type, get_param_name(param, i, fwd));

            WRITE_TEXTF(emit, "%s", paramType);

            if (i < totalParams - 1)
            {
                WRITE_TEXT(emit, ", ");
            }
        }

        if (type->arity == eArityVariable)
        {
            WRITE_TEXT(emit, ", ...");
        }
    }

    WRITE_TEXT(emit, ")");
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
        return get_flow_name(operand.global);

    case eOperandFunction:
        return get_flow_name(operand.function);

    case eOperandLocal:
        return format("local%zu", operand.local);

    case eOperandParam:
        return format("param%zu", operand.param);

    case eOperandEmpty:
        return "";

    default:
        ctu_assert(REPORTS(emit), "unhandled operand kind: %d", operand.kind);
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

static char *c89_gen_reg(const ssa_step_t *step)
{
    return format("reg%s", step->id);
}

static const char *c89_emit_compare(c89_ssa_emit_t *emit, const ssa_step_t *step)
{
    const ssa_compare_t compare = step->compare;
    const char *lhs = emit_operand(emit, compare.lhs);
    const char *rhs = emit_operand(emit, compare.rhs);

    const char *op = compare_symbol(compare.op);

    char *name = c89_gen_reg(step);
    const char *type = get_type_name(emit, step->type, name);
    map_set_ptr(emit->stepCache, step, name);
    
    return format("%s = (%s %s %s);", type, lhs, op, rhs);;
}

static const char *c89_emit_load(c89_ssa_emit_t *emit, const ssa_step_t *step)
{
    const ssa_load_t load = step->load;
    const char *src = emit_operand(emit, load.src);

    const char *deref = "*";
    if (load.src.kind == eOperandParam 
        || load.src.kind == eOperandGlobal
        || load.src.kind == eOperandLocal)
    {
        deref = "";
    }

    char *name = c89_gen_reg(step);
    const char *type = get_type_name(emit, step->type, name);
    map_set_ptr(emit->stepCache, step, name);

    return format("%s = %s%s;", type, deref, src);
}

static const char *c89_emit_store(c89_ssa_emit_t *emit, const ssa_step_t *step)
{
    const ssa_store_t store = step->store;
    const char *src = emit_operand(emit, store.src);
    const char *dst = emit_operand(emit, store.dst);

    const char *deref = "*";
    if (store.dst.kind == eOperandParam 
        || store.dst.kind == eOperandGlobal
        || store.dst.kind == eOperandLocal)
    {
        deref = "";
    }

    return format("%s%s = %s;", deref, dst, src);
}

static const char *c89_emit_unary(c89_ssa_emit_t *emit, const ssa_step_t *step)
{
    const ssa_unary_t unary = step->unary;

    const char *operand = emit_operand(emit, unary.operand);
    const char *op = unary_symbol(unary.op);

    char *name = c89_gen_reg(step);
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

    char *name = c89_gen_reg(step);
    const char *type = get_type_name(emit, step->type, name);
    map_set_ptr(emit->stepCache, step, name);

    return format("%s = (%s %s %s);", type, lhs, op, rhs);
}

static const char *c89_emit_imm(c89_ssa_emit_t *emit, const ssa_step_t *step)
{
    const ssa_imm_t imm = step->imm;

    const char *value = emit_operand(emit, imm.value);

    char *name = c89_gen_reg(step);
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
        char *name = c89_gen_reg(step);
        const char *type = get_type_name(emit, step->type, name);
        map_set_ptr(emit->stepCache, step, name);

        return format("%s = %s(%s);", type, symbol, args);
    }
    else
    {
        return format("%s(%s);", symbol, args);
    }
}

static const char *c89_emit_cast(c89_ssa_emit_t *emit, const ssa_step_t *step)
{
    const ssa_cast_t cast = step->cast;

    const char *operand = emit_operand(emit, cast.operand);
    const char *type = get_type_name(emit, cast.type, NULL);

    char *name = c89_gen_reg(step);
    const char *result = get_type_name(emit, step->type, name);
    map_set_ptr(emit->stepCache, step, name);

    return format("%s = (%s)%s;", result, type, operand);
}

static const char *c89_emit_addr(c89_ssa_emit_t *emit, const ssa_step_t *step)
{
    const ssa_addr_t addr = step->addr;

    const char *operand = emit_operand(emit, addr.expr);

    const char *ref = "";
    if (addr.expr.kind == eOperandParam 
        || addr.expr.kind == eOperandGlobal
        || addr.expr.kind == eOperandLocal)
    {
        ref = "&";
    }

    char *name = c89_gen_reg(step);
    const char *result = get_type_name(emit, step->type, name);
    map_set_ptr(emit->stepCache, step, name);

    return format("%s = %s%s;", result, ref, operand);
}

static const char *c89_emit_sizeof(c89_ssa_emit_t *emit, const ssa_step_t *step)
{
    const ssa_sizeof_t size = step->size;

    const char *type = get_type_name(emit, size.type, NULL);

    char *name = c89_gen_reg(step);
    const char *result = get_type_name(emit, step->type, name);
    map_set_ptr(emit->stepCache, step, name);

    return format("%s = sizeof(%s);", result, type);
}

static const char *c89_emit_index(c89_ssa_emit_t *emit, const ssa_step_t *step)
{
    const ssa_index_t index = step->index;

    const char *array = emit_operand(emit, index.array);
    const char *idx = emit_operand(emit, index.index);

    char *name = c89_gen_reg(step);
    const char *result = get_type_name(emit, ssa_type_ptr_new("", step->type), name);
    map_set_ptr(emit->stepCache, step, name);

    return format("%s = %s + %s;", result, array, idx);
}

static const char *c89_emit_offset(c89_ssa_emit_t *emit, const ssa_step_t *step)
{
    const ssa_offset_t offset = step->offset;

    const char *operand = emit_operand(emit, offset.object);
    const char *field = format("field%zu", offset.field);

    const ssa_type_t *type = ssa_get_operand_type(REPORTS(emit), emit->currentFlow, offset.object);
    const char *indirect = type->kind == eTypePointer ? "->" : ".";

    char *name = c89_gen_reg(step);
    const char *result = get_type_name(emit, ssa_type_ptr_new(name, step->type), name);
    map_set_ptr(emit->stepCache, step, name);

    return format("%s = &%s%s%s;", result, operand, indirect, field);
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

    case eOpCast:
        return c89_emit_cast(emit, step);

    case eOpAddr:
        return c89_emit_addr(emit, step);

    case eOpSizeOf:
        return c89_emit_sizeof(emit, step);

    case eOpIndex:
        return c89_emit_index(emit, step);

    case eOpOffset:
        return c89_emit_offset(emit, step);

    default:
        ctu_assert(REPORTS(emit), "unhandled opcode: %d", step->opcode);
        return "";
    }
}

static void c89_emit_block(c89_ssa_emit_t *emit, const ssa_block_t *block)
{
    if (!mark_edge_complete(emit, block))
    {
        return;
    }

    WRITE_SOURCEF(emit, "%s: {\n", block->id);
    emit_indent(&emit->emit);

    size_t len = vector_len(block->steps);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_step_t *step = vector_get(block->steps, i);
        const char *it = c89_emit_step(emit, step);
        WRITE_SOURCEF(emit, "%s\n", it);
    }

    emit_dedent(&emit->emit);
    WRITE_SOURCE(emit, "}\n");

    set_iter_t iter = set_iter(emit->pendingEdges);
    while (set_has_next(&iter))
    {
        const ssa_block_t *next = set_next(&iter);
        c89_emit_block(emit, next);
    }
}

static void c89_emit_local(c89_ssa_emit_t *emit, size_t idx, const ssa_type_t *local)
{
    const char *type = get_type_name(emit, local, format("local%zu", idx));
    WRITE_SOURCEF(emit, "%s;\n", type);
}

static void c89_emit_function(c89_ssa_emit_t *emit, const ssa_flow_t *function)
{
    if (is_extern(function)) { return; }

    map_reset(emit->stepCache);
    set_reset(emit->completeEdges);
    set_reset(emit->pendingEdges);

    emit->currentFlow = function;

    pick_dst(emit, function);

    c89_emit_function_decl(emit, function, false);
    WRITE_SOURCE(emit, " {\n");
    emit_indent(&emit->emit);

    size_t locals = vector_len(function->locals);
    for (size_t i = 0; i < locals; i++)
    {
        const ssa_type_t *local = vector_get(function->locals, i);
        c89_emit_local(emit, i, local);
    }

    c89_emit_block(emit, function->entry);

    emit_dedent(&emit->emit);
    WRITE_SOURCE(emit, "}\n");
}

static void c89_fwd_function(c89_ssa_emit_t *emit, const ssa_flow_t *function)
{
    if (is_entry_point(function->linkage)) { return; }

    pick_dst(emit, function);

    c89_emit_function_decl(emit, function, true);
    WRITE_TEXT(emit, ";\n");
}

static bool should_fwd_type(ssa_kind_t kind)
{
    return kind == eTypeStruct || kind == eTypeUnion;
}

static const char *get_aggregate_name(ssa_kind_t kind)
{
    switch (kind)
    {
    case eTypeStruct: return "struct";
    case eTypeUnion: return "union";
    default: return "";
    }
}

static void c89_fwd_type(c89_ssa_emit_t *emit, const ssa_type_t *type)
{
    if (!should_fwd_type(type->kind)) { return; }

    pick_dst(emit, type);

    WRITE_TEXTF(emit, "%s %s;\n", get_aggregate_name(type->kind), type->name);
}

static void c89_emit_type(c89_ssa_emit_t *emit, const ssa_type_t *type)
{
    if (!should_fwd_type(type->kind)) { return; }

    pick_dst(emit, type);

    WRITE_TEXTF(emit, "%s %s {\n", get_aggregate_name(type->kind), type->name);
    emit_indent(&emit->emit);

    size_t len = vector_len(type->fields);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_param_t *field = vector_get(type->fields, i);
        const char *type = get_type_name(emit, field->type, format("field%zu", i));
        WRITE_TEXTF(emit, "%s;\n", type);
    }

    emit_dedent(&emit->emit);
    WRITE_TEXT(emit, "};\n");
}

static void add_type_deps(vector_t **deps, set_t *fwd, const ssa_type_t *type)
{
    if (set_contains_ptr(fwd, type)) { return; }

    switch (type->kind)
    {
    case eTypeSignature: {
        for (size_t i = 0; i < vector_len(type->args); i++)
        {
            ssa_param_t *param = vector_get(type->args, i);
            add_type_deps(deps, fwd, param->type);
        }
        add_type_deps(deps, fwd, type->result);
        break;
    }
    case eTypeStruct: 
    case eTypeUnion: {
        for (size_t i = 0; i < vector_len(type->fields); i++)
        {
            ssa_param_t *field = vector_get(type->fields, i);
            add_type_deps(deps, fwd, field->type);
        }
        break;
    }
    default: return;
    }

    set_add_ptr(fwd, type);
    vector_push(deps, (void*)type);
}

static vector_t *c89_sort_types(vector_t *types)
{
    size_t inLen = vector_len(types);
    set_t *fwd = set_new(inLen);

    vector_t *result = vector_new(inLen);

    for (size_t i = 0; i < inLen; i++)
    {
        ssa_type_t *type = vector_get(types, i);
        add_type_deps(&result, fwd, type);
    }

    return result;
}

static bool is_public(const ssa_flow_t *flow)
{
    if (is_entry_point(flow->linkage)) { return false; }
    return flow->visibility == eVisiblePublic || flow->linkage == eLinkExported;
}

static void mark_type_public(c89_ssa_emit_t *emit, const ssa_type_t *type)
{
    add_public_symbol(emit, type);
    switch (type->kind)
    {
    case eTypeStruct:
    case eTypeUnion:
        for (size_t i = 0; i < vector_len(type->fields); i++)
        {
            ssa_param_t *field = vector_get(type->fields, i);
            mark_type_public(emit, field->type);
        }
        break;
    case eTypeSignature:
        mark_type_public(emit, type->result);
        for (size_t i = 0; i < vector_len(type->args); i++)
        {
            ssa_param_t *param = vector_get(type->args, i);
            mark_type_public(emit, param->type);
        }
        break;
    default: break;
    }
}

static void mark_public_symbols(c89_ssa_emit_t *emit, section_t symbols)
{
    size_t totalGlobals = vector_len(symbols.globals);
    size_t totalFunctions = vector_len(symbols.functions);

    for (size_t i = 0; i < totalGlobals; i++)
    {
        const ssa_flow_t *global = vector_get(symbols.globals, i);
        if (is_public(global))
        {
            add_public_symbol(emit, global);
            mark_type_public(emit, global->type);
        }
    }

    for (size_t i = 0; i < totalFunctions; i++)
    {
        const ssa_flow_t *function = vector_get(symbols.functions, i);
        if (is_public(function))
        {
            add_public_symbol(emit, function);
            mark_type_public(emit, function->type);
        }
    }
}

void c89_emit_ssa_modules(emit_config_t config, ssa_module_t *module)
{
    CTASSERT(config.reports != NULL);
    CTASSERT(config.source != NULL);
    
    c89_ssa_emit_t emit = {
        .reports = config.reports,
        .emit = {
            .io = config.source,
        },
        .header = {
            .io = config.header,
        },
        .stepCache = map_new(0x1000),
        .completeEdges = set_new(64),
        .pendingEdges = set_new(64),
        .publicSymbols = set_new(0x1000)
    };
    
    section_t symbols = module->symbols;

    size_t totalGlobals = vector_len(symbols.globals);
    size_t totalFunctions = vector_len(symbols.functions);

    vector_t *sortedTypes = c89_sort_types(symbols.types);
    size_t totalTypes = vector_len(sortedTypes);

    mark_public_symbols(&emit, symbols);

    if (has_header(&emit))
    {
        WRITE_SOURCEF(&emit, "#include \"%s\"\n", io_name(config.header));
    }
    else
    {
        WRITE_SOURCE(&emit, "#include <stdbool.h>\n");
        WRITE_SOURCE(&emit, "#include <stddef.h>\n");
    }

    WRITE_HEADER(&emit, "#pragma once\n");
    WRITE_HEADER(&emit, "#include <stdbool.h>\n");
    WRITE_HEADER(&emit, "#include <stddef.h>\n");

    for (size_t i = 0; i < totalTypes; i++)
    {
        c89_fwd_type(&emit, vector_get(sortedTypes, i));
    }

    for (size_t i = 0; i < totalTypes; i++)
    {
        c89_emit_type(&emit, vector_get(sortedTypes, i));
    }

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
