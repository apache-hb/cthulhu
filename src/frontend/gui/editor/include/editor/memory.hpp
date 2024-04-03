#pragma once

#include <string>

struct Memory {
    enum Unit {
        eBytes,
        eKilobytes,
        eMegabytes,
        eGigabytes,
        eTerabytes,
        eLimit
    };

    static constexpr size_t kByte = 1;
    static constexpr size_t kKilobyte = kByte * 1024;
    static constexpr size_t kMegabyte = kKilobyte * 1024;
    static constexpr size_t kGigabyte = kMegabyte * 1024;
    static constexpr size_t kTerabyte = kGigabyte * 1024;

    static constexpr size_t kSizes[eLimit] = {
        kByte,
        kKilobyte,
        kMegabyte,
        kGigabyte,
        kTerabyte
    };

    static constexpr const char *kNames[eLimit] = {
        "b", "kb", "mb", "gb", "tb"
    };

    constexpr Memory(size_t memory = 0, Unit unit = eBytes)
        : mBytes(memory * kSizes[unit])
    { }

    constexpr static Memory bytes(size_t bytes) { return Memory(bytes, eBytes); }
    constexpr static Memory kilobytes(size_t kilobytes) { return Memory(kilobytes, eKilobytes); }
    constexpr static Memory megabytes(size_t megabytes) { return Memory(megabytes, eMegabytes); }
    constexpr static Memory gigabytes(size_t gigabytes) { return Memory(gigabytes, eGigabytes); }
    constexpr static Memory terabytes(size_t terabytes) { return Memory(terabytes, eTerabytes); }

    constexpr size_t b() const { return mBytes; }
    constexpr size_t kb() const { return mBytes / kKilobyte; }
    constexpr size_t mb() const { return mBytes / kMegabyte; }
    constexpr size_t gb() const { return mBytes / kGigabyte; }
    constexpr size_t tb() const { return mBytes / kTerabyte; }

    constexpr size_t as_bytes() const { return mBytes; }
    constexpr size_t as_kilobytes() const { return mBytes / kKilobyte; }
    constexpr size_t as_megabytes() const { return mBytes / kMegabyte; }
    constexpr size_t as_gigabytes() const { return mBytes / kGigabyte; }
    constexpr size_t as_terabytes() const { return mBytes / kTerabyte; }

    friend constexpr auto operator<=>(const Memory& lhs, const Memory& rhs) = default;

    constexpr Memory operator+(const Memory& rhs) const { return Memory(mBytes + rhs.mBytes); }
    constexpr Memory operator-(const Memory& rhs) const { return Memory(mBytes - rhs.mBytes); }

    constexpr Memory& operator+=(const Memory& rhs) { mBytes += rhs.mBytes; return *this; }
    constexpr Memory& operator-=(const Memory& rhs) { mBytes -= rhs.mBytes; return *this; }

    std::string to_string() const;

private:
    size_t mBytes;
};
