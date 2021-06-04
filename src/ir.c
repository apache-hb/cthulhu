#include "ir.h"

#include "bison.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* IR building */

static unit_t *new_unit(const char *name) {
    unit_t *unit = malloc(sizeof(unit_t));
    unit->size = 8;
    unit->length = 0;
    unit->ops = malloc(sizeof(opcode_t) * unit->size);
    unit->name = name;
    return unit;
}

static size_t unit_add(unit_t *unit, opcode_t op) {
    if (unit->length + 1 > unit->size) {
        unit->size += 8;
        unit->ops = realloc(unit->ops, sizeof(opcode_t) * unit->size);
    }
    unit->ops[unit->length] = op;
    return unit->length++;
}

static size_t build_ir(unit_t *unit, node_t *node);

static opcode_t build_opcode(optype_t type) {
    opcode_t op = { type, false, SIZE_MAX, { } };
    return op;
}

static operand_t imm(int64_t val) {
    operand_t op = { IMM, { .num = val } };
    return op;
}

static operand_t reg(size_t reg) {
    operand_t op = { REG, { .reg = reg } };
    return op;
}

static size_t build_digit(unit_t *unit, char *text) {
    int64_t val = strtoll(text, NULL, 10);
    opcode_t op = build_opcode(OP_DIGIT);
    op.num = val;
    size_t idx = unit_add(unit, op);

    return idx;
}

static optype_t unary_optype(int op) {
    switch (op) {
    case ADD: return OP_ABS;
    case SUB: return OP_NEG;

    default: 
        fprintf(stderr, "unary_optype(%d)\n", op);
        return -1;
    }
}

static size_t build_unary(unit_t *unit, struct unary_t unary) {
    operand_t body = reg(build_ir(unit, unary.expr));
    opcode_t op = build_opcode(unary_optype(unary.op));
    op.expr = body;
    size_t idx = unit_add(unit, op);

    return idx;
}

static optype_t binary_optype(int op) {
    switch (op) {
    case ADD: return OP_ADD;
    case SUB: return OP_SUB;
    case DIV: return OP_DIV;
    case MUL: return OP_MUL;
    case REM: return OP_REM;

    default:
        fprintf(stderr, "binary_optype(%d)\n", op);
        return -1;
    }
}

static size_t build_binary(unit_t *unit, struct binary_t binary) {
    operand_t lhs = reg(build_ir(unit, binary.lhs)), 
              rhs = reg(build_ir(unit, binary.rhs));
    opcode_t op = build_opcode(binary_optype(binary.op));
    op.lhs = lhs;
    op.rhs = rhs;
    size_t idx = unit_add(unit, op);

    return idx;
}

static size_t build_return(unit_t *unit, node_t *expr) {
    operand_t body = reg(build_ir(unit, expr));

    opcode_t op = build_opcode(OP_RETURN);
    op.expr = body;

    return unit_add(unit, op);
}

static size_t build_call(unit_t *unit, struct call_t call) {
    operand_t body = reg(build_ir(unit, call.expr));
    operand_t *args = malloc(sizeof(operand_t) * call.args->len);
    for (size_t i = 0; i < call.args->len; i++) {
        args[i] = reg(build_ir(unit, call.args->data + i));
    }

    opcode_t op = build_opcode(OP_CALL);
    op.body = body;
    op.args = args;
    op.total = call.args->len;

    return unit_add(unit, op);
}

static size_t build_ir(unit_t *unit, node_t *node) {
    switch (node->type) {
    case NODE_DIGIT:
        return build_digit(unit, node->text);
    case NODE_UNARY:
        return build_unary(unit, node->unary);
    case NODE_BINARY:
        return build_binary(unit, node->binary);
    case NODE_RETURN:
        return build_return(unit, node->expr);
    case NODE_CALL:
        return build_call(unit, node->call);

    default:
        fprintf(stderr, "build_ir(%d)\n", node->type);
        return 0;
    }
}

unit_t *ir_gen(node_t *node, const char *name) {
    unit_t *unit = new_unit(name);

    build_ir(unit, node);

    unit->ops[unit->length - 1].keep = true;

    return unit;
}

/* ir optimization utils */

static opcode_t *opcode_at(unit_t *ctx, size_t idx) {
    return ctx->ops + idx;
}

static void emplace_opcode(unit_t *ctx, size_t idx, opcode_t op) {
    opcode_t *old = opcode_at(ctx, idx);
    
    /* propogate keep */
    if (old->keep)
        op.keep = true;

    memcpy(old, &op, sizeof(opcode_t));
}

