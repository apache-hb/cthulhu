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
message_t *report_shadow(WEAK reports_t *reports,
                        const char *name,
                        WEAK const node_t *shadowed,
                        WEAK const node_t *shadowing) NONULL;

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
message_t *report_recursive(WEAK reports_t *reports,
                            WEAK vector_t *stack,
                            WEAK lir_t *root) NONULL;

message_t *report_unknown_character(WEAK reports_t *reports,
                                    WEAK node_t *node,
                                    const char *str) NONULL;
