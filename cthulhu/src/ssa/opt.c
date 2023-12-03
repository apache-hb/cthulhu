#include "common/common.h"

#include "std/set.h"
#include "std/map.h"
#include "std/vector.h"

#include "std/typed/vector.h"

#include "report/report.h"

#include "core/macros.h"
#include "base/panic.h"

typedef struct ssa_vm_t {
    // externally provided values
    reports_t *reports;
    map_t *deps;

    // all globals
    set_t *globals;
} ssa_vm_t;

typedef struct ssa_scope_t {
    ssa_vm_t *vm;

    typevec_t *locals;

    const ssa_symbol_t *symbol;
    const ssa_value_t *returnValue;
    map_t *stepValues;
} ssa_scope_t;

static void add_global(ssa_vm_t *vm, ssa_symbol_t *global)
{
    CTASSERTF(!set_contains_ptr(vm->globals, global), "global %s already exists", global->name);
    set_add_ptr(vm->globals, global);
}

static void add_globals(ssa_vm_t *vm, const ssa_module_t *module)
{
    size_t len = vector_len(module->globals);
    for (size_t i = 0; i < len; i++)
    {
        ssa_symbol_t *global = vector_get(module->globals, i);
        add_global(vm, global);
    }
}

static void ssa_opt_global(ssa_vm_t *vm, ssa_symbol_t *global);

static const ssa_step_t *get_step_indexed(const ssa_block_t *block, size_t index)
{
    CTASSERT(block != NULL);
    return typevec_offset(block->steps, index);
}

static const ssa_value_t *ssa_opt_operand(ssa_scope_t *vm, ssa_operand_t operand)
{
    switch (operand.kind)
    {
    case eOperandEmpty: return NULL;
    case eOperandImm: return operand.value;
    case eOperandReg: return map_get_ptr(vm->stepValues, get_step_indexed(operand.vregContext, operand.vregIndex));
    case eOperandConst: return vector_get(vm->symbol->consts, operand.constant);
    case eOperandGlobal: {
        const ssa_symbol_t *global = operand.global;
        ssa_opt_global(vm->vm, (void*)global); // TODO: find a nice way to follow const
        return global->value;
    }

    default: NEVER("unhandled operand kind %d (inside %s)", operand.kind, vm->symbol->name);
    }
}

static const ssa_value_t *ssa_opt_load(ssa_scope_t *vm, ssa_load_t step)
{
    ssa_operand_t src = step.src;
    switch (src.kind)
    {
    case eOperandGlobal: {
        const ssa_symbol_t *global = src.global;
        ssa_opt_global(vm->vm, (void*)global); // TODO: find a nice way to follow const
        return global->value;
    }
    case eOperandLocal:
        NEVER("unhandled local %zu load (inside %s)", src.local, vm->symbol->name);

    default: NEVER("unhandled load kind %d (inside %s)", src.kind, vm->symbol->name);
    }
}

static const ssa_value_t *ssa_opt_return(ssa_scope_t *vm, ssa_return_t step)
{
    const ssa_value_t *value = ssa_opt_operand(vm, step.value);
    vm->returnValue = value;
    return value;
}

static bool value_is(const ssa_value_t *value, ssa_kind_t type)
{
    CTASSERT(value != NULL);
    CTASSERT(value->type != NULL);

    return value->type->kind == type;
}

static bool check_init(ssa_scope_t *vm, const ssa_value_t *value)
{
    CTASSERT(value != NULL);

    if (!value->init)
    {
        report(vm->vm->reports, eFatal, node_invalid(), "use of uninitialized value inside `%s`", vm->symbol->name);
        return false;
    }

    return true;
}

static const ssa_value_t *ssa_opt_unary(ssa_scope_t *vm, ssa_unary_t step)
{
    unary_t unary = step.unary;
    const ssa_value_t *operand = ssa_opt_operand(vm, step.operand);

    if (!check_init(vm, operand)) { return operand; }

    mpz_t result;
    mpz_init(result);

    switch (unary)
    {
    case eUnaryNeg:
        CTASSERTF(value_is(operand, eTypeDigit), "operand of unary %s is not a digit (inside %s)", unary_name(unary), vm->symbol->name);
        mpz_neg(result, operand->digitValue);
        break;

    case eUnaryAbs:
        CTASSERTF(value_is(operand, eTypeDigit), "operand of unary %s is not a digit (inside %s)", unary_name(unary), vm->symbol->name);
        mpz_abs(result, operand->digitValue);
        break;

    case eUnaryFlip:
        CTASSERTF(value_is(operand, eTypeDigit), "operand of unary %s is not a digit (inside %s)", unary_name(unary), vm->symbol->name);
        mpz_com(result, operand->digitValue);
        break;

    case eUnaryNot:
        CTASSERTF(value_is(operand, eTypeBool), "operand of unary %s is not a bool (inside %s)", unary_name(unary), vm->symbol->name);
        return ssa_value_bool(operand->type, !operand->boolValue);

    default: NEVER("unhandled unary %s (inside %s)", unary_name(unary), vm->symbol->name);
    }

    return ssa_value_digit(operand->type, result);
}

