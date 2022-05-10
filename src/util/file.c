#define I_WILL_BE_INCLUDING_PLATFORM_CODE

// clang-format off
#include "src/platform/platform.h"
#include "cthulhu/util/file.h"
// clang-format on

file_t file_open(const char *path, file_flags_t flags, cerror_t *error)
{
    file_format_t fmt = (flags & FILE_BINARY) ? FORMAT_BINARY : FORMAT_TEXT;
    file_mode_t mode = (flags & FILE_WRITE) ? MODE_WRITE : MODE_READ;

    native_cerror_t nativeError = 0;
    file_handle_t result = native_file_open(path, mode, fmt, &nativeError);
    *error = (cerror_t)nativeError;

    file_t file = {.handle = result, .path = path};

    return file;
}

void file_close(file_t file)
{
    native_file_close(file.handle);
}

size_t file_read(file_t file, void *buffer, size_t size, cerror_t *error)
{
    native_cerror_t nativeError = 0;
    size_t result = (size_t)native_file_read(file.handle, buffer, (file_read_t)size, &nativeError);
    *error = (cerror_t)nativeError;
    return result;
}

size_t file_write(file_t file, const void *buffer, size_t size, cerror_t *error)
{
    native_cerror_t nativeError = 0;
    size_t result = (size_t)native_file_write(file.handle, buffer, (file_write_t)size, &nativeError);
    *error = (cerror_t)nativeError;
    return result;
}

size_t file_size(file_t file, cerror_t *error)
{
    native_cerror_t nativeError = 0;
    size_t result = native_file_size(file.handle, &nativeError);
    *error = (cerror_t)nativeError;
    return result;
}

USE_DECL
const void *file_map(file_t file, cerror_t *error)
{
    native_cerror_t nativeError = 0;
    const void *result = native_file_map(file.handle, &nativeError);
    *error = (cerror_t)nativeError;
    return result;
}
