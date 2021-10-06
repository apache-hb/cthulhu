#include "driver.h"

#include "scan.h"
#include "sema.h"

pl0_t *pl0_parse(reports_t *reports, file_t *file) {
    return pl0_compile(reports, file);
}

lir_t *pl0_analyze(reports_t *reports, pl0_t *node) {
    return pl0_sema(reports, node);
}

#if 0
static const frontend_t DRIVER = {
    .version = "0.0.2",
    .name = "PL/0",
    .parse = (parse_t)pl0_parse,
    .analyze = (analyze_t)pl0_analyze
};

int main(int argc, const char **argv) {
    return common_main(&DRIVER, argc, argv);
}
#endif
