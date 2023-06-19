#include "common.h"

typedef struct physical_t
{
    const char *root; ///< absolute path to root directory
} physical_t;

typedef struct physical_file_t
{
    const char *path; ///< path to file relative to root
} physical_file_t;

typedef struct physical_dir_t
{
    const char *path; ///< path to directory relative to root
} physical_dir_t;

static inode2_t *physical_dir(const char *path)
{
    physical_dir_t dir = {
        .path = path
    };
    
    return inode2_dir(&dir, sizeof(physical_dir_t));
}

static const fs2_interface_t kPhysicalInterface = {
    .name = "Disk File System"
};

fs2_t *fs2_physical(reports_t *reports, const char *root)
{
    physical_t self = {
        .root = root
    };

    inode2_t *dir = physical_dir(".");

    return fs2_new(reports, dir, &kPhysicalInterface, &self, sizeof(physical_t));
}
