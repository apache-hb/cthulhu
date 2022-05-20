#pragma once

#include "cthulhu/report/report.h"

/**
 * @brief report a shadowing error
 *
 * @param reports the report sink
 * @param name the name of the shadowed symbol
 * @param shadowed the shadowed symbol
 * @param shadowing the shadowing symbol
 *
 * @return the message
 */
message_t *report_shadow(reports_t *reports, const char *name, node_t shadowed, node_t shadowing);

/**
 * @brief report one or more unknown characters in a file
 *
 * @param reports the report sink
 * @param node the location of the unknown characters
 * @param str the string containing the unknown characters
 *
 * @return the message
 */
message_t *report_unknown_character(reports_t *reports, node_t node, const char *str);
