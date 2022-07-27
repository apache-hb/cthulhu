#include "base/macros.h"
#include "base/panic.h"
#include "base/memory.h"

#include "cthulhu/interface/interface.h"

#include "cthulhu/emit/c89.h"

#include "io/io.h"

int main(int argc, const char **argv)
{
    UNUSED(argc);
    UNUSED(argv);
    
    common_init();

    driver_t driver = get_driver();

    report_config_t reportConfig = {
        .limit = 100,
        .warningsAreErrors = false,
    };

    verbose = true;

    CTASSERTM(argc == 2, "must provide one argument");

    io_t *io = io_file(&globalAlloc, argv[1], eFileRead | eFileText);
    vector_t *sources = vector_init(io);

    config_t config = {
        .reportConfig = reportConfig,
    };

    cthulhu_t *cthulhu = cthulhu_new(driver, sources, config);

    cthulhu_step_t steps[] = {
        cthulhu_init, cthulhu_parse, cthulhu_forward, cthulhu_resolve, cthulhu_compile,
    };

    size_t totalSteps = sizeof(steps) / sizeof(cthulhu_step_t);

    for (size_t i = 0; i < totalSteps; i++)
    {
        int status = steps[i](cthulhu);
        if (status != 0)
        {
            return status;
        }
    }

    vector_t *allModules = cthulhu_get_modules(cthulhu);

    io_t *dst = io_memory(&globalAlloc, "c89-output", NULL, 0x1000);

    c89_emit_modules(cthulhu->reports, allModules, dst);

    return end_reports(cthulhu->reports, "codegen", reportConfig);
}
