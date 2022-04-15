#include "cthulhu/loader/hlir.h"

#include "cthulhu/hlir/type.h"
#include "cthulhu/hlir/decl.h"
#include "cthulhu/hlir/query.h"

#include "cthulhu/ast/compile.h"
#include "version.h"

///
/// layouts
///

typedef enum {
    SPAN_INDEX = HLIR_TOTAL,
    ATTRIBUTE_INDEX,
    LAYOUTS_TOTAL
} hlir_layouts_t;

typedef enum {
    SOURCE_PATH,
    SOURCE_TEXT,

    SOURCE_TOTAL
} source_kind_t;

#define INDICES(SPAN, ...) \
    enum { SPAN, __VA_ARGS__ }; \
    STATIC_ASSERT(SPAN == 0, "span: " STR(SPAN) " must always come first");

///
/// non-hlir types
///

enum { HEADER_LANGUAGE, HEADER_PATH, HEADER_SOURCE };
static const field_t HEADER_FIELDS[] = {
    [HEADER_LANGUAGE] = FIELD("language", FIELD_STRING),
    [HEADER_PATH] = FIELD("path", FIELD_STRING),
    [HEADER_SOURCE] = FIELD("source", FIELD_STRING)
};
static const layout_t HEADER_LAYOUT = LAYOUT("header", HEADER_FIELDS);

enum { SPAN_FIRST_LINE, SPAN_FIRST_COLUMN, SPAN_LAST_LINE, SPAN_LAST_COLUMN };
static const field_t SPAN_FIELDS[] = {
    [SPAN_FIRST_LINE] = FIELD("first-line", FIELD_INT),
    [SPAN_FIRST_COLUMN] = FIELD("first-column", FIELD_INT),
    [SPAN_LAST_LINE] = FIELD("last-line", FIELD_INT),
    [SPAN_LAST_COLUMN] = FIELD("last-column", FIELD_INT)
};
static const layout_t SPAN_LAYOUT = LAYOUT("span", SPAN_FIELDS);

enum { ATTRIB_LINKAGE, ATTRIB_TAGS };
static const field_t ATTRIB_FIELDS[] = {
    [ATTRIB_LINKAGE] = FIELD("linkage", FIELD_INT),
    [ATTRIB_TAGS] = FIELD("tags", FIELD_INT)
};
static const layout_t ATTRIB_LAYOUT = LAYOUT("attributes", ATTRIB_FIELDS);

///
/// literals
///

INDICES(DIGIT_LITERAL_SPAN, DIGIT_LITERAL_TYPE, DIGIT_LITERAL_VALUE);
static const field_t DIGIT_LITERAL_FIELDS[] = {
    [DIGIT_LITERAL_SPAN] = FIELD("span", FIELD_REFERENCE),
    [DIGIT_LITERAL_TYPE] = FIELD("type", FIELD_REFERENCE),
    [DIGIT_LITERAL_VALUE] = FIELD("value", FIELD_INT)
};
static const layout_t DIGIT_LITERAL_LAYOUT = LAYOUT("digit-literal", DIGIT_LITERAL_FIELDS);

INDICES(BOOL_LITERAL_SPAN, BOOL_LITERAL_TYPE, BOOL_LITERAL_VALUE);
static const field_t BOOL_LITERAL_FIELDS[] = {
    [BOOL_LITERAL_SPAN] = FIELD("span", FIELD_REFERENCE),
    [BOOL_LITERAL_TYPE] = FIELD("type", FIELD_REFERENCE),
    [BOOL_LITERAL_VALUE] = FIELD("value", FIELD_BOOL)
};
static const layout_t BOOL_LITERAL_LAYOUT = LAYOUT("bool-literal", BOOL_LITERAL_FIELDS);

INDICES(STRING_LITERAL_SPAN, STRING_LITERAL_TYPE, STRING_LITERAL_VALUE);
static const field_t STRING_LITERAL_FIELDS[] = {
    [STRING_LITERAL_SPAN] = FIELD("span", FIELD_REFERENCE),
    [STRING_LITERAL_TYPE] = FIELD("type", FIELD_REFERENCE),
    [STRING_LITERAL_VALUE] = FIELD("value", FIELD_STRING)
};
static const layout_t STRING_LITERAL_LAYOUT = LAYOUT("string-literal", STRING_LITERAL_FIELDS);

///
/// expressions
///

INDICES(NAME_SPAN, NAME_EXPR);
static const field_t NAME_FIELDS[] = {
    [NAME_SPAN] = FIELD("span", FIELD_REFERENCE),
    [NAME_EXPR] = FIELD("expr", FIELD_REFERENCE)
};
static const layout_t NAME_LAYOUT = LAYOUT("name", NAME_FIELDS);

INDICES(UNARY_SPAN, UNARY_TYPE, UNARY_OP, UNARY_EXPR);
static const field_t UNARY_FIELDS[] = {
    [UNARY_SPAN] = FIELD("span", FIELD_REFERENCE),
    [UNARY_TYPE] = FIELD("type", FIELD_REFERENCE),
    [UNARY_OP] = FIELD("op", FIELD_INT),
    [UNARY_EXPR] = FIELD("expr", FIELD_REFERENCE)
};
static const layout_t UNARY_LAYOUT = LAYOUT("unary", UNARY_FIELDS);

INDICES(BINARY_SPAN, BINARY_TYPE, BINARY_OP, BINARY_LHS, BINARY_RHS);
static const field_t BINARY_FIELDS[] = {
    [BINARY_SPAN] = FIELD("span", FIELD_REFERENCE),
    [BINARY_TYPE] = FIELD("type", FIELD_REFERENCE),
    [BINARY_OP] = FIELD("op", FIELD_INT),
    [BINARY_LHS] = FIELD("lhs", FIELD_REFERENCE),
    [BINARY_RHS] = FIELD("rhs", FIELD_REFERENCE)
};
static const layout_t BINARY_LAYOUT = LAYOUT("binary", BINARY_FIELDS);

INDICES(COMPARE_SPAN, COMPARE_TYPE, COMPARE_OP, COMPARE_LHS, COMPARE_RHS);
static const field_t COMPARE_FIELDS[] = {
    [COMPARE_SPAN] = FIELD("span", FIELD_REFERENCE),
    [COMPARE_TYPE] = FIELD("type", FIELD_REFERENCE),
    [COMPARE_OP] = FIELD("op", FIELD_INT),
    [COMPARE_LHS] = FIELD("lhs", FIELD_REFERENCE),
    [COMPARE_RHS] = FIELD("rhs", FIELD_REFERENCE)
};
static const layout_t COMPARE_LAYOUT = LAYOUT("compare", COMPARE_FIELDS);

