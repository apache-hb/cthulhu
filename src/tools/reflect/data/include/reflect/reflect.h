#pragma once

#include <stdint.h>
#include <stddef.h>

namespace ctu {
    class ObjectField;
    class TypeInfoBase;

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
    };

    class Empty {};

    class ObjectId {
        size_t m_id;

    public:
        consteval ObjectId(size_t id) : m_id(id) { }

        constexpr size_t id() const { return m_id; }
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
}
