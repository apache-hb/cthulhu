#include "cthulhu/ast/compile.h"
#include "cthulhu/driver/driver.h"

void *cc_parse(reports_t *reports, scan_t *scan)
{
    UNUSED(reports);
    UNUSED(scan);

    return NULL;
}

static hlir_t *cc_sema(reports_t *reports, void *ast)
{
    UNUSED(reports);
    UNUSED(ast);

    return NULL;
}

static driver_t DRIVER = {
    .name = "C",
    .version = "1.0.0",
    .parse = cc_parse,
    .sema = cc_sema,
};

int main(int argc, const char **argv)
{
    common_init();

    return common_main(argc, argv, DRIVER);
}
