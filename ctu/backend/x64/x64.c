#include "x64.h"

static void compile_block(const block_t *block) {
    vector_t *locals = block->locals;
    size_t len = vector_len(locals);
    size_t stack = 0;
    
    for (size_t i = 0; i < len; i++) {

    }
}

bool x64_build(reports_t *reports, module_t *mod, const char *path) {

}
