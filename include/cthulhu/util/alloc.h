#pragma once

typedef struct
{
    void *nothing;
} alloc_t;

typedef struct
{
    alloc_t *nodeAlloc;
    alloc_t *hlirAlloc;
    alloc_t *userAlloc;
} alloc_config_t;
