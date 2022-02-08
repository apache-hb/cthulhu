#pragma once

typedef enum {
    LINK_IMPORTED,
    LINK_EXPORTED,
    LINK_INTERNAL
} hlir_linkage_t;

typedef struct {
    hlir_linkage_t linkage;
} hlir_attributes_t;

hlir_attributes_t *hlir_new_attributes(hlir_linkage_t linkage);
