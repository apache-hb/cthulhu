#include "x86.h"

typedef enum {
    RAX, RBX, RCX, RDX,
    RSI, RDI, RBP, RIP,
    RSP, 

    R8, R9, R10, R11,
    R12, R13, R14, R15,

    USABLE
} reg_t;

typedef struct {
    /* current used registers */
    bool used[USABLE];
} x86_regalloc_t;

blob_t *gen_x64(module_t *mod) {
    (void)mod;
    return NULL;
}
