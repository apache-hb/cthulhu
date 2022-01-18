#include "sema.h"

hlir_t *pl0_sema(reports_t *reports, void *node) {
    pl0_t *root = node;
    report(reports, INTERNAL, root->node, "pl0-sema unimplemented");
    return NULL;
}
