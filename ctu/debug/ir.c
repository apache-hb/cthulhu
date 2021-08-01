#include "ir.h"
#include "type.h"
#include "common.h"

static const char *get_flow_name(module_t *mod, size_t idx) {
    return mod->flows[idx].name;
}

static const char *get_var_name(module_t *mod, size_t idx) {
    return mod->vars[idx].name;
}

static void debug_imm(imm_t imm) {
    switch (imm.kind) {
    case IMM_INT: printf("int(%s)", mpz_get_str(NULL, 0, imm.num)); break;
    case IMM_BOOL: printf("bool(%s)", imm.b ? "true" : "false"); break;
    case IMM_SIZE: printf("size(%s)", mpz_get_str(NULL, 0, imm.num));
    }
}

static void debug_operand(module_t *mod, operand_t op) {
    switch (op.kind) {
    case VREG: printf("%%%zu", op.vreg); break;
    case ARG: printf("arg[%zu]", op.arg); break;
    case IMM: debug_imm(op.imm); break;
    case NONE: printf("none"); break;
    case BLOCK: printf(".%zu", op.label); break;
    case FUNC: printf("def(%zu:%s)", op.func, get_flow_name(mod, op.func)); break;
    case VAR: printf("var(%zu:%s)", op.var, get_var_name(mod, op.var)); break;
    case STRING: printf("string[%zu]", op.var); break;
    }

    if (op.offset != SIZE_MAX) {
        printf(" +%zu", op.offset);
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

static void debug_step(module_t *mod, size_t idx, step_t step) {
    switch (step.opcode) {
    case OP_EMPTY:
        return;
    case OP_RETURN:
        printf("  ret ");
        debug_type(step.type);
        printf(" ");
        debug_operand(mod, step.value);
        break;
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
    case OP_CONVERT:
        debug_index(idx);
        printf("cast ");
        debug_type(step.type);
        printf(" ");
        debug_operand(mod, step.value);
        break;
    case OP_RESERVE:
        debug_index(idx);
        printf("reserve ");
        debug_type(step.type);
        printf(" [%zu]", step.size);
        break;
    case OP_LOAD:
        debug_index(idx);
        printf("load ");
        debug_type(step.type);
        printf(" ");
        debug_operand(mod, step.src);
        break;
    case OP_STORE:
        printf("  store ");
        debug_type(step.type);
        printf(" ");
        debug_operand(mod, step.dst);
        printf(" ");
        debug_operand(mod, step.src);
        break;
    case OP_JUMP:
        printf("  jmp ");
        debug_operand(mod, step.block);
        break;
    case OP_OFFSET:
        debug_index(idx);
        printf("offset ");
        debug_type(step.type);
        printf(" ");
        debug_operand(mod, step.src);
        printf(" ");
        debug_operand(mod, step.index);
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

void debug_flow(flow_t flow) {
    printf("define %s", flow.name);
    debug_params(&flow);
    printf(": ");
    debug_type(flow.result);
    printf(" {\n");

    for (size_t i = 0; i < flow.len; i++) {
        debug_step(flow.mod, i, flow.steps[i]);
    }
    printf("}\n");
}

static void debug_global(size_t i, var_t var) {
    printf("global %zu = %s: ", i, var.name);
    debug_type(var.type);
}

static void debug_string(size_t i, char *str) {
    printf("string %zu = \"%s\"\n", i, str);
}

void debug_module(module_t mod) {
    printf("; types\n");
    for (size_t i = 0; i < num_types(&mod); i++) {
        if (i != 0) {
            printf("\n");
        }

        printf("type %zu = ", i);
        debug_type_verbose(mod.types[i]);
    }

    printf("\n; strings\n");
    for (size_t i = 0; i < num_strings(&mod); i++) {
        if (i != 0) {
            printf("\n");
        }

        debug_string(i, mod.strings[i]);
    }

    printf("\n; globals\n");
    for (size_t i = 0; i < num_vars(&mod); i++) {
        if (i != 0) {
            printf("\n");
        }

        debug_global(i, mod.vars[i]);
    }
    
    printf("\n; functions\n");
    for (size_t i = 0; i < num_flows(&mod); i++) {

        if (i != 0) {
            printf("\n");
        }
        debug_flow(mod.flows[i]);
    }
}