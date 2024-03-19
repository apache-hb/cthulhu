#pragma once

#include "core/analyze.h"

#include <stdint.h>

namespace ed
{
    /// @brief how to format the memory value
    enum memory_format_t : int
    {
        /// @brief use the IEC format (1024 bytes per kilobyte)
        eIEC,

        /// @brief use the SI format (1000 bytes per kilobyte)
        eSI,

        eFormatCount
    };

    /// @brief format a human readable string representation of the memory value
    ///
    /// @pre @p buffer points to a valid memory location of at least 32 bytes
    ///
    /// @param value the value to format
    /// @param buffer the buffer to write the string to
    /// @param format the format to use
    ///
    /// @return the number of characters written to the buffer
    size_t memory_to_chars(uintmax_t value, OUT_WRITES(32) char *buffer, memory_format_t format);

    /// @brief format a human readable string representation of the memory value in IEC format
    ///
    /// @pre @p buffer points to a valid memory location of at least 32 bytes
    ///
    /// @param value the value to format
    /// @param buffer the buffer to write the string to
    ///
    /// @return the number of characters written to the buffer
    size_t memory_to_chars_iec(uintmax_t value, OUT_WRITES(32) char *buffer);

    /// @brief format a human readable string representation of the memory value in SI format
    ///
    /// @pre @p buffer points to a valid memory location of at least 32 bytes
    ///
    /// @param value the value to format
    /// @param buffer the buffer to write the string to
    ///
    /// @return the number of characters written to the buffer
    size_t memory_to_chars_si(uintmax_t value, OUT_WRITES(32) char *buffer);

    memory_format_t get_memory_format();
    void format_memory(uintmax_t value, char *buffer);

    char get_thousands_separator();
    size_t format_large_number(intmax_t value, OUT_WRITES(64) char *buffer, char seperator);

    template<size_t N>
    class SmallString
    {
        char buffer[N];
        size_t length;

    public:
        static SmallString sprintf(const char *format, auto&&... args)
        {
            SmallString<N> result;
            result.length = snprintf(result.buffer, N, format, args...);
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

    template<size_t N>
    SmallString<N> numfmt(intmax_t value, char thousands = ed::get_thousands_separator())
    {
        SmallString<N> result;
        result.length = format_large_number(value, result.buffer, thousands);
        return result;
    }
}
