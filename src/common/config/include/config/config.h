#pragma once

#include "core/compiler.h"

#include <stdbool.h>
#include <stddef.h>

BEGIN_API

/// @defgroup Config Configuration
/// @brief Configuration system
/// @ingroup Common
/// @{

typedef struct arena_t arena_t;
typedef struct typevec_t typevec_t;
typedef struct vector_t vector_t;

/// @brief the type of a configuration field
typedef enum cfg_type_t
{
    /// an integer field
    eConfigInt,

    /// a yes/no field
    eConfigBool,

    /// a string field
    eConfigString,

    /// a vector of strings
    eConfigVector,

    /// a choice from a set of options
    eConfigEnum,

    /// one or more choices from a set of options
    eConfigFlags,

    eConfigTotal
} cfg_type_t;

typedef struct cfg_field_t cfg_field_t;
typedef struct config_t config_t;

/// @brief information about a configuration field
/// @note either @a short_args or @a long_args must have at least
/// one argument
typedef struct cfg_info_t
{
    /// @brief the name of this field
    const char *name;

    /// @brief a brief description of this field
    const char *brief;

    /// @brief a null terminated list of short argument names
    const char *const *short_args;

    /// @brief a null terminated list of long argument names
    const char *const *long_args;
} cfg_info_t;

/// @brief an integer field
typedef struct cfg_int_t
{
    /// @brief default value
    int initial;

    /// @brief minimum value
    /// @note if min == INT_MIN, there is no minimum
    int min;

    /// @brief maximum value
    /// @note if max == INT_MAX, there is no maximum
    int max;
} cfg_int_t;

/// @brief a yes/no field
typedef struct cfg_bool_t
{
    /// @brief default value
    bool initial;
} cfg_bool_t;

/// @brief a string field
typedef struct cfg_string_t
{
    /// @brief default value
    const char *initial;
} cfg_string_t;

/// @brief a choice in a set of options
typedef struct cfg_choice_t
{
    /// @brief the name of this choice
    const char *text;

    /// @brief the value of this choice
    size_t value;
} cfg_choice_t;

/// @brief a choice from a set of options
typedef struct cfg_enum_t
{
    /// @brief the choices in this set
    const cfg_choice_t *options;

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
    const cfg_choice_t *options;

    /// @brief the number of choices in this set
    size_t count;

    /// @brief the initial set of flags
    /// initial is treated as a bitfield
    size_t initial;
} cfg_flags_t;

/// @brief create a new configuration group
///
/// @param arena the allocator to use
/// @param info the information about this group
///
/// @return the new configuration group
config_t *config_new(arena_t *arena, const cfg_info_t *info);

/// @defgroup ConfigAdd Construction
/// @brief Configuration construction API
/// @ingroup Config
/// @{

/// @brief add a new integer field to a configuration group
///
/// @param group the configuration group to add the field to
/// @param info the information about this field
/// @param cfg the configuration information for this field
///
/// @return the new configuration field
cfg_field_t *config_int(config_t *group, const cfg_info_t *info, cfg_int_t cfg);

/// @brief add a new yes/no field to a configuration group
///
/// @param group the configuration group to add the field to
/// @param info the information about this field
/// @param cfg the configuration information for this field
///
/// @return the new configuration field
cfg_field_t *config_bool(config_t *group, const cfg_info_t *info, cfg_bool_t cfg);

/// @brief add a new string field to a configuration group
///
/// @param group the configuration group to add the field to
/// @param info the information about this field
/// @param cfg the configuration information for this field
///
/// @return the new configuration field
cfg_field_t *config_string(config_t *group, const cfg_info_t *info, cfg_string_t cfg);

cfg_field_t *config_vector(config_t *group, const cfg_info_t *info, vector_t *initial);

/// @brief add a new choice field to a configuration group
///
/// @param group the configuration group to add the field to
/// @param info the information about this field
/// @param cfg the configuration information for this field
///
/// @return the new configuration field
cfg_field_t *config_enum(config_t *group, const cfg_info_t *info, cfg_enum_t cfg);

/// @brief add a new flags field to a configuration group
///
/// @param group the configuration group to add the field to
/// @param info the information about this field
/// @param cfg the configuration information for this field
///
/// @return the new configuration field
cfg_field_t *config_flags(config_t *group, const cfg_info_t *info, cfg_flags_t cfg);

/// @brief add a new configuration group to a configuration group
///
/// @param group the configuration group to add the group to
/// @param info the information about this group
///
/// @return the new subgroup
config_t *config_group(config_t *group, const cfg_info_t *info);

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
cfg_type_t cfg_get_type(const cfg_field_t *field);

