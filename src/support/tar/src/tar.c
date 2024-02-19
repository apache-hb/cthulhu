#include "tar/tar.h"

#include "base/panic.h"
#include "base/util.h"
#include "fs/fs.h"

#include "io/io.h"

#include "std/str.h"

#include "core/macros.h"

#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <string.h>

#define TAR_NAME_SIZE 100

// POSIX 1003.1-1990 tar header block
typedef struct tar_header_t
{
    char name[TAR_NAME_SIZE];
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

static bool build_tar_header(tar_header_t *header, io_t *dst, char type, size_t size)
{
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

static tar_error_t write_tar_dir(const char *dir, const char *name, tar_context_t *ctx)
{
    tar_header_t header = { 0 };

    if (is_path_special(name))
        str_printf(header.name, sizeof(header.name), "%s/", dir);
    else
        str_printf(header.name, sizeof(header.name), "%s/%s/", dir, name);

    bool result = build_tar_header(&header, ctx->dst, TAR_TYPE_DIR, 0);

    return result ? ctx->error : eTarWriteError;
}

static tar_error_t write_tar_file(const char *dir, const char *name, tar_context_t *ctx)
{
    tar_header_t header = { 0 };

    if (is_path_special(dir))
        str_printf(header.name, sizeof(header.name), "%s", name);
    else
        str_printf(header.name, sizeof(header.name), "%s/%s", dir, name);

    io_t *src = fs_open(ctx->fs, header.name, eOsAccessRead);
    io_error_t err = io_error(src);
    CTASSERTF(err == 0, "failed to open file `%s` (%s)", header.name, os_error_string(err, ctx->arena));

    tar_error_t ret = ctx->error;

    size_t size = io_size(src);
    bool ok = build_tar_header(&header, ctx->dst, TAR_TYPE_FILE, size);
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

static void write_tar_entry(const char *dir, const char *name, os_dirent_t type, void *data)
{
    tar_context_t *ctx = data;
    if (str_startswith(dir, "./"))
        dir += 2;

    switch (type)
    {
    case eOsNodeFile:
        ctx->error = write_tar_file(dir, name, ctx);
        break;
    case eOsNodeDir:
        ctx->error = write_tar_dir(dir, name, ctx);
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

tar_error_t tar_extract(fs_t *dst, io_t *src)
{
    while (true)
    {
        tar_header_t header = { 0 };
        size_t read = io_read(src, &header, sizeof(tar_header_t));
        if (read != sizeof(tar_header_t)) break;

        if (header.name[0] == '\0') break;

        if (header.type == TAR_TYPE_DIR)
        {
            fs_dir_create(dst, header.name);
            continue;
        }

        CTASSERTF(header.type == TAR_TYPE_FILE, "unsupported tar entry type %c", header.type);
        fs_file_create(dst, header.name);

        size_t size = strtoull(header.size, NULL, 8);
        size_t blocks = (size + TAR_BLOCK_SIZE - 1) / TAR_BLOCK_SIZE;

        io_t *io = fs_open(dst, header.name, eOsAccessWrite | eOsAccessTruncate);
        for (size_t i = 0; i < blocks; ++i)
        {
            char block[TAR_BLOCK_SIZE] = { 0 };
            size_t read = io_read(src, block, sizeof(block));
            if (read != sizeof(block))
            {
                return eTarReadError;
            }

            size_t write = (i == blocks - 1) ? size % TAR_BLOCK_SIZE : sizeof(block);

            io_write(io, block, sizeof(write));
        }

        io_close(io);
    }

    return eTarOk;
}
