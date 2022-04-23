#include "cthulhu/ast/compile.h"
#include "cthulhu/driver/driver.h"

#include "sema.h"

#include "ctu-bison.h"
#include "ctu-flex.h"

CT_CALLBACKS(kCallbacks, ctu);

static void *ctu_parse(reports_t *reports, scan_t *scan) {
    UNUSED(reports);

    init_scan(scan);

    return compile_file(scan, &kCallbacks);
}

static const driver_t kDriver = {
    .name = "Cthulhu",
    .version = "1.0.0",
    .parse = ctu_parse,
    .sema = ctu_sema,
};

int main(int argc, const char **argv) {
    common_init();

    return common_main(argc, argv, kDriver);
}