/// @brief get the information about a configuration field
///
/// @param field the field to get the information about
///
/// @return the information about @p field
const cfg_info_t *cfg_get_info(const cfg_field_t *field);

/// @brief get the information about a configuration group
///
/// @param config the configuration group to get the information about
///
/// @return the information about @p config
const cfg_info_t *cfg_group_info(const config_t *config);

/// @brief get the information about an integer field
///
/// @param field the field to get the information about
///
/// @return the information about @p field
const cfg_int_t *cfg_int_info(const cfg_field_t *field);

/// @brief get the information about a yes/no field
///
/// @param field the field to get the information about
///
/// @return the information about @p field
const cfg_bool_t *cfg_bool_info(const cfg_field_t *field);

/// @brief get the information about a string field
///
/// @param field the field to get the information about
///
/// @return the information about @p field
const cfg_string_t *cfg_string_info(const cfg_field_t *field);

/// @brief get the information about a choice field
///
/// @param field the field to get the information about
///
/// @return the information about @p field
const cfg_enum_t *cfg_enum_info(const cfg_field_t *field);

/// @brief get the information about a flags field
///
/// @param field the field to get the information about
///
/// @return the information about @p field
const cfg_flags_t *cfg_flags_info(const cfg_field_t *field);

/// @brief get the name of a configuration type
///
/// @param type the type to get the name of
///
/// @return the name of @p type
const char *cfg_type_name(cfg_type_t type);

/// @brief get all subgroups in a configuration group
///
/// @param config the configuration group to get the subgroups from
///
/// @return the subgroups in @p config
typevec_t *cfg_get_groups(const config_t *config);

/// @brief get all fields in a configuration group
///
/// @param config the configuration group to get the fields from
///
/// @return the fields in @p config
vector_t *cfg_get_fields(const config_t *config);

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
int cfg_int_value(const cfg_field_t *field);

/// @brief get the current boolean value of a configuration field
///
/// @param field the field to get the value of
///
/// @return the current value of @p field
bool cfg_bool_value(const cfg_field_t *field);

/// @brief get the current string value of a configuration field
///
/// @param field the field to get the value of
///
/// @return the current value of @p field
const char *cfg_string_value(const cfg_field_t *field);

vector_t *cfg_vector_value(const cfg_field_t *field);

/// @brief get the current enum value of a configuration field
///
/// @param field the field to get the value of
///
/// @return the current value of @p field
size_t cfg_enum_value(const cfg_field_t *field);

/// @brief get the current flags value of a configuration field
///
/// @param field the field to get the value of
///
/// @return the current value of @p field
size_t cfg_flags_value(const cfg_field_t *field);

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
bool cfg_set_int(cfg_field_t *field, int value);

/// @brief set the current value of a bool field
///
/// @param field the field to set the value of
/// @param value the new value
void cfg_set_bool(cfg_field_t *field, bool value);

/// @brief set the current value of a string field
///
/// @param field the field to set the value of
/// @param value the new value
void cfg_set_string(cfg_field_t *field, char *value);

/// @brief push a new value onto an array field
///
/// @param field the field to push the value onto
/// @param value the new value
void cfg_vector_push(cfg_field_t *field, char *value);

/// @brief set the current value of an enum field
/// set the value via a string name
///
/// @param field the field to set the value of
/// @param choice the name of the choice to set
///
/// @return true if the choice was valid and the field was updated, false otherwise
bool cfg_set_enum(cfg_field_t *field, const char *choice);

/// @brief set the current value of an enum field
/// set the value via an integer value
/// @pre @p value must be a valid value from the enum choices
/// @warning this will assert if @p value is not a valid value from the flags choices
///
/// @param field the field to set the value of
/// @param value the value to set
void cfg_set_enum_value(cfg_field_t *field, size_t value);

/// @brief set the current value of a flags field
/// set the value via a string name
///
/// @param field the field to set the value of
/// @param choice the name of the choice to set
/// @param set true to set the flag, false to clear it
///
/// @return true if the choice was valid and the field was updated, false otherwise
bool cfg_set_flag(cfg_field_t *field, const char *choice, bool set);

/// @brief set the current value of a flags field
/// set the value via an integer value
/// @pre @p value must be a valid value from the flags choices
/// @warning this will assert if @p value is not a valid value from the flags choices
///
/// @param field the field to set the value of
/// @param value the value to set
void cfg_set_flag_value(cfg_field_t *field, size_t value);

/// @} // ConfigWrite

/// @} // Config

END_API
