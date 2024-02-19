#pragma once

#include <ctu_tar_api.h>

#include "core/analyze.h"

typedef struct fs_t fs_t;
typedef struct io_t io_t;
typedef struct arena_t arena_t;

CT_BEGIN_API

typedef enum tar_error_t
{
#define TAR_ERROR(id, str) id,
#include "tar.def"

    eTarCount
} tar_error_t;

RET_INSPECT
CT_TAR_API tar_error_t tar_archive(io_t *dst, IN_NOTNULL fs_t *src, IN_NOTNULL arena_t *arena);

RET_INSPECT
CT_TAR_API tar_error_t tar_extract(fs_t *dst, IN_NOTNULL io_t *tar);

CT_NODISCARD RET_STRING CT_CONSTFN
CT_TAR_API const char *tar_error_string(tar_error_t err);

CT_END_API
