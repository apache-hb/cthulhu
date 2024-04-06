// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <ctu_config_api.h>

#include "core/analyze.h"
#include "core/compiler.h"

#include <stdbool.h>
#include <stddef.h>

CT_BEGIN_API

typedef struct arena_t arena_t;
typedef struct typevec_t typevec_t;
typedef struct vector_t vector_t;

/// @defgroup config Configuration
/// @brief Configuration system
/// @ingroup common
/// @{

/// @brief the type of a configuration field
typedef enum cfg_type_t
{
#define CFG_TYPE(id, name) id,
#include "config.inc"

    eConfigCount
} cfg_type_t;

typedef struct cfg_field_t cfg_field_t;
typedef struct cfg_group_t cfg_group_t;

typedef enum arg_style_t
{
#define CFG_ARG(id, name, prefix) id,
#include "config.inc"
    eArgCount
} arg_style_t;

typedef struct cfg_arg_t
{
    arg_style_t style;
    const char *arg;
} cfg_arg_t;

// short args are turned into dos args when needed
// long args are discarded on windows
// dos args are discarded on unix

#define ARG_SHORT(name) { .style = eArgShort, .arg = (name) }
#define ARG_LONG(name) { .style = eArgLong, .arg = (name) }
#define ARG_DOS(name) { .style = eArgDOS, .arg = (name) }
#define CT_ARGS(it) { .args = (it), .count = sizeof(it) / sizeof(cfg_arg_t) }

typedef struct cfg_arg_array_t
{
    const cfg_arg_t *args;
    size_t count;
} cfg_arg_array_t;

/// @brief information about a configuration field
/// @note either @a short_args or @a long_args must have at least
/// one argument
typedef struct cfg_info_t
{
    /// @brief the name of this field
    FIELD_STRING const char *name;

    /// @brief a brief description of this field
    FIELD_STRING const char *brief;

    /// @brief the spellings to use for this field
    cfg_arg_array_t args;
} cfg_info_t;

/// @brief an integer field
typedef struct cfg_int_t
{
    /// @brief default value
    FIELD_RANGE(>, min)
    FIELD_RANGE(<, max)
    int initial;

    /// @brief minimum value
    /// @note if min == INT_MIN, there is no minimum
    FIELD_RANGE(<, max) int min;

    /// @brief maximum value
    /// @note if max == INT_MAX, there is no maximum
    FIELD_RANGE(>, min) int max;
} cfg_int_t;

/// @brief a choice in a set of options
typedef struct cfg_choice_t
{
    /// @brief the name of this choice
    FIELD_STRING const char *text;

    /// @brief the value of this choice
    size_t value;
} cfg_choice_t;

/// @brief a choice from a set of options
typedef struct cfg_enum_t
{
    /// @brief the choices in this set
    FIELD_SIZE(count) const cfg_choice_t *options;

    /// @brief the number of choices in this set
    size_t count;

    /// @brief the initial choice
    /// this must match the value of one of the choices
    size_t initial;
} cfg_enum_t;

/// @brief a set of choices
typedef struct cfg_flags_t
{
    /// @brief the choices in this set
    FIELD_SIZE(count) const cfg_choice_t *options;

    /// @brief the number of choices in this set
    size_t count;

    /// @brief the initial set of flags
    /// initial is treated as a bitfield
    size_t initial;
} cfg_flags_t;

/// @brief create a new configuration group
///
/// @param info the information about this group
/// @param arena the allocator to use
///
/// @return the new configuration group
CT_CONFIG_API cfg_group_t *config_root(IN_NOTNULL const cfg_info_t *info, IN_NOTNULL arena_t *arena);

/// @defgroup ConfigAdd Construction
/// @brief Configuration construction API
/// @ingroup Config
/// @{

/// @brief add a new configuration group to a configuration group
///
/// @param group the configuration group to add the group to
/// @param info the information about this group
///
/// @return the new subgroup
CT_CONFIG_API cfg_group_t *config_group(IN_NOTNULL cfg_group_t *group, IN_NOTNULL const cfg_info_t *info);

/// @brief add a new integer field to a configuration group
///
/// @param group the configuration group to add the field to
/// @param info the information about this field
/// @param cfg the configuration information for this field
///
/// @return the new configuration field
CT_CONFIG_API cfg_field_t *config_int(IN_NOTNULL cfg_group_t *group, IN_NOTNULL const cfg_info_t *info, cfg_int_t cfg);

/// @brief add a new yes/no field to a configuration group
///
/// @param group the configuration group to add the field to
/// @param info the information about this field
/// @param initial the initial value for this field
///
/// @return the new configuration field
CT_CONFIG_API cfg_field_t *config_bool(IN_NOTNULL cfg_group_t *group, IN_NOTNULL const cfg_info_t *info, bool initial);

