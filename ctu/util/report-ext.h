#pragma once

#include "ctu/lir/lir.h"
#include "report.h"

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

/**
 * report that a defition is recursive
 * 
 * @param reports the report sink
 * @param stack vector_t<lir_t*> 
 *              the stacktrace of the recursive definiton
 *              as returned from lir_recurses
 * @param root the root node where the stack start
 * 
 * @return the message
 */
message_t *report_recursive(reports_t *reports,
                            vector_t *stack,
                            lir_t *root) NONULL;

message_t *report_unknown_character(reports_t *reports,
                                    node_t *node,
                                    const char *str) NONULL;
