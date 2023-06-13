#pragma once

#include "cthulhu/mediator/common.h"

#include "std/map.h"

typedef struct sema_t sema_t;
typedef struct reports_t reports_t;

typedef struct mediator_t
{
    const char *id;
    version_info_t version;
} mediator_t;

typedef struct lifetime_t 
{
    mediator_t *parent;

    reports_t *reports;

    map_t *extensions;

    map_t *modules;
} lifetime_t;

typedef struct handle_t 
{
    lifetime_t *parent;
    const language_t *lang;
} handle_t;

typedef struct context_t 
{
    lifetime_t *parent;
    const language_t *lang;

    const char *name;
    void *ast;
    hlir_t *root;
    sema_t *sema;
} context_t;

bool context_requires_compiling(const context_t *ctx);

#define EXEC(mod, fn, ...) do { if (mod->fn != NULL) { logverbose("%s:" #fn "()", mod->id); mod->fn(__VA_ARGS__); } } while (0)
