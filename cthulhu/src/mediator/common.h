#pragma once

#include "cthulhu/mediator/common.h"

#include "cthulhu/tree/tree.h"

#include "std/map.h"

/// global level

typedef struct mediator_t
{
    arena_t *arena;

    const char *id;
    version_info_t version;
} mediator_t;

// per compiler run

typedef struct lifetime_t
{
    mediator_t *parent;

    logger_t *logger;
    arena_t *alloc;

    map_t *extensions;
    map_t *modules;

    cookie_t cookie;
} lifetime_t;

// per language inside a compiler run

typedef struct driver_t
{
    lifetime_t *parent;
    const language_t *lang;
} driver_t;

// per module in a compiler run

typedef struct context_t
{
    lifetime_t *parent;
    const language_t *lang;

    const char *name;
    void *ast;
    tree_t *root;
} context_t;

bool context_requires_compiling(const context_t *ctx);

#define EXEC(mod, fn, ...) do { if (mod->fn != NULL) { mod->fn(__VA_ARGS__); } } while (0)
