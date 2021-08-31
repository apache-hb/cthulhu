#include "sema.h"

typedef struct {
    map_t *consts;
    map_t *globals;
    map_t *procs;
} pl0_data_t;

static void *pl0_data_new(void) {
    pl0_data_t *sema = ctu_malloc(sizeof(pl0_data_t));
    sema->consts = map_new(4);
    sema->globals = map_new(4);
    sema->procs = map_new(4);
    return sema;
}

static void pl0_data_delete(void *data) {
    pl0_data_t *sema = data;
    map_delete(sema->consts);
    map_delete(sema->globals);
    map_delete(sema->procs);
    ctu_free(sema);
}

static lir_t *pl0_get_value(sema_t *sema, const char *name) {
    pl0_data_t *data = sema->fields;
    
    lir_t *value = map_get(data->consts, name);
    if (value != NULL) {
        return value;
    }

    lir_t *global = map_get(data->globals, name);
    if (global != NULL) {
        return global;
    }

    return NULL;
}

#define NEW_SEMA(parent) sema_new(parent, pl0_data_new, pl0_data_delete)
#define DELETE_SEMA(sema) sema_delete(sema)
#define GET_VALUE(sema, name) sema_get(sema, name, pl0_get_value)

lir_t *pl0_sema(node_t *node) {
    sema_t *sema = NEW_SEMA(NULL);

    DELETE_SEMA(sema);
} 