static const ssa_value_t *ssa_opt_binary(ssa_scope_t *vm, ssa_binary_t step)
{
    binary_t binary = step.binary;
    const ssa_value_t *lhs = ssa_opt_operand(vm, step.lhs);
    const ssa_value_t *rhs = ssa_opt_operand(vm, step.rhs);

    CTASSERTF(value_is(lhs, eTypeDigit), "lhs of binary %s is not a digit (inside %s)", binary_name(binary), vm->symbol->name);
    CTASSERTF(value_is(rhs, eTypeDigit), "rhs of binary %s is not a digit (inside %s)", binary_name(binary), vm->symbol->name);

    if (!check_init(vm, lhs)) { return lhs; }
    if (!check_init(vm, rhs)) { return rhs; }

    mpz_t result;
    mpz_init(result);

    switch (binary)
    {
    case eBinaryAdd:
        mpz_add(result, lhs->digitValue, rhs->digitValue);
        break;
    case eBinarySub:
        mpz_sub(result, lhs->digitValue, rhs->digitValue);
        break;
    case eBinaryMul:
        mpz_mul(result, lhs->digitValue, rhs->digitValue);
        break;
    case eBinaryDiv:
        mpz_divexact(result, lhs->digitValue, rhs->digitValue);
        break;
    case eBinaryRem:
        mpz_mod(result, lhs->digitValue, rhs->digitValue);
        break;

    /* TODO: do these produce correct values? */
    case eBinaryShl:
        mpz_mul_2exp(result, lhs->digitValue, mpz_get_ui(rhs->digitValue));
        break;
    case eBinaryShr:
        mpz_fdiv_q_2exp(result, lhs->digitValue, mpz_get_ui(rhs->digitValue));
        break;
    case eBinaryXor:
        mpz_xor(result, lhs->digitValue, rhs->digitValue);
        break;

    default: NEVER("unhandled binary %s (inside %s)", binary_name(binary), vm->symbol->name);
    }

    // TODO: make sure this is actually the correct type
    return ssa_value_digit(lhs->type, result);
}

static const ssa_value_t *cast_to_opaque(const ssa_type_t *type, const ssa_value_t *value)
{
    const ssa_type_t *src = type;
    switch (src->kind)
    {
    case eTypeOpaque: return value;

    case eTypeDigit:
        return ssa_value_pointer(type, (void*)(uintptr_t)mpz_get_ui(value->digitValue));

    default: NEVER("unhandled type %s", ssa_type_name(src->kind));
    }
}

static const ssa_value_t *ssa_opt_cast(ssa_scope_t *vm, ssa_cast_t cast)
{
    const ssa_value_t *value = ssa_opt_operand(vm, cast.operand);
    const ssa_type_t *type = cast.type;
    switch (type->kind)
    {
    case eTypeOpaque:
        return cast_to_opaque(type, value);

    default:
        NEVER("unhandled type %s", ssa_type_name(type->kind));
    }
}

static const ssa_value_t *ssa_opt_step(ssa_scope_t *vm, const ssa_step_t *step)
{
    switch (step->opcode)
    {
    case eOpLoad: return ssa_opt_load(vm, step->load);
    case eOpReturn: return ssa_opt_return(vm, step->ret);

    case eOpUnary: return ssa_opt_unary(vm, step->unary);
    case eOpBinary: return ssa_opt_binary(vm, step->binary);
    case eOpCast: return ssa_opt_cast(vm, step->cast);

    default: NEVER("unhandled opcode %d (inside %s)", step->opcode, vm->symbol->name);
    }
}

static void ssa_opt_block(ssa_scope_t *vm, const ssa_block_t *block)
{
    size_t len = typevec_len(block->steps);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_step_t *step = typevec_offset(block->steps, i);
        const ssa_value_t *value = ssa_opt_step(vm, step);
        map_set_ptr(vm->stepValues, step, (void*)value);

        if (vm->returnValue != NULL) { return; }
    }
}

static void ssa_opt_global(ssa_vm_t *vm, ssa_symbol_t *global)
{
    CTASSERTF(set_contains_ptr(vm->globals, global), "global %s does not exist", global->name);

    if (global->value != NULL)
    {
        return;
    }

    ssa_scope_t scope = {
        .vm = vm,
        .symbol = global,
        .returnValue = NULL,
        .stepValues = map_new(64)
    };

    ssa_opt_block(&scope, global->entry);
    CTASSERTF(scope.returnValue != NULL, "global %s failed to evaluate", global->name);

    global->value = scope.returnValue;
}

void ssa_opt(reports_t *reports, ssa_result_t result)
{
    ssa_vm_t vm = {
        .reports = reports,
        .deps = result.deps,

        .globals = set_new(64),
    };

    size_t len = vector_len(result.modules);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_module_t *mod = vector_get(result.modules, i);
        add_globals(&vm, mod);
    }

    set_iter_t iter = set_iter(vm.globals);
    while (set_has_next(&iter))
    {
        ssa_symbol_t *global = (ssa_symbol_t*)set_next(&iter);
        ssa_opt_global(&vm, global);
    }
}
