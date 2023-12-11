#pragma once

#include "core/analyze.h"
#include "core/compiler.h"

#include <stdbool.h>
#include <stddef.h>

BEGIN_API

typedef struct vector_t vector_t;
typedef struct node_t node_t;

/**
 * @defgroup Reports Error reporting
 * @brief Error reporting sink api
 * @ingroup Common
 *
 * an error reporting sink should be used in any code that has a chance of
 * failing. when an error occurs, an error should be pushed into the sink, and a
 * sentinel value should be returned from the function.
 *
 * these errors should then later be reported using @ref end_reports. if this
 * function returns a value other than @ref EXIT_OK. the application should then
 * clean up and exit with the returned error code.
 *
 * @code{.c}
 * // improper usage of the error sink api
 * int do_something_badly(reports_t *reports, int data) {
 *   if (data < 0) {
 *      report(reports, eFatal, NULL, "data must be positive");
 *      abort(); // NO, this means the error messages will never be printed
 *  }
 *  return data * 2;
 * }
 *
 * // proper usage
 * int do_something_well(reports_t *reports, int data) {
 *   if (data < 0) {
 *     report(reports, eFatal, NULL, "data must be positive");
 *     return INT_MAX; // an obvious error code is returned rather than exiting
 *   }
 *   return data * 2;
 * }
 * @endcode
 *
 * @{
 */

/**
 * @brief the severity of a message
 */
typedef enum
{
    eSorry,    ///< unimplemented feature has been hit
    eInternal, ///< an invalid state has been reached internally
    eFatal,    ///< a user issue that prevents the program from continuing
    eWarn,     ///< a user issue that may be resolved
    eInfo,     ///< a notification for logging

    eLevelTotal
} level_t;

typedef int status_t;

/**
 * @brief part of an error message
 */
typedef struct part_t
{
    char *message;      ///< associated message
    const node_t *node; ///< associated node
} part_t;

/**
 * @brief an error message
 */
typedef struct message_t
{
    /* the level of this error */
    level_t level;

    /* error message displayed at the top */
    char *message;
    char *underline;

    vector_t *parts;

    /* source and location, if node is NULL then location is ignored */
    const node_t *node;

    /* extra note */
    char *note;
} message_t;

typedef struct
{
    size_t limit;
    bool warningsAreErrors;
} report_config_t;

#define DEFAULT_REPORT_CONFIG                                                                                          \
    {                                                                                                                  \
        .limit = SIZE_MAX, .warningsAreErrors = false,                                                                 \
    }

/**
 * @brief an error reporting sink
 */
typedef struct reports_t
{
    vector_t *messages; ///< all messages in the sink
} reports_t;

/**
 * create a reporting context
 *
 * @return the new context
 */
NODISCARD
reports_t *begin_reports(void);

/// @brief flush a reporting context and return an exit code
///
/// @param reports the context to flush
/// @param name the name of this report
/// @param settings the settings for this report
///
/// @retval EXIT_OK if the sink only contains warnings or notes.
/// @retval EXIT_ERROR if the sink contained any errors.
/// @retval EXIT_INTERAL if the sink contained an internal error.
NODISCARD
status_t end_reports(reports_t *reports, const char *name, report_config_t settings);

/**
 * @brief push an internal compiler error into a reporting context
 *
 * @param reports the reporting context
 * @param fmt the format string
 * @param ... the arguments to the format string
 *
 * @return a message object to attach extra data to
 */
CT_PRINTF(2, 3)
message_t *ctu_assert(reports_t *reports, FORMAT_STRING const char *fmt, ...);

/**
 * push a compiler message into a reporting context
 *
 * @param reports the reporting context
 * @param level the severity of this message
 * @param node the location that this message applies to, can be NULL
 * @param fmt the format string
 * @param ... the arguments to the format string
 *
 * @return a message object to attach extra data to
 */
CT_PRINTF(4, 5)
message_t *report(reports_t *reports, level_t level, const node_t *node, FORMAT_STRING const char *fmt, ...);

/**
 * add another part to a message
 *
 * @param message the message to add to
 * @param node the location that this part applies to, can be NULL
 * @param fmt the format string
 * @param ... the arguments to the format string
 */
CT_PRINTF(3, 4)
void report_append(message_t *message, const node_t *node, FORMAT_STRING const char *fmt, ...);

/**
 * add an underline message to an existing message
 *
 * @param message the message to add to
 * @param fmt the format string
 * @param ... the arguments to the format string
 */
CT_PRINTF(2, 3)
void report_underline(message_t *message, FORMAT_STRING const char *fmt, ...);

/**
 * add a note to an existing message
 *
 * @param message the message to add to
 * @param fmt the format string
 * @param ... the arguments to the format string
 */
CT_PRINTF(2, 3)
void report_note(message_t *message, FORMAT_STRING const char *fmt, ...);

/** @} */

/**
 * @defgroup Verbose Verbose output
 * @brief Verbose output
 * @ingroup Common
 * @{
 */

/**
 * whether logverbose should print or not
 *
 * set to true to enable verbose logging.
 * false to disable
 */
extern bool verbose;

/**
 * log to console only when verbose is true
 *
 * @param fmt format string
 * @param ... arguments
 */
CT_PRINTF(1, 2)
void logverbose(FORMAT_STRING const char *fmt, ...);

/** @} */

END_API
