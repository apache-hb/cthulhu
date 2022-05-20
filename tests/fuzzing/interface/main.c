#include "cthulhu/util/macros.h"
#include "cthulhu/interface/interface.h"

#include "cthulhu/emit/c89.h"

// 16 pages should be enough to report a few errors to
#define REPORT_MEMORY (0x1000 * 16)
static char kReportMemory[REPORT_MEMORY];

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

    alloc_config_t allocConfig = {
        .generalAlloc = alloc_global(),
        .reportAlloc = alloc_bump(kReportMemory, sizeof(kReportMemory)),
        .runtimeAlloc = alloc_global(),
    };

    CTASSERT(argc == 2, "must provide one argument");

    source_t *src = source_file(allocConfig.generalAlloc, argv[1]);
    vector_t *sources = vector_init(src);

    config_t config = {
        .reportConfig = reportConfig,
        .allocConfig = allocConfig,
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

    c89_emit_modules(cthulhu->reports, allModules);

    return end_reports(cthulhu->reports, "codegen", reportConfig);
}