INDICES(CALL_SPAN, CALL_EXPR, CALL_ARGS);
static const field_t CALL_FIELDS[] = {
    [CALL_SPAN] = FIELD("span", FIELD_REFERENCE),
    [CALL_EXPR] = FIELD("expr", FIELD_REFERENCE),
    [CALL_ARGS] = FIELD("args", FIELD_ARRAY)
};
static const layout_t CALL_LAYOUT = LAYOUT("call", CALL_FIELDS);

///
/// statements
///
INDICES(STMTS_SPAN, STMTS_OPS);
static const field_t STMTS_FIELDS[] = {
    [STMTS_SPAN] = FIELD("span", FIELD_REFERENCE),
    [STMTS_OPS] = FIELD("ops", FIELD_ARRAY)
};
static const layout_t STMTS_LAYOUT = LAYOUT("stmts", STMTS_FIELDS);

INDICES(BRANCH_SPAN, BRANCH_COND, BRANCH_THEN, BRANCH_ELSE);
static const field_t BRANCH_FIELDS[] = {
    [BRANCH_SPAN] = FIELD("span", FIELD_REFERENCE),
    [BRANCH_COND] = FIELD("cond", FIELD_REFERENCE),
    [BRANCH_THEN] = FIELD("then", FIELD_REFERENCE),
    [BRANCH_ELSE] = FIELD("else", FIELD_REFERENCE)
};
static const layout_t BRANCH_LAYOUT = LAYOUT("branch", BRANCH_FIELDS);

INDICES(LOOP_SPAN, LOOP_COND, LOOP_BODY, LOOP_ELSE);
static const field_t LOOP_FIELDS[] = {
    [LOOP_SPAN] = FIELD("span", FIELD_REFERENCE),
    [LOOP_COND] = FIELD("cond", FIELD_REFERENCE),
    [LOOP_BODY] = FIELD("body", FIELD_REFERENCE),
    [LOOP_ELSE] = FIELD("else", FIELD_REFERENCE)
};
static const layout_t LOOP_LAYOUT = LAYOUT("loop", LOOP_FIELDS);

INDICES(ASSIGN_SPAN, ASSIGN_DST, ASSIGN_SRC);
static const field_t ASSIGN_FIELDS[] = {
    [ASSIGN_SPAN] = FIELD("span", FIELD_REFERENCE),
    [ASSIGN_DST] = FIELD("dst", FIELD_REFERENCE),
    [ASSIGN_SRC] = FIELD("src", FIELD_REFERENCE)
};
static const layout_t ASSIGN_LAYOUT = LAYOUT("assign", ASSIGN_FIELDS);

///
/// types
///

INDICES(DIGIT_SPAN, DIGIT_NAME, DIGIT_SIGN, DIGIT_WIDTH);
static const field_t DIGIT_FIELDS[] = {
    [DIGIT_SPAN] = FIELD("span", FIELD_REFERENCE),
    [DIGIT_NAME] = FIELD("name", FIELD_STRING),
    [DIGIT_SIGN] = FIELD("sign", FIELD_INT),
    [DIGIT_WIDTH] = FIELD("width", FIELD_INT)
};
static const layout_t DIGIT_LAYOUT = LAYOUT("digit", DIGIT_FIELDS);

INDICES(BOOL_SPAN, BOOL_TYPE, BOOL_NAME);
static const field_t BOOL_FIELDS[] = {
    [BOOL_SPAN] = FIELD("span", FIELD_REFERENCE),
    [BOOL_NAME] = FIELD("name", FIELD_STRING)
};
static const layout_t BOOL_LAYOUT = LAYOUT("bool", BOOL_FIELDS);

INDICES(STRING_SPAN, STRING_NAME);
static const field_t STRING_FIELDS[] = {
    [STRING_SPAN] = FIELD("span", FIELD_REFERENCE),
    [STRING_NAME] = FIELD("name", FIELD_STRING)
};
static const layout_t STRING_LAYOUT = LAYOUT("string", STRING_FIELDS);

INDICES(VOID_SPAN, VOID_NAME);
static const field_t VOID_FIELDS[] = {
    [VOID_SPAN] = FIELD("span", FIELD_REFERENCE),
    [VOID_NAME] = FIELD("name", FIELD_STRING)
};
static const layout_t VOID_LAYOUT = LAYOUT("void", VOID_FIELDS);

INDICES(CLOSURE_SPAN, CLOSURE_NAME, CLOSURE_ARGS, CLOSURE_RESULT, CLOSURE_VARIADIC);
static const field_t CLOSURE_FIELDS[] = {
    [CLOSURE_SPAN] = FIELD("span", FIELD_REFERENCE),
    [CLOSURE_NAME] = FIELD("name", FIELD_STRING),
    [CLOSURE_ARGS] = FIELD("args", FIELD_ARRAY),
    [CLOSURE_RESULT] = FIELD("result", FIELD_REFERENCE),
    [CLOSURE_VARIADIC] = FIELD("variadic", FIELD_BOOL)
};
static const layout_t CLOSURE_LAYOUT = LAYOUT("closure", CLOSURE_FIELDS);

INDICES(POINTER_SPAN, POINTER_NAME, POINTER_TYPE, POINTER_INDEXABLE);
static const field_t POINTER_FIELDS[] = {
    [POINTER_SPAN] = FIELD("span", FIELD_REFERENCE),
    [POINTER_NAME] = FIELD("name", FIELD_STRING),
    [POINTER_TYPE] = FIELD("type", FIELD_REFERENCE),
    [POINTER_INDEXABLE] = FIELD("indexable", FIELD_BOOL)
};
static const layout_t POINTER_LAYOUT = LAYOUT("pointer", POINTER_FIELDS);

INDICES(ARRAY_SPAN, ARRAY_NAME, ARRAY_TYPE, ARRAY_LENGTH);
static const field_t ARRAY_FIELDS[] = {
    [ARRAY_SPAN] = FIELD("span", FIELD_REFERENCE),
    [ARRAY_NAME] = FIELD("name", FIELD_STRING),
    [ARRAY_TYPE] = FIELD("type", FIELD_REFERENCE),
    [ARRAY_LENGTH] = FIELD("length", FIELD_INT)
};
static const layout_t ARRAY_LAYOUT = LAYOUT("array", ARRAY_FIELDS);

// HLIR_TYPE is omitted

///
/// declarations
///

INDICES(LOCAL_SPAN, LOCAL_ATTRIBS, LOCAL_NAME, LOCAL_TYPE, LOCAL_INDEX);
static const field_t LOCAL_FIELDS[] = {
    [LOCAL_SPAN] = FIELD("span", FIELD_REFERENCE),
    [LOCAL_ATTRIBS] = FIELD("attribs", FIELD_REFERENCE),
    [LOCAL_NAME] = FIELD("name", FIELD_STRING),
    [LOCAL_TYPE] = FIELD("type", FIELD_REFERENCE),
    [LOCAL_INDEX] = FIELD("index", FIELD_INT)
};
static const layout_t LOCAL_LAYOUT = LAYOUT("local", LOCAL_FIELDS);

