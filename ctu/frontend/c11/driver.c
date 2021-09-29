#include "driver.h"

c_t *c_parse(reports_t *reports, file_t *file) {
    report(reports, INTERNAL, NULL, "C is unimplemented %s", file->path);
    return NULL;
}

lir_t *c_analyze(reports_t *reports, c_t *node) {
    report(reports, INTERNAL, node->node, "C is unimplemented");
    return NULL;
}
