#pragma once

#include "core/analyze.h"

#include <compare> // IWYU pragma: keep

namespace sm {
    /// @brief represents a non-negative amount of memory
    class Memory {
        /// @brief the amount of memory in bytes
        uintmax_t value;

    public:
        /// @brief the unit of memory
        enum unit_t {
            /// @brief bytes
            eBytes,


            eBeginIEC,

            /// @brief kilobytes (1000 bytes)
            eKilo,

            /// @brief megabytes (1000 kilobytes)
            eMega,

            /// @brief gigabytes (1000 megabytes)
            eGiga,

            /// @brief terabytes (1000 gigabytes)
            eTera,

            eEndIEC,


            eBeginSI,

            /// @brief kibibytes (1024 bytes)
            eKibi,

            /// @brief mebibytes (1024 kibibytes)
            eMebi,

            /// @brief gibibytes (1024 mebibytes)
            eGibi,

            /// @brief tebibytes (1024 gibibytes)
            eTebi,

            eEndSI,


            eUnitCount
        };

        /// @brief printing format for memory
        enum format_t {
            /// @brief print in IEC 80000-13 format (KiB, MiB, GiB, TiB)
            IEC, // NOLINT(readability-identifier-naming)

            /// @brief print in SI format (kB, MB, GB, TB)
            SI, // NOLINT(readability-identifier-naming)

            eFormatCount
        };

        struct unit_info_t {
            const char *name;
            uintmax_t size;
        };

