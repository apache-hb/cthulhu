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

    struct ObjectId {
        uint64_t
    };

    class ObjectName {
        const char *m_front;
        const char *m_back;

    public:
        consteval ObjectName(const char *front, const char *back) noexcept
            : m_front(front)
            , m_back(back)
        { }

        constexpr operator const char*() const noexcept { return m_front; }
        size_t length() const noexcept { return m_back - m_front; }

        constexpr const char *begin() const noexcept { return m_front; }
        constexpr const char *end() const noexcept { return m_back; }
    };

    namespace impl {
        template<size_t N>
        consteval ObjectName name(const char (&str)[N]) noexcept {
            return ObjectName(str, str + N - 1);
        }
    }

    class TypeInfo {
        ObjectName m_name;
        size_t m_size;
        size_t m_align;

        size_t m_hash;

    public:
        consteval TypeInfo(ObjectName name, size_t size, size_t align) noexcept
            : m_name(name)
            , m_size(size)
            , m_align(align)
        { }

        constexpr ObjectName get_name() const noexcept { return m_name; }
        constexpr size_t get_size() const noexcept { return m_size; }
        constexpr size_t get_align() const noexcept { return m_align; }
    };

    template<typename T>
    class EnumCase {
        ObjectName m_name;
        T m_value;

    public:
        consteval EnumCase(ObjectName name, T value) noexcept
            : m_name(name)
            , m_value(value)
        { }

        constexpr ObjectName get_name() const noexcept { return m_name; }
        constexpr T get_value() const noexcept { return m_value; }
    };

    class RecordField {
        ObjectName m_name;
        size_t m_index;

    public:
        consteval RecordField(ObjectName name, size_t index) noexcept
            : m_name(name)
            , m_index(index)
        { }

        constexpr ObjectName get_name() const noexcept { return m_name; }
        constexpr size_t get_index() const noexcept { return m_index; }
    };

    template<typename T>
    class ReflectObject { };

    template<typename T>
    struct ReflectInfo {
        /// @brief the pretty name of the type
        static constexpr ObjectName kObjectName = impl::name("<unknown>");

        static constexpr size_t kTypeSize = sizeof(T);
        static constexpr size_t kTypeAlign = alignof(T);

        /// @brief alias to the ReflectObject of this type
        using reflect_t = void;

        using type_t = T;
        using const_type_t = const T;
        using pointer_t = T*;
        using const_pointer_t = const T*;
    };

    // customization point
    template<typename T>
    consteval auto reflect() noexcept;
}
