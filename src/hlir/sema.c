#include "cthulhu/hlir/sema.h"

sema_t *sema_new(sema_t *parent, reports_t *reports, size_t decls, size_t *sizes) {
    sema_t *sema = ctu_malloc(sizeof(sema_t));
    
    sema->parent = parent;
    sema->reports = reports;
    
    sema->decls = vector_of(decls);
    for (size_t i = 0; i < decls; i++) {
        map_t *map = optimal_map(sizes[i]);
        vector_set(sema->decls, i, map);
    }
    
    return sema;
}

void sema_delete(sema_t *sema) {
    ctu_free(sema);
}

void sema_set_data(sema_t *sema, void *data) {
    sema->data = data;
}

void *sema_get_data(sema_t *sema) {
    return sema->data;
}

void sema_set(sema_t *sema, size_t tag, const char *name, hlir_t *hlir) {
    map_t *map = sema_tag(sema, tag);
    map_set(map, name, hlir);
}

hlir_t *sema_get(sema_t *sema, size_t tag, const char *name) {
    map_t *map = sema_tag(sema, tag);

    hlir_t *hlir = map_get(map, name);
    if (hlir != NULL) {
        return hlir;
    }

    if (sema->parent != NULL) {
        return sema_get(sema->parent, tag, name);
    }

    return NULL;
}

map_t *sema_tag(sema_t *sema, size_t tag) {
    return vector_get(sema->decls, tag);
}
