#include "task.h"

#include "ctu/util/util.h"
#include "ctu/util/report.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    size_t id;
    task_pool_t *pool;
} worker_arg_t;

static void *thread_worker(void *arg) {
    worker_arg_t *data = arg;
    task_pool_t *pool = data->pool;

    logverbose("worker-thread %zu started", data->id);

    while (pool->running) {
        task_t *task = queue_read(pool->queue);
        if (task == NULL) {
            continue;
        }

        task->func(task->data);
    }

    return NULL;
}

static void start_thread(task_pool_t *pool, thread_t *it, int id) {
    worker_arg_t *data = ctu_malloc(sizeof(worker_arg_t));
    data->id = id;
    data->pool = pool;
    int err = pthread_create(&it->handle, NULL, thread_worker, data);
    if (err != 0) {
        printf("oh no %s\n", strerror(errno));
    }
}

task_pool_t *task_pool(size_t len) {
    task_pool_t *pool = ctu_malloc(sizeof(task_pool_t));
    pool->num = len;
    pool->threads = ctu_malloc(sizeof(thread_t) * len);
    pool->queue = queue_new(len);
    pool->running = true;
    
    for (size_t i = 0; i < len; i++) {
        start_thread(pool, &pool->threads[i], i);
    }

    return pool;
}

void pool_delete(task_pool_t *pool) {
    /* finish the tasks */
    while (!queue_is_empty(pool->queue)) { }

    pool->running = false;

    for (size_t i = 0; i < pool->num; i++) {
        pthread_join(pool->threads[i].handle, NULL);
    }
}

void task_run(task_pool_t *pool, task_t task) {
    task_t *temp = ctu_malloc(sizeof(task_t));
    *temp = task;

    /* block until there is room in the queue */
    queue_write(pool->queue, temp, true);
}
