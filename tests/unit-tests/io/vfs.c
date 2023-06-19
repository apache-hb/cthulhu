#include "io/fs.h"

#include "ct-test.h"

TEST(test_vfs_creation, {
    fs_t *fs = fs_virtual("test");
    SHOULD_PASS("fs_virtual() should return a valid fs_t pointer", fs != NULL);
})

TEST(test_vfs_mkdir, {
    fs_t *fs = fs_virtual("test");
    fs_mkdir(fs, "testdir/foo/bar");
})

HARNESS("vfs", {
    ENTRY("creation", test_vfs_creation),
    ENTRY("mkdir", test_vfs_mkdir),
})
