// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include "io/io.h"

namespace ctu
{
    constexpr auto kIoDelete = [](io_t *io) { io_close(io); };
    using unique_io_t = std::unique_ptr<io_t, decltype(kIoDelete)>;

    class OsError
    {
        os_error_t error;
        std::string msg;

        void get_message();

    public:
        OsError(os_error_t error);

        operator os_error_t() const { return error; }

        bool success() const;
        bool failed() const;

        const char *what() const;
    };

    class Io
    {
        unique_io_t io;

        Io(io_t *io) : io(io, kIoDelete) {}

    public:
        static Io file(const char *path);

        OsError error() const;

        size_t size() const;
        const void *map() const;

        std::string_view text() const;

        const char *name() const;

        io_t *get() const { return io.get(); }
    };

}
