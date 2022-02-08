#include "cthulhu/hlir/hlir.h"

bool hlir_is_imported(const hlir_t *self) {
    return self->attributes->linkage == LINK_IMPORTED;
}

hlir_attributes_t *hlir_new_attributes(hlir_linkage_t linkage) {
    hlir_attributes_t *self = ctu_malloc(sizeof(hlir_attributes_t));
    self->linkage = linkage;
    return self;
}
