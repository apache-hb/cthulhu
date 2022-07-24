#pragma once

#include <stdbool.h>

/**
 * @defgroup HlirAttributes HLIR Declaration attributes
 * @brief attributes to attatch to declarations and types
 * @{
 */

/**
 * @brief the visibility of a declaration
 */
typedef enum
{
#define HLIR_LINKAGE(ID, STR) ID,
#include "hlir-def.inc"
    eLinkTotal
} hlir_linkage_t;

typedef enum 
{
#define HLIR_VISIBILITY(ID, STR) ID,
#include "hlir-def.inc"
    eHlirVisibilityTotal
} hlir_visibility_t;

/**
 * @brief any modifiers for a type
 */
typedef enum
{
#define TYPE_QUALIFIER(ID, NAME, BIT) ID = (BIT),
#include "hlir-def.inc"
} hlir_tags_t;

/**
 * @brief attributes a declaration can have
 */
typedef struct
{
    hlir_linkage_t linkage; ///< the linkage of the current declaration
    hlir_visibility_t visibility; ///< the visibility of the current declaration
    hlir_tags_t tags;       ///< any modifiers for the current declaration
    const char *mangle;     ///< the name to use for the current declaration
} hlir_attributes_t;

#define DEFAULT_LINKAGE (eLinkInternal) ///< the default linkage for a declaration
#define DEFAULT_TAGS (0)                ///< the default tags for a declaration

/**
 * @brief create a new attributes object
 *
 * @param linkage the linkage of the declaration
 * @param tags the tags of the declaration
 * @return a new attributes object
 */
hlir_attributes_t *hlir_attributes(hlir_linkage_t linkage, hlir_tags_t tags, const char *name);

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

bool is_entry_point(hlir_linkage_t linkage);

/** @} */
