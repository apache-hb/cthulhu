#pragma once

#include <pthread.h>
#include <stdbool.h>

#include "ctu/util/util.h"

typedef struct {
    void *data;
    void(*func)(void*);
} task_t;

typedef struct {
    pthread_t handle;
    pthread_attr_t attrs;
    task_t task;
} thread_t;

typedef struct {
    size_t num;
    thread_t *threads;
    queue_t *queue;
    atomic_bool running;
} task_pool_t;

task_pool_t *task_pool(size_t len);
void pool_delete(task_pool_t *pool);
void task_run(task_pool_t *pool, task_t task);
