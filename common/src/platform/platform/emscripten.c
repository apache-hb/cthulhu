#include "platform.h"

#include "base/panic.h"

USE_DECL
library_handle_t native_library_open(const char *path, native_cerror_t *error)
{
    UNUSED(path);
    UNUSED(error);

    CTASSERTF(false, "not implemented");
}

void native_library_close(library_handle_t handle)
{
    UNUSED(handle);

    CTASSERTF(false, "not implemented");
}

USE_DECL
void *native_library_get_symbol(library_handle_t handle, const char *symbol, native_cerror_t *error)
{
    UNUSED(handle);
    UNUSED(symbol);
    UNUSED(error);

    CTASSERTF(false, "not implemented");
}

USE_DECL
native_cerror_t native_make_directory(const char *path)
{
    UNUSED(path);

    CTASSERTF(false, "not implemented");
}

USE_DECL
native_cerror_t native_delete_file(const char *path)
{
    UNUSED(path);

    CTASSERTF(false, "not implemented");
}

USE_DECL
file_handle_t native_file_open(const char *path, file_mode_t mode, file_format_t format, native_cerror_t *error)
{
    UNUSED(path);
    UNUSED(mode);
    UNUSED(format);
    UNUSED(error);

    CTASSERTF(false, "not implemented");
}

void native_file_close(file_handle_t handle)
{
    UNUSED(handle);

    CTASSERTF(false, "not implemented");
}

file_read_t native_file_read(file_handle_t handle, void *buffer, file_read_t size, native_cerror_t *error)
{
    UNUSED(handle);
    UNUSED(buffer);
    UNUSED(size);
    UNUSED(error);

    CTASSERTF(false, "not implemented");
}

file_write_t native_file_write(file_handle_t handle, const void *buffer, file_size_t size, native_cerror_t *error)
{
    UNUSED(handle);
    UNUSED(buffer);
    UNUSED(size);
    UNUSED(error);

    CTASSERTF(false, "not implemented");
}

USE_DECL
file_size_t native_file_size(file_handle_t handle, native_cerror_t *error)
{
    UNUSED(handle);
    UNUSED(error);

    CTASSERTF(false, "not implemented");
}

USE_DECL
const void *native_file_map(file_handle_t handle, native_cerror_t *error)
{
    UNUSED(handle);
    UNUSED(error);

    CTASSERTF(false, "not implemented");
}

void native_platform_init(void)
{
    // empty for now
}

USE_DECL
char *native_cerror_to_string(native_cerror_t error)
{
    UNUSED(error);
    
    CTASSERTF(false, "not implemented");
}

USE_DECL
native_cerror_t native_get_last_error(void)
{
    CTASSERTF(false, "not implemented");
}
