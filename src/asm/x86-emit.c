#include "x86-emit.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

#include "ast.h"

#define X86_ASSERT(expr) if (!(expr)) { fprintf(stderr, "[%s:%d]: " #expr "\n", __FILE__, __LINE__); return; }

static void emit_generic(blob_t *blob, void *data, size_t size) {
    if (blob->len + size >= blob->size) {
        blob->size += size + 64;
        blob->code = realloc(blob->code, blob->size);
    }
    memcpy(blob->code + blob->len, data, size);
    blob->len += size;
}

static void emit8(blob_t *blob, uint8_t byte) {
    emit_generic(blob, &byte, sizeof(uint8_t));
}

static void emit64(blob_t *blob, x86_imm64_t imm) {
    emit_generic(blob, &imm, sizeof(x86_imm64_t));
}

static void emit16(blob_t *blob, x86_imm16_t imm) {
    emit_generic(blob, &imm, sizeof(x86_imm16_t));
}

static void emit_many(blob_t *blob, size_t len, ...) {
    va_list args;
    va_start(args, len);
    
    X86_ASSERT(len > 0);

    if (blob->len + len >= blob->size) {
        blob->size += len + 64;
        blob->code = realloc(blob->code, blob->size);
    }
    for (size_t i = 0; i < len; i++) {
        blob->code[blob->len++] = va_arg(args, int);
    }

    va_end(args);
}

static bool op_is_reg64(x86_operand_t op) {
    return op.type == X86_REG64;
}

static bool op_is_imm64(x86_operand_t op) {
    return op.type == X86_IMM64;
}

static bool op_is_noop(x86_operand_t op) {
    return op.type == X86_NOOP;
}

static bool op_is_imm16(x86_operand_t op) {
    return op.type == X86_IMM16;
}

static bool op_is_near(x86_operand_t op) {
    return op.kind == X86_NEAR;
}

static bool op_is_far(x86_operand_t op) {
    return op.kind == X86_FAR;
}

static bool op_is_mem(x86_operand_t op) {
    return op.indirect;
}

blob_t x86_blob(size_t init) {
    blob_t blob = { malloc(sizeof(uint8_t) * init), 0, init };
    return blob;
}

x86_operand_t x86_operand(int type) {
    x86_operand_t op = { .type = type, .kind = X86_NONE, .indirect = false, { } };
    return op;
}

x86_operand_t x86_reg64(x86_reg64_t reg) {
    x86_operand_t op = x86_operand(X86_REG64);
    op.reg64 = reg;
    return op;
}

x86_operand_t x86_imm64(x86_imm64_t imm) {
    x86_operand_t op = x86_operand(X86_IMM64);
    op.imm64 = imm;
    return op;
}

x86_operand_t x86_imm16(x86_imm16_t imm) {
    x86_operand_t op = x86_operand(X86_IMM16);
    op.imm16 = imm;
    return op;
}

x86_operand_t x86_noop(void) {
    x86_operand_t op = x86_operand(X86_NOOP);
    return op;
}

x86_operand_t x86_near(x86_operand_t op) {
    op.kind = X86_NEAR;
    return op;
}

x86_operand_t x86_far(x86_operand_t op) {
    op.kind = X86_FAR;
    return op;
}

x86_operand_t x86_mem(x86_operand_t op) {
    op.indirect = true;
    return op;
}

#define REX_W (1 << 3)
#define REX_R (1 << 2)
#define REX_B (1 << 0)

#define MODRM_REG 0b11000000
#define MODRM_MEM 0b00000000

typedef uint8_t rex_t;
typedef uint8_t modrm_mod_t;
typedef uint8_t modrm_reg_t;
typedef uint8_t modrm_rm_t;
typedef uint8_t modrm_t;

static rex_t rex(rex_t fields) {
    return 0b01000000 | fields;
}

/**
 * reg: the register to encode into modrm
 * rex: the rex prefix
 * extend: the flag needed to extend rex if needed
 */
static modrm_reg_t modrm_reg64(x86_reg64_t reg, rex_t *rex, rex_t extend) {
    /* R8 - R15 require a rex bit to be set */
    if (reg >= R8 && reg <= R15)
        *rex |= extend;

    switch (reg) {
    case RAX: case R8: return 0b000;
    case RCX: case R9: return 0b001;
    case RDX: case R10: return 0b010;
    case RBX: case R11: return 0b011;
    case RSP: case R12: return 0b100;
    case RBP: case R13: return 0b101;
    case RSI: case R14: return 0b110;
    case RDI: case R15: return 0b111;
    default:
        fprintf(stderr, "modrm_reg(%d)\n", reg);
        return 0b11111111;
    }
}

static modrm_t modrm(modrm_mod_t mod, modrm_reg_t reg, modrm_rm_t rm) {
    return mod | (reg << 3) | rm;
}

