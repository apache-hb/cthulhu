#include "common.h"

#include "std/str.h"

typedef struct fs_physical_t
{
    const char *root;
} fs_physical_t;

static const fs_callbacks_t kPhysical = {
    .inodeQuery = NULL,
};

fs_t *fs_physical(const char *root)
{
    fs_physical_t fs = {
        .root = root
    };

    // TODO: physical root inode
    return fs_new(&kPhysical, NULL, &fs, sizeof(fs_physical_t));
}
