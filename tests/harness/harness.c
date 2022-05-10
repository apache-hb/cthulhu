#include "cthulhu/interface/interface.h"

int main(int argc, const char **argv)
{
    common_init();
    driver_t driver = get_driver();

    vector_t *sources = vector_of(argc - 1);
    for (int i = 1; i < argc; i++)
    {
        const char *file = argv[i];
        source_t *source = source_file(file);
        vector_set(sources, i - 1, source);
    }

    report_config_t reportConfig = {.limit = 20, .warningsAreErrors = false,};
    config_t config = {.reportConfig = reportConfig};

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
