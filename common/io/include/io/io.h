#pragma once

#include "os/os.h"

#include "core/analyze.h"

#include <stddef.h>

BEGIN_API

/// @defgroup IO Generic IO objects
/// @ingroup Common
/// @{

/// @brief an opaque handle to an IO object
typedef struct io_t io_t;

/// @brief an io error code
typedef os_error_t io_error_t;

/// @brief destroy an IO object
///
/// @param io the io object
void io_close(OUT_PTR_INVALID io_t *io);

/// @brief create an IO object from a file
///
/// @param path the path to the file
/// @param mode the access mode of the file
///
/// @return the io object, or NULL on error
NODISCARD CT_ALLOC(io_close)
io_t *io_file(const char *path, os_access_t mode);

/// @brief create an IO object from an initial view of memory
/// @note this copies @p size bytes from @p data into a new buffer
///
/// @param name the name of the io block
/// @param data the data to copy into the initial buffer
/// @param size the size of the data
/// @param flags the access mode of the file
///
/// @return the io object
NODISCARD CT_ALLOC(io_close)
io_t *io_memory(const char *name, const void *data, size_t size, os_access_t flags);

/// @brief create an IO object in memory of a given size
/// @p size specifies the initial internal buffer size, the file seek position starts at 0
///
/// @param name the name of the io object
/// @param size the starting size of the buffer
/// @param flags the access mode
///
/// @return the io object
NODISCARD CT_ALLOC(io_close)
io_t *io_blob(const char *name, size_t size, os_access_t flags);

/// @brief create a readonly IO object for a given view of memory
/// @pre @p data must point to a valid memory region of @p size bytes
///
/// @param name the name of the IO view
/// @param data the data to provide in the view
/// @param size the size of the data
///
/// @return the IO view
NODISCARD CT_ALLOC(io_close)
io_t *io_view(const char *name, const void *data, size_t size);

/// @brief create an IO view of a string
/// create a readonly IO view of a string
///
/// @param name the name of the IO view
/// @param string the backing string view
///
/// @return the io object
NODISCARD CT_ALLOC(io_close)
io_t *io_string(const char *name, const char *string);

/// @brief read from an io object
/// @pre the io object must have been created with the @a eAccessRead flag
///
/// @param io the io object
/// @param dst the dst buffer to read into
/// @param size the number of bytes to read
///
/// @return the number of bytes actually read
size_t io_read(io_t *io, OUT_WRITES(size) void *dst, size_t size);

/// @brief write to an io object
/// @pre the io object must have been created with the @a eAccessWrite flag
///
/// @param io the io object
/// @param src the source buffer to copy from
/// @param size the number of bytes to copy into the file
///
/// @return the number of bytes actually written
size_t io_write(io_t *io, IN_READS(size) const void *src, size_t size);

/// @brief get the name of an io object
///
/// @param io the io object
///
/// @return the name of the object
NODISCARD
const char *io_name(IN_NOTNULL const io_t *io);

/// @brief get the total size of an io objects contents
///
/// @param io the io object
///
/// @return the total size in bytes of its contents
NODISCARD
size_t io_size(IN_NOTNULL io_t *io);

/// @brief seek to an absolute offset in a file
///
/// @param io the io object
/// @param offset the offset to seek to
///
/// @return the offset after seeking
NODISCARD
size_t io_seek(IN_NOTNULL io_t *io, size_t offset);


/// @brief map an io objects backing into memory
///
/// @param io the io object to map from
///
/// @return memory mapping to the contents
NODISCARD
const void *io_map(IN_NOTNULL io_t *io);

/// @brief get the last error from the io object
///
/// @param io the io object
///
/// @return the last set error
NODISCARD MUST_INSPECT
io_error_t io_error(IN_NOTNULL const io_t *io);

/// @} // IO

END_API
