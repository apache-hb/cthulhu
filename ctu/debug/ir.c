#include "ir.h"
#include "common.h"

static void debug_operand(operand_t op) {
    switch (op.kind) {
    case VREG: printf("%%%zu", op.vreg); break;
    case IMM: printf("$%lu", op.imm); break;
    case NAME: printf("@%s", op.name); break;
    case NONE: printf("void"); break;
    }
}

static void debug_index(size_t idx) {
    printf("  %%%zu = ", idx);
}

static void debug_step(size_t idx, step_t step) {
    switch (step.opcode) {
    case OP_RETURN:
        printf("  ret ");
        debug_operand(step.value);
        break;
    case OP_EMPTY:
        break;
    case OP_BINARY:
        debug_index(idx); 
        printf("%s ", binary_name(step.binary));
        debug_operand(step.lhs);
        printf(" ");
        debug_operand(step.rhs);
        break;
    case OP_UNARY:
        debug_index(idx); 
        printf("%s ", unary_name(step.unary));
        debug_operand(step.expr);
        break;
    case OP_VALUE:
        debug_index(idx); 
        debug_operand(step.value);
        break;
    case OP_BLOCK:
        printf("@%zu:", idx);
        break;
    case OP_JUMP:
        printf("  jmp");
        break;
    }

    printf("\n");
}

static void debug_flow(flow_t flow) {
    for (size_t i = 0; i < flow.len; i++) {
        debug_step(i, flow.steps[i]);
    }
}

void debug_module(module_t mod) {
    for (size_t i = 0; i < mod.len; i++) {
        debug_flow(mod.flows[i]);
    }
}