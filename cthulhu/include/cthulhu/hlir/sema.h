#pragma once

#include <stddef.h>

typedef struct hlir_t hlir_t;
typedef struct reports_t reports_t;
typedef struct map_t map_t;

typedef struct sema_t sema_t;

typedef enum sema_tags_t {
    eSemaValues, // hlir_t *
    eSemaProcs, // hlir_t *
    eSemaTypes, // hlir_t *
    eSemaModules, // sema_t *

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
