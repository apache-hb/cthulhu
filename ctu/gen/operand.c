#include "operand.h"

oplist_t *oplist_new(size_t size) {
    operand_t *ops = ctu_malloc(sizeof(operand_t) * size);
    oplist_t *oplist = ctu_malloc(sizeof(oplist_t));
   
    oplist->ops = ops;
    oplist->used = 0;
    oplist->size = size;

    return oplist;
}

oplist_t *oplist_of(size_t size) {
    oplist_t *oplist = oplist_new(size);
    oplist->used = size;
    return oplist;
}

void oplist_push(oplist_t *list, operand_t op) {
    if (list->size >= list->used + 1) {
        list->size *= 2;
        list->ops = ctu_realloc(list->ops, sizeof(operand_t) * list->size);
    }
    list->ops[list->used++] = op;
}

void oplist_set(oplist_t *list, size_t index, operand_t op) {
    list->ops[index] = op;
}

operand_t oplist_get(oplist_t *list, size_t index) {
    return list->ops[index];
}

size_t oplist_len(oplist_t *list) {
    return list->used;
}

static operand_t operand_new(optype_t kind) {
    operand_t operand;
    operand.kind = kind;
    return operand;
}

operand_t operand_imm(reports_t *reports, value_t *imm) {
    if (is_literal(imm->type)) {
        ctu_assert(reports, "immediate cannot be untyped");
    }
    
    if (is_any(imm->type)) {
        ctu_assert(reports, "immediate must be typed");
    }

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
    printf("address: %p\n", block);
    operand_t operand = operand_new(ADDRESS);
    operand.block = block;
    return operand;
}

operand_t operand_empty(void) {
    operand_t operand = operand_new(EMPTY);
    return operand;
}
