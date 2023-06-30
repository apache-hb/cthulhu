#pragma once

typedef struct ssa_module_t ssa_module_t;
typedef struct reports_t reports_t;
typedef struct fs_t fs_t;

typedef enum emit_flags_t {
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
    eEmitFlat = (1 << 1)
} emit_flags_t;

typedef struct emit_options_t {
    reports_t *reports;
    fs_t *fs;
    ssa_module_t *mod;
    emit_flags_t flags;
} emit_options_t;

void emit_c89(const emit_options_t *options);