/// @brief add a new string field to a configuration group
///
/// @param group the configuration group to add the field to
/// @param info the information about this field
/// @param initial the initial value for this field
///
/// @return the new configuration field
CT_CONFIG_API cfg_field_t *config_string(IN_NOTNULL cfg_group_t *group, IN_NOTNULL const cfg_info_t *info, const char *initial);

/// @brief add a new vector field to a configuration group
///
/// @param group the configuration group to add the field to
/// @param info the information about this field
/// @param initial the initial values for this field
///
/// @return the new configuration field
CT_CONFIG_API cfg_field_t *config_vector(IN_NOTNULL cfg_group_t *group, IN_NOTNULL const cfg_info_t *info, vector_t *initial);

/// @brief add a new choice field to a configuration group
///
/// @param group the configuration group to add the field to
/// @param info the information about this field
/// @param cfg the configuration information for this field
///
/// @return the new configuration field
CT_CONFIG_API cfg_field_t *config_enum(IN_NOTNULL cfg_group_t *group, IN_NOTNULL const cfg_info_t *info, cfg_enum_t cfg);

/// @brief add a new flags field to a configuration group
///
/// @param group the configuration group to add the field to
/// @param info the information about this field
/// @param cfg the configuration information for this field
///
/// @return the new configuration field
CT_CONFIG_API cfg_field_t *config_flags(IN_NOTNULL cfg_group_t *group, IN_NOTNULL const cfg_info_t *info, cfg_flags_t cfg);

/// @} // ConfigAdd

/// @defgroup ConfigReflect Reflection
/// @brief Reflection API for configuration
/// @ingroup Config
/// @{

/// @brief get the type of a configuration field
///
/// @param field the field to get the type of
///
/// @return the type of @p field
CT_PUREFN
CT_CONFIG_API cfg_type_t cfg_get_type(IN_NOTNULL const cfg_field_t *field);

/// @brief get the information about a configuration field
///
/// @param field the field to get the information about
///
/// @return the information about @p field
CT_PUREFN RET_NOTNULL
CT_CONFIG_API const cfg_info_t *cfg_get_info(IN_NOTNULL const cfg_field_t *field);

/// @brief get the information about a configuration group
///
/// @param config the configuration group to get the information about
///
/// @return the information about @p config
CT_PUREFN RET_NOTNULL
CT_CONFIG_API const cfg_info_t *cfg_group_info(IN_NOTNULL const cfg_group_t *config);

/// @brief get the information about an integer field
///
/// @param field the field to get the information about
///
/// @return the information about @p field
CT_PUREFN RET_NOTNULL
CT_CONFIG_API const cfg_int_t *cfg_int_info(IN_NOTNULL const cfg_field_t *field);

/// @brief get the information about a yes/no field
///
/// @param field the field to get the information about
///
/// @return the information about @p field
CT_PUREFN
CT_CONFIG_API bool cfg_bool_info(IN_NOTNULL const cfg_field_t *field);

/// @brief get the information about a string field
///
/// @param field the field to get the information about
///
/// @return the information about @p field
CT_PUREFN
CT_CONFIG_API const char *cfg_string_info(IN_NOTNULL const cfg_field_t *field);

/// @brief get the information about a vector field
///
/// @param field the field to get the information about
///
/// @return the information about @p field
CT_PUREFN
CT_CONFIG_API const vector_t *cfg_vector_info(IN_NOTNULL const cfg_field_t *field);

/// @brief get the information about a choice field
///
/// @param field the field to get the information about
///
/// @return the information about @p field
CT_PUREFN RET_NOTNULL
CT_CONFIG_API const cfg_enum_t *cfg_enum_info(IN_NOTNULL const cfg_field_t *field);

/// @brief get the information about a flags field
///
/// @param field the field to get the information about
///
/// @return the information about @p field
CT_PUREFN RET_NOTNULL
CT_CONFIG_API const cfg_flags_t *cfg_flags_info(IN_NOTNULL const cfg_field_t *field);

/// @brief get the name of a configuration type
///
/// @param type the type to get the name of
///
/// @return the name of @p type
CT_CONSTFN RET_NOTNULL
CT_CONFIG_API const char *cfg_type_string(IN_RANGE(<, eConfigCount) cfg_type_t type);

/// @brief get the name of an argument style
///
/// @param style the style to get the name of
///
/// @return the name of @p style
CT_CONSTFN RET_NOTNULL
CT_CONFIG_API const char *cfg_arg_string(IN_RANGE(<, eArgCount) arg_style_t style);

/// @brief get the prefix for an argument style
///
/// @param style the style to get the prefix for
///
/// @return the prefix for @p style
CT_CONSTFN RET_NOTNULL
CT_CONFIG_API const char *cfg_arg_prefix(IN_RANGE(<, eArgCount) arg_style_t style);

