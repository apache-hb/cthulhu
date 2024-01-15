#pragma once

#include <stddef.h>

#include <ctu_support_api.h>

#include "cthulhu/runtime/runtime.h"

BEGIN_API

/// @defgroup langs Language driver enumeration
/// @ingroup support
/// @brief Language driver enumeration for staticly linked languages
/// @{

/// @brief language driver list
typedef struct langs_t
{
    /// @brief the language drivers
    FIELD_SIZE(size) const language_t *const *langs;

    /// @brief the number of language drivers
    size_t size;
} langs_t;

/// @brief get the language drivers
///
/// @return the language drivers
CT_SUPPORT_API langs_t get_langs(void);

/// @}

END_API
