#include "driver.h"

#include "scan.h"
#include "sema/sema.h"

ctu_t *ctu_parse(reports_t *reports, file_t *file) {
    return ctu_compile(reports, file);
}

lir_t *ctu_analyze(reports_t *reports, ctu_t *node) {
    return ctu_sema(reports, node);
}
