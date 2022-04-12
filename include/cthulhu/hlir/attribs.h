#pragma once

/**
 * the visibility of a declaration
 */
typedef enum {
    /* this declaration is external, and is not part of this compilation unit */
    LINK_IMPORTED,

    /* this declaration is internal, and is visibile outside of this compilation unit */
    LINK_EXPORTED,

    /* this declaration is internal, and is not visible outside of this compilation unit */
    LINK_INTERNAL,

    LINK_TOTAL
} hlir_linkage_t;

/**
 * attributes a declaration can have
 */
typedef struct {
    /* the visibility of the current declaration */
    hlir_linkage_t linkage; 
} hlir_attributes_t;

hlir_attributes_t *hlir_new_attributes(hlir_linkage_t linkage);
