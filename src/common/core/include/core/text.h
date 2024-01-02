#pragma once

#include <stddef.h>

/// @ingroup Core
/// @brief a range of text
typedef struct text_t
{
    char *text;  ///< the text itself
    size_t size; ///< the number of bytes in the text
} text_t;

inline text_t text_make(char *text, size_t size)
{
    text_t result = { text, size };

    return result;
}

/// @ingroup Core
/// @brief a non-owning view of text
typedef struct text_view_t
{
    const char *text; ///< the text itself
    size_t size;      ///< the number of bytes in the text
} text_view_t;

inline text_view_t text_view_make(const char *text, size_t size)
{
    text_view_t result = { text, size };

    return result;
}
