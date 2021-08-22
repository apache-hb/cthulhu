#include "sema.h"

typedef struct sema_t {
    traits_t *traits;
    struct sema_t *parent;
} sema_t;

static sema_t *sema_new(traits_t *traits, sema_t *parent) {
    sema_t *sema = ctu_malloc(sizeof(sema_t));
    sema->traits = traits;
    sema->parent = parent;
    return sema;
}

static node_t *sema_query(sema_t *sema, node_t *name) {

}

void sema_program(traits_t *traits, node_t *program) {
    sema_t *sema = sema_new(traits, NULL);
}
