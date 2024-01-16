#pragma once

#include <stddef.h>

/// @ingroup core
/// @brief a range of text
typedef struct text_t
{
    char *text;  ///< the text itself
    size_t length; ///< the number of characters in the text
} text_t;

/// @ingroup core
/// @brief a non-owning view of text
typedef struct text_view_t
{
    const char *text; ///< the text itself
    size_t length;      ///< the number of characters in the text
} text_view_t;