INDICES(GLOBAL_SPAN, GLOBAL_ATTRIBS, GLOBAL_NAME, GLOBAL_TYPE, GLOBAL_INIT);
static const field_t GLOBAL_FIELDS[] = {
    [GLOBAL_SPAN] = FIELD("span", FIELD_REFERENCE),
    [GLOBAL_ATTRIBS] = FIELD("attribs", FIELD_REFERENCE),
    [GLOBAL_NAME] = FIELD("name", FIELD_STRING),
    [GLOBAL_TYPE] = FIELD("type", FIELD_REFERENCE),
    [GLOBAL_INIT] = FIELD("init", FIELD_REFERENCE)
};
static const layout_t GLOBAL_LAYOUT = LAYOUT("global", GLOBAL_FIELDS);

INDICES(FUNCTION_SPAN, FUNCTION_ATTRIBS, FUNCTION_NAME, FUNCTION_PARAMS, FUNCTION_RESULT, FUNCTION_VARIADIC, FUNCTION_LOCALS, FUNCTION_BODY);
static const field_t FUNCTION_FIELDS[] = {
    [FUNCTION_SPAN] = FIELD("span", FIELD_REFERENCE),
    [FUNCTION_ATTRIBS] = FIELD("attribs", FIELD_REFERENCE),
    [FUNCTION_NAME] = FIELD("name", FIELD_STRING),
    [FUNCTION_PARAMS] = FIELD("params", FIELD_ARRAY),
    [FUNCTION_RESULT] = FIELD("result", FIELD_REFERENCE),
    [FUNCTION_VARIADIC] = FIELD("variadic", FIELD_BOOL),
    [FUNCTION_LOCALS] = FIELD("locals", FIELD_ARRAY),
    [FUNCTION_BODY] = FIELD("body", FIELD_REFERENCE)
};
static const layout_t FUNCTION_LAYOUT = LAYOUT("function", FUNCTION_FIELDS);

INDICES(STRUCT_SPAN, STRUCT_ATTRIBS, STRUCT_NAME, STRUCT_ITEMS);
static const field_t STRUCT_FIELDS[] = {
    [STRUCT_SPAN] = FIELD("span", FIELD_REFERENCE),
    [STRUCT_ATTRIBS] = FIELD("attribs", FIELD_REFERENCE),
    [STRUCT_NAME] = FIELD("name", FIELD_STRING),
    [STRUCT_ITEMS] = FIELD("items", FIELD_ARRAY)
};
static const layout_t STRUCT_LAYOUT = LAYOUT("struct", STRUCT_FIELDS);

INDICES(UNION_SPAN, UNION_ATTRIBS, UNION_NAME, UNION_ITEMS);
static const field_t UNION_FIELDS[] = {
    [UNION_SPAN] = FIELD("span", FIELD_REFERENCE),
    [UNION_ATTRIBS] = FIELD("attribs", FIELD_REFERENCE),
    [UNION_NAME] = FIELD("name", FIELD_STRING),
    [UNION_ITEMS] = FIELD("items", FIELD_ARRAY)
};
static const layout_t UNION_LAYOUT = LAYOUT("union", UNION_FIELDS);

INDICES(ALIAS_SPAN, ALIAS_ATTRIBS, ALIAS_NAME, ALIAS_TYPE, ALIAS_NEWTYPE);
static const field_t ALIAS_FIELDS[] = {
    [ALIAS_SPAN] = FIELD("span", FIELD_REFERENCE),
    [ALIAS_ATTRIBS] = FIELD("attribs", FIELD_REFERENCE),
    [ALIAS_NAME] = FIELD("name", FIELD_STRING),
    [ALIAS_TYPE] = FIELD("type", FIELD_REFERENCE),
    [ALIAS_NEWTYPE] = FIELD("newtype", FIELD_BOOL)
};
static const layout_t ALIAS_LAYOUT = LAYOUT("alias", ALIAS_FIELDS);

INDICES(FIELD_SPAN, FIELD_NAME, FIELD_TYPE);
static const field_t FIELD_FIELDS[] = {
    [FIELD_SPAN] = FIELD("span", FIELD_REFERENCE),
    [FIELD_NAME] = FIELD("name", FIELD_STRING),
    [FIELD_TYPE] = FIELD("type", FIELD_REFERENCE)
};
static const layout_t FIELD_LAYOUT = LAYOUT("field", FIELD_FIELDS);

INDICES(MODULE_SPAN, MODULE_NAME, MODULE_TYPES, MODULE_GLOBALS, MODULE_FUNCTIONS);
static const field_t MODULE_FIELDS[] = {
    [MODULE_SPAN] = FIELD("span", FIELD_REFERENCE),
    [MODULE_NAME] = FIELD("name", FIELD_STRING),
    [MODULE_TYPES] = FIELD("types", FIELD_ARRAY),
    [MODULE_GLOBALS] = FIELD("globals", FIELD_ARRAY),
    [MODULE_FUNCTIONS] = FIELD("functions", FIELD_ARRAY)
};
static const layout_t MODULE_LAYOUT = LAYOUT("module", MODULE_FIELDS);

///
/// final type table
///

static const layout_t ALL_TYPES[LAYOUTS_TOTAL] = {
    [SPAN_INDEX] = SPAN_LAYOUT,
    [ATTRIBUTE_INDEX] = ATTRIB_LAYOUT,

    [HLIR_DIGIT_LITERAL] = DIGIT_LITERAL_LAYOUT,
    [HLIR_BOOL_LITERAL] = BOOL_LITERAL_LAYOUT,
    [HLIR_STRING_LITERAL] = STRING_LITERAL_LAYOUT,

    [HLIR_NAME] = NAME_LAYOUT,
    [HLIR_UNARY] = UNARY_LAYOUT,
    [HLIR_BINARY] = BINARY_LAYOUT,
    [HLIR_COMPARE] = COMPARE_LAYOUT,
    [HLIR_CALL] = CALL_LAYOUT,

    [HLIR_STMTS] = STMTS_LAYOUT,
    [HLIR_BRANCH] = BRANCH_LAYOUT,
    [HLIR_LOOP] = LOOP_LAYOUT,
    [HLIR_ASSIGN] = ASSIGN_LAYOUT,

    [HLIR_DIGIT] = DIGIT_LAYOUT,
    [HLIR_BOOL] = BOOL_LAYOUT,
    [HLIR_STRING] = STRING_LAYOUT,
    [HLIR_VOID] = VOID_LAYOUT,
    [HLIR_CLOSURE] = CLOSURE_LAYOUT,
    [HLIR_POINTER] = POINTER_LAYOUT,
    [HLIR_ARRAY] = ARRAY_LAYOUT,
    
    [HLIR_STRUCT] = STRUCT_LAYOUT,
    [HLIR_UNION] = UNION_LAYOUT,
    [HLIR_ALIAS] = ALIAS_LAYOUT,
    [HLIR_FIELD] = FIELD_LAYOUT,

    [HLIR_LOCAL] = LOCAL_LAYOUT,
    [HLIR_GLOBAL] = GLOBAL_LAYOUT,
    [HLIR_FUNCTION] = FUNCTION_LAYOUT,

    [HLIR_MODULE] = MODULE_LAYOUT
};

