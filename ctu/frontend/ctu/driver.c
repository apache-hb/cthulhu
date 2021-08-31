#include "driver.h"

#include "scan.h"

node_t *ctu_parse(file_t *file) {
    return ctu_compile(file);
}

lir_t *ctu_analyze(node_t *node) {
    (void)node;
    return NULL;
}