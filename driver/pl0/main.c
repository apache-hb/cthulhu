#include "cthulhu/driver/driver.h"
#include "cthulhu/ast/compile.h"

#include "sema.h"

#include "pl0-bison.h"
#include "pl0-flex.h"

CT_CALLBACKS(CALLBACKS, pl0);

void *pl0_parse(reports_t *reports, scan_t *scan) {
    UNUSED(reports);
    
    return compile_file(scan, &CALLBACKS);
}

static driver_t DRIVER = {
    .name = "PL/0",
    .version = "2.1.0",
    .parse = pl0_parse,
    .sema = pl0_sema
};

int main(int argc, const char **argv) {
    common_init();
    
    pl0_init();
    
    return common_main(argc, argv, DRIVER);
}
