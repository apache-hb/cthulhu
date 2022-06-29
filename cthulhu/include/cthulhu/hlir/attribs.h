#pragma once

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
    eLinkImported, ///< this declaration is external, and is not part of this
                   ///< compilation unit
    eLinkExported, ///< this declaration is internal, and is visibile outside of
                   ///< this compilation unit
    eLinkInternal, ///< this declaration is internal, and is not visible outside
                   ///< of this compilation unit

    eLinkEntryCli, ///< standard cli entry point
    eLinkEntryGui, ///< windows gui entry point

    eLinkTotal
} hlir_linkage_t;

/**
 * @brief any modifiers for a type
 */
typedef enum
{
    eTagConst = (1 << 0),    ///< this type is const, and cannot be modified or assigned
    eTagVolatile = (1 << 1), ///< this type is volatile, all modifications, assigns,
                             ///  and accesses are treated as side effects
    eTagAtomic = (1 << 2),   ///< this type is atomic, treated the same as
                             ///< volatile but also synchronizes

    eTagTotal
} hlir_tags_t;

/**
 * @brief attributes a declaration can have
 */
typedef struct
{
    hlir_linkage_t linkage; ///< the visibility of the current declaration
    hlir_tags_t tags;       ///< any modifiers for the current declaration
    const char *mangle;     ///< the name to use for the current declaration
    const char *module;     ///< the module name this comes from if its an external
                            ///< declaration
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
hlir_attributes_t *hlir_attributes(hlir_linkage_t linkage, hlir_tags_t tags, const char *name, const char *module);

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