static const format_t GLOBAL = FORMAT(HEADER_LAYOUT, ALL_TYPES);

#define HLIR_SUBMAGIC 0x484C4952 // 'HLIR'
#define HLIR_VERSION NEW_VERSION(CTHULHU_MAJOR, CTHULHU_MINOR, CTHULHU_PATCH)

///
/// hlir loading 
///

#define READ_OR_RETURN(data, index, values) do { bool result = read_entry(data, index, values); if (!result) return false; } while (0)

typedef struct {
    reports_t *reports;
    scan_t *scan;
    data_t *data;
} load_t;

static const node_t *load_span(load_t *load, index_t index) {
    value_t values[FIELDLEN(SPAN_FIELDS)];
    bool ok = read_entry(load->data, index, values);
    if (!ok) { return node_builtin(); }

    where_t where = {
        .first_line = get_int(values[SPAN_FIRST_LINE]),
        .first_column = get_int(values[SPAN_FIRST_COLUMN]),
        .last_line = get_int(values[SPAN_LAST_LINE]),
        .last_column = get_int(values[SPAN_LAST_COLUMN])
    };

    return node_new(load->scan, where);
}

static hlir_attributes_t *load_attributes(load_t *load, value_t value) {
    value_t values[FIELDLEN(ATTRIB_FIELDS)];
    READ_OR_RETURN(load->data, get_reference(value), values);

    return hlir_attributes(
        get_int(values[ATTRIB_LINKAGE]),
        get_int(values[ATTRIB_TAGS])
    );
}

static hlir_t *load_node(load_t *load, index_t index, const char *trace);

static hlir_t *load_opt_node(load_t *load, index_t index, const char *trace) {
    if (index.type == UINT32_MAX) { return NULL; }

    return load_node(load, index, trace);
}

static const node_t *get_span(load_t *load, value_t *values) {
    return load_span(load, get_reference(values[0])); // the ENUM_FIELDS macro makes sure that values[0] is a span
}

#define GET_REF(load, values, name) load_node(load, get_reference((values)[name]), "loading " #name)
#define GET_REF_OPT(load, values, name) load_opt_node(load, get_reference((values)[name]), "loading " #name)

static vector_t *load_array(load_t *load, array_t array) {
    size_t len = array.length;
    index_t *indices = ctu_malloc(sizeof(index_t) * len);
    vector_t *vec = vector_of(len);

    read_array(load->data, array, indices);

    for (size_t i = 0; i < len; i++) {
        hlir_t *hlir = load_node(load, indices[i], format("array@%zu[%zu + %zu]", array.offset, array.length, i));
        vector_set(vec, i, hlir);
    }

    return vec;
}

static vector_t *get_arr(load_t *load, value_t value) {
    return load_array(load, get_array(value));
}

// expressions

static hlir_t *load_digit_literal_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(DIGIT_LITERAL_FIELDS)];
    READ_OR_RETURN(load->data, index, values);

    mpz_t digit;
    get_digit(digit, values[DIGIT_LITERAL_VALUE]);

    return hlir_digit_literal(
        get_span(load, values), 
        GET_REF(load, values, DIGIT_LITERAL_TYPE), 
        digit
    );
}

static hlir_t *load_bool_literal_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(BOOL_LITERAL_FIELDS)];
    READ_OR_RETURN(load->data, index, values);

    return hlir_bool_literal(
        get_span(load, values), 
        GET_REF(load, values, BOOL_LITERAL_TYPE), 
        get_bool(values[BOOL_LITERAL_VALUE])
    );
}

static hlir_t *load_string_literal_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(STRING_LITERAL_FIELDS)];
    READ_OR_RETURN(load->data, index, values);

    return hlir_string_literal(
        get_span(load, values), 
        GET_REF(load, values, STRING_LITERAL_TYPE), 
        get_string(values[STRING_LITERAL_VALUE])
    );
}

static hlir_t *load_name_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(NAME_FIELDS)];
    READ_OR_RETURN(load->data, index, values);

    return hlir_name(
        get_span(load, values),
        GET_REF(load, values, NAME_EXPR)
    );
}

static hlir_t *load_unary_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(UNARY_FIELDS)];
    READ_OR_RETURN(load->data, index, values);

    return hlir_unary(
        get_span(load, values),
        GET_REF(load, values, UNARY_TYPE),
        GET_REF(load, values, UNARY_EXPR),
        get_int(values[UNARY_OP])
    );
}

static hlir_t *load_binary_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(BINARY_FIELDS)];
    READ_OR_RETURN(load->data, index, values);

    return hlir_binary(
        get_span(load, values),
        GET_REF(load, values, BINARY_TYPE),
        get_int(values[BINARY_OP]),
        GET_REF(load, values, BINARY_LHS),
        GET_REF(load, values, BINARY_RHS)
    );
}

static hlir_t *load_compare_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(COMPARE_FIELDS)];
    READ_OR_RETURN(load->data, index, values);

    return hlir_compare(
        get_span(load, values),
        GET_REF(load, values, COMPARE_TYPE),
        get_int(values[COMPARE_OP]),
        GET_REF(load, values, COMPARE_LHS),
        GET_REF(load, values, COMPARE_RHS)
    );
}

static hlir_t *load_call_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(CALL_FIELDS)];
    READ_OR_RETURN(load->data, index, values);

    return hlir_call(
        get_span(load, values),
        GET_REF(load, values, CALL_EXPR),
        get_arr(load, values[CALL_ARGS])
    );
}

// statements

static hlir_t *load_stmts_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(STMTS_FIELDS)];
    READ_OR_RETURN(load->data, index, values);

    return hlir_stmts(
        get_span(load, values),
        get_arr(load, values[STMTS_OPS])
    );
}

static hlir_t *load_branch_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(BRANCH_FIELDS)];
    READ_OR_RETURN(load->data, index, values);

    return hlir_branch(
        get_span(load, values),
        GET_REF(load, values, BRANCH_COND),
        GET_REF(load, values, BRANCH_THEN),
        GET_REF_OPT(load, values, BRANCH_ELSE)
    );
}

