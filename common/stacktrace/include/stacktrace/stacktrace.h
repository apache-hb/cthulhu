#pragma once

#include "core/compiler.h"
#include "core/analyze.h"

#include <stdio.h>
#include <stdbool.h>

BEGIN_API

/// @defgroup Stacktrace Stacktrace library
/// @brief Stacktrace library
/// @ingroup Common
/// @{

#define STACKTRACE_NAME_LENGTH 256
#define STACKTRACE_PATH_LENGTH 1024

typedef struct symbol_t
{
    size_t line;
    char name[STACKTRACE_NAME_LENGTH];
    char file[STACKTRACE_PATH_LENGTH];
} symbol_t;

/// @brief a stacktrace frame
typedef struct frame_t
{
    size_t address;
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

void frame_resolve(
        IN_NOTNULL const frame_t *frame,
        symbol_t *symbol);

/// @brief print a stacktrace to a file
/// @note this follows the same precondition as @ref stacktrace_get
///
/// @param file the file to print to
void stacktrace_print(IN_NOTNULL FILE *file);

/// @}

END_API
