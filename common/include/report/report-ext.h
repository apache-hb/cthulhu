#pragma once

#include "scan/node.h"

#include "platform/error.h"

typedef struct message_t message_t;

/**
 * @brief report a shadowing error
 *
 * @param reports the report sink
 * @param name the name of the shadowed symbol
 * @param prevDefinition the previous symbol definition
 * @param newDefinition the new symbol definition
 *
 * @return the message
 */
message_t *report_shadow(reports_t *reports, const char *name, const node_t *prevDefinition, const node_t *newDefinition);

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

message_t *report_errno(reports_t *reports, const char *msg, cerror_t err);