static hlir_t *load_loop_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(LOOP_FIELDS)];
    READ_OR_RETURN(load->data, index, values);

    return hlir_loop(
        get_span(load, values),
        GET_REF(load, values, LOOP_COND),
        GET_REF(load, values, LOOP_BODY),
        GET_REF_OPT(load, values, LOOP_ELSE)
    );
}

static hlir_t *load_assign_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(ASSIGN_FIELDS)];
    READ_OR_RETURN(load->data, index, values);

    return hlir_assign(
        get_span(load, values),
        GET_REF(load, values, ASSIGN_DST),
        GET_REF(load, values, ASSIGN_SRC)
    );
}

// types

static hlir_t *load_digit_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(DIGIT_FIELDS)];
    READ_OR_RETURN(load->data, index, values);

    return hlir_digit(
        get_span(load, values), 
        get_string(values[DIGIT_NAME]), 
        get_int(values[DIGIT_SIGN]), 
        get_int(values[DIGIT_WIDTH])
    );
}

static hlir_t *load_bool_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(BOOL_FIELDS)];
    READ_OR_RETURN(load->data, index, values);

    return hlir_bool(
        get_span(load, values), 
        get_string(values[BOOL_NAME])
    );
}

static hlir_t *load_string_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(STRING_FIELDS)];
    READ_OR_RETURN(load->data, index, values);

    return hlir_string(
        get_span(load, values), 
        get_string(values[STRING_NAME])
    );
}

static hlir_t *load_void_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(VOID_FIELDS)];
    READ_OR_RETURN(load->data, index, values);

    return hlir_void(
        get_span(load, values), 
        get_string(values[VOID_NAME])
    );
}

static hlir_t *load_closure_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(CLOSURE_FIELDS)];
    READ_OR_RETURN(load->data, index, values);

    return hlir_closure(
        get_span(load, values), 
        get_string(values[CLOSURE_NAME]),
        get_arr(load, values[CLOSURE_ARGS]),
        GET_REF(load, values, CLOSURE_RESULT),
        get_bool(values[CLOSURE_VARIADIC])
    );
}

static hlir_t *load_pointer_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(POINTER_FIELDS)];
    READ_OR_RETURN(load->data, index, values);

    return hlir_pointer(
        get_span(load, values), 
        get_string(values[POINTER_NAME]),
        GET_REF(load, values, POINTER_TYPE),
        get_bool(values[POINTER_INDEXABLE])
    );
}

static hlir_t *load_array_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(ARRAY_FIELDS)];
    READ_OR_RETURN(load->data, index, values);

    return hlir_array(
        get_span(load, values), 
        get_string(values[ARRAY_NAME]),
        GET_REF(load, values, ARRAY_TYPE),
        GET_REF(load, values, ARRAY_LENGTH)
    );
}

static hlir_t *load_local_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(LOCAL_FIELDS)];
    READ_OR_RETURN(load->data, index, values);

    return hlir_indexed_local(
        get_span(load, values), 
        get_string(values[LOCAL_NAME]), 
        get_int(values[LOCAL_INDEX]),
        GET_REF(load, values, LOCAL_TYPE)
    );
}

static hlir_t *load_global_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(GLOBAL_FIELDS)];
    READ_OR_RETURN(load->data, index, values);

    hlir_t *hlir = hlir_global(
        get_span(load, values), 
        get_string(values[GLOBAL_NAME]), 
        GET_REF(load, values, GLOBAL_TYPE),
        GET_REF_OPT(load, values, GLOBAL_INIT)
    );

    hlir_set_attributes(hlir, load_attributes(load, values[GLOBAL_ATTRIBS]));

    return hlir;
}

static hlir_t *load_function_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(FUNCTION_FIELDS)];
    READ_OR_RETURN(load->data, index, values);

    signature_t signature = {
        .params = get_arr(load, values[FUNCTION_PARAMS]),
        .result = GET_REF(load, values, FUNCTION_RESULT),
        .variadic = get_bool(values[FUNCTION_VARIADIC])
    };

    hlir_t *hlir = hlir_function(
        get_span(load, values), 
        get_string(values[FUNCTION_NAME]), 
        signature,
        get_arr(load, values[FUNCTION_LOCALS]),
        GET_REF_OPT(load, values, FUNCTION_BODY)
    );

    hlir_set_attributes(hlir, load_attributes(load, values[FUNCTION_ATTRIBS]));

    return hlir;
}

static hlir_t *load_struct_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(STRUCT_FIELDS)];
    READ_OR_RETURN(load->data, index, values);

    hlir_t *hlir = hlir_struct(
        get_span(load, values), 
        get_string(values[STRUCT_NAME]), 
        get_arr(load, values[STRUCT_ITEMS])
    );

    hlir_set_attributes(hlir, load_attributes(load, values[STRUCT_ATTRIBS]));

    return hlir;
}

static hlir_t *load_union_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(UNION_FIELDS)];
    READ_OR_RETURN(load->data, index, values);

    hlir_t *hlir = hlir_union(
        get_span(load, values), 
        get_string(values[UNION_NAME]), 
        get_arr(load, values[UNION_ITEMS])
    );

    hlir_set_attributes(hlir, load_attributes(load, values[UNION_ATTRIBS]));

    return hlir;
}

static hlir_t *load_alias_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(ALIAS_FIELDS)];
    READ_OR_RETURN(load->data, index, values);

    hlir_t *hlir = hlir_alias(
        get_span(load, values), 
        get_string(values[ALIAS_NAME]), 
        GET_REF(load, values, ALIAS_TYPE),
        get_bool(values[ALIAS_NEWTYPE])
    );

    hlir_set_attributes(hlir, load_attributes(load, values[ALIAS_ATTRIBS]));

    return hlir;
}

static hlir_t *load_field_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(FIELD_FIELDS)];
    READ_OR_RETURN(load->data, index, values);

    return hlir_field(
        get_span(load, values), 
        GET_REF(load, values, FIELD_TYPE),
        get_string(values[FIELD_NAME])
    );
}

static hlir_t *load_module_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(MODULE_FIELDS)];
    READ_OR_RETURN(load->data, index, values);

    return hlir_module(
        get_span(load, values), 
        get_string(values[MODULE_NAME]),
        get_arr(load, values[MODULE_TYPES]),
        get_arr(load, values[MODULE_GLOBALS]), 
        get_arr(load, values[MODULE_FUNCTIONS])
    );
}

