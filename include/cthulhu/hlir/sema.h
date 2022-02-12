#pragma once

#include "cthulhu/util/report.h"

#include "hlir.h"

typedef struct sema_t {
    struct sema_t *parent;
    reports_t *reports;

    /**
     * an array of maps
     * each map is its own namespace which maps from symbol name to hlir node
     */
    vector_t *decls;

    void *data;
} sema_t;

sema_t *sema_new(sema_t *parent, reports_t *reports, size_t decls, size_t *sizes);

void sema_delete(sema_t *sema);

void sema_set_data(sema_t *sema, void *data);
void *sema_get_data(sema_t *sema);

void sema_set(sema_t *sema, size_t tag, const char *name, hlir_t *hlir);
hlir_t *sema_get(sema_t *sema, size_t tag, const char *name);
hlir_t *sema_get_with_depth(sema_t *sema, size_t tag, const char *name, size_t *depth);
map_t *sema_tag(sema_t *sema, size_t tag);

void check_module(reports_t *reports, hlir_t *mod);
