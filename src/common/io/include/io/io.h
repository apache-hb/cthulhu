// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <ctu_io_api.h>

#include "core/analyze.h"

#include "os/core.h"

// include the implementation headers when doing analysis
// for the size macros
#if STA_PRESENT
#   include "io/impl/buffer.h"
#   include "io/impl/file.h"
#   include "io/impl/view.h"
#endif

#include <stddef.h>
#include <stdarg.h>

CT_BEGIN_API

/// @ingroup io
/// @{

typedef struct io_t io_t;

/// @brief destroy an IO object and free its memory
///
/// @param io the io object
///
/// @return an error code if the io object could not be closed
CT_IO_API os_error_t io_free(STA_RELEASE io_t *io);

/// @brief destroy an IO object
/// @warning this does not free the memory of the object itself
/// This should only be used when the io object is allocated on the stack and initialized
/// with an io_*_init function.
///
/// @param io the io object
///
/// @return an error code if the object could not be destroyed
CT_IO_API os_error_t io_close(INOUT_NOTNULL io_t *io);

/// @brief create an IO object from a file
///
/// @param path the path to the file
/// @param mode the access mode of the file
/// @param arena the arena to allocate from
///
/// @return the io object, or NULL on error
CT_NODISCARD CT_ALLOC(io_free)
CT_IO_API io_t *io_file(IN_STRING const char *path, os_access_t mode, IN_NOTNULL arena_t *arena);

/// @brief create an IO object from an initial view of memory
/// @note this copies @p size bytes from @p data into a new buffer
///
/// @param name the name of the io block
/// @param data the data to copy into the initial buffer
/// @param size the size of the data
/// @param flags the access mode of the file
/// @param arena the arena to allocate from
///
/// @return the io object
CT_NODISCARD CT_ALLOC(io_free)
CT_IO_API io_t *io_memory(IN_STRING const char *name, const void *data, size_t size, os_access_t flags, IN_NOTNULL arena_t *arena);

/// @brief create an IO object in memory of a given size
/// @p size specifies the initial internal buffer size, the file seek position starts at 0
///
/// @param name the name of the io object
/// @param size the starting size of the buffer
/// @param flags the access mode
/// @param arena the arena to allocate from
///
/// @return the io object
CT_NODISCARD CT_ALLOC(io_free)
CT_IO_API io_t *io_blob(IN_STRING const char *name, size_t size, os_access_t flags, IN_NOTNULL arena_t *arena);

/// @brief create a readonly IO object for a given view of memory
/// @pre @p data must point to a valid memory region of @p size bytes
///
/// @param name the name of the IO view
/// @param data the data to provide in the view
/// @param size the size of the data
/// @param arena the arena to allocate from
///
/// @return the IO view
CT_NODISCARD CT_ALLOC(io_free)
CT_IO_API io_t *io_view(IN_STRING const char *name, IN_NOTNULL const void *data, size_t size, IN_NOTNULL arena_t *arena);

/// @brief create an IO view of a string
/// create a readonly IO view of a string
///
/// @param name the name of the IO view
/// @param string the backing string view
/// @param arena the arena to allocate from
///
/// @return the io object
CT_NODISCARD CT_ALLOC(io_free)
CT_IO_API io_t *io_string(IN_STRING const char *name, IN_STRING const char *string, IN_NOTNULL arena_t *arena);

/// @brief create an io object from a file
/// initializes an io object using a preallocated buffer.
/// @note the buffer must be available for the lifetime of the io object.
///       ending the lifetime of the buffer before the io object will result in undefined behavior.
///
/// @pre @p buffer must be at least @ref IO_FILE_SIZE bytes large
/// @pre @p buffer must be aligned to @c alignof(io_t)
///
/// @param buffer the buffer to use for the io object
/// @param path the path to the file
/// @param mode the access mode of the file
///
/// @return the initialized io object
CT_NODISCARD CT_ALLOC(io_close)
CT_IO_API io_t *io_file_init(OUT_WRITES(IO_FILE_SIZE) void *buffer, IN_STRING const char *path, os_access_t mode);

/// @brief create an io object from a memory buffer
/// initializes an io object using a preallocated buffer.
/// @note the buffer must be available for the lifetime of the io object.
///       ending the lifetime of the buffer before the io object will result in undefined behavior.
///
/// @pre @p buffer must be at least @ref IO_MEMORY_SIZE bytes large
/// @pre @p buffer must be aligned to @c alignof(io_t)
///
/// @param buffer the buffer to use for the io object
/// @param name the name of the io block
/// @param data the data to copy into the initial buffer
/// @param size the size of the data
/// @param flags the access mode of the file
/// @param arena the arena to allocate from
///
/// @return the initialized io object
CT_NODISCARD CT_ALLOC(io_close)
CT_IO_API io_t *io_memory_init(OUT_WRITES(IO_BUFFER_SIZE) void *buffer, IN_STRING const char *name, const void *data, size_t size, os_access_t flags, IN_NOTNULL arena_t *arena);

