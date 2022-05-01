#pragma once

#include "cthulhu/hlir/hlir.h"
#include "cthulhu/plugins/plugin.h"
#include "cthulhu/util/report.h"

typedef enum
{
    OPTION_BOOL,
    OPTION_STRING,
    OPTION_INT,
    OPTION_VECTOR,
} option_type_t;

typedef union {
    bool boolean;
    const char *string;
    int integer;
    vector_t *vector;
} option_data_t;

typedef struct
{
    option_type_t type;
    option_data_t data;
    option_data_t defaultValue;
    size_t numNames;
    const char **names;
} option_t;

typedef struct
{
    reports_t *reports;
    size_t id;
} plugin_t;

typedef struct
{
    const char *name;
    const char *version;
    const char *description;
    const char *license;
} plugin_info_t;

#ifdef _WIN32
#    define PLUGIN_EXPORT __declspec(dllexport)
#    define CTHULHU_API __declspec(dllexport)
#elif defined(__GNUC__) || defined(__clang__)
#    define PLUGIN_EXPORT __attribute__((visibility("default")))
#    define CTHULHU_API __attribute__((visibility("default")))
#else
#    define PLUGIN_EXPORT
#    define CTHULHU_API
#endif

#define PLUGIN_INFO(NAME, VERSION, DESC, LICENSE)                                                                      \
    PLUGIN_EXPORT const plugin_info_t kPluginInfo = {                                                                  \
        .name = (NAME), .version = (VERSION), .description = (DESC), .license = (LICENSE)}

typedef bool (*check_module_t)(plugin_t *self, const char *path);
CTHULHU_API void plugin_check_module(plugin_t *self, check_module_t module);

typedef hlir_t *(*load_module_t)(plugin_t *self, const char *path);
CTHULHU_API void plugin_load_module(plugin_t *self, load_module_t module);

typedef void (*save_module_t)(plugin_t *self, const char *path, hlir_t *hlir);
CTHULHU_API void plugin_save_module(plugin_t *self, save_module_t module);

typedef void (*save_output_t)(plugin_t *self, const char *path, hlir_t *hlir);
CTHULHU_API void plugin_save_output(plugin_t *self, save_output_t output);
