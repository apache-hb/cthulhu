#include "async.h"

#include "ctu/util/task/task.h"
#include "driver.h"

typedef struct {
    file_t *file;
} task_data_t;

static void async_task(void *arg) {
    task_data_t *data = arg;

    (void)data;
}

int async_main(int threads, vector_t *sources) {
    size_t len = vector_len(sources);
    queue_t *queue = queue_new(len);

    task_pool_t pool = task_pool(threads);

    task_t task = { NULL, async_task }; 

    task_run(&pool, task);

    (void)threads;
    (void)queue;

    return 0;
}
