#include "driver.h"

#include "scan.h"
#include "sema.h"

pl0_t *pl0_parse(file_t *file) {
    return pl0_compile(file);
}

lir_t *pl0_analyze(pl0_t *node) {
    return pl0_sema(node);
}
