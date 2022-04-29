#include "cthulhu/util/giga-windows.h"

#include "common.h"

#include "cthulhu/util/error.h"
#include "cthulhu/util/str.h"
#include "cthulhu/util/util.h"

#include <stdint.h>

typedef struct
{
    HANDLE handle;
} windows_file_t;

#define TOTAL_SIZE (sizeof(file_t) + sizeof(windows_file_t))

#define SELF(file) ((windows_file_t *)(file)->data)

static size_t windows_read(file_t *self, void *dst, size_t total)
{
    CTASSERTF(total < UINT32_MAX, "cannot read > DWORD_MAX, attempted read of %zu", total);
    windows_file_t *file = SELF(self);
    DWORD read;

    BOOL result = ReadFile(
        /* hFile = */ file->handle,
        /* lpBuffer = */ dst,
        /* nNumberOfBytesToRead = */ (DWORD)total,
        /* lpNumberOfBytesRead = */ &read,
        /* lpOverlapped = */ NULL);

    if (!result)
    {
        return 0;
    }

    return read;
}

static size_t windows_write(file_t *self, const void *src, size_t total)
{
    CTASSERTF(total < UINT32_MAX, "cannot write > DWORD_MAX, attempted write of %zu", total);
    windows_file_t *file = SELF(self);
    DWORD written = 0;

    BOOL result = WriteFile(
        /* hFile = */ file->handle,
        /* lpBuffer = */ src,
        /* nNumberOfBytesToWrite = */ (DWORD)total,
        /* lpNumberOfBytesWritten = */ &written,
        /* lpOverlapped = */ NULL);

    if (!result)
    {
        return 0;
    }

    return written;
}

static size_t windows_seek(file_t *self, size_t offset)
{
    windows_file_t *file = SELF(self);
    LARGE_INTEGER distanceToMove = {.QuadPart = (LONGLONG)offset};
    LARGE_INTEGER newPosition;

    BOOL result = SetFilePointerEx(
        /* hFile = */ file->handle,
        /* liDistanceToMove = */ distanceToMove,
        /* lpNewFilePointer = */ &newPosition,
        /* dwMoveMethod = */ FILE_BEGIN);

    if (!result)
    {
        return SIZE_MAX;
    }

    return newPosition.QuadPart;
}

static size_t windows_size(file_t *self)
{
    windows_file_t *file = SELF(self);
    LARGE_INTEGER size = {.QuadPart = 0};

    BOOL result = GetFileSizeEx(
        /* hFile = */ file->handle,
        /* lpFileSize = */ &size);

    if (!result)
    {
        return SIZE_MAX;
    }

    return size.QuadPart;
}

static size_t windows_tell(file_t *self)
{
    windows_file_t *file = SELF(self);
    LARGE_INTEGER distanceToMove = {.QuadPart = 0};
    LARGE_INTEGER newPosition = {.QuadPart = 0};

    BOOL result = SetFilePointerEx(
        /* hFile = */ file->handle,
        /* lpDistanceToMove = */ distanceToMove,
        /* lpNewFilePointer = */ &newPosition,
        /* dwMoveMethod = */ FILE_CURRENT);

    if (!result)
    {
        return SIZE_MAX;
    }

    return newPosition.QuadPart;
}

