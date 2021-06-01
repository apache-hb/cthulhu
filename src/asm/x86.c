#include "x86.h"

#include <stdio.h>
#include <stdlib.h>

static void add_range(live_graph_t *graph, size_t first, size_t last) {
    live_range_t range = { first, last };
    
    if (graph->len + 1 >= graph->size) {
        graph->size += 4;
        graph->ranges = realloc(graph->ranges, sizeof(live_range_t) * graph->size);
    }

    graph->ranges[graph->len++] = range;
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

    if (seen == idx && !ctx->ops[idx].keep)
        return;

    add_range(graph, idx, seen);
}

live_graph_t build_graph(unit_t *ir) {
    live_graph_t graph = { malloc(sizeof(live_range_t) * 4), 4, 0 };

    for (size_t i = 0; i < ir->length; i++) 
        track_range(&graph, ir, i);

    return graph;
}