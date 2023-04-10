#pragma once

#include <stddef.h>

#include "base/analyze.h"

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

sema_t *sema_root_new(reports_t *reports, size_t decls, size_t *sizes);
sema_t *sema_new(sema_t *parent, size_t decls, size_t *sizes);

/**
 * @brief badly named, this is a version of sema_new that checks the sizes of the
 *        decls to make sure no maps are created with 0 buckets
 * 
 * @param parent 
 * @param decls 
 * @param sizes 
 * @return sema_t* 
 */
sema_t *sema_new_checked(sema_t *parent, size_t decls, size_t *sizes);

NODISCARD PUREFN
reports_t *sema_reports(sema_t *sema);

NODISCARD PUREFN
sema_t *sema_parent(sema_t *sema);

void sema_delete(sema_t *sema);

void sema_set_data(sema_t *sema, void *data);
void *sema_get_data(sema_t *sema);

void sema_set(sema_t *sema, size_t tag, const char *name, void *data);

NODISCARD PUREFN
void *sema_get(sema_t *sema, size_t tag, const char *name);

NODISCARD PUREFN
map_t *sema_tag(sema_t *sema, size_t tag);

typedef hlir_t *(*sema_resolve_t)(sema_t *, void *);

hlir_t *sema_resolve(sema_t *root, hlir_t *unresolved, sema_resolve_t resolve);
