#pragma once

#include "cthulhu/ast/ast.h"

#include "cthulhu/util/macros.h"
#include "cthulhu/util/util.h"
#include "cthulhu/util/vector.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

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
 * @defgroup ErrorApi Error reporting sink
 * @brief error reporting sink api
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
 *      report(reports, ERROR, NULL, "data must be positive");
 *      abort(); // NO, this means the error messages will never be printed
 *  }
 *  return data * 2;
 * }
 *
 * // proper usage
 * int do_something_well(reports_t *reports, int data) {
 *   if (data < 0) {
 *     report(reports, ERROR, NULL, "data must be positive");
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
    char *message;      ///< associated message
    const node_t *node; ///< associated node
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
    const node_t *node;

    /* extra note */
    char *note;
} message_t;

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
reports_t *begin_reports(void);

typedef struct
{
    size_t limit;
    bool warningsAreErrors;
} report_config_t;

/**
 * flush a reporting context and return an exit code
 *
 * @param reports the context to flush
 * @param limit the maximum number of errors to report
 * @param name the name of this report
 *
 * @return an exit code.
 *         EXIT_OK if the sink only contains warnings or notes.
 *         EXIT_ERROR if the sink contained any errors.
 *         EXIT_INTERAL if the sink contained an internal error.
 */
int end_reports(reports_t *reports, const char *name, const report_config_t *settings);

/**
 * @brief push an internal compiler error into a reporting context
 *
 * @param reports the reporting context
 * @param fmt the format string
 * @param ... the arguments to the format string
 *
 * @return a message object to attach extra data to
 */
message_t *ctu_assert(reports_t *reports, FORMAT_STRING const char *fmt, ...) FORMAT_ATTRIBUTE(2, 3);

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
message_t *report(reports_t *reports, level_t level, const node_t *node, FORMAT_STRING const char *fmt, ...)
    FORMAT_ATTRIBUTE(4, 5);

/**
 * add another part to a message
 *
 * @param message the message to add to
 * @param node the location that this part applies to, can be NULL
 * @param fmt the format string
 * @param ... the arguments to the format string
 */
void report_append(message_t *message, const node_t *node, FORMAT_STRING const char *fmt, ...) FORMAT_ATTRIBUTE(3, 4);

/**
 * add an underline message to an existing message
 *
 * @param message the message to add to
 * @param fmt the format string
 * @param ... the arguments to the format string
 */
void report_underline(message_t *message, FORMAT_STRING const char *fmt, ...) FORMAT_ATTRIBUTE(2, 3);

/**
 * add a note to an existing message
 *
 * @param message the message to add to
 * @param fmt the format string
 * @param ... the arguments to the format string
 */
void report_note(message_t *message, FORMAT_STRING const char *fmt, ...) FORMAT_ATTRIBUTE(2, 3);

/** @} */

/**
 * @defgroup Verbose Verbose output
 * @brief verbose output
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
void logverbose(FORMAT_STRING const char *fmt, ...) FORMAT_ATTRIBUTE(1, 2);

/** @} */
