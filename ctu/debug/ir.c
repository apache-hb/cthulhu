#include "ir.h"
#include "common.h"

static const char *get_flow_name(module_t *mod, size_t idx) {
    return mod->flows[idx].name;
}

static void debug_operand(module_t *mod, operand_t op) {
    switch (op.kind) {
    case VREG: printf("%%%zu", op.vreg); break;
    case ARG: printf("arg[%zu]", op.arg); break;
    case IMM: printf("$%lu", op.imm); break;
    case NAME: printf("@%s", op.name); break;
    case NONE: printf("none"); break;
    case BLOCK: printf(".%zu", op.label); break;
    case FUNC: printf("%zu:%s", op.func, get_flow_name(mod, op.func)); break;
    }
}

static void debug_index(size_t idx) {
    printf("  %%%zu = ", idx);
}

static void debug_args(module_t *mod, step_t step) {
    printf("(");
    for (size_t i = 0; i < step.len; i++) {
        if (i != 0) {
            printf(", ");
        }
        debug_operand(mod, step.args[i]);
    }
    printf(")");
}

static const char *integer_name(type_t *type) {
    bool sign = is_signed(type);
    switch (type->integer) {
    case INTEGER_CHAR: return sign ? "char" : "uchar";
    case INTEGER_SHORT: return sign ? "short" : "ushort";
    case INTEGER_INT: return sign ? "int" : "uint";
    case INTEGER_LONG: return sign ? "long" : "ulong";
    case INTEGER_SIZE: return sign ? "isize" : "usize";
    case INTEGER_INTPTR: return sign ? "intptr" : "uintptr";
    case INTEGER_INTMAX: return sign ? "intmax" : "uintmax";
    default: return "unknown";
    }
}

static void debug_type(type_t *type);

static void debug_callable(type_t *type) {
    types_t *args = type->args;
    printf("(");
    for (size_t i = 0; i < typelist_len(args); i++) {
        if (i != 0) {
            printf(", ");
        }
        debug_type(typelist_at(args, i));
    }
    printf(") -> ");
    debug_type(type->result);
}

static void debug_type(type_t *type) {
    switch (type->kind) {
    case TYPE_INTEGER: printf("%s", integer_name(type)); break;
    case TYPE_BOOLEAN: printf("bool"); break;
    case TYPE_VOID: printf("void"); break;
    case TYPE_CALLABLE: debug_callable(type); break;
    case TYPE_POISON: printf("poison(%s)", type->text); break;
    case TYPE_UNRESOLVED: printf("unresolved"); break;
    }
}

static void debug_step(module_t *mod, size_t idx, step_t step) {
    switch (step.opcode) {
    case OP_RETURN:
        printf("  ret ");
        debug_type(step.type);
        printf(" ");
        debug_operand(mod, step.value);
        break;
    case OP_EMPTY:
        return;
    case OP_BINARY:
        debug_index(idx); 
        debug_type(step.type);
        printf(" %s ", binary_name(step.binary));
        debug_operand(mod, step.lhs);
        printf(" ");
        debug_operand(mod, step.rhs);
        break;
    case OP_UNARY:
        debug_index(idx); 
        debug_type(step.type);
        printf(" %s ", unary_name(step.unary));
        debug_operand(mod, step.expr);
        break;
    case OP_VALUE:
        debug_index(idx); 
        debug_type(step.type);
        printf(" ");
        debug_operand(mod, step.value);
        break;
    case OP_BLOCK:
        printf(".%zu:", idx);
        break;
    case OP_BRANCH:
        printf("  branch ");
        debug_operand(mod, step.cond);
        printf(" ");
        debug_operand(mod, step.block);
        printf(" else ");
        debug_operand(mod, step.other);
        break;
    case OP_CALL:
        debug_index(idx);
        printf("call ");
        debug_type(step.type);
        printf(" ");
        debug_operand(mod, step.value);
        debug_args(mod, step);
        break;
    }

    printf("\n");
}

static void debug_params(flow_t *flow) {
    printf("(");

    for (size_t i = 0; i < flow->nargs; i++) {
        arg_t arg = flow->args[i];
        if (i != 0) {
            printf(", ");
        }
        printf("%zu = [%s: ", i, arg.name);
        debug_type(arg.type);
        printf("]");
    }

    printf(")");
}

static void debug_flow(module_t *mod, flow_t flow) {
    printf("define %s", flow.name);
    debug_params(&flow);
    printf(" {\n");

    for (size_t i = 0; i < flow.len; i++) {
        debug_step(mod, i, flow.steps[i]);
    }
    printf("}\n");
}

void debug_module(module_t mod) {
    for (size_t i = 0; i < mod.len; i++) {
        debug_flow(&mod, mod.flows[i]);
    }
}