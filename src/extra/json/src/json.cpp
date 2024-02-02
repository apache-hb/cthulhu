#include "json/json.hpp"

#include "json/json.h"

#include "base/panic.h"
#include "notify/notify.h"
#include "std/vector.h"

using namespace ctu;
using namespace ctu::json;

Json Object::get(const char *key) const { return (json_t*)map_get(m_object, key); }
Json Object::operator[](const char *key) const { return get(key); }

ObjectIterator Object::begin() const { return map_iter(m_object); }
ObjectIterator Object::end() const { return {}; }

bool ObjectIterator::operator!=(const ObjectIterator&) const { return map_has_next(&m_iter); }
ObjectIterator &ObjectIterator::operator++() { m_entry = map_next(&m_iter); return *this; }
member_t ObjectIterator::operator*() const { return { (const char*)m_entry.key, (json_t*)m_entry.value }; }

Json Array::get(size_t index) const { return (json_t*)vector_get(m_array, index); }
Json Array::operator[](size_t index) const { return get(index); }
size_t Array::length() const { return vector_len(m_array); }

ArrayIterator Array::begin() const { return { m_array, 0 }; }
ArrayIterator Array::end() const { return { m_array, vector_len(m_array) }; }

bool ArrayIterator::operator!=(const ArrayIterator &other) const { return m_index != other.m_index; }
ArrayIterator &ArrayIterator::operator++() { m_index++; return *this; }
Json ArrayIterator::operator*() const { return (json_t*)vector_get(m_array, m_index); }

bool Json::is_string() const { return is_kind(eJsonString); }
bool Json::is_integer() const { return is_kind(eJsonInteger); }
bool Json::is_float() const { return is_kind(eJsonFloat); }
bool Json::is_bool() const { return is_kind(eJsonBoolean); }
bool Json::is_array() const { return is_kind(eJsonArray); }
bool Json::is_object() const { return is_kind(eJsonObject); }
bool Json::is_null() const { return is_kind(eJsonNull); }

bool Json::is_kind(json_kind_t kind) const { return get_kind() == kind; }

json_kind_t Json::get_kind() const {
    CTASSERT(is_valid());
    return m_ast->kind;
}

text_view_t Json::as_string() const {
    CTASSERT(is_string());
    text_t *string = &m_ast->string;
    return { string->text, string->length };
}

void Json::as_integer(mpz_t integer) const {
    CTASSERT(is_integer());
    mpz_init_set(integer, m_ast->integer);
}

float Json::as_float() const {
    CTASSERT(is_float());
    return m_ast->real;
}

bool Json::as_bool() const {
    CTASSERT(is_bool());
    return m_ast->boolean;
}

Array Json::as_array() const {
    CTASSERT(is_array());
    return { m_ast->array };
}

Object Json::as_object() const {
    CTASSERT(is_object());
    return { m_ast->object };
}

Json Json::get(const char *key) const {
    CTASSERT(is_object());
    return (json_t*)map_get(m_ast->object, key);
}

Json Json::get(size_t index) const {
    CTASSERT(is_array());
    return (json_t*)vector_get(m_ast->array, index);
}

JsonParser::JsonParser(arena_t *arena)
    : m_arena(arena)
    , m_logger(logger_new(arena))
{ }

Json JsonParser::parse(io_t *io) {
    return json_scan(io, m_logger, m_arena);
}
