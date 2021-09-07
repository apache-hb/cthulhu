#include "sema.h"

#include "ctu/util/report.h"
#include "ctu/util/str.h"

sema_t *sema_new(sema_t *parent, reports_t *reports, sema_new_t create) {
    sema_t *sema = ctu_malloc(sizeof(sema_t));
    
    sema->parent = parent;
    sema->reports = reports;
    sema->fields = create();

    return sema;
}

void sema_delete(sema_t *sema, sema_delete_t destroy) {
    destroy(sema->fields);
}

void sema_set(sema_t *sema, const char *name, lir_t *lir, sema_set_t set) {
    set(sema, name, lir);
}

lir_t *sema_get(sema_t *sema, const char *name, sema_get_t get) {
    lir_t *lir = get(sema, name);
    if (lir != NULL) {
        return lir;
    }

    if (sema->parent != NULL) {
        return sema_get(sema->parent, name, get);
    }

    return NULL;
}
