#pragma once

#include "cthulhu/hlir/hlir.h"
#include "cthulhu/plugins/plugin.h"
#include "cthulhu/util/report.h"
#include "cthulhu/util/version-def.h"

typedef struct
{
    reports_t *reports;
    size_t id;
} plugin_t;

typedef struct
{
    const char *name;
    version_t version;
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
