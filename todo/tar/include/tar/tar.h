// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <ctu_tar_api.h>

#include "core/analyze.h"

#include <stddef.h>

typedef struct fs_t fs_t;
typedef struct io_t io_t;
typedef struct arena_t arena_t;

CT_BEGIN_API

typedef enum tar_error_t
{
#define TAR_ERROR(id, str) id,
#include "tar.inc"

    eTarCount
} tar_error_t;

typedef struct tar_result_t
{
    tar_error_t error;
    union {
        /* eTarUnknownEntry */
        char type;

        /* eTarReadError, eTarWriteError */
        struct {
            size_t expected;
            size_t actual;
        };

        /* eTarInvalidDirName */
        char name[100];
    };
} tar_result_t;

RET_INSPECT
CT_TAR_API tar_error_t tar_archive(io_t *dst, IN_NOTNULL fs_t *src, IN_NOTNULL arena_t *arena);

RET_INSPECT
CT_TAR_API tar_result_t tar_extract(fs_t *dst, IN_NOTNULL io_t *tar);

CT_NODISCARD STA_RET_STRING CT_CONSTFN
CT_TAR_API const char *tar_error_string(tar_error_t err);

CT_END_API
