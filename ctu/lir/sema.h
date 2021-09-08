#pragma once

#include "lir.h"

typedef void*(*sema_new_t)(void);
typedef void(*sema_delete_t)(void*);

typedef struct sema_t {
    struct sema_t *parent;

    /* user provided fields */
    void *fields;
} sema_t;

typedef lir_t*(*sema_get_t)(sema_t *sema, const char *name);
typedef void(*sema_set_t)(sema_t *sema, const char *name, lir_t *lir);

sema_t *sema_new(sema_t *parent, sema_new_t create);
void sema_delete(sema_t *sema, sema_delete_t destroy);

void sema_set(sema_t *sema, const char *name, lir_t *lir, sema_set_t set);
lir_t *sema_get(sema_t *sema, const char *name, sema_get_t get);
