#pragma once

#include "core/macros.h"
#include "core/analyze.h"

#include <stdio.h>

BEGIN_API

#define STACKTRACE_NAME_LENGTH 256
#define STACKTRACE_PATH_LENGTH 1024

/// @brief a stacktrace frame
typedef struct frame_t
{
    /// the line number this frame might have originated from (0 if unknown)
    size_t line;

    /// the function name this frame might have originated from (empty if unknown)
    char name[STACKTRACE_NAME_LENGTH];

    /// the path this frame might have originated from (empty if unknown)
    char path[STACKTRACE_PATH_LENGTH];
} frame_t;

/// @brief initialize the stacktrace backend
/// @note this function must be called before any other stacktrace function
void stacktrace_init(void);

/// @brief get the stacktrace backend name
/// @return the stacktrace backend name
RET_STRING
const char *stacktrace_backend(void);

/// @brief get a stacktrace from the current location
/// @note this function is not thread safe
/// @note @ref stacktrace_init must be called before calling this function
///
/// @param frames the frames to fill
/// @param size the size of @a frames
///
/// @return the number of frames filled
RET_RANGE(<=, size)
size_t stacktrace_get(
        frame_t *frames, // OUT_WRITES(return)
        IN_RANGE(>, 0) size_t size);

/// @brief print a stacktrace to a file
/// @note this follows the same precondition as @ref stacktrace_get
///
/// @param file the file to print to
void stacktrace_print(IN_NOTNULL FILE *file);

END_API
