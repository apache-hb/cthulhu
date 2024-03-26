// SPDX-License-Identifier: GPL-3.0-only
#include "editor/utils/io.hpp"

#include "memory/memory.h"
#include "os/os.h"

using namespace ctu;

bool OsError::success() const
{
    return error == eOsSuccess;
}

bool OsError::failed() const
{
    return error != eOsSuccess;
}

const char *OsError::what() const
{
    if (failed() && msg.empty())
    {
        size_t size = os_error_get_string(error, nullptr, 0);
        msg.resize(size);

        size_t written = os_error_get_string(error, msg.data(), size);
        msg.resize(written);
    }

    return msg.c_str();
}

Io Io::file(const char *path)
{
    return io_file(path, eOsAccessRead, get_global_arena());
}

OsError Io::error() const
{
    return io_error(io.get());
}

size_t Io::size() const
{
    return io_size(io.get());
}

const void *Io::map() const
{
    return io_map(io.get(), eOsProtectRead);
}

std::string_view Io::text() const
{
    const char *data = static_cast<const char*>(map());
    return std::string_view{data, size()};
}

const char *Io::name() const
{
    return io_name(io.get());
}
