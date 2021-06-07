#include "x64.h"

#include <string.h>

#include "regalloc.h"

typedef enum {
    RAX,
    RMAX,
    
    RBX, RCX, RDX,
    R8, R9,

    //RMAX,

    RDI, RSI,

    RBP, RSP
} reg_t;

static FILE *fp;

static const char *x64_reg(reg_t reg) {
    switch (reg) {
    case RAX: return "rax";
    case RBX: return "rbx";
    case RCX: return "rcx";
    case RDX: return "rdx";
    case R8: return "r8";
    case R9: return "r9";
    default: return "err";
    }
}

static void emit_alloc(alloc_t alloc) {
    switch (alloc.type) {
    case ALREG: printf("reg:%s", x64_reg(alloc.reg)); break;
    case ALSPILL: printf("spill:%zu", alloc.addr); break;
    case ALNULL: printf("null"); break;
    }
}

static void emit_range(size_t first, size_t last) {
    printf("[%zu..%zu]", first, last);
}

void x64_emit_asm(unit_t *unit, FILE *out) {
    fp = out;

    regalloc_t alloc = regalloc_assign(unit, RMAX);

    for (size_t i = 0; i < unit->len; i++) {
        alloc_t it = alloc.data[i];
        printf("%zu(", i); emit_alloc(it); printf("): "); emit_range(it.first, it.last); printf("\n");
    }
}

#if 0
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>

FILE *out;

#define EMIT(msg) fprintf(out, msg)
#define EMITF(msg, ...) fprintf(out, msg, __VA_ARGS__)

static alloc_t alloc_reg(reg_t reg) {
    alloc_t out = { IN_REG, { .reg = reg } };
    return out;
}

static alloc_t alloc_spill(size_t offset) {
    alloc_t out = { SPILL, { .offset = offset } };
    return out;
}

