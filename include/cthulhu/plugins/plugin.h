#pragma once

#include "cthulhu/util/report.h"

typedef enum { OPTION_BOOL, OPTION_STRING, OPTION_INT, OPTION_VECTOR } option_type_t;

typedef union {
    bool boolean;
    const char *string;
    int integer;
    vector_t *vector;
} option_data_t;

typedef struct {
    option_type_t type;
    option_data_t data;
    option_data_t defaultValue;
    size_t numNames;
    const char **names;
} option_t;

typedef struct {
    reports_t *reports;
} plugin_t;

typedef struct {
    const char *name;
    const char *version;
    const char *description;
    const char *license;
} plugin_info_t;

#ifdef _WIN32
#    define PLUGIN_EXPORT __declspec(dllexport)
#elif defined(__GNUC__) || defined(__clang__)
#    define PLUGIN_EXPORT __attribute__((visibility("default")))
#else
#    define PLUGIN_EXPORT
#endif
