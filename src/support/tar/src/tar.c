#include "tar/tar.h"

#include "base/panic.h"
#include "fs/fs.h"

#include "io/io.h"

#include "std/str.h"

#include "core/macros.h"

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>

// POSIX 1003.1-1990 tar header block
typedef struct tar_header_t
{
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char checksum[8];
    char type;
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
    char padding[12];
} tar_header_t;

CT_STATIC_ASSERT(sizeof(tar_header_t) == 512, "tar header size mismatch");

#define TAR_REJECT(x, err) do { if (x) return err; } while (0)
#define TAR_BLOCK_SIZE 512

#define TAR_TYPE_FILE '0'
#define TAR_TYPE_DIR '5'

static void checksum(tar_header_t *header)
{
    memset(header->checksum, ' ', sizeof(header->checksum)); // exclude the last space

    unsigned int sum = 0;
    for (size_t i = 0; i < sizeof(*header); ++i)
    {
        sum += ((unsigned char *)header)[i];
    }

    str_printf(header->checksum, sizeof(header->checksum), "%06o", sum);
}

typedef struct tar_context_t
{
    fs_t *fs;
    io_t *dst;
    arena_t *arena;
    tar_error_t error;
} tar_context_t;

static bool build_tar_header(tar_header_t *header, io_t *dst, char type, const char *path, size_t size)
{
    memset(header, 0, sizeof(*header));
    strncpy(header->name, path, sizeof(header->name));
    memcpy(header->magic, "ustar", sizeof(header->magic));
    memcpy(header->version, "00", sizeof(header->version));

    header->type = type;

    str_printf(header->size, sizeof(header->size), "%011o", (unsigned int)size);
    str_printf(header->mode, sizeof(header->mode), "%07o", 0644);
    str_printf(header->mtime, sizeof(header->mtime), "%011o", 0);

    checksum(header);

    size_t written = io_write(dst, header, sizeof(tar_header_t));
    return written == sizeof(tar_header_t);
}

static tar_error_t write_tar_dir(const char *path, tar_context_t *ctx)
{
    tar_header_t header = { 0 };
    bool result = build_tar_header(&header, ctx->dst, TAR_TYPE_DIR, path, 0);
    return result ? ctx->error : eTarWriteError;
}

static tar_error_t write_tar_file(const char *path, tar_context_t *ctx)
{
    tar_error_t ret = ctx->error;

    tar_header_t header = { 0 };
    io_t *src = fs_open(ctx->fs, path, eOsAccessRead);
    CTASSERTF(src != NULL, "failed to open file `%s`", path);
    io_error_t err = io_error(src);
    CTASSERTF(err == 0, "failed to open file `%s` (%s)", path, os_error_string(err, ctx->arena));
    size_t size = io_size(src);

    bool ok = build_tar_header(&header, ctx->dst, TAR_TYPE_FILE, path, size);
    if (!ok)
    {
        ret = eTarWriteError;
        goto cleanup;
    }

    size_t blocks = (size + TAR_BLOCK_SIZE - 1) / TAR_BLOCK_SIZE;

    char block[TAR_BLOCK_SIZE] = { 0 };

    for (size_t i = 0; i < blocks; ++i)
    {
        size_t read = io_read(src, block, sizeof(block));
        if (read != sizeof(block))
        {
            memset(block + read, 0, sizeof(block) - read);
        }

        size_t written = io_write(ctx->dst, block, sizeof(block));
        if (written != sizeof(block))
        {
            ret = eTarWriteError;
            goto cleanup;
        }
    }

cleanup:
    io_close(src);
    return ret;
}

static void write_tar_entry(const char *path, os_dirent_t type, void *data)
{
    tar_context_t *ctx = data;
    printf("tar entry: %s %d\n", path, type);

    switch (type)
    {
    case eOsNodeFile:
        ctx->error = write_tar_file(path, ctx);
        break;
    case eOsNodeDir:
        ctx->error = write_tar_dir(path, ctx);
        break;

    default:
        break;
    }
}

tar_error_t tar_archive(io_t *dst, fs_t *src, arena_t *arena)
{
    tar_context_t ctx = {
        .fs = src,
        .dst = dst,
        .arena = arena,
        .error = eTarOk,
    };

    fs_iter_dirents(src, ".", &ctx, write_tar_entry);

    return ctx.error;
}

tar_error_t tar_extract(fs_t *dst, io_t *src, arena_t *arena)
{
    CT_UNUSED(dst);
    CT_UNUSED(arena);

    tar_header_t header;
    size_t read = io_read(src, &header, sizeof(header));
    TAR_REJECT(read != sizeof(header), eTarInvalidHeader);

    return eTarOk;
}
