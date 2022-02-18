#include "cthulhu/driver/driver.h"
#include "cthulhu/ast/compile.h"

#include "sema.h"

#include "ctu-bison.h"
#include "ctu-flex.h"

CT_CALLBACKS(CALLBACKS, ctu);

static void *ctu_parse(reports_t *reports, scan_t *scan) {
    UNUSED(reports);
    
    init_scan(scan);

    return compile_file(scan, &CALLBACKS);
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