static hlir_t *load_node(load_t *load, index_t index, const char *trace) {
    switch (index.type) {
    /* literals */
    case HLIR_DIGIT_LITERAL: return load_digit_literal_node(load, index);
    case HLIR_BOOL_LITERAL: return load_bool_literal_node(load, index);
    case HLIR_STRING_LITERAL: return load_string_literal_node(load, index);

    /* expressions */
    case HLIR_NAME: return load_name_node(load, index);
    case HLIR_UNARY: return load_unary_node(load, index);
    case HLIR_BINARY: return load_binary_node(load, index);
    case HLIR_COMPARE: return load_compare_node(load, index);
    case HLIR_CALL: return load_call_node(load, index);

    /* statements */
    case HLIR_STMTS: return load_stmts_node(load, index);
    case HLIR_BRANCH: return load_branch_node(load, index);
    case HLIR_LOOP: return load_loop_node(load, index);
    case HLIR_ASSIGN: return load_assign_node(load, index);

    /* types */
    case HLIR_DIGIT: return load_digit_node(load, index);
    case HLIR_BOOL: return load_bool_node(load, index);
    case HLIR_STRING: return load_string_node(load, index);
    case HLIR_VOID: return load_void_node(load, index);
    case HLIR_CLOSURE: return load_closure_node(load, index);
    case HLIR_POINTER: return load_pointer_node(load, index);
    case HLIR_ARRAY: return load_array_node(load, index);

    /* declarations */
    case HLIR_LOCAL: return load_local_node(load, index);
    case HLIR_GLOBAL: return load_global_node(load, index);
    case HLIR_FUNCTION: return load_function_node(load, index);

    case HLIR_STRUCT: return load_struct_node(load, index);
    case HLIR_UNION: return load_union_node(load, index);
    case HLIR_ALIAS: return load_alias_node(load, index);
    case HLIR_FIELD: return load_field_node(load, index);

    case HLIR_MODULE: return load_module_node(load, index);

    default:
        ctu_assert(load->reports, "loading unknown node type %d due to %s", index.type, trace);
        return NULL;
    }
}

static scan_t make_scanner(reports_t *reports, const char *lang, const char *path, const char *source) {
    if (source != NULL) {
        return scan_string(reports, lang, path, source);
    }

    file_t file = ctu_fopen(path, "r");
    file_t *fp = BOX(file);

    return scan_file(reports, lang, fp);
}

hlir_t *load_module(reports_t *reports, const char *path) {
    value_t values[FIELDLEN(HEADER_FIELDS)];
    record_t record = {
        .layout = &HEADER_LAYOUT,
        .values = values
    };

    header_t header = { 
        .reports = reports,
        .format = &GLOBAL,
        .path = path,
        .header = record,
        .submagic = HLIR_SUBMAGIC,
        .semver = HLIR_VERSION
    };

    data_t data;
    bool ok = begin_load(&data, header);
    if (!ok) { return NULL; }
    
    const char *language = get_string(data.header.header.values[HEADER_LANGUAGE]);
    const char *where = get_string(data.header.header.values[HEADER_PATH]);
    const char *source = get_string(data.header.header.values[HEADER_SOURCE]);

    logverbose("loading(lang=%s, path=%s, source=%s)", language, where, source != NULL ? "embedded" : "external");

    scan_t scan = make_scanner(reports, language, where, source);

    load_t load = {
        .reports = reports,
        .scan = BOX(scan),
        .data = &data
    };
    index_t index = { .type = HLIR_MODULE };
    hlir_t *hlir = load_node(&load, index, "root load");

    end_load(&data);

    return hlir;
}

///
/// hlir saving
///

static index_t save_span(data_t *data, const node_t *node) {
    if (node == NULL) { return NULL_INDEX; }

    value_t values[FIELDLEN(SPAN_FIELDS)] = {
        [SPAN_FIRST_LINE] = int_value(node->where.first_line),
        [SPAN_FIRST_COLUMN] = int_value(node->where.first_column),
        [SPAN_LAST_LINE] = int_value(node->where.last_line),
        [SPAN_LAST_COLUMN] = int_value(node->where.last_column)
    };
    
    return write_entry(data, SPAN_INDEX, values);
}

static index_t save_attributes(data_t *data, const hlir_attributes_t *attributes) {
    value_t values[FIELDLEN(ATTRIB_FIELDS)] = {
        [ATTRIB_LINKAGE] = int_value(attributes->linkage),
        [ATTRIB_TAGS] = int_value(attributes->tags)
    };

    return write_entry(data, ATTRIBUTE_INDEX, values);
}

static index_t save_node(data_t *data, const hlir_t *hlir);

static value_t make_ref(data_t *data, const hlir_t *hlir) {
    index_t index = save_node(data, hlir);
    return reference_value(index);
}

static value_t make_ref_opt(data_t *data, const hlir_t *hlir) {
    if (hlir == NULL) { return reference_value(NULL_INDEX); }

    index_t index = save_node(data, hlir);
    return reference_value(index);
}

static value_t span_ref(data_t *data, const hlir_t *hlir) {
    index_t index = save_span(data, hlir->node);
    return reference_value(index);
}

static value_t attrib_ref(data_t *data, const hlir_t *hlir) {
    index_t index = save_attributes(data, hlir->attributes);
    return reference_value(index);
}

static array_t save_array(data_t *data, vector_t *vec) {
    size_t len = vector_len(vec);
    index_t *indices = ctu_malloc(sizeof(index_t) * len);

    for (size_t i = 0; i < len; i++) {
        indices[i] = save_node(data, vector_get(vec, i));
    }

    array_t array = write_array(data, indices, len);

    return array;
}

// expressions

static index_t save_digit_literal_node(data_t *data, const hlir_t *hlir) {
    value_t values[] = {
        [DIGIT_LITERAL_SPAN] = span_ref(data, hlir),
        [DIGIT_LITERAL_TYPE] = make_ref(data, get_hlir_type(hlir)),
        [DIGIT_LITERAL_VALUE] = digit_value(hlir->digit)
    };

    return write_entry(data, HLIR_DIGIT_LITERAL, values);
}

static index_t save_bool_literal_node(data_t *data, const hlir_t *hlir) {
    value_t values[] = {
        [BOOL_LITERAL_SPAN] = span_ref(data, hlir),
        [BOOL_LITERAL_TYPE] = make_ref(data, get_hlir_type(hlir)),
        [BOOL_LITERAL_VALUE] = bool_value(hlir->boolean)
    };

    return write_entry(data, HLIR_BOOL_LITERAL, values);
}

static index_t save_string_literal_node(data_t *data, const hlir_t *hlir) {
    value_t values[] = {
        [STRING_LITERAL_SPAN] = span_ref(data, hlir),
        [STRING_LITERAL_TYPE] = make_ref(data, get_hlir_type(hlir)),
        [STRING_LITERAL_VALUE] = string_value(hlir->string)
    };

    return write_entry(data, HLIR_STRING_LITERAL, values);
}

static index_t save_name_node(data_t *data, const hlir_t *hlir) {
    value_t values[] = {
        [NAME_SPAN] = span_ref(data, hlir),
        [NAME_EXPR] = make_ref(data, hlir->read)
    };

    return write_entry(data, HLIR_NAME, values);
}

