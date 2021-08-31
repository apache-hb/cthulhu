#include "driver.h"

#include "scan.h"
#include "sema.h"

node_t *pl0_parse(file_t *file) {
    return pl0_compile(file);
}

lir_t *pl0_analyze(node_t *node) {
    return pl0_sema(node);
}
