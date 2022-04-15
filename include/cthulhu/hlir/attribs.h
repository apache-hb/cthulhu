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
 * any modifiers for a type
 */
typedef enum {
    /* this type is const, and cannot be modified or assigned */
    TAG_CONST = (1 << 0),
    /* this type is volatile, all modifications, assigns, and accesses are treated as side effects */
    TAG_VOLATILE = (1 << 1),
    /* this type is atomic, treated the same as volatile but also synchronizes */
    TAG_ATOMIC = (1 << 2),

    TAG_TOTAL
} hlir_tags_t;

/**
 * attributes a declaration can have
 */
typedef struct {
    /* the visibility of the current declaration */
    hlir_linkage_t linkage;
    /* any modifiers for the current declaration */
    hlir_tags_t tags; 
} hlir_attributes_t;

#define DEFAULT_LINKAGE (LINK_INTERNAL)
#define DEFAULT_TAGS (0)

hlir_attributes_t *hlir_attributes(hlir_linkage_t linkage, hlir_tags_t tags);
hlir_attributes_t *hlir_linkage(hlir_linkage_t linkage);
hlir_attributes_t *hlir_tags(hlir_tags_t tags);
