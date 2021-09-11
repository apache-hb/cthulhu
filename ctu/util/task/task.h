#pragma once

#include <pthread.h>

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
} task_pool_t;

task_pool_t task_pool(size_t len);
void task_run(task_pool_t *pool, task_t task);
