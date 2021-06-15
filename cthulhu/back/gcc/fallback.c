#include "gcc.h"

#include "cthulhu/util/report.h"

void gcc_debug(gcc_context *ctx) {
    (void)ctx;
    reportf("gcc disabled");
}

bool gcc_enabled(void) {
    return false;
}

gcc_context *gcc_compile(unit_t *unit, bool debug) {
    (void)unit;
    reportf("gcc disabled");
    return NULL;
}

void gcc_output(gcc_context *ctx, const char *file) {
    (void)ctx;
    (void)file;
    reportf("gcc disabled");
}
