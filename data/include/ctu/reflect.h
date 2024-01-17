#pragma once

#include <array> // IWYU pragma: export
#include <span> // IWYU pragma: export

#define CTU_CXX_REFLECT 1

#if defined(_MSC_VER) && !defined(__clang__)
#   define CTU_UNREACHABLE() __assume(0)
#else
#   define CTU_UNREACHABLE() __builtin_unreachable()
#endif

namespace ctu {
    template<typename T>
    concept ReflectData = requires (T it) {
        { it.get_name() } -> std::convertible_to<const char*>;
    };

    class TypeInfo {
        const char *name;

    public:
        virtual ~TypeInfo() = default;

        consteval TypeInfo(const char *name) noexcept
            : name(name)
        { }

        constexpr const char *get_name() const noexcept { return name; }
    };

    template<typename T>
    class EnumCase {
        const char *name;
        T value;

    public:
        consteval EnumCase(const char *name, T value) noexcept
            : name(name)
            , value(value)
        { }

        constexpr const char *get_name() const noexcept { return name; }
        constexpr T get_value() const noexcept { return value; }
    };

    class RecordField {
        const char *name;

    public:
        consteval RecordField(const char *name) noexcept
            : name(name)
        { }

        constexpr const char *get_name() const noexcept { return name; }
    };

    namespace impl {
        template<typename T>
        class TypeInfoHandle;
    }

    // customization point
    template<typename T>
    consteval auto reflect() noexcept;
}