/* ir constant folding */

static operand_t get_operand(unit_t *ctx, operand_t op) {
    if (op.type == IMM)
        return op;

    opcode_t *inst = opcode_at(ctx, op.reg);

    if (inst->op == OP_DIGIT) 
        return imm(inst->num);

    return op;
}

static opcode_t make_digit(int64_t it) {
    opcode_t op = build_opcode(OP_DIGIT);
    op.num = it;
    return op;
}

static void fold_unary(
    bool *dirty, unit_t *ctx,
    size_t idx, opcode_t *op,
    int64_t(*func)(int64_t)) {

    operand_t val = get_operand(ctx, op->expr);

    if (val.type == IMM) {
        opcode_t it = make_digit(func(val.num));
        emplace_opcode(ctx, idx, it);
        *dirty = true;
    }
}

static int64_t map_abs(int64_t it) {
    return llabs(it);
}

static int64_t map_neg(int64_t it) {
    return it * -1;
}

static void fold_binary(
    bool *dirty, unit_t *ctx,
    size_t idx, opcode_t *op,
    int64_t(*func)(int64_t, int64_t)) {

    operand_t lhs = get_operand(ctx, op->lhs),
              rhs = get_operand(ctx, op->rhs);

    if (lhs.type == IMM && rhs.type == IMM) {
        opcode_t it = make_digit(func(lhs.num, rhs.num));
        emplace_opcode(ctx, idx, it);
        *dirty = true;
    }
}

static int64_t map_add(int64_t lhs, int64_t rhs) {
    return lhs + rhs;
}

static int64_t map_sub(int64_t lhs, int64_t rhs) {
    return lhs - rhs;
}

static int64_t map_div(int64_t lhs, int64_t rhs) {
    return lhs / rhs;
}

static int64_t map_mul(int64_t lhs, int64_t rhs) {
    return lhs * rhs;
}

static int64_t map_rem(int64_t lhs, int64_t rhs) {
    return lhs % rhs;
}


static void fold_return(unit_t *ctx, opcode_t *op) {
    op->expr = get_operand(ctx, op->expr);
}

static void fold_call(unit_t *ctx, opcode_t *op) {
    for (size_t i = 0; i < op->total; i++) {
        op->args[i] = get_operand(ctx, op->args[i]);
    }
    op->body = get_operand(ctx, op->body);
}

static void fold_opcode(bool *dirty, unit_t *ctx, size_t idx) {
    opcode_t *op = opcode_at(ctx, idx);

    switch (op->op) {
    case OP_NEG:
        fold_unary(dirty, ctx, idx, op, map_neg);
        break;
    case OP_ABS:
        fold_unary(dirty, ctx, idx, op, map_abs);
        break;

    case OP_ADD: 
        fold_binary(dirty, ctx, idx, op, map_add);
        break;
    case OP_SUB:         
        fold_binary(dirty, ctx, idx, op, map_sub);
        break;
    case OP_REM:        
        fold_binary(dirty, ctx, idx, op, map_rem);
        break;
    case OP_MUL:         
        fold_binary(dirty, ctx, idx, op, map_mul);
        break;
    case OP_DIV:        
        fold_binary(dirty, ctx, idx, op, map_div);
        break;

    case OP_RETURN:
        fold_return(ctx, op);
        break;

    case OP_CALL:
        fold_call(ctx, op);
        break;

    case OP_DIGIT: case OP_EMPTY:
        break;
    default:
        fprintf(stderr, "fold_opcode(%d)\n", op->op);
    }
}

bool ir_const_fold(unit_t *unit) {
    bool dirty = false;

    for (size_t i = 0; i < unit->length; i++)
        fold_opcode(&dirty, unit, i);

    return dirty;
}


/* ir dead code removal */

static opcode_t empty_op() {
    opcode_t op = { OP_EMPTY };
    return op;
}

static bool refs_operand(operand_t op, size_t reg) {
    if (op.type == REG)
        return op.reg == reg;

    return false;
}

