#pragma once

#include "hlir.h"
#include "report/report.h"
#include "std/map.h"

typedef struct sema_t sema_t;

typedef enum sema_tags_t {
    eSemaValues,
    eSemaProcs,
    eSemaTypes,
    eSemaModules,

    eSemaMax
} sema_tags_t;

sema_t *sema_new(sema_t *parent, reports_t *reports, size_t decls, size_t *sizes);

reports_t *sema_reports(sema_t *sema);
sema_t *sema_parent(sema_t *sema);

void sema_delete(sema_t *sema);

void sema_set_data(sema_t *sema, void *data);
void *sema_get_data(sema_t *sema);

void sema_set(sema_t *sema, size_t tag, const char *name, void *data);
void *sema_get(sema_t *sema, size_t tag, const char *name);
map_t *sema_tag(sema_t *sema, size_t tag);

typedef struct
{
    reports_t *reports;

    // keep pointers to the entry points for error reporting
    // we can only have one of each
    const hlir_t *cliEntryPoint;
    const hlir_t *guiEntryPoint;
} check_t;

void check_module(check_t *check, hlir_t *mod);
