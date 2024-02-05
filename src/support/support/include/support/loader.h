#pragma once

#include <ctu_support_api.h>

#include "core/analyze.h"
#include "os/core.h"

typedef struct arena_t arena_t;
typedef struct typevec_t typevec_t;

typedef struct language_t language_t;
typedef struct plugin_t plugin_t;
typedef struct target_t target_t;

CT_BEGIN_API

typedef struct loader_t loader_t;

typedef enum loader_config_t
{
#define LOADER_FEATURE(ID, STR, BIT) ID = (BIT),
#include "loader.def"
} loader_config_t;

typedef enum module_type_t
{
#define LOADER_MODULE(ID, STR, BIT) ID = (BIT),
#include "loader.def"

    eModCount
} module_type_t;

typedef enum load_error_t
{
#define LOADER_ERROR(ID, STR) ID,
#include "loader.def"

    eErrorCount
} load_error_t;

typedef struct loaded_module_t
{
    module_type_t type;

    const language_t *lang;
    const plugin_t *plugin;
    const target_t *target;
    load_error_t error;
    os_error_t os;
} loaded_module_t;

CT_SUPPORT_API loader_t *loader_new(IN_NOTNULL arena_t *arena);

CT_CONSTFN
CT_SUPPORT_API loader_config_t loader_config(void);

CT_SUPPORT_API typevec_t *load_default_modules(IN_NOTNULL loader_t *loader);

CT_SUPPORT_API loaded_module_t load_module(IN_NOTNULL loader_t *loader, module_type_t mask, IN_STRING const char *name);

CT_SUPPORT_API loaded_module_t load_static_module(IN_NOTNULL loader_t *loader, module_type_t mask, IN_STRING const char *name);

CT_SUPPORT_API loaded_module_t load_shared_module(IN_NOTNULL loader_t *loader, module_type_t mask, IN_STRING const char *name);

CT_END_API
