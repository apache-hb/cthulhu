#include "cthulhu/interface/interface.h"

int main(int argc, const char **argv)
{
    common_init();
    driver_t driver = get_driver();
    
    report_config_t reportConfig = {
        .limit = 20, 
        .warningsAreErrors = false,
    };

    alloc_config_t allocConfig = {
        .generalAlloc = alloc_global(),
        .reportAlloc = alloc_global(),
        .runtimeAlloc = alloc_global(),
    };

    vector_t *sources = vector_of(argc - 1);
    for (int i = 1; i < argc; i++)
    {
        const char *file = argv[i];
        source_t *source = source_file(allocConfig.generalAlloc, file);
        vector_set(sources, i - 1, source);
    }

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

    return 0;
}
