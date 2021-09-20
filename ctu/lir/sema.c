#include "sema.h"

#include "ctu/util/report.h"
#include "ctu/util/str.h"

sema_t *sema_new(sema_t *parent, reports_t *reports, size_t decls) {
    sema_t *sema = NEW(sema_t);
    
    sema->parent = parent;
    sema->reports = reports;

    sema->decls = vector_of(decls);
    for (size_t i = 0; i < decls; i++) {
        map_t *map = map_new(MAP_SMALL);
        vector_set(sema->decls, i, map);
    }

    return sema;
}

void sema_delete(sema_t *sema) {
    DELETE(sema);
}

void sema_set_data(sema_t *sema, void *data) {
    sema->data = data;
}

void *sema_get_data(sema_t *sema) {
    return sema->data;
}

map_t *sema_tag(sema_t *sema, size_t tag) {
    return vector_get(sema->decls, tag);
}

void sema_set(sema_t *sema, size_t tag, const char *name, lir_t *lir) {
    map_t *map = sema_tag(sema, tag);
    map_set(map, name, lir);
}

lir_t *sema_get(sema_t *sema, size_t tag, const char *name) {
    map_t *map = sema_tag(sema, tag);

    lir_t *lir = map_get(map, name);
    if (lir != NULL) {
        return lir;
    }

    if (sema->parent != NULL) {
        return sema_get(sema->parent, tag, name);
    }

    return NULL;
}
