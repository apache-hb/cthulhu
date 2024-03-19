#include "editor/units.hpp"

#include "base/util.h"
#include "std/str.h"

using namespace ed;

struct unit_info_t
{
    const char *name;
    uintmax_t factor;
};

enum unit_t
{
    eB,
    eKB,
    eMB,
    eGB,
    eTB,

    eUnitCount
};

// base 2
static constexpr unit_info_t kUnitInfoIEC[eUnitCount] = {
    { "B",   1 },
    { "KiB", 0x400 },
    { "MiB", 0x100000 },
    { "GiB", 0x40000000 },
    { "TiB", 0x10000000000 },
};

// base 10
static constexpr unit_info_t kUnitInfoSI[eUnitCount] = {
    { "B",   1 },
    { "KB",  1000 },
    { "MB",  1000'000 },
    { "GB",  1000'000'000 },
    { "TB",  1000'000'000'000 },
};

static size_t to_chars_inner(uintmax_t value, char *buffer, const unit_info_t *info)
{
    CTASSERT(buffer != nullptr);

    if (value == 0)
    {
        ctu_memcpy(buffer, "0b", 2);
        return 2;
    }

    char *ptr = buffer;
    uintmax_t count = value;

    for (int i = eUnitCount - 1; i > 0; i--)
    {
        const unit_info_t &unit = info[i];
        const uintmax_t factor = unit.factor;
        const uintmax_t size = count / factor;
        if (size > 0)
        {
            // we know we'll always have enough space
            ptr += str_sprintf(ptr, 32, "%ju%s", size, unit.name);

            count %= factor;

            if (count > 0)
            {
                ctu_memcpy(ptr, "+", 1);
                ptr += 1;
            }
        }
    }

    // write the remaining bytes
    if (count > 0)
    {
        ptr += str_sprintf(ptr, 32, "%jub", count);
    }

    // write the null terminator
    *ptr = '\0';

    return (ptr - buffer);
}

size_t ed::memory_to_chars(uintmax_t value, char *buffer, memory_format_t format)
{
    switch (format)
    {
    case eIEC: return memory_to_chars_iec(value, buffer);
    case eSI: return memory_to_chars_si(value, buffer);
    default: CT_NEVER("invalid format");
    }
}

size_t ed::memory_to_chars_iec(uintmax_t value, char *buffer)
{
    return to_chars_inner(value, buffer, kUnitInfoIEC);
}

size_t ed::memory_to_chars_si(uintmax_t value, char *buffer)
{
    return to_chars_inner(value, buffer, kUnitInfoSI);
}
