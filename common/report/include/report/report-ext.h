#pragma once

#include "os/error.h"

typedef struct message_t message_t;
typedef struct reports_t reports_t;
typedef struct node_t node_t;

BEGIN_API

/// @ingroup Report
/// @{

/**
 * @brief report a shadowing error
 *
 * @param reports the report sink
 * @param name the name of the shadowed symbol
 * @param previous the previous symbol definition
 * @param redefine the new symbol definition
 *
 * @return the message
 */
message_t *report_shadow(reports_t *reports, const char *name, const node_t *previous, const node_t *redefine);

/**
 * @brief report one or more unknown characters in a file
 *
 * @param reports the report sink
 * @param node the location of the unknown characters
 * @param str the string containing the unknown characters
 *
 * @return the message
 */
message_t *report_unknown_character(reports_t *reports, const node_t *node, const char *str);

/// @brief report an os error
///
/// @param reports the report sink
/// @param msg the message to report
/// @param err the os error
///
/// @return the message
message_t *report_os(reports_t *reports, const char *msg, os_error_t err);

/// @}

END_API
