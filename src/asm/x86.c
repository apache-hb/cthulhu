#include "x86.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>

static void add_range(live_graph_t *graph, size_t first, size_t last, opcode_t *op) {
    live_range_t range = { first, last, REG_NONE };
    
    if (graph->len + 1 >= graph->size) {
        graph->size += 4;
        graph->ranges = realloc(graph->ranges, sizeof(live_range_t) * graph->size);
    }

    graph->ranges[graph->len] = range;
    op->range = graph->len;
    graph->len++;
}

/* check if an operand references an instruction by index */
static bool op_refs(operand_t op, size_t idx) {
    return op.type == REG ? op.reg == idx : false;
}

static bool refs_val(unit_t *ctx, size_t idx, size_t inst) {
    opcode_t op = ctx->ops[idx];

    switch (op.op) {
    case OP_ABS: case OP_NEG:
        return op_refs(op.expr, inst);
        
    case OP_ADD: case OP_SUB: case OP_DIV:
    case OP_MUL: case OP_REM:
        return op_refs(op.lhs, inst) || op_refs(op.rhs, inst);

    case OP_RETURN:
        return op_refs(op.expr, inst);

    case OP_EMPTY: case OP_DIGIT:
        return false;

    default:
        fprintf(stderr, "refs_val(%d)\n", op.op);
        return false;
    }
}

static void track_range(live_graph_t *graph, unit_t *ctx, size_t idx) {
    size_t seen = idx;

    for (size_t i = idx; i < ctx->length; i++) {
        if (refs_val(ctx, i, idx)) {
            seen = i;
        }
    }

    add_range(graph, idx, seen, ctx->ops + idx);
}

live_graph_t build_graph(const char *name, unit_t *ir) {
    live_graph_t graph = { name, malloc(sizeof(live_range_t) * 4), 4, 0 };

    for (size_t i = 0; i < ir->length; i++) 
        track_range(&graph, ir, i);

    return graph;
}

typedef struct {
    /* if a register is used or not */
    bool used[REG_NONE];

    /* associate an opcode with a register */
    live_graph_t *graph;
} regalloc_t;

static live_range_t *range_at(regalloc_t *alloc, size_t idx) {
    if (idx > alloc->graph->len) {
        fprintf(stderr, "range_at(%zu > %zu)\n", idx, alloc->graph->len);
        return NULL;
    }
    return alloc->graph->ranges + idx;
}

static live_range_t *range_for_op_nullable(regalloc_t *alloc, size_t op) {
    for (size_t i = 0; i < alloc->graph->len; i++) {
        live_range_t *range = range_at(alloc, i);
        if (range->first == op) {
            return range;
        }
    }

    return NULL;
}   

static live_range_t *range_for_op(regalloc_t *alloc, size_t op) {
    live_range_t *range = range_for_op_nullable(alloc, op);
    if (range) {
        return range;
    }

    fprintf(stderr, "range_for_op(%zu) = NULL\n", op);
    return NULL;
}

static live_range_t *range_for_opcode(regalloc_t *alloc, opcode_t op) {
    if (op.range == SIZE_MAX) {
        fprintf(stderr, "range_for_opcode(SIZE_MAX)\n");
    }
    return range_at(alloc, op.range);
}

static bool is_range_live(live_range_t *it, size_t at) {
    return it->first <= at && at < it->last;
}

static void mark_used(regalloc_t *alloc, size_t idx, bool used) {
    if (idx > REG_NONE) {
        fprintf(stderr, "mark_used(%zu > %d)\n", idx, REG_NONE);
    }
    alloc->used[idx] = used;
}

static void free_regs(regalloc_t *alloc, size_t idx) {
    for (size_t i = 0; i < REG_NONE; i++) {
        bool used = false;
        /* check each live range */
        for (size_t j = 0; j < alloc->graph->len; j++) {
            live_range_t *range = range_at(alloc, j);

            /* if any range is alive at this point and is using this register */
            if (is_range_live(range, idx) && range->assoc == i) {
                /* then mark it as used */
                used = true;
                break;
            }
        }

        /* if no live ranges as using this register at this point */
        if (!used) {
            /* then mark the register as free */
            mark_used(alloc, i, false);
        }
    }
}