static void add_range(live_graph_t *graph, size_t first, size_t last, opcode_t *op) {
    live_range_t range = { first, last, alloc_spill(SIZE_MAX) };
    
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

    size_t i = 0;

    switch (op.op) {
    case OP_ABS: case OP_NEG: case OP_RETURN:
        return op_refs(op.expr, inst);
        
    case OP_ADD: case OP_SUB: case OP_DIV:
    case OP_MUL: case OP_REM:
        return op_refs(op.lhs, inst) || op_refs(op.rhs, inst);

    case OP_BRANCH:
        return op_refs(op.cond, inst) || op_refs(op.label, inst);

    case OP_CALL:
        for (; i < op.total; i++)
            if (op_refs(op.args[i], inst))
                return true;
        
        return op_refs(op.body, inst);

    case OP_EMPTY: case OP_VALUE: case OP_LABEL:
        return false;

    case OP_COPY:
        return op_refs(op.dst, inst) || op_refs(op.src, inst);

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

live_graph_t build_graph(unit_t *ir) {
    live_graph_t graph = { malloc(sizeof(live_range_t) * 4), 4, 0 };

    for (size_t i = 0; i < ir->length; i++) 
        track_range(&graph, ir, i);

    return graph;
}

typedef struct {
    /* if a register is used or not */
    bool used[REG_NONE];

    /* associate an opcode with a register */
    live_graph_t *graph;
    
    size_t stack;
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

static bool is_alloc_reg(alloc_t alloc, reg_t reg) {
    return alloc.type == IN_REG && alloc.reg == reg;
}

static bool reg_is_used(regalloc_t *alloc, size_t at, reg_t reg) {
    for (size_t j = 0; j < alloc->graph->len; j++) {
        live_range_t *range = range_at(alloc, j);

        /* if any range is alive at this point and is using this register */
        if (is_range_live(range, at) && is_alloc_reg(range->assoc, reg)) {
            /* then mark it as used */
            return true;
        }
    }
    return false;
}

static void free_regs(regalloc_t *alloc, size_t idx) {
    for (reg_t i = 0; i < REG_NONE; i++) {
        /* if no live ranges as using this register at this point */
        if (!reg_is_used(alloc, idx, i)) {
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

    live_range_t *range = range_for_op_nullable(alloc, idx);
    if (!range) {
        return;
    }

    for (size_t i = 0; i < REG_NONE; i++) {
        /* try and find a register */

        if (!alloc->used[i]) {
            mark_used(alloc, i, true);
            range->assoc = alloc_reg(i);
            found = true;
            break;
        }
    }

    /* if we run out of registers then spill */
    if (!found) {
        range->assoc = alloc_spill(alloc->stack++);
    }
}

static const char *reg_name(reg_t reg) {
    switch (reg) {
    case RAX: return "rax";
    case RBX: return "rbx";
    case RCX: return "rcx";
    case RDX: return "rdx";
    
    case RDI: return "rdi";
    case RSI: return "rsi";
    case R8: return "r8";
    case R9: return "r9";

    default:
        fprintf(stderr, "reg_name(%d)\n", reg);
        return NULL;
    }
}

static void emit_alloc(alloc_t alloc) {
    if (alloc.type == IN_REG) {
        EMITF("%s", reg_name(alloc.reg));
    } else {
        EMITF("[rbp - %zu]", alloc.offset * 8);
    }
}

static void emit_alloc_imm_mov(alloc_t dst, int64_t num) {
    if (num == 0 && dst.type == IN_REG) {
        const char *name = reg_name(dst.reg);
        EMITF("  xor %s, %s\n", name, name);
    } else {
        EMIT("  mov ");
        emit_alloc(dst);
        EMITF(", %ld\n", num);
    }
}

static void emit_alloc_sym_mov(alloc_t dst, const char *text) {
    EMIT("  mov ");
    emit_alloc(dst);
    EMITF(", %s\n", text);
}

static void emit_reg_mov(reg_t dst, alloc_t src) {
    if (!is_alloc_reg(src, dst)) {
        EMITF("  mov %s, ", reg_name(dst));
        emit_alloc(src);
    }
}

static void emit_operand(regalloc_t *alloc, operand_t op) {
    if (op.type == IMM) { 
        EMITF("%ld", op.num);
    } else if (op.type == SYM) {
        EMITF("%s", op.name);
    } else if (op.type == LABEL) {
        EMITF(".%zu", op.reg);
    } else {
        emit_alloc(range_for_op(alloc, op.reg)->assoc);
    }
}

static bool operand_is_reg(regalloc_t *alloc, operand_t op, alloc_t reg) {
    return op.type == REG && reg.type == IN_REG
        ? is_alloc_reg(range_for_op(alloc, op.reg)->assoc, reg.reg)
        : false;
}

static void mov_to_rax(regalloc_t *alloc, operand_t op) {
    if (op.type == REG) {
        alloc_t dst = range_for_op(alloc, op.reg)->assoc;

        emit_reg_mov(RAX, dst);
    } else {
        EMITF("  mov rax, %ld\n", op.num);
    }
}

static void emit_mov(regalloc_t *alloc, reg_t reg, operand_t val) {
    const char *dst = reg_name(reg);
    if (val.type == IMM && val.num == 0) {
        EMITF("  xor %s, %s\n", dst, dst);
    } else {
        EMITF("  mov %s, ", dst);
        emit_operand(alloc, val);
        EMIT("\n");
    }
}

static void emit_mul(regalloc_t *alloc, size_t idx, opcode_t op) {
    bool used = reg_is_used(alloc, idx, RAX);
    alloc_t dst = range_for_opcode(alloc, op)->assoc;

    bool save_rax = used && is_alloc_reg(dst, RAX);

    if (save_rax) {
        EMIT("  push rax\n");
    }

    mov_to_rax(alloc, op.lhs);
    EMIT("  imul ");
    emit_operand(alloc, op.rhs);
    EMIT("\n");

    if (save_rax) {
        EMIT("  pop rax\n");
    }
}

static void emit_div(regalloc_t *alloc, size_t idx, opcode_t op) {
    alloc_t dst = range_for_opcode(alloc, op)->assoc;
    
    // idiv clobbers rax <- quotient and rdx <- remainder
    bool save_rax = reg_is_used(alloc, idx, RAX) && is_alloc_reg(dst, RAX);
    bool save_rdx = reg_is_used(alloc, idx, RDX);

    if (save_rax) {
        EMIT("  push rax\n");
    }

    if (save_rdx) {
        EMIT("  push rdx\n");
    }

    mov_to_rax(alloc, op.lhs);
    EMIT("  idiv ");
    emit_operand(alloc, op.rhs);
    EMIT("\n");

    if (save_rdx) {
        EMIT("  pop rdx\n");
    }

    if (save_rax) {
        EMIT("  pop rax\n");
    }
}

static void emit_rem(regalloc_t *alloc, size_t idx, opcode_t op) {
    alloc_t dst = range_for_opcode(alloc, op)->assoc;

    // idiv clobbers rax <- quotient and rdx <- remainder
    bool save_rax = reg_is_used(alloc, idx, RAX);
    bool save_rdx = reg_is_used(alloc, idx, RDX) && is_alloc_reg(dst, RDX);

    if (save_rax) {
        EMIT("  push rax\n");
    }

    if (save_rdx) {
        EMIT("  push rdx\n");
    }

    mov_to_rax(alloc, op.lhs);
    EMIT("  idiv ");
    emit_operand(alloc, op.rhs);
    EMIT("\n");

    if (save_rdx) {
        EMIT("  pop rdx\n");
    }

    if (save_rax) {
        EMIT("  pop rax\n");
    }
}

static void emit_add(regalloc_t *alloc, opcode_t op) {
    alloc_t reg = range_for_opcode(alloc, op)->assoc;

    if (!operand_is_reg(alloc, op.lhs, reg)) {
        EMIT("  add ");
        emit_alloc(reg);
        EMIT(", ");
        emit_operand(alloc, op.lhs);
        EMIT("\n");
    }

    EMIT("  add ");
    emit_alloc(reg);
    EMIT(", ");
    emit_operand(alloc, op.rhs);
    EMIT("\n");
}

static void emit_sub(regalloc_t *alloc, opcode_t op) {
    alloc_t reg = range_for_opcode(alloc, op)->assoc;

    if (!operand_is_reg(alloc, op.lhs, reg)) {
        EMIT("  sub ");
        emit_alloc(reg);
        EMIT(", ");
        emit_operand(alloc, op.lhs);
        EMIT("\n");
    }

    EMIT("  sub ");
    emit_alloc(reg);
    EMIT(", ");
    emit_operand(alloc, op.rhs);
    EMIT("\n");
}

static void emit_ret(regalloc_t *alloc, opcode_t op) {
    operand_t ret = op.expr;

    if (ret.type == IMM) {
        emit_mov(alloc, RAX, ret);
    } else {
        alloc_t reg = range_for_opcode(alloc, op)->assoc;
        emit_reg_mov(RAX, reg);
    }

    EMIT("  leave\n");
    EMIT("  ret\n");
}

static void emit_push(regalloc_t *alloc, operand_t arg) {
    EMIT("  push ");
    emit_operand(alloc, arg);
    EMIT("\n");
}

static size_t emit_param(regalloc_t *alloc, size_t i, operand_t arg) {
    /* we unconditionally save rdx and rcx */
    switch (i) {
    case 0:
        emit_mov(alloc, RDI, arg);
        break;
    case 1:
        emit_mov(alloc, RSI, arg);
        break;
    case 2:
        emit_mov(alloc, RDX, arg);
        break;
    case 3:
        emit_mov(alloc, RCX, arg);
        break;
    case 4:
        emit_mov(alloc, R8, arg);
        break;
    case 5:
        emit_mov(alloc, R9, arg);
        break;

    default:
        emit_push(alloc, arg);
        return 8;
    }

    return 0;
}

static void emit_alloc_mov(alloc_t dst, reg_t src) {
    if (is_alloc_reg(dst, src))
        return;

    EMIT("  mov ");
    emit_alloc(dst);
    EMITF(", %s\n", reg_name(src));
}

static void emit_call(regalloc_t *alloc, opcode_t op) {
    live_range_t *range = range_for_opcode(alloc, op);
    alloc_t dst = range->assoc;
    operand_t body = op.body;

    size_t used_stack = 0;

    bool save_rcx = reg_is_used(alloc, range->last + 1, RCX);
    bool save_rdx = reg_is_used(alloc, range->last + 1, RDX);

    if (save_rcx)
        EMIT("  push rcx\n");

    if (save_rdx)
        EMIT("  push rdx\n");

    /* sysv pushes arguments in reverse order */
    for (int64_t i = op.total; i--;) {
        used_stack += emit_param(alloc, i, op.args[i]);
    }

    if (body.type == IMM) {
        EMITF("  call %ld\n", body.num);
    } else {
        EMIT("  call ");
        emit_operand(alloc, body);
        EMIT("\n");
    }

    if (save_rcx)
        EMIT("  pop rcx\n");

    if (save_rdx)
        EMIT("  pop rdx\n");

    if (used_stack) {
        EMITF("  add rsp, %zu\n", used_stack);
    }

    emit_alloc_mov(dst, RAX);
}

static void emit_value(regalloc_t *alloc, opcode_t op) {
    alloc_t reg = range_for_opcode(alloc, op)->assoc;
    switch (op.expr.type) {
    case SYM:
        emit_alloc_sym_mov(reg, op.expr.name);
        break;
    case IMM:
        emit_alloc_imm_mov(reg, op.expr.num);
        break;

    default:
        fprintf(stderr, "emit_value(%d)\n", op.expr.type);
        break;
    }
}

static bool is_const_true(operand_t op) {
    return op.type == IMM && op.num != 0;
}

static void emit_branch(regalloc_t *alloc, opcode_t op) {
    if (!is_const_true(op.cond)) {
        EMIT("  cmp ");
        emit_operand(alloc, op.cond);
        EMIT(", 0\n");

        EMIT("  jne ");
    } else {
        EMIT("  jmp ");
    }

    emit_operand(alloc, op.label);
    EMIT("\n");
}

static void emit_label(size_t idx) {
    EMITF(".%zu:\n", idx);
}

static bool is_mov_redundant(regalloc_t *alloc, operand_t lhs, operand_t rhs) {
    if (lhs.type != REG || rhs.type != REG) {
        return false;
    }
    
    alloc_t dst = range_for_op(alloc, lhs.reg)->assoc,
            src = range_for_op(alloc, rhs.reg)->assoc;

    if (dst.type == IN_REG && src.type == IN_REG) {
        return dst.reg == src.reg;
    } else {
        return dst.offset == src.offset;
    }
}

static void emit_copy(regalloc_t *alloc, opcode_t op) {
    if (!is_mov_redundant(alloc, op.src, op.dst)) {
        EMIT("  mov ");
        emit_operand(alloc, op.dst);
        EMIT(", ");
        emit_operand(alloc, op.src);
        EMIT("\n");
    }
}

static void emit_inst(regalloc_t *alloc, size_t idx, opcode_t op) {
    switch (op.op) {
    case OP_EMPTY:
        break;

    case OP_VALUE:
        emit_value(alloc, op);
        break;

    case OP_ADD:
        emit_add(alloc, op);
        break;
    case OP_SUB:
        emit_sub(alloc, op);
        break;
    case OP_MUL:
        emit_mul(alloc, idx, op);
        break;
    case OP_REM:
        emit_rem(alloc, idx, op);
        break;
    case OP_DIV:
        emit_div(alloc, idx, op);
        break;

    case OP_RETURN:
        emit_ret(alloc, op);
        break;

    case OP_CALL:
        emit_call(alloc, op);
        break;

    case OP_BRANCH:
        emit_branch(alloc, op);
        break;
    case OP_LABEL:
        emit_label(idx);
        break;

    case OP_COPY:
        emit_copy(alloc, op);
        break;

    default:
        fprintf(stderr, "emit_inst(%d)\n", op.op);
        break;
    }
}

static size_t round_stack_size(size_t offset) {
    return (offset + 16 - 1) & ~16;
}

void emit_asm(unit_t *ir, FILE *output) {
    out = output;

    live_graph_t graph = build_graph(ir);
    regalloc_t alloc = { {}, &graph, 0 };

    /* link registers to live ranges */
    for (size_t i = 0; i < ir->length; i++) {
        link_reg_to_range(&alloc, i);
    }

    EMITF("%s:\n", ir->name);
    EMIT("  push rbp\n");
    EMIT("  mov rbp, rsp\n");
    
    if (alloc.stack)
        EMITF("  sub rsp, %zu\n", round_stack_size(alloc.stack * 8));

    for (size_t i = 0; i < ir->length; i++) {
        emit_inst(&alloc, i, ir->ops[i]);
    }
}
#endif