#pragma once

#include "ctu/ir/ir.h"

/**
 * ir optimization
 * hence called speed
 */

/**
 * remove unreachable code
 */
bool remove_dead_code(module_t *mod);

/**
 * remove unused ir nodes
 */
bool remove_unused_code(module_t *mod);

/**
 * remove code that can be safely removed
 */
bool remove_pure_code(module_t *mod);

/**
 * remove unreferenced block labels
 */
bool remove_unused_blocks(module_t *mod);

/**
 * remove empty blocks and fixup related branches
 */
bool remove_empty_blocks(module_t *mod);

/**
 * when possible convert OP_RESERVE OP_STORE
 * into OP_VALUE
 */
bool mem2reg(module_t *mod);

/**
 * remove unneeded OP_VALUE
 */
bool propogate_consts(module_t *mod);

/**
 * remove unneeded branches
 */
bool remove_branches(module_t *mod);

/**
 * remove unneeded jumps
 */
bool remove_jumps(module_t *mod);
