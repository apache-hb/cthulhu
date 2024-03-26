#pragma once

#include "std/str.h"

#include <stdint.h>

namespace ed
{
    template<size_t N>
    class SmallString
    {
        char buffer[N];
        size_t length;

    public:
        static SmallString sprintf(const char *format, auto&&... args)
        {
            SmallString<N> result;
            result.length = str_sprintf(result.buffer, N, format, args...);
            return result;
        }

        operator const char *() const { return buffer; }

        const char *c_str() const { return buffer; }
        size_t size() const { return length; }
    };

    template<size_t N>
    SmallString<N> strfmt(const char *format, auto&&... args)
    {
        return SmallString<N>::sprintf(format, args...);
    }
}
