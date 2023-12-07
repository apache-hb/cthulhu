#pragma once

#include "core/compiler.h"

#include <stdbool.h>

BEGIN_API

typedef struct alloc_t alloc_t;
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

    /// a choice from a set of options
    eConfigEnum,

    /// one or more choices from a set of options
    eConfigFlags,

    eConfigTotal
} cfg_type_t;

typedef struct cfg_field_t cfg_field_t;
typedef struct config_t config_t;

/// @brief information about a configuration field
/// @note either arg_long or arg_short must be non-NULL
typedef struct cfg_info_t
{
    /// the name of this field
    const char *name;

    /// a brief description of this field (one line, optional)
    const char *brief;

    /// a longer description of this field (optional)
    const char *description;

    /// the argparse long name for this field
    /// (if NULL, the field is not exposed to argparse)
    const char *arg_long;

    /// the argparse short name for this field
    /// (if NULL, the field is not exposed to argparse)
    const char *arg_short;
} cfg_info_t;

typedef struct cfg_int_t
{
    /// default value
    int initial;

    int min;
    int max;
} cfg_int_t;

typedef struct cfg_bool_t
{
    bool initial;
} cfg_bool_t;

typedef struct cfg_string_t
{
    const char *initial;
} cfg_string_t;

/// @brief a choice in a set of options
typedef struct cfg_choice_t
{
    /// the name of this choice
    const char *text;

    /// the value of this choice
    size_t value;
} cfg_choice_t;

/// initial is treated as a value
typedef struct cfg_enum_t
{
    const cfg_choice_t *options;
    size_t count;

    size_t initial;
} cfg_enum_t;

/// initial is treated as a bitfield
typedef struct cfg_flags_t
{
    const cfg_choice_t *options;
    size_t count;

    size_t initial;
} cfg_flags_t;

config_t *config_new(alloc_t *alloc, const cfg_info_t *info);

// construction api

cfg_field_t *config_int(config_t *group, const cfg_info_t *info, cfg_int_t cfg);
cfg_field_t *config_bool(config_t *group, const cfg_info_t *info, cfg_bool_t cfg);
cfg_field_t *config_string(config_t *group, const cfg_info_t *info, cfg_string_t cfg);
cfg_field_t *config_enum(config_t *group, const cfg_info_t *info, cfg_enum_t cfg);
cfg_field_t *config_flags(config_t *group, const cfg_info_t *info, cfg_flags_t cfg);

config_t *config_group(config_t *group, const cfg_info_t *info);

// reflection api

cfg_type_t cfg_get_type(const cfg_field_t *field);
const cfg_info_t *cfg_get_info(const cfg_field_t *field);
const cfg_info_t *cfg_group_info(const config_t *config);

const cfg_int_t *cfg_int_info(const cfg_field_t *field);
const cfg_bool_t *cfg_bool_info(const cfg_field_t *field);
const cfg_string_t *cfg_string_info(const cfg_field_t *field);
const cfg_enum_t *cfg_enum_info(const cfg_field_t *field);
const cfg_flags_t *cfg_flags_info(const cfg_field_t *field);

const char *cfg_type_name(cfg_type_t type);

vector_t *cfg_get_groups(const config_t *config);
vector_t *cfg_get_fields(const config_t *config);

// access api

int cfg_int_value(const cfg_field_t *field);
bool cfg_bool_value(const cfg_field_t *field);
const char *cfg_string_value(const cfg_field_t *field);
size_t cfg_enum_value(const cfg_field_t *field);
size_t cfg_flags_value(const cfg_field_t *field);

// update api

bool cfg_set_int(cfg_field_t *field, int value);
bool cfg_set_bool(cfg_field_t *field, bool value);
bool cfg_set_string(cfg_field_t *field, const char *value);
bool cfg_set_enum(cfg_field_t *field, const char *choice);

bool cfg_set_flag(cfg_field_t *field, const char *choice, bool value);

END_API
