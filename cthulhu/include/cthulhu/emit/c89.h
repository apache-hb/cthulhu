#pragma once

typedef struct reports_t reports_t;
typedef struct vector_t vector_t;
typedef struct io_t io_t;
typedef struct fs_t fs_t;

typedef struct ssa_module_t ssa_module_t;

typedef enum c89_config_t
{
    eEmitMonolithic = (1 << 0), ///< emit a single source file, if not set a file is generated per module
    eEmitHeaders = (1 << 1), ///< emit header files, if not set only source files are generated
} c89_config_t;

typedef enum cxx_version_t
{
    eCxxVersionNone, ///< dont generate a cxx wrapper
    eCxxVersion11, ///< generate a cxx wrapper with c++11 (no compound namespaces)
    eCxxVersion17, ///< generate a cxx wrapper with c++17 (compound namespaces)
    eCxxVersion20, ///< generate a cxx wrapper with c++20 (modules)

    eCxxVersionTotal
} cxx_version_t;

typedef struct c89_emit_t
{
    reports_t *reports;
    
    c89_config_t config;
    cxx_version_t cxxVersion;

    fs_t *fs;
} c89_emit_t;

void c89_emit(c89_emit_t config, ssa_module_t *module);
