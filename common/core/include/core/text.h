#pragma once

/// @ingroup Core
/// @brief a range of text
typedef struct text_t
{
    char *text;  ///< the text itself
    size_t size; ///< the number of bytes in the text
} text_t;

/// @ingroup Core
/// @brief a non-owning view of text
typedef struct text_view_t
{
    const char *text; ///< the text itself
    size_t size;      ///< the number of bytes in the text
} text_view_t;
