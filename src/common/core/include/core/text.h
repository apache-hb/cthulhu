// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "core/analyze.h"

#include <stddef.h>

/// @ingroup core
/// @{

/// @brief a range of text
typedef struct text_t
{
    /// @brief text data
    FIELD_SIZE(length) char *text;
    /// @brief the number of characters in the text
    /// @note this does not include the null terminator
    size_t length;
} text_t;

/// @brief a non-owning view of text
typedef struct text_view_t
{
    /// @brief the text itself
    FIELD_SIZE(length) const char *text;

    /// @brief the number of characters in the text
    /// @note this does not include the null terminator
    size_t length;
} text_view_t;

/// @def CT_TEXT_VIEW
/// @brief create a text view from a string literal
///
/// @param text the string literal to create a view of
/// @note this does not include the null terminator
/// @warning this does not work with string variables
#define CT_TEXT_VIEW(text) { text, sizeof(text) - 1 }

/// @}