        /// @brief information about each unit
        static constexpr unit_info_t kUnits[eUnitCount] = {
            { "b",  1                },
            {}, // eBeginIEC
            { "kb", 1000             },
            { "mb", 1000'000         },
            { "gb", 1000'000'000     },
            { "tb", 1000'000'000'000 },
            {}, // eEndIEC
            {}, // eBeginSI
            { "ki", 0x400            },
            { "mi", 0x100'000        },
            { "gi", 0x40'000'000     },
            { "ti", 0x10'000'000'000 },
            {}, // eEndSI
        };

        struct format_info_t {
            const char *name;
            uintmax_t base;
        };

        /// @brief information about each format
        static constexpr format_info_t kBase[eFormatCount] = {
            /* IEC */ { "IEC", 1024 },
            /* SI  */ { "SI",  1000 }
        };

        /// @brief get the factor for the given unit
        ///
        /// @param unit the unit to get the factor for
        ///
        /// @return the factor for the given unit
        static constexpr uintmax_t get_unit_factor(unit_t unit) { return kUnits[unit].size; }

        /// @brief get the name of the given unit
        ///
        /// @param unit the unit to get the name of
        ///
        /// @return the name of the given unit
        static constexpr const char *get_unit_name(unit_t unit) { return kUnits[unit].name; }


        /// @brief get the base for the given format
        ///
        /// @param format the format to get the base for
        ///
        /// @return the base for the given format
        static constexpr uintmax_t get_base(format_t format) { return kBase[format].base; }

        /// @brief get the name of the given format
        ///
        /// @param format the format to get the name of
        ///
        /// @return the name of the given format
        static constexpr const char *get_format_name(format_t format) { return kBase[format].name; }

        // in the future these may be wrong if uintmax_t gets larger than 64 bits
        static constexpr size_t kMaxLengthIEC = 64;
        static constexpr size_t kMaxLengthSI = 64;

        /// @brief format a human readable string representation of the memory value
        ///
        /// @pre @p buffer points to a valid memory location with at least @p length bytes
        /// @pre if @p format is IEC, @p length is at least kMaxLengthIEC
        /// @pre if @p format is SI, @p length is at least kMaxLengthSI
        ///
        /// @param buffer the buffer to write the string to
        /// @param length the length of the buffer
        /// @param format the format to use
        ///
        /// @return the number of characters written to the buffer
        /// @return SIZE_MAX if @p format is not a valid format
        size_t to_chars(OUT_WRITES(length) char *buffer, size_t length, format_t format) const;

        /// @brief format a human readable string representation of the memory value in IEC format
        ///
        /// @pre @p length is at least kMaxLengthIEC
        /// @pre @p buffer points to a valid memory location with at least @p length bytes
        ///
        /// @param buffer the buffer to write the string to
        /// @param length the length of the buffer
        ///
        /// @return the number of characters written to the buffer
        size_t to_chars_iec(OUT_WRITES(length) char *buffer, size_t length) const;

        /// @brief format a human readable string representation of the memory value in SI format
        ///
        /// @pre @p length is at least kMaxLengthSI
        /// @pre @p buffer points to a valid memory location with at least @p length bytes
        ///
        /// @param buffer the buffer to write the string to
        /// @param length the length of the buffer
        ///
        /// @return the number of characters written to the buffer
        size_t to_chars_si(OUT_WRITES(length) char *buffer, size_t length) const;

        constexpr Memory() = default;

        constexpr Memory(uintmax_t bytes, unit_t unit = eBytes)
            : value(bytes * get_unit_factor(unit))
        { }

        constexpr static Memory bytes(uintmax_t bytes) { return Memory(bytes, Memory::eBytes); }

        constexpr static Memory kilobytes(uintmax_t kilobytes) { return Memory(kilobytes, Memory::eKilo); }
        constexpr static Memory megabytes(uintmax_t megabytes) { return Memory(megabytes, Memory::eMega); }
        constexpr static Memory gigabytes(uintmax_t gigabytes) { return Memory(gigabytes, Memory::eGiga); }
        constexpr static Memory terabytes(uintmax_t terabytes) { return Memory(terabytes, Memory::eTera); }

        constexpr static Memory kibibytes(uintmax_t kibibytes) { return Memory(kibibytes, Memory::eKibi); }
        constexpr static Memory mebibytes(uintmax_t mebibytes) { return Memory(mebibytes, Memory::eMebi); }
        constexpr static Memory gibibytes(uintmax_t gibibytes) { return Memory(gibibytes, Memory::eGibi); }
        constexpr static Memory tebibytes(uintmax_t tebibytes) { return Memory(tebibytes, Memory::eTebi); }

        constexpr uintmax_t b() const { return value; }
        constexpr uintmax_t as_bytes() const { return value; }

        constexpr uintmax_t as(unit_t unit) const { return value / get_unit_factor(unit); }

        constexpr uintmax_t kb() const { return as(eKilo); }
        constexpr uintmax_t mb() const { return as(eMega); }
        constexpr uintmax_t gb() const { return as(eGiga); }
        constexpr uintmax_t tb() const { return as(eTera); }

        constexpr uintmax_t ki() const { return as(eKibi); }
        constexpr uintmax_t mi() const { return as(eMebi); }
        constexpr uintmax_t gi() const { return as(eGibi); }
        constexpr uintmax_t ti() const { return as(eTebi); }

        constexpr uintmax_t as_kilobytes() const { return as(eKilo); }
        constexpr uintmax_t as_megabytes() const { return as(eMega); }
        constexpr uintmax_t as_gigabytes() const { return as(eGiga); }
        constexpr uintmax_t as_terabytes() const { return as(eTera); }

        constexpr uintmax_t as_kibibytes() const { return as(eKibi); }
        constexpr uintmax_t as_mebibytes() const { return as(eMebi); }
        constexpr uintmax_t as_gibibytes() const { return as(eGibi); }
        constexpr uintmax_t as_tebibytes() const { return as(eTebi); }

        friend constexpr auto operator<=>(const Memory& lhs, const Memory& rhs) = default;

        constexpr Memory operator+(const Memory& other) const { return Memory(value + other.value); }
        constexpr Memory operator-(const Memory& other) const { return Memory(value - other.value); }
        constexpr Memory operator*(const Memory& other) const { return Memory(value * other.value); }
        constexpr Memory operator/(const Memory& other) const { return Memory(value / other.value); }

        constexpr Memory& operator+=(const Memory& other) { value += other.value; return *this; }
        constexpr Memory& operator-=(const Memory& other) { value -= other.value; return *this; }
        constexpr Memory& operator*=(const Memory& other) { value *= other.value; return *this; }
        constexpr Memory& operator/=(const Memory& other) { value /= other.value; return *this; }
    };

    constexpr Memory operator""_b(unsigned long long bytes) { return Memory(bytes, Memory::eBytes); }

    constexpr Memory operator""_kb(unsigned long long kilobytes) { return Memory(kilobytes, Memory::eKilo); }
    constexpr Memory operator""_mb(unsigned long long megabytes) { return Memory(megabytes, Memory::eMega); }
    constexpr Memory operator""_gb(unsigned long long gigabytes) { return Memory(gigabytes, Memory::eGiga); }
    constexpr Memory operator""_tb(unsigned long long terabytes) { return Memory(terabytes, Memory::eTera); }

    constexpr Memory operator""_ki(unsigned long long kibibytes) { return Memory(kibibytes, Memory::eKibi); }
    constexpr Memory operator""_mi(unsigned long long mebibytes) { return Memory(mebibytes, Memory::eMebi); }
    constexpr Memory operator""_gi(unsigned long long gibibytes) { return Memory(gibibytes, Memory::eGibi); }
    constexpr Memory operator""_ti(unsigned long long tebibytes) { return Memory(tebibytes, Memory::eTebi); }
}
