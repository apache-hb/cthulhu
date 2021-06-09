#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t *code;
    size_t len;
    size_t size;
} blob_t;

blob_t x86_blob(size_t init);

typedef enum {
    /* REX.X = 0 */
    RAX, RBX, RCX, RDX,

    /* REX.X = 1 */
    R8, R9, R10, R11,
    R12, R13, R14, R15,

    /* unused during regalloc */

    /* REX.X = 0 */
    RSP, RBP, RSI, RDI,
} x86_reg64_t;

typedef uint64_t x86_addr64_t;

typedef uint16_t x86_imm16_t;
typedef uint64_t x86_imm64_t;

typedef struct {
    enum { 
        X86_REG64, 
        X86_IMM16,
        X86_IMM64, 
        X86_ADDR64,

        X86_NOOP
    } type;

    enum { X86_NEAR, X86_SHORT, X86_FAR, X86_NONE } kind;

    bool indirect:1;

    union {
        x86_imm16_t imm16; /* IMM16 */
        x86_imm64_t imm64; /* IMM64 */
        x86_reg64_t reg64; /* REG64 */
        x86_addr64_t addr64; /* ADDR */
    };
} x86_operand_t;

x86_operand_t x86_reg64(x86_reg64_t reg);
x86_operand_t x86_imm16(x86_imm16_t imm);
x86_operand_t x86_imm64(x86_imm64_t imm);
x86_operand_t x86_noop(void);

x86_operand_t x86_near(x86_operand_t op);
x86_operand_t x86_far(x86_operand_t op);
x86_operand_t x86_mem(x86_operand_t op);

void x86_mov(blob_t *blob, x86_operand_t dst, x86_operand_t src);
void x86_ret(blob_t *blob, x86_operand_t op);

void x86_cmp(blob_t *blob, x86_operand_t lhs, x86_operand_t rhs);
void x86_jmp(blob_t *blob, x86_operand_t op);
