#pragma once

typedef struct ssa_module_t ssa_module_t;
typedef struct reports_t reports_t;
typedef struct fs_t fs_t;

typedef struct vector_t vector_t;

///
/// common api
///

typedef struct emit_options_t {
    reports_t *reports;
    fs_t *fs;
    ssa_module_t *mod;
} emit_options_t;

///
/// c89 api
///

typedef enum c89_flags_t {
    eEmitNone = 0,

    /**
     * emit c89 api headers rather than just source files
     */
    eEmitHeaders = (1 << 0),

    /**
     * emit all headers in a single folder rather than in subdirs.
     *
     * modules will be renamed to their namespace name seperated with `.`
     * e.g. `java/lang/Object.h` would be emitted as `java.lang.Object.h`
     */
    eEmitFlat = (1 << 1),
} c89_flags_t;

typedef struct c89_emit_options_t {
    emit_options_t opts;
    c89_flags_t flags;
} c89_emit_options_t;

typedef struct c89_emit_result_t {
    vector_t *sources; ///< vector<const char*> containing the relative paths to all emitted source files
} c89_emit_result_t;

/**
 * @brief emit c89 for final compilation by a c compiler
 *
 * @param options
 */
c89_emit_result_t emit_c89(const c89_emit_options_t *options);

///
/// ssa api
///

typedef struct ssa_emit_result_t {
    void *stub;
} ssa_emit_result_t;

/**
 * @brief emit ssa form for debugging
 *
 * @param options the options to use
 */
ssa_emit_result_t emit_ssa(const emit_options_t *options);