/// @brief get all subgroups in a configuration group
///
/// @param config the configuration group to get the subgroups from
///
/// @return the subgroups in @p config
CT_PUREFN RET_NOTNULL
CT_CONFIG_API typevec_t *cfg_get_groups(IN_NOTNULL const cfg_group_t *config);

/// @brief get all fields in a configuration group
///
/// @param config the configuration group to get the fields from
///
/// @return the fields in @p config
CT_PUREFN RET_NOTNULL
CT_CONFIG_API vector_t *cfg_get_fields(IN_NOTNULL const cfg_group_t *config);

/// @} // ConfigReflect

/// @defgroup ConfigRead Reading
/// @brief Reading API for configuration
/// @ingroup Config
/// @{

/// @brief get the current integer value of a configuration field
///
/// @param field the field to get the value of
///
/// @return the current value of @p field
CT_PUREFN
CT_CONFIG_API int cfg_int_value(IN_NOTNULL const cfg_field_t *field);

/// @brief get the current boolean value of a configuration field
///
/// @param field the field to get the value of
///
/// @return the current value of @p field
CT_PUREFN
CT_CONFIG_API bool cfg_bool_value(IN_NOTNULL const cfg_field_t *field);

/// @brief get the current string value of a configuration field
///
/// @param field the field to get the value of
///
/// @return the current value of @p field
CT_PUREFN
CT_CONFIG_API const char *cfg_string_value(IN_NOTNULL const cfg_field_t *field);

/// @brief get the current vector value of a configuration field
///
/// @param field the field to get the value of
///
/// @return the current value of @p field
CT_PUREFN
CT_CONFIG_API vector_t *cfg_vector_value(IN_NOTNULL const cfg_field_t *field);

/// @brief get the current enum value of a configuration field
///
/// @param field the field to get the value of
///
/// @return the current value of @p field
CT_PUREFN
CT_CONFIG_API size_t cfg_enum_value(IN_NOTNULL const cfg_field_t *field);

/// @brief get the current flags value of a configuration field
///
/// @param field the field to get the value of
///
/// @return the current value of @p field
CT_PUREFN
CT_CONFIG_API size_t cfg_flags_value(IN_NOTNULL const cfg_field_t *field);

/// @} // ConfigRead

/// @defgroup ConfigWrite Writing
/// @brief Writing API for configuration
/// @ingroup Config
/// @{

/// @brief set the current value of an int field
///
/// @param field the field to set the value of
/// @param value the new value
///
/// @return true if the value was valid, false otherwise
CT_NODISCARD
CT_CONFIG_API bool cfg_set_int(IN_NOTNULL cfg_field_t *field, int value);

/// @brief set the current value of a bool field
///
/// @param field the field to set the value of
/// @param value the new value
CT_CONFIG_API void cfg_set_bool(IN_NOTNULL cfg_field_t *field, bool value);

/// @brief set the current value of a string field
///
/// @param field the field to set the value of
/// @param value the new value
CT_CONFIG_API void cfg_set_string(IN_NOTNULL cfg_field_t *field, char *value);

/// @brief push a new value onto an array field
///
/// @param field the field to push the value onto
/// @param value the new value
CT_CONFIG_API void cfg_vector_push(IN_NOTNULL cfg_field_t *field, char *value);

/// @brief set the current value of an enum field
/// set the value via a string name
///
/// @param field the field to set the value of
/// @param choice the name of the choice to set
///
/// @retval true if the choice was valid and the field was updated,
/// @retval false otherwise
CT_CONFIG_API bool cfg_set_enum(IN_NOTNULL cfg_field_t *field, const char *choice);

/// @brief set the current value of an enum field
/// set the value via an integer value
/// @pre @p value must be a valid value from the enum choices
/// @warning this will assert if @p value is not a valid value from the flags choices
///
/// @param field the field to set the value of
/// @param value the value to set
CT_CONFIG_API void cfg_set_enum_value(IN_NOTNULL cfg_field_t *field, size_t value);

/// @brief set the current value of a flags field
/// set the value via a string name
///
/// @param field the field to set the value of
/// @param choice the name of the choice to set
/// @param set true to set the flag, false to clear it
///
/// @retval true if the choice was valid and the field was updated
/// @retval false otherwise
CT_NODISCARD
CT_CONFIG_API bool cfg_set_flag(IN_NOTNULL cfg_field_t *field, const char *choice, bool set);

/// @brief set the current value of a flags field
/// set the value via an integer value
/// @pre @p value must be a valid value from the flags choices
/// @warning this will assert if @p value is not a valid value from the flags choices
///
/// @param field the field to set the value of
/// @param value the value to set
CT_CONFIG_API void cfg_set_flag_value(IN_NOTNULL cfg_field_t *field, size_t value);

/// @} // ConfigWrite

/// @} // Config

CT_END_API
