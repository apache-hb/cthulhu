#include "driver.h"

#include "scan.h"
#include "sema.h"

pl0_t *pl0_parse(reports_t *reports, file_t *file) {
    return pl0_compile(reports, file);
}

lir_t *pl0_analyze(reports_t *reports, pl0_t *node) {
    return pl0_sema(reports, node);
}
