#pragma once

typedef struct ssa_module_t ssa_module_t;
typedef struct reports_t reports_t;
typedef struct fs_t fs_t;

void emit_c89(reports_t *reports, fs_t *fs, ssa_module_t *module);
