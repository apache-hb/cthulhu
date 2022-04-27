#include "cthulhu/ast/compile.h"
#include "cthulhu/driver/driver.h"

#include "sema.h"

#include "pl0-bison.h"
#include "pl0-flex.h"

CT_CALLBACKS(kCallbacks, pl0);

void *pl0_parse(runtime_t *runtime, scan_t *scan) {
    UNUSED(runtime);

    return compile_file(scan, &kCallbacks);
}

static const driver_t kDriver = {
    .name = "PL/0",
    .version = "2.1.0",
    .parse = pl0_parse,
    .sema = pl0_sema,
};

int main(int argc, const char **argv) {
    common_init();

    pl0_init();

    return common_main(argc, argv, kDriver);
}
