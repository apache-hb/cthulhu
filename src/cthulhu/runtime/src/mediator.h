#pragma once

#include "cthulhu/runtime/runtime.h"

#include "cthulhu/tree/tree.h"

#include "arena/arena.h"
#include "std/map.h"

/// global level

typedef struct mediator_t
{
    arena_t *arena;
} mediator_t;

// per compiler run

typedef struct lifetime_t
{
    mediator_t *parent;
    node_t *builtin;

    logger_t *logger;
    arena_t *arena;

    map_t *extensions;
    map_t *modules;

    tree_cookie_t cookie;
} lifetime_t;

// per language inside a compiler run

typedef struct driver_t
{
    lifetime_t *parent;
    node_t *builtin;
    const language_t *lang;
} driver_t;

// per module in a compiler run

typedef struct context_t
{
    lifetime_t *parent;
    const language_t *lang;

    const char *name;
    void *ast;
    tree_t *tree;
} context_t;

#define EXEC(mod, fn, ...) do { if (mod->fn != NULL) { mod->fn(__VA_ARGS__); } } while (0)
