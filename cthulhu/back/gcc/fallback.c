#include "gcc.h"

void gcc_debug(gcc_context *ctx) {
    (void)ctx;
    reportf("gcc disabled");
}

bool gcc_enabled(void) {
    return false;
}

gcc_context *gcc_compile(unit_t *unit) {
    (void)unit;
    reportf("gcc disabled");
    return NULL;
}

void gcc_output(gcc_context *ctx, FILE *file) {
    (void)ctx;
    (void)file;
    reportf("gcc disabled");
}
