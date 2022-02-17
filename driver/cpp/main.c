#include "cthulhu/driver/driver.h"

static void *cpp_parse(reports_t *reports, scan_t *scan) {
    UNUSED(reports);
    UNUSED(scan);

    return NULL;
}

static hlir_t *cpp_sema(reports_t *reports, void *ast) {
    UNUSED(reports);
    UNUSED(ast);

    return NULL;
}

static driver_t DRIVER = {
    .name = "c-pre-processor",
    .version = "1.0.0",
    .parse = cpp_parse,
    .sema = cpp_sema
};

int main(int argc, const char **argv) {
    common_init();
    
    return common_main(argc, argv, DRIVER);
}
