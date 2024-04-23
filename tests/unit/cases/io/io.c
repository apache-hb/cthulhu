#include "unit/ct-test.h"

#include "arena/arena.h"
#include "setup/memory.h"

#include "io/io.h"
#include "io/impl/file.h"
#include "io/impl/view.h"
#include "io/impl/buffer.h"

int main(void)
{
    test_install_panic_handler();
    test_install_electric_fence();

    arena_t *arena = ctu_default_alloc();
    test_suite_t suite = test_suite_new("io", arena);

    // simple
    {
        test_group_t group = test_group(&suite, "simple");
        io_t *io = io_string("test", "test text", arena);
        GROUP_EXPECT_PASS(group, "io_string() should return a valid io_t pointer", io != NULL);
        GROUP_EXPECT_PASS(group, "io_string() should have no error", io_error(io) == eOsSuccess);
        os_error_t err = io_free(io);
        GROUP_EXPECT_PASS(group, "io_free() should have no error", err == eOsSuccess);
    }

    // in place
    {
        test_group_t group = test_group(&suite, "in place");
        char buffer[IO_BUFFER_SIZE];
        io_t *io = io_blob_init(buffer, "test", 1024, eOsAccessRead | eOsAccessWrite, arena);
        GROUP_EXPECT_PASS(group, "io_blob_init() should return a valid io_t pointer", io != NULL);
        GROUP_EXPECT_PASS(group, "io_blob_init() should have no error", io_error(io) == eOsSuccess);
        os_error_t err = io_close(io);
        GROUP_EXPECT_PASS(group, "io_close() should have no error", err == eOsSuccess);
    }

    return test_suite_finish(&suite);
}
