#pragma once

#include "scan/node.h"

#include "std/vector.h"

#include <stddef.h>
#include <stdbool.h>

/**
 * @defgroup ErrorCodes Error code macros
 * @brief exit codes that line up with GNU standard codes
 * @{
 */

#define EXIT_OK 0        ///< no compiler errors or internal errors
#define EXIT_ERROR 1     ///< only compiler errors occurred
#define EXIT_INTERNAL 99 ///< at least one internal error occured

/** @} */


/**
 * @brief the severity of a message
 */
typedef enum
{
    INTERNAL, ///< an invalid state has been reached internally
    ERROR,    ///< a user issue that prevents the program from continuing
    WARNING,  ///< a user issue that may be resolved
    NOTE,     ///< a notification for logging

    LEVEL_TOTAL
} level_t;

/**
 * @brief part of an error message
 */
typedef struct
{
    char *message; ///< associated message
    node_t node;   ///< associated node
} part_t;

/**
 * @brief an error message
 */
typedef struct
{
    /* the level of this error */
    level_t level;

    /* error message displayed at the top */
    char *message;
    char *underline;

    vector_t *parts;

    /* source and location, if node is NULL then location is ignored */
    node_t node;

    /* extra note */
    char *note;
} message_t;

typedef struct
{
    size_t limit;
    bool warningsAreErrors;
} report_config_t;

typedef bool (*report_begin_t)(void *user);
typedef void (*report_add_t)(void *user, const message_t *message);
typedef void (*report_end_t)(void *user, const char *name, report_config_t *config);

typedef struct 
{
    void *user;
} report_sink_t;
