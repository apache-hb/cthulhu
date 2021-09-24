#pragma once

#include "lir.h"
#include "ctu/util/report.h"

typedef void(*sema_delete_t)(void*);

typedef struct sema_t {
    struct sema_t *parent;
    reports_t *reports;

    /** vector_t<map_t<const char *, lir_t*>*> */
    vector_t *decls;

    void *data; /// user data
} sema_t;

/**
 * create a new semantic environment
 * 
 * @param parent the parent environment or NULL if this is the root
 * @param reports the reporting context to use
 * @param data user defined data
 * @param size the estimated size of the context layer
 * 
 * @return the new semantic environment
 */
sema_t *sema_new(sema_t *parent, 
                 reports_t *reports, 
                 size_t decls,
                 map_size_t size);

/**
 * delete a semantic enviroment. user data will not be deleted.
 * 
 * @param sema the semantic environment to delete
 */
void sema_delete(sema_t *sema);

/**
 * get and set user defined data
 */
void sema_set_data(sema_t *sema, void *data);
void *sema_get_data(sema_t *sema);

void sema_set(sema_t *sema, size_t tag, const char *name, lir_t *lir);
lir_t *sema_get(sema_t *sema, size_t tag, const char *name);
map_t *sema_tag(sema_t *sema, size_t tag);
