#pragma once

#include "core/compiler.h"

BEGIN_API

typedef enum severity_t
{
#define SEVERITY(ID, NAME) ID,
#include "notify/notify.inc"

    eSeverityTotal
} severity_t;

typedef struct diagnostic_t
{
    severity_t severity;
    const char *id;

    const char *brief;
    const char *description;
} diagnostic_t;

typedef struct diagnostic_list_t
{
    const diagnostic_t * const *diagnostics;
    size_t count;
} diagnostic_list_t;

END_API
