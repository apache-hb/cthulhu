#pragma once

typedef struct ssa_module_t ssa_module_t;
typedef struct reports_t reports_t;
typedef struct fs_t fs_t;

typedef struct vector_t vector_t;
typedef struct map_t map_t;

///
/// common api
///

typedef struct emit_options_t {
    reports_t *reports;
    fs_t *fs;

    vector_t *modules; // vector<ssa_module>
    map_t *deps; // map<ssa_symbol, set<ssa_symbol>>
} emit_options_t;

///
/// ssa api
///

typedef struct ssa_emit_options_t {
    emit_options_t opts;
} ssa_emit_options_t;

typedef struct ssa_emit_result_t {
    void *stub;
} ssa_emit_result_t;

/**
 * @brief emit ssa form for debugging
 *
 * @param options the options to use
 */
ssa_emit_result_t emit_ssa(const ssa_emit_options_t *options);
