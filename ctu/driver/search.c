#include "search.h"

#include "ctu/util/util.h"

static vector_t *paths = NULL;

void init_fs(void) {
    paths = vector_new(1);
    vector_push(&paths, ".");
}
