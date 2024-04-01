// SPDX-License-Identifier: LGPL-3.0-only

#include "tar/tar.h"

#include "base/panic.h"
#include "base/util.h"
#include "fs/fs.h"

#include "io/io.h"
#include "os/os.h"

#include "std/str.h"

#include "core/macros.h"

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
    ctu_memset(header->checksum, ' ', sizeof(header->checksum)); // exclude the last space

    unsigned int sum = 0;
    for (size_t i = 0; i < sizeof(*header); ++i)
    {
        sum += ((unsigned char *)header)[i];
    }

    str_sprintf(header->checksum, sizeof(header->checksum) - 1, "%06o", sum);
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
    ctu_memcpy(header->magic, "ustar", sizeof(header->magic));
    ctu_memcpy(header->version, "00", sizeof(header->version));

    ctu_memset(header->uid, '0', sizeof(header->uid) - 1);
    ctu_memset(header->gid, '0', sizeof(header->gid) - 1);

    header->type = type;

    str_sprintf(header->size, sizeof(header->size), "%011o", (unsigned int)size);
    str_sprintf(header->mode, sizeof(header->mode), "%07o", 0644);
    str_sprintf(header->mtime, sizeof(header->mtime), "%011o", 0);

    ctu_memset(header->checksum, ' ', sizeof(header->checksum) - 1);

    checksum(header);

    size_t written = io_write(dst, header, sizeof(tar_header_t));
    return written == sizeof(tar_header_t);
}

static tar_error_t write_tar_dir(const char *dir, const char *name, tar_context_t *ctx)
{
    tar_header_t header = { 0 };

    if (is_path_special(name))
        str_sprintf(header.name, sizeof(header.name), "%s/", dir);
    else
        str_sprintf(header.name, sizeof(header.name), "%s/%s/", dir, name);

    bool result = build_tar_header(&header, ctx->dst, TAR_TYPE_DIR, 0);

    return result ? ctx->error : eTarWriteError;
}

static tar_error_t write_tar_file(const char *dir, const char *name, tar_context_t *ctx)
{
    tar_header_t header = { 0 };

    if (is_path_special(dir))
        str_sprintf(header.name, sizeof(header.name), "%s", name);
    else
        str_sprintf(header.name, sizeof(header.name), "%s/%s", dir, name);

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
            ctu_memset(block + read, 0, sizeof(block) - read);
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

static void archive_file(const char *dir, const char *name, os_dirent_t type, void *data)
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

static tar_error_t archive_dir(io_t *dst, fs_t *src, fs_iter_t *iter, const char *dir, arena_t *arena)
{
    tar_context_t ctx = {
        .fs = src,
        .dst = dst,
        .arena = arena,
        .error = eTarOk,
    };

    fs_inode_t *inode;

    while (fs_iter_next(iter, &inode) == eOsSuccess)
    {
        os_dirent_t type = fs_inode_type(inode);
        const char *name = fs_inode_name(inode);

        if (type == eOsNodeDir)
        {
            fs_iter_t *inner;
            os_error_t err = fs_iter_begin(src, inode, &inner);
            if (err != eOsSuccess)
            {
                return eTarReadError;
            }

            archive_dir(dst, src, inner, name, arena);

            fs_iter_end(inner);
        }
        else if (type == eOsNodeFile)
        {
            archive_file(dir, name, type, &ctx);
        }
    }

    return ctx.error;
}

tar_error_t tar_archive(io_t *dst, fs_t *src, arena_t *arena)
{
    tar_context_t ctx = {
        .fs = src,
        .dst = dst,
        .arena = arena,
        .error = eTarOk,
    };

    fs_iter_t *iter;
    os_error_t err;

    err = fs_iter_begin(src, fs_root_inode(src), &iter);
    if (err != eOsSuccess)
    {
        return eTarReadError;
    }

    archive_dir(dst, src, iter, ".", arena);

    fs_iter_end(iter);

    return ctx.error;
}

static size_t parse_tar_size(const tar_header_t *header)
{
    size_t result = 0;
    for (size_t i = 0; i < sizeof(header->size); ++i)
    {
        char c = header->size[i];
        if (c == '\0') break;
        if (c == ' ') continue;

        result = result * 8 + (header->size[i] - '0');
    }
    return result;
}

tar_result_t tar_extract(fs_t *dst, io_t *src)
{
    while (true)
    {
        tar_header_t header = { 0 };
        size_t read = io_read(src, &header, sizeof(tar_header_t));
        if (read != sizeof(tar_header_t)) break;

        if (header.name[0] == '\0') break;

        if (header.type == TAR_TYPE_DIR)
        {
            if (!fs_dir_create(dst, header.name))
            {
                tar_result_t result = {
                    .error = eTarWriteError,
                };
                ctu_memcpy(result.name, header.name, sizeof(header.name) - 1);
                return result;
            }

            continue;
        }

        if (header.type != TAR_TYPE_FILE)
        {
            tar_result_t result = {
                .error = eTarUnknownEntry,
                .type = header.type,
            };
            return result;
        }

        fs_file_create(dst, header.name);

        size_t size = parse_tar_size(&header);
        size_t blocks = (size + TAR_BLOCK_SIZE - 1) / TAR_BLOCK_SIZE;

        io_t *io = fs_open(dst, header.name, eOsAccessWrite | eOsAccessTruncate);
        for (size_t i = 0; i < blocks; ++i)
        {
            // TODO: we should map the source file and copy all the blocks at once
            // rather than this block by block approach
            char block[TAR_BLOCK_SIZE] = { 0 };
            size_t read_size = io_read(src, block, sizeof(block));
            if (read_size != sizeof(block))
            {
                tar_result_t result = {
                    .error = eTarReadError,
                    .expected = sizeof(block),
                    .actual = read_size,
                };
                return result;
            }

            size_t write = (i == blocks - 1) ? size % TAR_BLOCK_SIZE : sizeof(block);

            io_write(io, block, write);
        }

        io_close(io);
    }

    tar_result_t result = {
        .error = eTarOk,
    };

    return result;
}

static const char *const kErrorNames[eTarCount] = {
#define TAR_ERROR(id, name) [id] = (name),
#include "tar/tar.inc"
};

USE_DECL
const char *tar_error_string(tar_error_t err)
{
    CTASSERTF(err < eTarCount, "invalid error code %d", err);
    return kErrorNames[err];
}
