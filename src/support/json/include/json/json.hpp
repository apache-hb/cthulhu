#pragma once

#include "json/json.h"

#include "std/map.h"

#include "base/panic.h"

#include <string_view>

typedef struct logger_t logger_t;
typedef struct io_t io_t;

/// @ingroup json
/// @{

namespace ctu::json {
    class Json;
    class Object;
    class Array;

    class ObjectIterator;
    class ArrayIterator;

    /// @brief a json object value
    class CT_JSON_API Object {
        friend class Json;

        const map_t *m_object;

        constexpr Object(const map_t *object)
            : m_object(object)
        { }

    public:
        /// @brief get a value
        /// access a value from the object by key
        /// @note if the key does not exist an invalid json value is returned
        ///
        /// @param key the key to access
        ///
        /// @return the value at @p key
        Json get(const char *key) const;

        /// @brief get a value
        /// same as @ref get
        ///
        /// @param key the key to access
        ///
        /// @return the value at @p key
        Json operator[](const char *key) const;

        /// @brief start iterating
        /// iterate over the members of the object
        /// @warning the order of iteration is not guaranteed
        ///
        /// @return the start of the iteration
        ObjectIterator begin() const;

        /// @brief end iterating
        ///
        /// @return the end of the iteration
        ObjectIterator end() const;

        ObjectIterator iter() const;
    };

    /// @brief a json array value
    class CT_JSON_API Array {
        friend class Json;

        const typevec_t m_array;

        constexpr Array(typevec_t array)
            : m_array(array)
        { }

    public:
        /// @brief get a value
        /// access a value from the array by index
        /// @warning if the index is out of bounds the function will assert
        ///
        /// @param index the index to access
        ///
        /// @return the value at @p index
        Json get(size_t index) const;

        /// @brief get a value
        /// same as @ref get
        ///
        /// @param index the index to access
        ///
        /// @return the value at @p index
        Json operator[](size_t index) const;

        /// @brief get the length of the array
        ///
        /// @return the length of the array
        size_t length() const;

        /// @brief start iterating
        /// iterate over the values of the array
        ///
        /// @return the start of the iteration
        ArrayIterator begin() const;

        /// @brief end iterating
        ///
        /// @return the end of the iteration
        ArrayIterator end() const;
    };

    /// @brief a json value
    /// a json node from a parsed json document
    /// @warning this class may be in an invalid state in certain cases
    ///          always check the validity of the value with @ref is_valid
    ///          before using it
    class CT_JSON_API Json {
        friend class JsonParser;
        friend class Object;
        friend class Array;
        friend class ObjectIterator;
        friend class ArrayIterator;

        json_t *m_ast;

    public:
        constexpr Json(json_t *ast)
            : m_ast(ast)
        { }

        constexpr Json()
            : m_ast(nullptr)
        { }

        /// @brief check if the value is a string
        /// @retval true if the value is a string
        /// @retval false if the value is not a string
        bool is_string() const;

        /// @brief check if the value is an integer
        /// @retval true if the value is an integer
        /// @retval false if the value is not an integer
        bool is_integer() const;

        /// @brief check if the value is a float
        /// @retval true if the value is a float
        /// @retval false if the value is not a float
        bool is_float() const;

        /// @brief check if the value is a boolean
        /// @retval true if the value is a boolean
        /// @retval false if the value is not a boolean
        bool is_bool() const;

        /// @brief check if the value is an array
        /// @retval true if the value is an array
        /// @retval false if the value is not an array
        bool is_array() const;

        /// @brief check if the value is an object
        /// @retval true if the value is an object
        /// @retval false if the value is not an object
        bool is_object() const;

        /// @brief check if the value is null
        /// @retval true if the value is null
        /// @retval false if the value is not null
        bool is_null() const;

        /// @brief check if the value is valid
        /// this should always be checked before using any other methods
        ///
        /// @retval true if the value is valid
        /// @retval false if the value is not valid
        bool is_valid() const { return m_ast != nullptr; }

        /// @brief check if the value is valid
        /// @pre #ctu::json::Json::is_valid() is true
        ///
        /// @retval true if the value is valid
        /// @retval false if the value is not valid
        operator bool() const { return is_valid(); }

        /// @brief get the kind of the value
        ///
        /// @return the kind of the value
        json_kind_t get_kind() const;

