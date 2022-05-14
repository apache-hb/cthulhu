#include "cthulhu/util/macros.h"
#include "cthulhu/interface/interface.h"

#include "cthulhu/emit/c89.h"

int main(int argc, const char **argv)
{
    UNUSED(argc);
    UNUSED(argv);
    
    common_init();

    driver_t driver = get_driver();

    reports_t *reports = begin_reports();

    report_config_t reportConfig = {
        .limit = 100,
        .warningsAreErrors = false,
    };

    alloc_config_t allocConfig = {
        .nothing = NULL,
    };

    verbose = true;

    CTASSERT(argc == 2, "must provide one argument");

    source_t *src = source_file(argv[1]);
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

    c89_emit_modules(reports, allModules);

    return end_reports(reports, "codegen", reportConfig);
}
