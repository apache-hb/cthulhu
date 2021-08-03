#include "ir.h"

#include "ctu/util/report.h"

static bool op_used(operand_t op, size_t reg) {
    return op.kind == VREG && op.vreg == reg;
}

static bool any_arg_uses(operand_t *args, size_t num, size_t vreg) {
    for (size_t i = 0; i < num; i++) {
        if (op_used(args[i], vreg)) {
            return true;
        }
    }

    return false;
}

bool is_vreg_used(const step_t *step, vreg_t vreg) {
    switch (step->opcode) {
    case OP_VALUE: 
        return op_used(step->value, vreg);

    case OP_UNARY:
        return op_used(step->expr, vreg);

    case OP_BINARY:
        return op_used(step->lhs, vreg) 
            || op_used(step->rhs, vreg);

    case OP_CONVERT:
        return op_used(step->value, vreg);

    case OP_RETURN:
        return op_used(step->value, vreg);

    case OP_STORE:
        return op_used(step->src, vreg) 
            || op_used(step->dst, vreg);

    case OP_LOAD:
        return op_used(step->src, vreg);

    case OP_BRANCH:
        return op_used(step->cond, vreg)
            || op_used(step->block, vreg)
            || op_used(step->other, vreg);

    case OP_CALL:
        return op_used(step->value, vreg)
            || any_arg_uses(step->args, step->len, vreg);

    case OP_JUMP:
        return op_used(step->block, vreg);

    case OP_OFFSET:
        return op_used(step->src, vreg)
            || op_used(step->index, vreg);

    case OP_RESERVE:
    case OP_EMPTY:
    case OP_BLOCK:
        return false;

    default:
        assert("unknown IR opcode %d", step->opcode);
        return true;
    }
}