        /// @brief check if the value is a specific kind
        ///
        /// @param kind the kind to check for
        ///
        /// @retval true if the value is of kind @p kind
        /// @retval false if the value is not of kind @p kind
        bool is_kind(json_kind_t kind) const;

        /// @brief get the string value
        /// @pre #ctu::json::Json::is_string() is true
        /// returns a text view as string values may have embedded nulls
        ///
        /// @return the string value
        std::string_view as_string() const;

        /// @brief get the integer value
        /// @pre #ctu::json::Json::is_integer() is true
        ///
        /// @param[out] integer the integer output value
        void as_integer(mpz_t integer) const;

        /// @brief get the float value
        /// @pre #ctu::json::Json::is_float() is true
        ///
        /// @return the float value
        float as_float() const;

        /// @brief get the boolean value
        /// @pre #ctu::json::Json::is_bool() is true
        ///
        /// @return the boolean value
        bool as_bool() const;

        /// @brief get the array value
        /// @pre #ctu::json::Json::is_array() is true
        /// mostly useful for iterating over the array
        ///
        /// @return the array value
        Array as_array() const;

        /// @brief get the object value
        /// @pre #ctu::json::Json::is_object() is true
        /// mostly useful for iterating over the object
        ///
        /// @return the object value
        Object as_object() const;

        /// @brief get the length of an array
        /// @pre #ctu::json::Json::is_array() is true
        ///
        /// @return the length of the array
        size_t length() const;

        /// @brief get a value from an object by key
        /// @pre #ctu::json::Json::is_object() is true
        ///
        /// @param key the key to access
        ///
        /// @return the value at @p key
        Json get(const char *key) const;

        /// @brief get a value from an array by index
        /// @pre #ctu::json::Json::is_array() and @p index < @ref length
        ///
        /// @param index the index to access
        ///
        /// @return the value at @p index
        Json get(size_t index) const;

        /// @brief get the underlying ast node
        ///
        /// @return the ast node
        json_t *get_ast() const { return m_ast; }

        /// @brief get a value from an object by key
        /// same as get(const char*)
        ///
        /// @param key the key to access
        ///
        /// @return the value at @p key
        Json operator[](const char *key) const;

        /// @brief get a value from an array by index
        /// same as get(size_t)
        ///
        /// @param index the index to access
        ///
        /// @return the value at @p index
        Json operator[](size_t index) const;

        /// @brief visit the value
        /// calls the appropriate function for the value kind
        ///
        /// @tparam F the function type
        /// @param func the function to call
        ///
        /// @return the result of the function
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
            default: CT_NEVER("invalid json kind %d", get_kind());
            }
        }
    };

    /// @brief a member of a json object
    struct CT_JSON_API member_t {
        /// @brief the key of the member
        std::string_view key;

        /// @brief the value of the member
        Json value;
    };

    /// @brief an iterator over the members of a json object
    class CT_JSON_API ObjectIterator {
        friend class Object;

        map_iter_t m_iter;
        map_entry_t m_entry;

        constexpr ObjectIterator(map_iter_t iter)
            : m_iter(iter)
        { }

        constexpr ObjectIterator()
            : m_iter()
        { }

    public:
        bool operator!=(const ObjectIterator& other) const;
        ObjectIterator &operator++();
        member_t operator*() const;

        bool has_next() const;
        member_t next();
    };

    /// @brief an iterator over the values of a json array
    class CT_JSON_API ArrayIterator {
        friend class Array;

        typevec_t m_array;
        size_t m_index;

        constexpr ArrayIterator(typevec_t array, size_t index)
            : m_array(array)
            , m_index(index)
        { }

    public:
        bool operator!=(const ArrayIterator& other) const;
        ArrayIterator &operator++();
        Json operator*() const;
    };

    /// @brief a json parser
    class CT_JSON_API JsonParser {
        arena_t *m_arena;
        logger_t *m_logger;

    public:
        /// @brief create a json parser
        ///
        /// @param arena the arena to allocate memory from
        JsonParser(arena_t *arena);

        /// @brief parse a json value
        /// parse the contents of an io object into a json value
        /// @note if the parse fails, #ctu::json::JsonParser::get_logger will contain error information
        ///
        /// @param io the io object to parse
        ///
        /// @return the parsed json value, invalid if the parse failed
        Json parse(io_t *io);

        /// @brief get the logger
        ///
        /// @return the logger
        logger_t *get_logger() const { return m_logger; }
    };
}

/// @}
