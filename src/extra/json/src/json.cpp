#include "json/json.hpp"

#include "base/panic.h"

using namespace ctu;
using namespace ctu::json;

bool Json::is_string() const { return m_ast->kind == eJsonString; }
bool Json::is_integer() const { return m_ast->kind == eJsonInteger; }
bool Json::is_float() const { return m_ast->kind == eJsonFloat; }
bool Json::is_boolean() const { return m_ast->kind == eJsonBoolean; }
bool Json::is_array() const { return m_ast->kind == eJsonArray; }
bool Json::is_object() const { return m_ast->kind == eJsonObject; }
bool Json::is_null() const { return m_ast->kind == eJsonNull; }

json_kind_t Json::get_kind() const { CTASSERT(m_ast != nullptr); return m_ast->kind; }
