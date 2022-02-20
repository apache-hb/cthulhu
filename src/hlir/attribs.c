#include "cthulhu/hlir/hlir.h"

hlir_attributes_t *hlir_new_attributes(hlir_linkage_t linkage) {
    hlir_attributes_t *self = ctu_malloc(sizeof(hlir_attributes_t));
    self->linkage = linkage;
    return self;
}
