#pragma once

#include "core/analyze.h"
#include "core/compiler.h"


#include "core/text.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

BEGIN_API

/// @defgroup backtrace Stacktrace library
/// @brief Backtrace library
/// @ingroup common
/// @{

/// @brief an address of a symbol
typedef uint_least64_t bt_address_t;

#define PRI_ADDRESS PRIuLEAST64

/// @brief a symbol
typedef struct bt_symbol_t
{
    /// @brief the line number
    size_t line;

    /// @brief a buffer to hold the name
    /// when neither @a eResolveName nor @a eResolveDemangledName is set this is set to 0
    text_t name;

    /// @brief a buffer to hold the path to the file
    /// when @a eResolveFile is not set the first character is set to 0
    text_t path;
} bt_symbol_t;

/// @brief a backtrace frame
typedef struct bt_frame_t
{
    /// @brief the frame address
    bt_address_t address;
} bt_frame_t;

/// @brief how much of a frame was reconstructed
typedef enum frame_resolve_t
{
    /// @brief nothing was resolved
    eResolveNothing = (0),

    /// @brief the line number was found
    /// @note this does not imply @a eResolveFile
    eResolveLine = (1 << 0),

    /// @brief the symbol name was found
    /// @note this does not imply @a eResolveDemangledName
    eResolveName = (1 << 1),

    /// @brief the symbol name was demangled
    eResolveDemangledName = (1 << 2) | eResolveName,

    /// @brief the file path was found
    eResolveFile = (1 << 3),

    eResolveCount
} frame_resolve_t;

/// @brief user callback for @a bt_read
typedef void (*bt_trace_t)(void *user, const bt_frame_t *frame);

/// @brief system level fatal error callback
/// called on events such as segfaults
typedef void (*bt_fatal_callback_t)(void);

/// @brief called once when a system error occurs
///
/// @param error the error to report
///
/// @return the user data to pass to @a bt_error_end
typedef void *(*bt_error_begin_t)(size_t error);

/// @brief called once when a system error occurs
///
/// @param error the error to report
typedef void (*bt_error_end_t)(void *error);

/// @brief system error handling callbacks
typedef struct bt_error_t
{
    /// @brief called once when a system error occurs
    bt_error_begin_t begin;

    /// @brief called after all frames have been collected
    bt_error_end_t end;

    /// @brief called once for each frame
    bt_trace_t frame;
} bt_error_t;

/// @brief the global system error handler
extern bt_error_t gErrorReport;

/// @brief initialize the backtrace backend
/// @note this function must be called before any other backtrace function
void bt_init(void);

/// @brief get the backtrace backend name
///
/// @return the backtrace backend name
RET_STRING CONSTFN
const char *bt_backend(void);

/// @brief get a backtrace from the current location using a callback
/// @note this function is not thread safe
/// @note @a bt_init must be called before calling this function
///
/// @param callback the callback to call for each frame
/// @param user the user data to pass to the callback
void bt_read(IN_NOTNULL bt_trace_t callback, void *user);

/// @brief resolve a frame to a symbol
///
/// @note @a bt_init must be called before calling this function
///
/// @param frame the frame to resolve
/// @param symbol the symbol to fill
frame_resolve_t bt_resolve_symbol(IN_NOTNULL const bt_frame_t *frame, IN_NOTNULL bt_symbol_t *symbol);

/// @}

END_API
