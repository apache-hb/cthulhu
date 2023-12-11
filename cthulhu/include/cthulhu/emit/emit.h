#pragma once

#include "core/compiler.h"

BEGIN_API

/// @defgroup Emit SSA emitter
/// @brief Emitter for SSA form
/// @ingroup Runtime
/// @{

typedef struct ssa_module_t ssa_module_t;
typedef struct reports_t reports_t;

typedef struct io_t io_t;
typedef struct fs_t fs_t;

typedef struct vector_t vector_t;
typedef struct map_t map_t;

///
/// common api
///

typedef struct emit_options_t
{
    reports_t *reports;
    fs_t *fs;

    vector_t *modules; // vector<ssa_module>
    map_t *deps;       // map<ssa_symbol, set<ssa_symbol>>
} emit_options_t;

///
/// ssa api
///

typedef struct ssa_emit_options_t
{
    emit_options_t opts;
} ssa_emit_options_t;

typedef struct ssa_emit_result_t
{
    void *stub;
} ssa_emit_result_t;

/**
 * @brief emit ssa form for debugging
 *
 * @param options the options to use
 */
ssa_emit_result_t emit_ssa(const ssa_emit_options_t *options);

///
/// c89 api
///

typedef struct c89_emit_options_t
{
    emit_options_t opts;
} c89_emit_options_t;

typedef struct c89_emit_result_t
{
    vector_t *sources; ///< vector<string> a list of source files to compile
} c89_emit_result_t;

/// @brief emit c89 form for compilation in another compiler
///
/// @param options the options to use
///
/// @return the result of the emit
c89_emit_result_t emit_c89(const c89_emit_options_t *options);

/// @}

END_API
