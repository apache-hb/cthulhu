#pragma once

typedef struct reports_t reports_t;
typedef struct vector_t vector_t;
typedef struct io_t io_t;
typedef struct fs_t fs_t;

typedef struct ssa_module_t ssa_module_t;

typedef struct jvm_emit_t 
{
    reports_t *reports;

    fs_t *fs;
} jvm_emit_t;

void jvm_emit(jvm_emit_t config, ssa_module_t *module);