static index_t save_unary_node(data_t *data, const hlir_t *hlir) {
    value_t values[] = {
        [UNARY_SPAN] = span_ref(data, hlir),
        [UNARY_TYPE] = make_ref(data, get_hlir_type(hlir)),
        [UNARY_OP] = int_value(hlir->unary),
        [UNARY_EXPR] = make_ref(data, hlir->operand)
    };

    return write_entry(data, HLIR_UNARY, values);
}

static index_t save_binary_node(data_t *data, const hlir_t *hlir) {
    value_t values[] = {
        [BINARY_SPAN] = span_ref(data, hlir),
        [BINARY_TYPE] = make_ref(data, get_hlir_type(hlir)),
        [BINARY_OP] = int_value(hlir->binary),
        [BINARY_LHS] = make_ref(data, hlir->lhs),
        [BINARY_RHS] = make_ref(data, hlir->rhs)
    };

    return write_entry(data, HLIR_BINARY, values);
}

static index_t save_compare_node(data_t *data, const hlir_t *hlir) {
    value_t values[] = {
        [COMPARE_SPAN] = span_ref(data, hlir),
        [COMPARE_TYPE] = make_ref(data, get_hlir_type(hlir)),
        [COMPARE_OP] = int_value(hlir->compare),
        [COMPARE_LHS] = make_ref(data, hlir->lhs),
        [COMPARE_RHS] = make_ref(data, hlir->rhs)
    };

    return write_entry(data, HLIR_COMPARE, values);
}

static index_t save_call_node(data_t *data, const hlir_t *hlir) {
    array_t args = save_array(data, hlir->args);

    value_t values[] = {
        [CALL_SPAN] = span_ref(data, hlir),
        [CALL_EXPR] = make_ref(data, hlir->call),
        [CALL_ARGS] = array_value(args)
    };

    return write_entry(data, HLIR_CALL, values);
}

// statements

static index_t save_stmts_node(data_t *data, const hlir_t *hlir) {
    array_t stmts = save_array(data, hlir->stmts);

    value_t values[] = {
        [STMTS_SPAN] = span_ref(data, hlir),
        [STMTS_OPS] = array_value(stmts)
    };

    return write_entry(data, HLIR_STMTS, values);
}

static index_t save_branch_node(data_t *data, const hlir_t *hlir) {
    value_t values[] = {
        [BRANCH_SPAN] = span_ref(data, hlir),
        [BRANCH_COND] = make_ref(data, hlir->cond),
        [BRANCH_THEN] = make_ref(data, hlir->then),
        [BRANCH_ELSE] = make_ref_opt(data, hlir->other)
    };
    
    return write_entry(data, HLIR_BRANCH, values);
}

static index_t save_loop_node(data_t *data, const hlir_t *hlir) {
    value_t values[] = {
        [LOOP_SPAN] = span_ref(data, hlir),
        [LOOP_COND] = make_ref(data, hlir->cond),
        [LOOP_BODY] = make_ref(data, hlir->then),
        [LOOP_ELSE] = make_ref_opt(data, hlir->other)
    };

    return write_entry(data, HLIR_LOOP, values);
}

static index_t save_assign_node(data_t *data, const hlir_t *hlir) {
    value_t values[] = {
        [ASSIGN_SPAN] = span_ref(data, hlir),
        [ASSIGN_DST] = make_ref(data, hlir->dst),
        [ASSIGN_SRC] = make_ref(data, hlir->src)
    };

    return write_entry(data, HLIR_ASSIGN, values);
}

// types

static index_t save_digit_node(data_t *data, const hlir_t *hlir) {
    value_t values[] = {
        [DIGIT_SPAN] = span_ref(data, hlir),
        [DIGIT_NAME] = string_value(hlir->name),
        [DIGIT_SIGN] = int_value(hlir->sign),
        [DIGIT_WIDTH] = int_value(hlir->width)
    };

    return write_entry(data, HLIR_DIGIT, values);
}

static index_t save_bool_node(data_t *data, const hlir_t *hlir) {
    value_t values[] = {
        [BOOL_SPAN] = span_ref(data, hlir),
        [BOOL_NAME] = string_value(hlir->name)
    };

    return write_entry(data, HLIR_BOOL, values);
}

static index_t save_string_node(data_t *data, const hlir_t *hlir) {
    value_t values[] = {
        [STRING_SPAN] = span_ref(data, hlir),
        [STRING_NAME] = string_value(hlir->name)
    };

    return write_entry(data, HLIR_STRING, values);
}

static index_t save_void_node(data_t *data, const hlir_t *hlir) {
    value_t values[] = {
        [VOID_SPAN] = span_ref(data, hlir),
        [VOID_NAME] = string_value(hlir->name)
    };

    return write_entry(data, HLIR_VOID, values);
}

static index_t save_closure_node(data_t *data, const hlir_t *hlir) {
    array_t args = save_array(data, hlir->params);

    value_t values[] = {
        [CLOSURE_SPAN] = span_ref(data, hlir),
        [CLOSURE_NAME] = string_value(hlir->name),
        [CLOSURE_ARGS] = array_value(args),
        [CLOSURE_RESULT] = make_ref(data, hlir->result),
        [CLOSURE_VARIADIC] = bool_value(hlir->variadic)
    };

    return write_entry(data, HLIR_CLOSURE, values);
}

static index_t save_pointer_node(data_t *data, const hlir_t *hlir) {
    value_t values[] = {
        [POINTER_SPAN] = span_ref(data, hlir),
        [POINTER_NAME] = string_value(hlir->name),
        [POINTER_TYPE] = make_ref(data, hlir->ptr),
        [POINTER_INDEXABLE] = bool_value(hlir->indexable)
    };

    return write_entry(data, HLIR_POINTER, values);
}

static index_t save_array_node(data_t *data, const hlir_t *hlir) {
    value_t values[] = {
        [ARRAY_SPAN] = span_ref(data, hlir),
        [ARRAY_NAME] = string_value(hlir->name),
        [ARRAY_TYPE] = make_ref(data, hlir->element),
        [ARRAY_LENGTH] = make_ref(data, hlir->length)
    };

    return write_entry(data, HLIR_ARRAY, values);
}

/// declarations

static index_t save_local_node(data_t *data, const hlir_t *hlir) {
    value_t values[] = {
        [LOCAL_SPAN] = span_ref(data, hlir),
        [LOCAL_ATTRIBS] = attrib_ref(data, hlir),
        [LOCAL_NAME] = string_value(hlir->name),
        [LOCAL_TYPE] = make_ref(data, get_hlir_type(hlir)),
        [LOCAL_INDEX] = int_value(hlir->index)
    };
    
    return write_entry(data, HLIR_LOCAL, values);
}

