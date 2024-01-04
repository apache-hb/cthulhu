#pragma once

#include "core/compiler.h"
#include "core/analyze.h"

#include <stddef.h>

BEGIN_API

/// @ingroup Notify
/// @{

typedef enum severity_t
{
#define SEVERITY(ID, NAME) ID,
#include "notify.inc"

    eSeverityTotal
} severity_t;

/// @brief a diagnostic
typedef struct diagnostic_t
{
    /// @brief the severity of the diagnostic
    severity_t severity;

    /// @brief the id of the diagnostic
    /// should be in the format `[A-Z]{2,3}[0-9]{4}` e.g. `CLI0001`
    /// this is not enforced, but is recommended
    const char *id;

    /// @brief a brief description of the diagnostic
    /// a single line description of the diagnostic
    const char *brief;

    /// @brief a description of the diagnostic
    /// a more involved description of the diagnostic
    /// this is optional
    const char *description;
} diagnostic_t;

/// @brief a list of diagnostics
typedef struct diagnostic_list_t
{
    /// @brief the list of diagnostics
    FIELD_SIZE(count)
    const diagnostic_t * const *diagnostics;

    /// @brief the number of diagnostics in the list
    size_t count;
} diagnostic_list_t;

/// @}

END_API