/// @brief create an io object from a memory buffer
/// initializes an io object using a preallocated buffer.
/// @note the buffer must be available for the lifetime of the io object.
///       ending the lifetime of the buffer before the io object will result in undefined behavior.
///
/// @pre @p buffer must be at least @ref IO_MEMORY_SIZE bytes large
/// @pre @p buffer must be aligned to @c alignof(io_t)
///
/// @param buffer the buffer to use for the io object
/// @param name the name of the io object
/// @param size the starting size of the buffer
/// @param flags the access mode
/// @param arena the arena to allocate from
///
/// @return the initialized io object
CT_NODISCARD CT_ALLOC(io_close)
CT_IO_API io_t *io_blob_init(OUT_WRITES(IO_BUFFER_SIZE) void *buffer, IN_STRING const char *name, size_t size, os_access_t flags, IN_NOTNULL arena_t *arena);

/// @brief create an io object from a memory buffer
/// initializes an io object using a preallocated buffer.
/// @note the buffer must be available for the lifetime of the io object.
///       ending the lifetime of the buffer before the io object will result in undefined behavior.
///
/// @pre @p buffer must be at least @ref IO_VIEW_SIZE bytes large
/// @pre @p buffer must be aligned to @c alignof(io_t)
///
/// @param buffer the buffer to use for the io object
/// @param name the name of the IO view
/// @param data the data to provide in the view
/// @param size the size of the data
///
/// @return the initialized io object
CT_NODISCARD CT_ALLOC(io_close)
CT_IO_API io_t *io_view_init(OUT_WRITES(IO_VIEW_SIZE) void *buffer, IN_STRING const char *name, IN_NOTNULL const void *data, size_t size);

/// @brief create an io object from a memory buffer
/// initializes an io object using a preallocated buffer.
/// @note the buffer must be available for the lifetime of the io object.
///       ending the lifetime of the buffer before the io object will result in undefined behavior.
///
/// @pre @p buffer must be at least @ref IO_VIEW_SIZE bytes large
/// @pre @p buffer must be aligned to @c alignof(io_t)
///
/// @param buffer the buffer to use for the io object
/// @param name the name of the IO view
/// @param string the backing string view
///
/// @return the initialized io object
CT_NODISCARD CT_ALLOC(io_close)
CT_IO_API io_t *io_string_init(OUT_WRITES(IO_VIEW_SIZE) void *buffer, IN_STRING const char *name, IN_STRING const char *string);

/// @brief read from an io object
/// @pre the io object must have been created with the @a eOsAccessRead flag
///
/// @param io the io object
/// @param dst the dst buffer to read into
/// @param size the number of bytes to read
///
/// @return the number of bytes actually read
CT_IO_API size_t io_read(IN_NOTNULL io_t *io, OUT_WRITES(size) void *dst, size_t size);

/// @brief write to an io object
/// @pre the io object must have been created with the @a eOsAccessWrite flag
///
/// @param io the io object
/// @param src the source buffer to copy from
/// @param size the number of bytes to copy into the file
///
/// @return the number of bytes actually written
CT_IO_API size_t io_write(IN_NOTNULL io_t *io, IN_READS(size) const void *src, size_t size);

/// @brief printf to an io object
/// @pre the io object must have been created with the @a eOsAccessWrite flag
///
/// @param io the io object
/// @param fmt the format string
/// @param ... the format arguments
///
/// @return the number of bytes actually written
STA_PRINTF(2, 3)
CT_IO_API size_t io_printf(STA_IN io_t *io, STA_FORMAT_STRING const char *fmt, ...);

/// @brief vprintf to an io object
/// @pre the io object must have been created with the @a eOsAccessWrite flag
///
/// @param io the io object
/// @param fmt the format string
/// @param args the format arguments
///
/// @return the number of bytes actually written
CT_IO_API size_t io_vprintf(IN_NOTNULL io_t *io, IN_STRING const char *fmt, va_list args);

/// @brief get the name of an io object
///
/// @param io the io object
///
/// @return the name of the object
CT_NODISCARD
CT_IO_API const char *io_name(IN_NOTNULL const io_t *io);

/// @brief get the total size of an io objects contents
///
/// @param io the io object
///
/// @return the total size in bytes of its contents
CT_NODISCARD
CT_IO_API size_t io_size(IN_NOTNULL io_t *io);

/// @brief seek to an absolute offset in a file
///
/// @param io the io object
/// @param offset the offset to seek to
///
/// @return the offset after seeking
CT_NODISCARD
CT_IO_API size_t io_seek(IN_NOTNULL io_t *io, size_t offset);

/// @brief map an io object into memory
/// maps an io objects entire contents into memory.
///
/// @warning while the return value is a mutable pointer, the writability
/// of the memory is determined by @p protect as well as the access mode
/// the io object was created with.
///
/// @param io the io object to map from
/// @param protect the protection level of the memory
///
/// @return memory mapping to the contents
CT_NODISCARD
CT_IO_API void *io_map(IN_NOTNULL io_t *io, os_protect_t protect);

/// @brief get the last error from the io object
///
/// @param io the io object
///
/// @return the last set error
CT_NODISCARD RET_INSPECT
CT_IO_API os_error_t io_error(IN_NOTNULL const io_t *io);

/// @} // IO

CT_END_API
