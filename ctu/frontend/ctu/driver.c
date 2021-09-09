#include "driver.h"

#include "scan.h"

node_t *ctu_parse(reports_t *reports, file_t *file) {
    return ctu_compile(reports, file);
}

lir_t *ctu_analyze(reports_t *reports, node_t *node) {
    (void)reports;
    (void)node;
    return NULL;
}