#include "base/macros.h"
#include "base/panic.h"
#include "base/memory.h"

#include "cthulhu/interface/interface.h"

#include "cthulhu/ssa/ssa.h"
#include "cthulhu/emit/c89.h"

#include "io/io.h"
#include "report/report.h"

#define CHECK_REPORTS(msg) \
    do { \
        status_t err = end_reports(cthulhu->reports, msg, reportConfig); \
        if (err != 0) { \
            return err; \
        } \
    } while (0)

int main(int argc, const char **argv)
{
    UNUSED(argc);
    UNUSED(argv);
    
    common_init();

    driver_t driver = get_driver();

    report_config_t reportConfig = DEFAULT_REPORT_CONFIG;

    verbose = true;

    CTASSERTM(argc == 2, "must provide one argument");

    io_t *io = io_file(argv[1], eFileRead | eFileText);
    vector_t *sources = vector_init(io);

    config_t config = {
        .reportConfig = reportConfig,
    };

    cthulhu_t *cthulhu = cthulhu_new(driver, sources, config);

    cthulhu_step_t steps[] = {
        cthulhu_init, 
        cthulhu_parse, 
        cthulhu_forward, 
        cthulhu_resolve, 
        cthulhu_compile,
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

    vector_t *allModules = cthulhu_get_modules(cthulhu);

    io_t *dst = io_memory("ssa-output", NULL, 0x1000);

    ssa_module_t *mod = ssa_gen_module(cthulhu->reports, allModules);
    CHECK_REPORTS("emitting ssa");

    ssa_opt_module(cthulhu->reports, mod);
    CHECK_REPORTS("optimizing ssa");

    ssa_emit_module(cthulhu->reports, mod);
    CHECK_REPORTS("emitting ssa");

    c89_emit_ssa_modules(cthulhu->reports, mod, dst);

    return end_reports(cthulhu->reports, "codegen", reportConfig);
}
