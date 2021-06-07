#include "aarch64.h"
#include "regalloc.h"

static FILE *fp;

typedef enum {
    R0, R1, R2, R3,
    R4, R5, R6, R7,
    R8, R9, R10, R11,
    R12, R13, R14, R15,
    R16, R17, R18, R19,
    R20, R21, R22, R23,
    R24, R25, R26, R27,
    R28, R29, R30,

    RMAX
} reg_t;

static void a64_reg(reg_t reg) {
    printf("r%d", reg);
}

static void emit_alloc(alloc_t alloc) {
    switch (alloc.type) {
    case ALREG: printf("reg:"); a64_reg(alloc.reg); break;
    case ALSPILL: printf("spill:%zu", alloc.addr); break;
    case ALNULL: printf("null"); break;
    }
}

static void emit_range(size_t first, size_t last) {
    printf("[%zu..%zu]", first, last);
}

void a64_emit_asm(unit_t *unit, FILE *out) {
    fp = out;

    regalloc_t alloc = regalloc_assign(unit, RMAX);

    for (size_t i = 0; i < unit->len; i++) {
        alloc_t it = alloc.data[i];
        printf("%zu(", i); emit_alloc(it); printf("): "); emit_range(it.first, it.last); printf("\n");
    }
}
