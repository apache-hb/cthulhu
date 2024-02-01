#pragma once

#include "json/json.h"
#include "json/ast.h"

#include "base/panic.h"

namespace ctu::json {
    struct gmp_integer_t {
        mpz_t m_value;
    };

    class Json {
        json_ast_t *m_ast;

    public:
        bool is_string() const;
        bool is_integer() const;
        bool is_float() const;
        bool is_boolean() const;
        bool is_array() const;
        bool is_object() const;
        bool is_null() const;

        json_kind_t get_kind() const;

        text_view_t as_string() const;
        gmp_integer_t as_integer() const;
        float as_float() const;
        bool as_boolean() const;

        template<typename F>
        auto visit(F&& func) const {
            switch (get_kind()) {
            case eJsonString: return func(as_string());
            case eJsonInteger: return func(as_integer());
            case eJsonFloat: return func(as_float());
            case eJsonBoolean: return func(as_boolean());
            case eJsonArray: return func(as_array());
            case eJsonObject: return func(as_object());
            case eJsonNull: return func(as_null());
            default: NEVER("invalid json kind %d", get_kind());
            }
        }
    };

    class JsonParser {

    };
}
