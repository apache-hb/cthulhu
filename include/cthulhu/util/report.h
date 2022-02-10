#pragma once

#include "cthulhu/ast/ast.h"

#include "macros.h"
#include "util.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>

typedef enum {
    INTERNAL, /// an invalid state has been reached internally
    ERROR, /// a user issue that prevents the program from continuing
    WARNING, // a user issue that may be resolved
    NOTE, // a notification for logging

    LEVEL_TOTAL
} level_t;

typedef struct {
    char *message;
    const node_t *node;
} part_t;

typedef struct {
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

typedef struct {
    vector_t *messages;
} reports_t;

/**
 * create a reporting context
 * 
 * @return the new context
 */
reports_t *begin_reports(void);

/**
 * flush a reporting context and return an exit code
 * 
 * @param reports the context to flush
 * @param limit the maximum number of errors to report
 * @param name the name of this report
 * 
 * @return an exit code
 */
int end_reports(reports_t *reports, 
                size_t limit, 
                const char *name) NOTNULL(1, 3);

/**
 * push an internal compiler error into a reporting context
 * 
 * @param reports the reporting context
 * @param fmt the format string
 * @param ... the arguments to the format string
 * 
 * @return a message object to attach extra data to
 */
PRINT(2, 3)
message_t *ctu_assert(reports_t *reports, 
                   const char *fmt, ...) NOTNULL(1, 2);

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
PRINT(4, 5)
message_t *report(reports_t *reports, 
                   level_t level, 
                   const node_t *node, 
                   const char *fmt, ...) NOTNULL(1, 4);

/**
 * add another part to a message
 * 
 * @param message the message to add to
 * @param node the location that this part applies to, can be NULL
 * @param fmt the format string
 * @param ... the arguments to the format string
 */
PRINT(3, 4)
void report_append(message_t *message, 
                    const node_t *node, 
                    const char *fmt, ...) NOTNULL(1, 3);

/**
 * add an underline message to an existing message
 * 
 * @param message the message to add to
 * @param fmt the format string
 * @param ... the arguments to the format string
 */
PRINT(2, 3)
void report_underline(message_t *message, 
                      const char *fmt, ...) NOTNULL(1, 2);

/**
 * add a note to an existing message
 * 
 * @param message the message to add to
 * @param fmt the format string
 * @param ... the arguments to the format string
 */
PRINT(2, 3)
void report_note(message_t *message, 
                  const char *fmt, ...) NOTNULL(1, 2);

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
void logverbose(const char *fmt, ...) NOTNULL(1);
