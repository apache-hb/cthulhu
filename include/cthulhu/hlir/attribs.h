#pragma once

/**
 * @defgroup HlirAttributes HLIR Declaration attributes
 * @{
 */

/**
 * @brief the visibility of a declaration
 */
typedef enum {
    LINK_IMPORTED, ///< this declaration is external, and is not part of this compilation unit
    LINK_EXPORTED, ///< this declaration is internal, and is visibile outside of this compilation unit
    LINK_INTERNAL, ///< this declaration is internal, and is not visible outside of this compilation unit

    LINK_TOTAL
} hlir_linkage_t;

/**
 * @brief any modifiers for a type
 */
typedef enum {
    TAG_CONST = (1 << 0), ///< this type is const, and cannot be modified or assigned
    TAG_VOLATILE = (1 << 1), ///< this type is volatile, all modifications, assigns, and accesses are treated as side effects
    TAG_ATOMIC = (1 << 2), ///< this type is atomic, treated the same as volatile but also synchronizes

    TAG_TOTAL
} hlir_tags_t;

/**
 * @brief attributes a declaration can have
 */
typedef struct {
    hlir_linkage_t linkage; ///< the visibility of the current declaration
    hlir_tags_t tags; ///< any modifiers for the current declaration
} hlir_attributes_t;

#define DEFAULT_LINKAGE (LINK_INTERNAL) ///< the default linkage for a declaration
#define DEFAULT_TAGS (0) ///< the default tags for a declaration

/**
 * @brief create a new attributes object
 * 
 * @param linkage the linkage of the declaration
 * @param tags the tags of the declaration
 * @return a new attributes object
 */
hlir_attributes_t *hlir_attributes(hlir_linkage_t linkage, hlir_tags_t tags);

/**
 * @brief create a new attributes object with the default tags
 * 
 * @param linkage the linkage of the declaration
 * @return the new attributes object
 */
hlir_attributes_t *hlir_linkage(hlir_linkage_t linkage);

/**
 * @brief create a new attributes object with the default linkage
 * 
 * @param tags the tags of the declaration
 * @return the new attributes object
 */
hlir_attributes_t *hlir_tags(hlir_tags_t tags);

/** @} */
