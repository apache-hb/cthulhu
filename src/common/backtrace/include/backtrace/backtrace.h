#pragma once

#include "core/compiler.h"
#include "core/analyze.h"
#include "core/text.h"

#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

BEGIN_API

/// @defgroup backtrace Stacktrace library
/// @brief Backtrace library
/// @ingroup Common
/// @{

/// @brief an address of a symbol
typedef uint_least64_t bt_address_t;

#define PRI_ADDRESS PRIuLEAST64

/// @brief a symbol
typedef struct symbol_t
{
    /// @brief the line number
    size_t line;

    /// @brief a buffer to hold the name
    /// when neither @a eResolveName nor @a eResolveDemangledName is set this is set to 0
    text_t name;

    /// @brief a buffer to hold the path to the file
    /// when @a eResolveFile is not set the first character is set to 0
    text_t path;
} symbol_t;

/// @brief a backtrace frame
typedef struct frame_t
{
    /// @brief the frame address
    bt_address_t address;
} frame_t;

/// @brief how much of a frame was reconstructed
typedef enum frame_resolve_t
{
    /// @brief nothing was resolved
    eResolveNothing         = (0),

    /// @brief the line number was found
    /// @note this does not imply @a eResolveFile
    eResolveLine            = (1 << 0),

    /// @brief the symbol name was found
    /// @note this does not imply @a eResolveDemangledName
    eResolveName            = (1 << 1),

    /// @brief the symbol name was demangled
    eResolveDemangledName   = (1 << 2) | eResolveName,

    /// @brief the file path was found
    eResolveFile            = (1 << 3),

    eResolveCount
} frame_resolve_t;

/// @brief user callback for @a bt_read
typedef void (*bt_frame_t)(void *user, const frame_t *frame);

/// @brief system level fatal error callback
/// called on events such as segfaults
typedef void (*bt_fatal_callback_t)(void);

typedef void *(*bt_error_begin_t)(size_t error);
typedef void (*bt_error_end_t)(void *error);

typedef struct bt_error_t
{
    bt_error_begin_t begin;
    bt_error_end_t end;
    bt_frame_t frame;
} bt_error_t;

extern bt_error_t gErrorReport;

/// @brief initialize the backtrace backend
/// @note this function must be called before any other backtrace function
void bt_init(void);

/// @brief get the backtrace backend name
/// @return the backtrace backend name
RET_STRING
const char *bt_backend(void);

/// @brief get a backtrace from the current location using a callback
/// @note this function is not thread safe
/// @note @a bt_init must be called before calling this function
///
/// @param callback the callback to call for each frame
/// @param user the user data to pass to the callback
void bt_read(bt_frame_t callback, void *user);

/// @brief resolve a frame to a symbol
///
/// @note @a bt_init must be called before calling this function
///
/// @param frame the frame to resolve
/// @param symbol the symbol to fill
frame_resolve_t bt_resolve_symbol(IN_NOTNULL const frame_t *frame, symbol_t *symbol);

/// @brief print a backtrace to a file
/// @note this follows the same precondition as @a bt_read
///
/// @param file the file to print to
void bt_print_trace(IN_NOTNULL FILE *file);

/// @}

END_API
