#pragma once

#include <stdint.h>
#include <stddef.h>

#define REFLECT_ENUM_BITFLAGS(T, U) \
    constexpr T operator|(T lhs, T rhs) { return (T)((U)lhs | (U)rhs); } \
    constexpr T operator&(T lhs, T rhs) { return (T)((U)lhs & (U)rhs); } \
    constexpr T operator^(T lhs, T rhs) { return (T)((U)lhs ^ (U)rhs); } \
    constexpr T operator~(T lhs) { return (T)(~(U)lhs); }

#define REFLECT_ENUM_ARITHMATIC(T, U) \
    constexpr T operator+(T lhs, T rhs) { return (T)((U)lhs + (U)rhs); } \
    constexpr T operator-(T lhs, T rhs) { return (T)((U)lhs - (U)rhs); } \
    constexpr T operator*(T lhs, T rhs) { return (T)((U)lhs * (U)rhs); } \
    constexpr T operator/(T lhs, T rhs) { return (T)((U)lhs / (U)rhs); } \
    constexpr T operator%(T lhs, T rhs) { return (T)((U)lhs % (U)rhs); }

#define REFLECT_ENUM_COMPARE(T, U) \
    constexpr bool operator==(T lhs, T rhs) { return (U)lhs == (U)rhs; } \
    constexpr bool operator!=(T lhs, T rhs) { return (U)lhs != (U)rhs; } \
    constexpr bool operator<(T lhs, T rhs) { return (U)lhs < (U)rhs; } \
    constexpr bool operator>(T lhs, T rhs) { return (U)lhs > (U)rhs; } \
    constexpr bool operator<=(T lhs, T rhs) { return (U)lhs <= (U)rhs; } \
    constexpr bool operator>=(T lhs, T rhs) { return (U)lhs >= (U)rhs; }

namespace ctu {
    struct ObjectField;
    struct TypeInfoBase;

    class ObjectName {
        const char *m_front;
        const char *m_back;

    public:
        consteval ObjectName(const char *front, const char *back)
            : m_front(front)
            , m_back(back)
        { }

        constexpr const char *begin() const { return m_front; }
        constexpr const char *end() const { return m_back; }

        constexpr size_t size() const { return m_back - m_front; }
        constexpr const char *data() const { return m_front; }

        constexpr char operator[](size_t index) const {
            return m_front[index];
        }
    };

    template<size_t N>
    class SmallString {
        static_assert(N > 0, "SmallString is too small");

        char m_data[N + 1] = {};
        size_t m_size;

        constexpr static size_t int_length(auto value, int base) {
            size_t len = 0;
            while (value) {
                value /= base;
                len++;
            }

            return len;
        }

        constexpr static size_t str_length(const char *str) {
            size_t len = 0;
            while (*str++) {
                len++;
            }

            return len;
        }

    public:
        constexpr SmallString(const char *str = "") : m_size(str_length(str)) {
            for (size_t i = 0; i < m_size; i++) {
                m_data[i] = str[i];
            }
        }

        constexpr SmallString(const ObjectName& name) : m_size(name.size()) {
            for (size_t i = 0; i < m_size; i++) {
                m_data[i] = name[i];
            }
        }

        template<size_t M>
        constexpr SmallString(const char (&str)[M]) : m_size(M - 1) {
            static_assert(M <= N, "string is too large");
            for (size_t i = 0; i < M; i++) {
                m_data[i] = str[i];
            }
        }

        constexpr SmallString(auto number, int base) : m_size(int_length(number, base)) {
            for (size_t i = 0; i < m_size; i++) {
                auto digit = number % base;
                number /= base;

                m_data[m_size - i - 1] = '0' + digit;
            }
        }

        constexpr void append(const char *str) {
            auto len = str_length(str);
            for (size_t i = 0; i < len; i++) {
                m_data[m_size + i] = str[i];
            }

            m_size += len;
        }

