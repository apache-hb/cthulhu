#pragma once

#include "ctu/ir/ir.h"

/**
 * ir optimization
 * hence called speed
 */

typedef struct {
    module_t *mod;
    size_t *dirty;
} pass_t;

pass_t new_pass(module_t *mod);

bool run_pass(pass_t *pass);

/**
 * remove unreachable code
 */
bool remove_dead_code(flow_t *flow);

/**
 * remove unused ir nodes
 */
bool remove_unused_code(flow_t *flow);

/**
 * remove code that can be safely removed
 */
bool remove_pure_code(flow_t *flow);

/**
 * remove unreferenced block labels
 */
bool remove_unused_blocks(flow_t *flow);

/**
 * remove empty blocks and fixup related branches
 */
bool remove_empty_blocks(flow_t *flow);

/**
 * when possible convert OP_RESERVE OP_STORE
 * into OP_VALUE
 */
bool mem2reg(flow_t *flow);

/**
 * remove unneeded OP_VALUE
 */
bool propogate_consts(flow_t *flow);

/**
 * remove unneeded branches
 */
bool remove_branches(flow_t *flow);

/**
 * remove unneeded jumps
 */
bool remove_jumps(flow_t *flow);

/**
 * constant folding
 */
bool fold_consts(flow_t *flow);
