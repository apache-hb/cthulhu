#pragma once

#include "lir.h"
#include "ctu/util/report.h"

typedef void(*sema_delete_t)(void*);

typedef struct sema_t {
    struct sema_t *parent;

    reports_t *reports;

    /* user provided fields */
    void *fields;
} sema_t;

typedef lir_t*(*sema_get_t)(sema_t *sema, const char *name);
typedef void(*sema_set_t)(sema_t *sema, const char *name, lir_t *lir);

/**
 * create a new semantic environment
 * 
 * @param parent the parent environment or NULL if this is the root
 * @param reports the reporting context to use
 * @param data user defined data
 * 
 * @return the new semantic environment
 */
sema_t *sema_new(sema_t *parent, reports_t *reports, void *data);

/**
 * delete a semantic enviroment. user data will not be deleted.
 * 
 * @param sema the semantic environment to delete
 */
void sema_delete(sema_t *sema);

/**
 * get the user provided data from the semantic environment
 * 
 * @param sema the semantic environment
 * 
 * @return the user provided data
 */
void *sema_data(sema_t *sema);

void sema_set(sema_t *sema, const char *name, lir_t *lir, sema_set_t set);
lir_t *sema_get(sema_t *sema, const char *name, sema_get_t get);
