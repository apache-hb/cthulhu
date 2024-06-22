#include "simcoe/units.hpp"

#include "base/panic.h"

#include "base/util.h"
#include "std/str.h"

using namespace sm;

using unit_t = Memory::unit_t;

static size_t to_chars_inner(char *buffer, size_t length, uintmax_t value, unit_t first, unit_t last)
{
    CTASSERT(buffer != nullptr);
    CTASSERTF(length >= 32, "buffer length must be at least 32 (%zu)", length);

    if (value == 0)
    {
        ctu_memcpy(buffer, "0b", 2);
        return 2;
    }

    char *ptr = buffer;
    uintmax_t count = value;

    for (int i = last - 1; i > first; i--)
    {
        const unit_t unit = static_cast<unit_t>(i);
        const uintmax_t factor = Memory::get_unit_factor(unit);
        const uintmax_t size = count / factor;
        if (size > 0)
        {
            size_t remaining = length - (ptr - buffer);
            size_t written = str_sprintf(ptr, remaining, "%ju%s", size, Memory::get_unit_name(unit));
            ptr += written;

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
        size_t remaining = length - (ptr - buffer);
        size_t written = str_sprintf(ptr, remaining, "%jub", count);
        ptr += written;
    }

    return (ptr - buffer);
}

size_t Memory::to_chars(char *buffer, size_t length, format_t format) const
{
    switch (format)
    {
    case Memory::IEC: return to_chars_iec(buffer, length);
    case Memory::SI: return to_chars_si(buffer, length);
    default: return SIZE_MAX;
    }
}

size_t Memory::to_chars_iec(char *buffer, size_t length) const
{
    return to_chars_inner(buffer, length, value, eBeginIEC, eEndIEC);
}

size_t Memory::to_chars_si(char *buffer, size_t length) const
{
    return to_chars_inner(buffer, length, value, eBeginSI, eEndSI);
}