        constexpr void append(const ObjectName& name) {
            for (size_t i = 0; i < name.size(); i++) {
                m_data[m_size + i] = name[i];
            }

            m_size += name.size();
        }

        constexpr SmallString& operator+=(const ObjectName& name) {
            append(name);
            return *this;
        }

        constexpr SmallString& operator+=(const char *str) {
            append(str);
            return *this;
        }

        constexpr void append_int(uint64_t value, int base = 10) {
            auto len = int_length(value, base);

            for (size_t i = 0; i < len; i++) {
                auto digit = value % base;
                value /= base;

                m_data[m_size + len - i - 1] = '0' + digit;
            }
        }

        constexpr size_t size() const { return m_size; }
        constexpr const char *data() const { return m_data; }
        constexpr const char *begin() const { return m_data; }
        constexpr const char *end() const { return m_data + m_size; }
    };

    class Empty {};

    class ObjectId {
        uint32_t m_id;

    public:
        consteval ObjectId(uint32_t id) : m_id(id) { }

        constexpr uint32_t id() const { return m_id; }
    };

    class OutOfBounds {
        size_t m_index;

    public:
        constexpr OutOfBounds(size_t index) : m_index(index) { }
        constexpr size_t index() const { return m_index; }
    };

    namespace impl {
        template<size_t N>
        consteval ObjectName objname(const char (&name)[N]) {
            return ObjectName(name, name + N - 1);
        }

        constexpr ObjectName kUnknown = objname("<unknown>");
    }

    template<typename T>
    struct EnumCase {
        const ObjectName name;
        const T value;
    };

    enum Access {
        ePublic,
        eProtected,
        ePrivate,
    };

    enum Attribs {
        eAttribNone = 0,
        eAttribTransient = 1 << 0,
    };

    struct ObjectField {
        const ObjectName name;
        const size_t index;
        const Access access;
        const Attribs attribs;
    };

    struct ObjectMethod {
        const ObjectName name;
        const size_t index;
    };

    struct TypeInfoBase {
        const ObjectName name;
        const size_t size;
        const size_t align;
        const ObjectId id;
    };

    template<typename T>
    class TypeInfo;

#define PRIMITIVE_TYPEINFO(TY, ID) \
    template<> class TypeInfo<TY> : TypeInfoBase { \
    public: \
        using type_t = TY; \
        \
        static constexpr ObjectName kFullName = impl::objname(#TY); \
        static constexpr ObjectName kName = impl::objname(#TY); \
        static constexpr ObjectId kTypeId = ID; \
        \
        consteval TypeInfo() : TypeInfoBase(kName, sizeof(type_t), alignof(type_t), kTypeId) { } \
    };

    template<> class TypeInfo<void> : TypeInfoBase {
    public:
        using type_t = void;

        static constexpr ObjectName kFullName = impl::objname("void");
        static constexpr ObjectName kName = impl::objname("void");
        static constexpr ObjectId kTypeId = 0;

        consteval TypeInfo() : TypeInfoBase(kName, 0, 0, kTypeId) { }
    };

    PRIMITIVE_TYPEINFO(bool, 1)
    PRIMITIVE_TYPEINFO(char, 2)
    PRIMITIVE_TYPEINFO(int8_t, 3)
    PRIMITIVE_TYPEINFO(int16_t, 4)
    PRIMITIVE_TYPEINFO(int32_t, 5)
    PRIMITIVE_TYPEINFO(int64_t, 6)
    PRIMITIVE_TYPEINFO(uint8_t, 7)
    PRIMITIVE_TYPEINFO(uint16_t, 8)
    PRIMITIVE_TYPEINFO(uint32_t, 9)
    PRIMITIVE_TYPEINFO(uint64_t, 10)
    PRIMITIVE_TYPEINFO(float, 11)

    template<typename T>
    consteval auto reflect();

    template<typename T>
    concept Reflected = requires {
        reflect<T>();
    };
}
