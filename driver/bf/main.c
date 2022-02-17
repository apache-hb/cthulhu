#include "cthulhu/driver/driver.h"

void *bf_parse(reports_t *reports, scan_t *scan) {
    UNUSED(reports);
    UNUSED(scan);

    return NULL;
}

static hlir_t *bf_sema(reports_t *reports, void *ast) {
    UNUSED(reports);
    UNUSED(ast);

    return NULL;
}

static const driver_t DRIVER = {
    .name = "brainfuck",
    .version = "1.0.0",
    .parse = bf_parse,
    .sema = bf_sema
};

int main(int argc, const char **argv) {
    common_init();

    return common_main(argc, argv, DRIVER);
}