/* try and link a register to a range, or fail */
static void link_reg_to_range(regalloc_t *alloc, size_t idx) {
    free_regs(alloc, idx);

    /* now that registers have been sweeped */
    bool found = false;

    for (size_t i = 0; i < REG_NONE; i++) {
        /* try and find a register */
        live_range_t *range = range_for_op_nullable(alloc, idx);
        if (!range) {
            return;
        }

        if (!alloc->used[i]) {
            mark_used(alloc, i, true);
            range->assoc = i;
            found = true;
            break;
        }
    }

    if (!found) {
        fprintf(stderr, "out of registers\n");
    }
}

static const char *reg_name(reg_t reg) {
    switch (reg) {
    case RAX: return "rax";
    case RBX: return "rbx";
    case RCX: return "rcx";
    case RDX: return "rdx";
    
    default:
        fprintf(stderr, "reg_name(%d)\n", reg);
        return NULL;
    }
}

static void emit_digit(int64_t num, reg_t reg) {
    printf("  mov %s, %lu\n", reg_name(reg), num);
}

static void emit_operand(regalloc_t *alloc, operand_t op) {
    if (op.type == IMM) { 
        printf("%lu", op.num);
    } else {
        printf("%s", reg_name(range_for_op(alloc, op.reg)->assoc));
    }
}

static bool operand_is_reg(regalloc_t *alloc, operand_t op, reg_t reg) {
    return op.type == REG
        ? range_for_op(alloc, op.reg)->assoc == reg
        : false;
}

static void emit_add(regalloc_t *alloc, opcode_t op) {
    reg_t reg = range_for_opcode(alloc, op)->assoc;
    const char *dst = reg_name(reg);
    
    if (!operand_is_reg(alloc, op.lhs, reg)) {
        printf("  add %s, ", dst);
        emit_operand(alloc, op.lhs);
        printf("\n");
    }

    printf("  add %s, ", dst);
    emit_operand(alloc, op.rhs);
    printf("\n");
}

static void emit_ret(regalloc_t *alloc, opcode_t op) {
    operand_t ret = op.expr;

    if (ret.type == IMM) {
        printf("  mov rax, %lu\n", ret.num);
    } else {
        reg_t reg = range_for_opcode(alloc, op)->assoc;

        if (reg != RAX) {
            printf("  mov rax, %s\n", reg_name(reg));
        }
    }

    printf("  pop rbp\n");
    printf("  ret\n");
}

static void emit_inst(regalloc_t *alloc, opcode_t op) {
    switch (op.op) {
    case OP_EMPTY:
        break;

    case OP_DIGIT:
        emit_digit(op.num, range_for_opcode(alloc, op)->assoc);
        break;

    case OP_ADD:
        emit_add(alloc, op);
        break;

    case OP_RETURN:
        emit_ret(alloc, op);
        break;

    default:
        fprintf(stderr, "emit_inst(%d)\n", op.op);
        break;
    }
}

void emit_asm(unit_t *ir) {
    live_graph_t graph = build_graph("main", ir);
    
    for (size_t i = 0; i < graph.len; i++) {
        if (i) {
            printf("\n");
        }
        live_range_t range = graph.ranges[i];
        printf("%zu -> %zu", range.first, range.last);
    }
    printf("\n");

    regalloc_t alloc = { {}, &graph };

    /* link registers to live ranges */
    for (size_t i = 0; i < ir->length; i++) {
        link_reg_to_range(&alloc, i);
    }

    for (size_t i = 0; i < graph.len; i++) {
        live_range_t range = graph.ranges[i];
        printf("%s : %zu -> %zu\n", reg_name(range.assoc), range.first, range.last);
    }

    printf("%s:\n", graph.name);
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    for (size_t i = 0; i < ir->length; i++) {
        emit_inst(&alloc, ir->ops[i]);
    }
}
