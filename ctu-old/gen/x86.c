#include "x86.h"

#include <stdlib.h>

typedef struct {
    uint8_t bits;
} regbits_t;

regbits_t unused_reg(void) {
    regbits_t r = { 0 };
    return r;
}

typedef enum {
    RAX, EAX, AX, AL, AH
} reg_t;

typedef struct { 
    enum { REG, MEM } kind;
    union {
        reg_t reg;
        size_t addr;
    };
} alloc_t;
    
typedef struct { 
    size_t start;
    size_t end;
} range_t;

#define REG_MASK_HIGH8  0b00000010
#define REG_MASK_LOW8   0b00000001
#define REG_MASK16      0b00000011
#define REG_MASK32      0b00001111
#define REG_MASK64      0b11111111

typedef struct {
    regbits_t a;
    regbits_t b;
    regbits_t c;
    regbits_t d;
} regalloc_t;

range_t range_for(flow_t *flow, size_t idx) {
    size_t start = idx;
    size_t end = idx;

    for (size_t i = idx; i < flow->len; i++) {
        step_t *step = step_at(flow, i);
        if (is_vreg_used(step, idx)) {
            end = i;
        }
    }

    range_t range = { start, end };
    return range;
}

regalloc_t assign_regs(flow_t *flow) {
    regalloc_t self = { 
        unused_reg(), unused_reg(), 
        unused_reg(), unused_reg() 
    };

    for (size_t i = 0; i < flow->len; i++) {
        range_for(flow, i);
    }

    return self;
}

typedef struct {
    range_t range;
    reg_t reg;
    size_t addr;
} step_data_t;

typedef struct { 
    void *data;
    size_t size;
} blob_t;

blob_t compile_flow(flow_t *flow) {
    /** 
     * store the address of every instruction in this flow.
     * this is so we know where to jump to with labels
     * and control flow steps
    */
    size_t *data = malloc(sizeof(step_data_t) * flow->len);

    ctu_free(data);

    blob_t blob = {
#ifdef _WIN32
        NULL, 0
#endif
    };

    return blob;
}

void gen_x86(FILE *out, module_t *mod) {
    for (size_t i = 0; i < num_flows(mod); i++) {
        blob_t blob = compile_flow(mod->blocks + i);
        fwrite(out, blob.size, 1, blob.data);
    }
}
