#pragma once

#include "json/ast.h"

#include "base/panic.h"
#include "std/map.h"

typedef struct logger_t logger_t;
typedef struct io_t io_t;

namespace ctu::json {
    class Json;
    class Object;
    class Array;

    class ObjectIterator;
    class ArrayIterator;

    struct gmp_integer_t {
        mpz_t m_value;
    };

    class Object {
        friend class Json;

        map_t *m_object;

        Object(map_t *object)
            : m_object(object)
        { }

    public:
        Json get(const char *key) const;

        Json operator[](const char *key) const;

        ObjectIterator begin() const;
        ObjectIterator end() const;
    };

    class Array {
        friend class Json;

        vector_t *m_array;

        Array(vector_t *array)
            : m_array(array)
        { }

    public:
        Json get(size_t index) const;
        size_t length() const;

        Json operator[](size_t index) const;

        ArrayIterator begin() const;
        ArrayIterator end() const;
    };

    class Json {
        friend class JsonParser;
        friend class Object;
        friend class Array;
        friend class ObjectIterator;
        friend class ArrayIterator;

        json_ast_t *m_ast;

        Json(json_ast_t *ast)
            : m_ast(ast)
        { }

    public:
        bool is_string() const;
        bool is_integer() const;
        bool is_float() const;
        bool is_bool() const;
        bool is_array() const;
        bool is_object() const;
        bool is_null() const;

        bool is_valid() const { return m_ast != nullptr; }
        operator bool() const { return is_valid(); }

        json_kind_t get_kind() const;
        bool is_kind(json_kind_t kind) const;

        text_view_t as_string() const;
        void as_integer(mpz_t integer) const;
        float as_float() const;
        bool as_bool() const;
        Array as_array() const;
        Object as_object() const;

        Json get(const char *key) const;
        Json get(size_t index) const;

        Json operator[](const char *key) const { return get(key); }
        Json operator[](size_t index) const { return get(index); }

        template<typename F>
        auto visit(F&& func) const {
            switch (get_kind()) {
            case eJsonString: return func(as_string());
            case eJsonInteger: return func(m_ast->integer);
            case eJsonFloat: return func(as_float());
            case eJsonBoolean: return func(as_bool());
            case eJsonArray: return func(as_array());
            case eJsonObject: return func(as_object());
            case eJsonNull: return func(nullptr);
            default: NEVER("invalid json kind %d", get_kind());
            }
        }
    };

    struct member_t {
        const char *key;
        Json value;
    };

    class ObjectIterator {
        map_iter_t m_iter;
        map_entry_t m_entry;

    public:
        ObjectIterator(map_iter_t iter)
            : m_iter(iter)
        { }

        ObjectIterator()
            : m_iter()
        { }

        bool operator!=(const ObjectIterator& other) const;
        ObjectIterator &operator++();
        member_t operator*() const;
    };

    class ArrayIterator {
        vector_t *m_array;
        size_t m_index;

    public:
        ArrayIterator(vector_t *array, size_t index)
            : m_array(array), m_index(index)
        { }

        bool operator!=(const ArrayIterator& other) const;
        ArrayIterator &operator++();
        Json operator*() const;
    };

    class JsonParser {
        arena_t *m_arena;
        logger_t *m_logger;

    public:
        JsonParser(arena_t *arena);

        Json parse(io_t *io);
    };
}
