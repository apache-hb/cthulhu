#include "async.h"

#include "ctu/util/task/task.h"
#include "ctu/util/task/critical.h"
#include "ctu/util/str.h"
#include "ctu/gen/emit.h"
#include "driver.h"

#include <unistd.h>

typedef struct {
    file_t *file;
    size_t id;
    const driver_t *driver;
} task_data_t;

static atomic_int err = 0;

static ctu_critical_t lock = CTU_CRITICAL_INIT;

#define CHECK_ERROR(fmt, ...) do { \
        res = end_reports(reports, SIZE_MAX, format(fmt, __VA_ARGS__)); \
        if (res != 0) { \
            err = MAX(err, res); \
            return; \
        } \
    } while (0)

static void async_task(void *arg) {
    task_data_t *data = arg;
    const char *path = data->file->path;
    int res;
    reports_t *reports = begin_reports();
    const driver_t *driver = select_driver_by_extension(reports, data->driver, path);
    void *tree;
    lir_t *lir;

    logverbose("compilation task %ld for %s began", data->id, path);

    CHECK_ERROR("driver selection for %s", path);


    tree = driver->parse(reports, data->file);
    CHECK_ERROR("parsing %s", path);

    lir = driver->analyze(reports, tree);
    CHECK_ERROR("analyzing %s", path);

    CTU_CRITICAL(&lock, {
        module_t *mod = module_build(reports, lir);
        module_print(stdout, mod);
    });
}

static task_data_t *task_data(file_t *file, size_t id, const driver_t *driver) {
    task_data_t *data = ctu_malloc(sizeof(task_data_t));
    data->file = file;
    data->id = id;
    data->driver = driver;
    return data;
}

int async_main(settings_t settings) {
    vector_t *sources = settings.sources;
    size_t threads = settings.threads;

    size_t len = vector_len(sources);
    task_pool_t *pool = task_pool(threads);

    task_t *tasks = ctu_malloc(sizeof(task_t) * len);

    for (size_t i = 0; i < len; i++) {
        file_t *file = vector_get(sources, i);
        task_t task = { task_data(file, i, settings.driver), async_task };
        tasks[i] = task;
    }
    
    for (size_t i = 0; i < len; i++) {
        task_run(pool, tasks[i]);
    }

    pool_delete(pool);

    return err;
}
