#include "operand.h"

static operand_t operand_new(optype_t kind) {
    operand_t operand;
    operand.kind = kind;
    return operand;
}

operand_t operand_imm(value_t *imm) {
    operand_t operand = operand_new(IMM);
    operand.imm = imm;
    return operand;
}

operand_t operand_vreg(vreg_t vreg) {
    operand_t operand = operand_new(VREG);
    operand.vreg = vreg;
    return operand;
}

operand_t operand_arg(vreg_t arg) {
    operand_t operand = operand_new(ARG);
    operand.arg = arg;
    return operand;
}

operand_t operand_label(label_t label) {
    operand_t operand = operand_new(LABEL);
    operand.label = label;
    return operand;
}

operand_t operand_address(struct block_t *block) {
    operand_t operand = operand_new(ADDRESS);
    operand.block = block;
    return operand;
}

operand_t operand_empty(void) {
    operand_t operand = operand_new(EMPTY);
    return operand;
}
