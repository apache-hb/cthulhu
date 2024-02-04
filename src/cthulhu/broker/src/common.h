#pragma once

#include "cthulhu/broker/broker.h"

#include "cthulhu/tree/tree.h"

typedef struct language_list_t
{
    const language_t **langs;
    size_t count;
} language_list_t;

typedef struct plugin_list_t
{
    const plugin_t **plugins;
    size_t count;
} plugin_list_t;

typedef struct target_list_t
{
    const target_t **targets;
    size_t count;
} target_list_t;

typedef struct broker_t
{
    const frontend_t *frontend;

    language_list_t languages;
    plugin_list_t plugins;
    target_list_t targets;

    arena_t *arena;
    logger_t *logger;
    node_t *node;
    tree_t *root;
    tree_cookie_t cookie;
} broker_t;

typedef struct language_runtime_t
{
    node_t *builtin;
} language_runtime_t;

typedef struct compile_unit_t
{
    const char *name;
    void *ast;
    tree_t *tree;
} compile_unit_t;
