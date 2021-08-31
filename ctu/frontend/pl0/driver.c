#include "driver.h"

#include "ctu/util/report.h"

#include "scan.h"

node_t *pl0_parse(file_t *file) {
    return pl0_compile(file);
}

lir_t *pl0_analyze(node_t *node) {
    (void)node;
    return NULL;
}
