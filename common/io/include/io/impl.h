#pragma once

#include "io/io.h"

/// @brief io object read callback
///
/// @param self the invoked io object
/// @param dst the destination buffer to copy into
/// @param size the size of the provided buffer
///
/// @return the total number of bytes copied to the buffer
typedef size_t (*io_read_t)(io_t *self, void *dst, size_t size);

/// @brief io write callback
///
/// @param self the invoked io object
/// @param src the source buffer to copy from
/// @param size the size of the source buffer
///
/// @return the total number of bytes copied into the io object
typedef size_t (*io_write_t)(io_t *self, const void *src, size_t size);

/// @brief io size callback
/// get the total size of an io objects backing data
///
/// @param self the io object
///
/// @return the total size, in bytes of the backing data
typedef size_t (*io_size_t)(io_t *self);

/// @brief io seek callback
/// seek from start callback
///
/// @param self the io object
/// @param offset the offset to seek to
///
/// @return the actual offset after seeking
typedef size_t (*io_seek_t)(io_t *self, size_t offset);

/// @brief io map callback
/// map an io objects backing data into memory
///
/// @param self the io object
///
/// @return the backing memory
typedef const void *(*io_map_t)(io_t *self);

/// @brief io close callback
/// destroy an io objects backing data and any associated resources
///
/// @param self the io object
typedef void (*io_close_t)(OUT_PTR_INVALID io_t *self);

/// @brief io callback interface
typedef struct io_callbacks_t
{
    /// @brief read callback
    /// may be NULL on non-readable objects
    io_read_t fn_read;

    /// @brief write callback
    /// may be NULL on non-writable objects
    io_write_t fn_write;

    /// @brief total size callback
    /// must always be provided
    io_size_t fn_get_size;

    /// @brief absolute seek callback
    /// must always be provided
    io_seek_t fn_seek;

    /// @brief file map callback
    /// must always be provided
    io_map_t fn_map;

    /// @brief close callback
    /// optional if backing data does not require lifetime management
    io_close_t fn_close;
} io_callbacks_t;

/// @brief io object implementation
typedef struct io_t
{
    /// @brief callback struct
    const io_callbacks_t *cb;

    /// the last error set on this object
    io_error_t error;

    /// the access flags for this object
    os_access_t flags;

    /// the name of this object
    const char *name;

    /// user data space
    char data[];
} io_t;

/// @brief get the user data from an io object
/// @warning does not perform any validation
///
/// @param io the io object
///
/// @return the user data
PUREFN
void *io_data(IN_NOTNULL io_t *io);

/// @brief create a new IO object for a given interface
///
/// @param cb the callback set
/// @param flags the access flags for this object
/// @param name the name of the object
/// @param data the user data, this is copied into the io object
/// @param size the size of the user data
///
/// @return a new IO interface
io_t *io_new(
    IN_NOTNULL const io_callbacks_t *cb,
    os_access_t flags,
    IN_STRING const char *name,
    IN_READS(size) const void *data,
    IN_RANGE(>, 0) size_t size);
