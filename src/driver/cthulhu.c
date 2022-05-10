#include "cthulhu/driver/driver.h"

static runtime_t runtime_new(reports_t *reports, size_t size)
{
    runtime_t runtime = {
        .reports = reports,
        .modules = map_optimal(size)
    };

    return runtime;
}

cthulhu_t *cthulhu_new(driver_t driver, vector_t *sources, vector_t *plugins)
{
    size_t totalSources = vector_len(sources);
    reports_t *reports = begin_reports();
    runtime_t runtime = runtime_new(reports, totalSources);

    cthulhu_t *cthulhu = ctu_malloc(sizeof(cthulhu_t));

    cthulhu->driver = driver;
    cthulhu->status = EXIT_INTERNAL;
    cthulhu->reports = reports;

    cthulhu->runtime = runtime;
    cthulhu->compiles = vector_new(totalSources);

    cthulhu->sources = sources;
    cthulhu->plugins = plugins;

    return cthulhu;
}
