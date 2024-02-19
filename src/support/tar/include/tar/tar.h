#pragma once

#include <ctu_tar_api.h>

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

tar_error_t tar_archive(io_t *dst, fs_t *src, arena_t *arena);
tar_error_t tar_extract(fs_t *dst, io_t *tar);

CT_END_API
