#pragma once

#include "report.h"

char *node_string(const node_t *node);

/**
 * report a shadowing error
 * 
 * @param reports the report sink
 * @param name the name of the shadowed symbol
 * @param shadowed the shadowed symbol
 * @param shadowing the shadowing symbol
 * 
 * @return the message 
 */
message_t *report_shadow(reports_t *reports,
                        const char *name,
                        const node_t *shadowed,
                        const node_t *shadowing) NONULL;

message_t *report_unknown_character(reports_t *reports,
                                    const node_t *node,
                                    const char *str) NONULL;
