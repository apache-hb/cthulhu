#include "cthulhu/interface/interface.h"

#include "cthulhu/ssa/ssa.h"

#include "std/stream.h"

#include "base/macros.h"
#include "base/memory.h"

#include <stdio.h>

int main(int argc, const char **argv)
{
    common_init();
    driver_t driver = get_driver();
    verbose = true;
    
    report_config_t reportConfig = {
        .limit = 20, 
        .warningsAreErrors = false,
    };

    vector_t *sources = vector_of(argc - 1);
    for (int i = 1; i < argc; i++)
    {
        const char *file = argv[i];
        source_t *source = source_file(&globalAlloc, file);
        vector_set(sources, i - 1, source);
    }

    config_t config = {
        .alloc = &globalAlloc,
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
        if (status != EXIT_OK)
        {
            return status;
        }
    }

    module_t *mod = ssa_compile(cthulhu->reports, cthulhu_get_modules(cthulhu));

    status_t status = end_reports(cthulhu->reports, "ssa codegen", reportConfig);
    if (status != EXIT_OK) { return status; }

    stream_t *out = ssa_debug(cthulhu->reports, mod);

    printf("%s\n", stream_data(out));

    return end_reports(cthulhu->reports, "ssa debug output", reportConfig);
}
