#include "cthulhu/driver/driver.h"
#include "cthulhu/ast/compile.h"

#include "ctu-bison.h"
#include "ctu-flex.h"

CT_CALLBACKS(CALLBACKS, ctu);

static void *ctu_parse(reports_t *reports, scan_t *scan) {
    UNUSED(reports);
    
    lex_extra_t *extra = ctu_malloc(sizeof(lex_extra_t));
    extra->depth = 0;
    scan_set(scan, extra);

    return compile_file(scan, &CALLBACKS);
}

static hlir_t *ctu_sema(reports_t *reports, void *ast) {
    return NULL;
}

static driver_t DRIVER = {
    .name = "Cthulhu",
    .version = "1.0.0",
    .parse = ctu_parse,
    .sema = ctu_sema
};

int main(int argc, const char **argv) {
    common_init();

    return common_main(argc, argv, DRIVER);
}
