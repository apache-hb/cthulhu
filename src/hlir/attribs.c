#include "cthulhu/hlir/hlir.h"

hlir_attributes_t *hlir_attributes(hlir_linkage_t linkage, hlir_tags_t tags) {
    hlir_attributes_t *attributes = ctu_malloc(sizeof(hlir_attributes_t));
    attributes->linkage = linkage;
    attributes->tags = tags;
    return attributes;
}

hlir_attributes_t *hlir_linkage(hlir_linkage_t linkage) {
    return hlir_attributes(linkage, DEFAULT_TAGS);
}

hlir_attributes_t *hlir_tags(hlir_tags_t tags) {
    return hlir_attributes(DEFAULT_LINKAGE, tags);
}
