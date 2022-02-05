#include "cthulhu/driver/driver.h"
#include "parse.h"

/**
 * this actually does both the parsing and semantics
 * C is context sensitive so this is required.
 * as such it returns an hlir tree rather than an ast
 */
static void *c11_parse(reports_t *reports, scan_t *scan) {
    return c11_compile(reports, scan);
}

/**
 * just forward the hlir
 */
static hlir_t *c11_sema(reports_t *reports, void *ast) {
    UNUSED(reports);
    return ast;
}

static driver_t DRIVER = {
    .name = "C11",
    .version = "1.0.0",
    .parse = c11_parse,
    .sema = c11_sema
};

int main(int argc, const char **argv) {
    common_init();

    c11_keyword_init();
    c11_init_types();
    
    return common_main(argc, argv, DRIVER);
}