static index_t save_global_node(data_t *data, const hlir_t *hlir) {
    value_t values[] = {
        [GLOBAL_SPAN] = span_ref(data, hlir),
        [GLOBAL_ATTRIBS] = attrib_ref(data, hlir),
        [GLOBAL_NAME] = string_value(hlir->name),
        [GLOBAL_TYPE] = make_ref(data, get_hlir_type(hlir)),
        [GLOBAL_INIT] = make_ref_opt(data, hlir->value)
    };
    
    return write_entry(data, HLIR_GLOBAL, values);
}

static index_t save_function_node(data_t *data, const hlir_t *hlir) {
    array_t locals = save_array(data, hlir->locals);
    array_t params = save_array(data, hlir->params);

    value_t values[] = {
        [FUNCTION_SPAN] = span_ref(data, hlir),
        [FUNCTION_ATTRIBS] = attrib_ref(data, hlir),
        [FUNCTION_NAME] = string_value(hlir->name),
        [FUNCTION_PARAMS] = array_value(params),
        [FUNCTION_RESULT] = make_ref(data, hlir->result),
        [FUNCTION_VARIADIC] = bool_value(hlir->variadic),
        [FUNCTION_LOCALS] = array_value(locals),
        [FUNCTION_BODY] = make_ref_opt(data, hlir->body)
    };

    return write_entry(data, HLIR_FUNCTION, values);
}

static index_t save_struct_node(data_t *data, const hlir_t *hlir) {
    array_t fields = save_array(data, hlir->fields);

    value_t values[] = {
        [STRUCT_SPAN] = span_ref(data, hlir),
        [STRUCT_ATTRIBS] = attrib_ref(data, hlir),
        [STRUCT_NAME] = string_value(hlir->name),
        [STRUCT_ITEMS] = array_value(fields)
    };

    return write_entry(data, HLIR_STRUCT, values);
}

static index_t save_union_node(data_t *data, const hlir_t *hlir) {
    array_t fields = save_array(data, hlir->fields);

    value_t values[] = {
        [UNION_SPAN] = span_ref(data, hlir),
        [UNION_ATTRIBS] = attrib_ref(data, hlir),
        [UNION_NAME] = string_value(hlir->name),
        [UNION_ITEMS] = array_value(fields)
    };

    return write_entry(data, HLIR_UNION, values);
}

static index_t save_alias_node(data_t *data, const hlir_t *hlir) {
    value_t values[] = {
        [ALIAS_SPAN] = span_ref(data, hlir),
        [ALIAS_ATTRIBS] = attrib_ref(data, hlir),
        [ALIAS_NAME] = string_value(hlir->name),
        [ALIAS_TYPE] = make_ref(data, hlir->alias)
    };

    return write_entry(data, HLIR_ALIAS, values);
}

static index_t save_field_node(data_t *data, const hlir_t *hlir) {
    value_t values[] = {
        [FIELD_SPAN] = span_ref(data, hlir),
        [FIELD_NAME] = string_value(hlir->name),
        [FIELD_TYPE] = make_ref(data, get_hlir_type(hlir))
    };

    return write_entry(data, HLIR_FIELD, values);
}

static index_t save_module_node(data_t *data, const hlir_t *hlir) {
    array_t types = save_array(data, hlir->types);
    array_t globals = save_array(data, hlir->globals);
    array_t functions = save_array(data, hlir->functions);
    
    value_t values[] = {
        [MODULE_SPAN] = span_ref(data, hlir),
        [MODULE_NAME] = string_value(hlir->name),
        [MODULE_TYPES] = array_value(types),
        [MODULE_GLOBALS] = array_value(globals),
        [MODULE_FUNCTIONS] = array_value(functions)
    };
    
    return write_entry(data, HLIR_MODULE, values);
}

static index_t save_node(data_t *data, const hlir_t *hlir) {
    switch (hlir->type) {
    /* literals */
    case HLIR_DIGIT_LITERAL: return save_digit_literal_node(data, hlir);
    case HLIR_BOOL_LITERAL: return save_bool_literal_node(data, hlir);
    case HLIR_STRING_LITERAL: return save_string_literal_node(data, hlir);

    /* expressions */
    case HLIR_NAME: return save_name_node(data, hlir);
    case HLIR_UNARY: return save_unary_node(data, hlir);
    case HLIR_BINARY: return save_binary_node(data, hlir);
    case HLIR_COMPARE: return save_compare_node(data, hlir);
    case HLIR_CALL: return save_call_node(data, hlir);

    /* statements */
    case HLIR_STMTS: return save_stmts_node(data, hlir);
    case HLIR_BRANCH: return save_branch_node(data, hlir);
    case HLIR_LOOP: return save_loop_node(data, hlir);
    case HLIR_ASSIGN: return save_assign_node(data, hlir);

    /* types */
    case HLIR_DIGIT: return save_digit_node(data, hlir);
    case HLIR_BOOL: return save_bool_node(data, hlir);
    case HLIR_STRING: return save_string_node(data, hlir);
    case HLIR_VOID: return save_void_node(data, hlir);
    case HLIR_CLOSURE: return save_closure_node(data, hlir);
    case HLIR_POINTER: return save_pointer_node(data, hlir);
    case HLIR_ARRAY: return save_array_node(data, hlir);
    case HLIR_TYPE: return NULL_INDEX;

    /* declarations */
    case HLIR_LOCAL: return save_local_node(data, hlir);
    case HLIR_GLOBAL: return save_global_node(data, hlir);
    case HLIR_FUNCTION: return save_function_node(data, hlir);

    case HLIR_STRUCT: return save_struct_node(data, hlir);
    case HLIR_UNION: return save_union_node(data, hlir);
    case HLIR_ALIAS: return save_alias_node(data, hlir);
    case HLIR_FIELD: return save_field_node(data, hlir);

    case HLIR_MODULE: return save_module_node(data, hlir);

    default:
        ctu_assert(data->header.reports, "saving unknown node type %u", hlir->type);
        return NULL_INDEX;
    }
}

void save_module(reports_t *reports, save_settings_t *settings, hlir_t *module, const char *path) {
    scan_t *scan = module->node->scan;

    value_t values[] = {
        [HEADER_LANGUAGE] = string_value(scan->language),
        [HEADER_PATH] = string_value(scan->path),
        [HEADER_SOURCE] = string_value(NULL)
    };

    if (settings->embed_source) {
        values[HEADER_SOURCE] = string_value(scan_text(scan));
    }

    record_t record = {
        .layout = &HEADER_LAYOUT,
        .values = values
    };

    header_t header = { 
        .reports = reports,
        .format = &GLOBAL,
        .path = path,
        .header = record,
        .submagic = HLIR_SUBMAGIC,
        .semver = HLIR_VERSION
    };

    data_t data;
    begin_save(&data, header);

    save_node(&data, module);

    end_save(&data);
}
