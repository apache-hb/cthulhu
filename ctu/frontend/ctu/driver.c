#include "driver.h"

#include "scan.h"
#include "sema/sema.h"

#include "ctu/driver/driver.h"
#include "sema/attrib.h"

ctu_t *ctu_parse(reports_t *reports, file_t *file) {
    return ctu_compile(reports, file);
}

lir_t *ctu_analyze(reports_t *reports, ctu_t *node) {
    return ctu_sema(reports, node);
}

static const frontend_t DRIVER = {
    .version = "0.0.1",
    .name = "Cthulhu",
    .parse = (parse_t)ctu_parse,
    .analyze = (analyze_t)ctu_analyze
};

static void ctu_init(void) {
    init_attribs();
}

int main(int argc, char **argv) {
    return common_main(&DRIVER, argc, argv, ctu_init);
}