static modrm_mod_t modrm_mod(x86_operand_t op) {
    return op_is_mem(op) ? MODRM_MEM : MODRM_REG;
}

#define X86_MOV_R64_RM64_OPCODE 0x8B
#define X86_MOV_R64_IMM64_OPCODE 0xB8

#define X86_NEAR_RET_OPCODE 0xC3
#define X86_FAR_RET_OPCODE 0xCB
#define X86_NEAR_RET_IMM16_OPCODE 0xC2
#define X86_FAR_RET_IMM16_OPCODE 0xCA

#define X86_CMP_R64_RM64_OPCODE 0x3B
#define X86_CMP_RM64_R64_OPCODE 0x39

/* mov r64, r/m64 */
static void x86_mov_r64_rm64(blob_t *blob, x86_reg64_t dst, x86_reg64_t src, modrm_mod_t mod) {
    rex_t prefix = rex(REX_W);
    modrm_reg_t from = modrm_reg64(dst, &prefix, REX_R);
    modrm_rm_t to = modrm_reg64(src, &prefix, REX_B);
    modrm_t mr = modrm(mod, from, to);
    emit_many(blob, 3, prefix, X86_MOV_R64_RM64_OPCODE, mr);
}

/* movabs r64, imm64 */
static void x86_mov_r64_imm64(blob_t *blob, x86_reg64_t dst, x86_imm64_t imm) {
    rex_t prefix = rex(REX_W);
    modrm_reg_t from = modrm_reg64(dst, &prefix, REX_B);
    emit_many(blob, 2, prefix, X86_MOV_R64_IMM64_OPCODE | from);
    emit64(blob, imm);
}

void x86_mov(blob_t *blob, x86_operand_t dst, x86_operand_t src) {
    if (op_is_reg64(dst) && op_is_reg64(src)) {
        x86_mov_r64_rm64(blob, dst.reg64, src.reg64, modrm_mod(src));
    } else if (op_is_reg64(dst) && op_is_imm64(src)) {
        x86_mov_r64_imm64(blob, dst.reg64, src.imm64);
    } else {
        fprintf(stderr, "x86_mov(%d <- %d)\n", dst.type, src.type);
    }
}

void x86_ret(blob_t *blob, x86_operand_t op) {
    X86_ASSERT(op_is_near(op) || op_is_far(op));

    if (op_is_imm16(op)) {
        emit8(blob, op_is_near(op) ? X86_NEAR_RET_IMM16_OPCODE : X86_FAR_RET_IMM16_OPCODE);
        emit16(blob, op.imm16);
    } else if (op_is_noop(op)) {
        emit8(blob, op_is_near(op) ? X86_NEAR_RET_OPCODE : X86_FAR_RET_OPCODE);
    } else {
        fprintf(stderr, "x86_ret(op.type = %d)\n", op.type);
    }
}

static void x86_cmp_r64_rm64(blob_t *blob, x86_reg64_t lhs, x86_reg64_t rhs, modrm_mod_t m) {
    rex_t prefix = rex(REX_W);
    modrm_mod_t l = modrm_reg64(lhs, &prefix, REX_B);
    modrm_rm_t r = modrm_reg64(rhs, &prefix, REX_R);
    modrm_t mod = modrm(m, r, l);
    emit_many(blob, 3, prefix, X86_CMP_R64_RM64_OPCODE, mod);
}

static void x86_cmp_rm64_r64(blob_t *blob, x86_reg64_t lhs, x86_reg64_t rhs, modrm_mod_t m) {
    rex_t prefix = rex(REX_W);
    modrm_mod_t l = modrm_reg64(lhs, &prefix, REX_B);
    modrm_rm_t r = modrm_reg64(rhs, &prefix, REX_R);
    modrm_t mod = modrm(m, l, r);
    emit_many(blob, 3, prefix, X86_CMP_RM64_R64_OPCODE, mod);
}

void x86_cmp(blob_t *blob, x86_operand_t lhs, x86_operand_t rhs) {
    if (op_is_reg64(lhs) && op_is_reg64(rhs)) {
        X86_ASSERT(!(op_is_mem(lhs) && op_is_mem(rhs)));
        
        if (op_is_mem(lhs)) {
            x86_cmp_rm64_r64(blob, lhs.reg64, rhs.reg64, MODRM_MEM);
        } else {
            x86_cmp_r64_rm64(blob, lhs.reg64, rhs.reg64, modrm_mod(rhs));
        }
    } else {
        fprintf(stderr, "x86_cmp(lhs.type = %d, rhs.type = %d)\n", lhs.type, rhs.type);
    }
}

void x86_jmp(blob_t *blob, x86_operand_t op) {
    (void)blob;

    if (op_is_near(op)) {

    } else {
        fprintf(stderr, "x86_jmp(op.type = %d)\n", op.type);
    }
}