/* check if ir step `op` depends on step `idx` */
static bool refs_opcode(opcode_t *op, size_t idx) {
    size_t i = 0;

    switch (op->op) {
    /* digits and empty cant reference anything */
    case OP_DIGIT: case OP_EMPTY:
        return false;

    case OP_ABS: case OP_NEG:
        return refs_operand(op->expr, idx);

    case OP_ADD: case OP_SUB: case OP_DIV:
    case OP_REM: case OP_MUL:
        return refs_operand(op->lhs, idx) || refs_operand(op->rhs, idx);

    case OP_RETURN:
        return refs_operand(op->expr, idx);

    case OP_CALL:
        for (; i < op->total; i++)
            if (refs_operand(op->args[i], idx))
                return true;
        return refs_operand(op->body, idx);

    default:
        fprintf(stderr, "refs_opcode(%d)\n", op->op);
        return true;
    }
}

static void reduce_opcode(bool *dirty, unit_t *ctx, size_t idx) {
    opcode_t *op = opcode_at(ctx, idx);

    /* if this op is empty we can skip it */
    /* also dont reduce opcodes marked with keep */
    if (op->op == OP_EMPTY || op->keep)
        return;

    /* these have side effects so dont remove them */
    if (op->op == OP_RETURN || op->op == OP_CALL)
        return;

    /* see if anything references this step */
    for (size_t i = idx; i < ctx->length; i++) {
        if (refs_opcode(opcode_at(ctx, i), idx)) {
            return;
        }
    }

    *dirty = true;
    emplace_opcode(ctx, idx, empty_op());
}

bool ir_reduce(unit_t *unit) {
    bool dirty = false;

    for (size_t i = 0; i < unit->length; i++) 
        reduce_opcode(&dirty, unit, i);

    return dirty;
}

units_t symbol_table(void) {
    units_t units = { malloc(sizeof(unit_t*) * 4), 0, 4 };
    return units;
}

void add_symbol(units_t *units, unit_t *unit) {
    if (units->len + 1 > units->size) {
        units->size += 4;
        units->units = realloc(units->units, sizeof(unit_t*) * units->size);
    }

    units->units[units->len++] = unit;
}

static size_t used_opcodes(unit_t *unit) {
    size_t count = 0;

    for (size_t i = 0; i < unit->length; i++)
        if (opcode_at(unit, i)->op != OP_EMPTY)
            count += 1;

    return count;
}

static void shift_operand(size_t start, size_t by, operand_t *operand) {
    if (operand->type == REG && operand->reg >= start) {
        operand->reg += by;
    }
}

static void shift_opcode(unit_t *unit, size_t idx, size_t start, size_t by) {
    opcode_t *op = opcode_at(unit, idx);

    size_t i = 0;

    switch (op->op) {
    case OP_EMPTY: case OP_DIGIT:
        break;

    case OP_ABS: case OP_NEG: case OP_RETURN:
        shift_operand(start, by, &op->expr);
        break;

    case OP_ADD: case OP_SUB: case OP_MUL:
    case OP_DIV: case OP_REM:
        shift_operand(start, by, &op->lhs);
        shift_operand(start, by, &op->rhs);
        break;

    case OP_CALL:
        for (; i < op->total; i++)
            shift_operand(start, by, op->args + i);
        shift_operand(start, by, &op->body);
        break;

    default:
        fprintf(stderr, "shift_opcode(%d)\n", op->op);
        break;
    }
}

static void shift_opcodes(unit_t *unit, size_t start, size_t len) {
    for (size_t i = unit->length; i--;) {
        shift_opcode(unit, i, start, len);
    }
}

static size_t inline_func(unit_t *unit, unit_t *other, size_t idx) {
    size_t size = used_opcodes(other);

    opcode_t *call = opcode_at(unit, idx);

    /* dont inline stuff thats larger than the call overhead */
    if (size < call->total) {
        return 0;
    }

    shift_opcodes(unit, idx, size);
    return size;
}

static unit_t *find_symbol(operand_t func, units_t *ctx) {
    if (func.type == SYM) {
        for (size_t i = 0; i < ctx->len; i++) {
            if (strcmp(func.name, ctx->units[i]->name) == 0) {
                return ctx->units[i];
            }
        }
    }

    return NULL;
}

bool ir_inline(unit_t *unit, units_t *world) {
    size_t len = unit->length;
    size_t i = 0;
    bool dirty = false;

    while (i < len) {
        opcode_t *op = opcode_at(unit, i++);

        if (op->op == OP_CALL) {
            size_t limit = inline_func(unit, find_symbol(op->body, world), i);
        
            if (limit) {
                dirty = true;
                len += limit;
            }
        }

        len -= 1;
    }

    return dirty;
}