static void *windows_map(file_t *self)
{
    size_t size = windows_size(self);
    void *data = ctu_malloc(size);
    windows_read(self, data, size);
    return data;

#if 0
    // file mapping names cant have \\ in them
    // LPCSTR name = str_replace(self->path, "\\", "-");

    DWORD protect = (self->access & WRITE) ? PAGE_READWRITE : PAGE_READONLY;

    HANDLE mapping = CreateFileMappingA(
        /* hFile = */ handle,
        /* lpFileMappingAttributes = */ NULL,
        /* flProtect = */ protect,
        /* dwMaximumSizeHigh = */ 0,
        /* dwMaximumSizeLow = */ 0, // we want everything mapped
        /* lpName = */ NULL
    );

    ctu_errno_t err = ctu_last_error();
    printf("map: %p %s\n", mapping, ctu_err_string(err));

    if (GetLastError() == ERROR_FILE_INVALID) { return NULL; }
    if (mapping == NULL) { return NULL; }

    DWORD access = (self->access & WRITE) ? FILE_MAP_WRITE : FILE_MAP_READ;

    void *ptr = MapViewOfFile(
        /* hFileMappingObject = */ mapping,
        /* dwDesiredAccess = */ access,
        /* dwFileOffsetHigh = */ 0,
        /* dwFileOffsetLow = */ 0,      // start at the beginning
        /* dwNumberOfBytesToMap = */ 0  // and map the entire file
    );

    ctu_errno_t err2 = ctu_last_error();
    printf("view: %s\n", ctu_err_string(err2));

    return ptr;
#endif
}

static bool windows_ok(file_t *self)
{
    windows_file_t *file = SELF(self);
    return file->handle != INVALID_HANDLE_VALUE;
}

static const file_ops_t kFileOps = {
    .read = windows_read,
    .write = windows_write,
    .seek = windows_seek,
    .size = windows_size,
    .tell = windows_tell,
    .mapped = windows_map,
    .ok = windows_ok,
};

#define UTF8_CODEPAGE 65001

static char *get_absolute(const char *path)
{
    // the following is needed so we are aware of paths longer than MAX_PATH
    // GetFullPathNameA doesnt work on paths longer than MAX_PATH
    // but GetFullPathNameW does

    char *correctEndings = str_replace(path, "/", "\\");

    // convert our path to a wide string
    size_t len = strlen(correctEndings);
    int mbsize = MultiByteToWideChar(UTF8_CODEPAGE, 0, correctEndings, (int)len, NULL, 0);
    LPWSTR total = ctu_malloc(sizeof(WCHAR));
    MultiByteToWideChar(UTF8_CODEPAGE, 0, correctEndings, (int)len, total, mbsize);

    // use the wide string to get the full path
    DWORD size = GetFullPathNameW(total, 0, NULL, NULL);
    LPWSTR result = ctu_malloc((size + 1) * sizeof(WCHAR));
    GetFullPathNameW(total, size, result, NULL);

    // turn the wide string back into a utf-8 string
    char *out = ctu_malloc(size + 1);
    wcstombs(out, result, size);

    // free the memory we dont need anymore
    ctu_free(result);
    ctu_free(total);

    return out;
}

void platform_close(file_t *file)
{
    windows_file_t *self = SELF(file);
    if (file->access & WRITE)
    {
        FlushFileBuffers(self->handle);
    }
    CloseHandle(self->handle);
}

void platform_open(file_t **file, const char *path, contents_t format, access_t access)
{
    char *absolute = get_absolute(path);
    file_t *self = ctu_malloc(TOTAL_SIZE);
    self->path = absolute;
    self->format = format;
    self->access = access;
    self->backing = FD;
    self->ops = &kFileOps;

    DWORD desiredAccess = (access & READ ? GENERIC_READ : 0) | (access & WRITE ? GENERIC_WRITE : 0);

    DWORD createDisposition = (access & WRITE) ? CREATE_ALWAYS : OPEN_EXISTING;

    DWORD flags = FILE_ATTRIBUTE_NORMAL;

    HANDLE handle = CreateFileA(
        /* lpFileName = */ absolute,
        /* dwDesiredAccess = */ desiredAccess,
        /* dwShareMode = */ FILE_SHARE_READ,
        /* lpSecurityAttributes = */ NULL,
        /* dwCreationDisposition = */ createDisposition,
        /* dwFlagsAndAttributes = */ flags,
        /* hTemplateFile = */ NULL);

    ctu_errno_t err = ctu_last_error();
    printf("%s: %s\n", absolute, ctu_err_string(err));

    windows_file_t *data = SELF(self);
    data->handle = handle;

    *file = self;
}

bool file_exists(const char *path)
{
    return GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES;
}
