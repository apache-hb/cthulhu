// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "io/io.h"

#include "os/core.h"

#include <stddef.h>
#include <stdarg.h>

CT_BEGIN_API

/// @ingroup io
/// @{

/// @brief an io object
typedef struct io_t io_t;

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

/// @brief io write format callback
/// seperate from @a io_write_t to allow for more efficient implementations
/// if this is not provided, @a io_write_t will be used instead
///
/// @param self the invoked io object
/// @param fmt the format string
/// @param args the format arguments
///
/// @return the total number of bytes copied into the io object
typedef size_t (*io_fwrite_t)(io_t *self, const char *fmt, va_list args);

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
/// @param protect the protection flags for the memory
///
/// @return the backing memory
typedef void *(*io_map_t)(io_t *self, os_protect_t protect);

/// @brief io close callback
/// destroy an io objects backing data and any associated resources
///
/// @param self the io object
typedef os_error_t (*io_close_t)(io_t *self);

/// @brief io callback interface
typedef struct io_callbacks_t
{
    /// @brief read callback
    /// may be NULL on non-readable objects
    io_read_t fn_read;

    /// @brief write callback
    /// may be NULL on non-writable objects
    io_write_t fn_write;

    /// @brief write format callback
    /// may be NULL on non-writable objects
    /// @note if this is NULL, @a fn_write will be used instead
    io_fwrite_t fn_fwrite;

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

    /// @brief the size of the io objects private data
    size_t size;
} io_callbacks_t;

/// @brief io object implementation
typedef struct io_t
{
    /// @brief callback struct
    const io_callbacks_t *cb;

    /// @brief the last error set on this object
    io_error_t error;

    /// @brief the access flags for this object
    os_access_t flags;

    /// @brief the arena this object was allocated from
    arena_t *arena;

    /// @brief the name of this object
    const char *name;

    /// @brief user data region
    char data[];
} io_t;

// TODO: io_data and io_new should be private to the io module
// currently not due to the fs module

/// @brief get the user data from an io object
/// @warning does not perform any validation on the type of the user data
///
/// @param io the io object
///
/// @return the user data
CT_PUREFN
CT_IO_API void *io_data(IN_NOTNULL io_t *io);

/// @brief create a new IO object for a given interface
/// @pre @p cb must point to a valid callback set
/// @pre @p data must point to a valid memory region of @p cb->size bytes. if @p cb->size is 0, @p data may be NULL
///
/// @param cb the callback set
/// @param flags the access flags for this object
/// @param name the name of the object
/// @param data the user data, this is copied into the io object
/// @param arena the arena to allocate the io object from
///
/// @return a new IO interface
CT_IO_API io_t *io_new(
    IN_NOTNULL const io_callbacks_t *cb,
    os_access_t flags,
    IN_STRING const char *name,
    IN_READS(size) const void *data,
    IN_NOTNULL arena_t *arena);

/// @}

CT_END_API
