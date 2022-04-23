#include "cthulhu/loader/hlir.h"

#include "cthulhu/hlir/decl.h"
#include "cthulhu/hlir/query.h"
#include "cthulhu/hlir/type.h"

#include "common.h"
#include "cthulhu/ast/compile.h"
#include "version.h"

///
/// layouts
///

typedef enum {
    LAYOUTS_START = HLIR_TOTAL,

    SPAN_INDEX,
    ATTRIBUTE_INDEX,

    LAYOUTS_TOTAL
} hlir_layouts_t;

typedef enum {
    SOURCE_PATH,
    SOURCE_TEXT,

    SOURCE_TOTAL
} source_kind_t;

#define INDICES(SPAN, ...) \
    enum {                 \
        SPAN,              \
        __VA_ARGS__        \
    };

///
/// non-hlir types
///

enum {
    HEADER_LANGUAGE,
    HEADER_PATH,
    HEADER_SOURCE
};
static const field_t kHeaderFields[] = {
    [HEADER_LANGUAGE] = FIELD("language", FIELD_STRING),
    [HEADER_PATH] = FIELD("path", FIELD_STRING),
    [HEADER_SOURCE] = FIELD("source", FIELD_STRING),
};
static const layout_t kHeaderLayout = LAYOUT("header", kHeaderFields);

enum {
    SPAN_FIRST_LINE,
    SPAN_FIRST_COLUMN,
    SPAN_LAST_LINE,
    SPAN_LAST_COLUMN
};
static const field_t kSpanFields[] = {
    [SPAN_FIRST_LINE] = FIELD("first-line", FIELD_INT),
    [SPAN_FIRST_COLUMN] = FIELD("first-column", FIELD_INT),
    [SPAN_LAST_LINE] = FIELD("last-line", FIELD_INT),
    [SPAN_LAST_COLUMN] = FIELD("last-column", FIELD_INT),
};
static const layout_t kSpanLayout = LAYOUT("span", kSpanFields);

enum {
    ATTRIB_LINKAGE,
    ATTRIB_TAGS
};
static const field_t kAttribFields[] = {
    [ATTRIB_LINKAGE] = FIELD("linkage", FIELD_INT),
    [ATTRIB_TAGS] = FIELD("tags", FIELD_INT),
};
static const layout_t kAttribLayout = LAYOUT("attributes", kAttribFields);

///
/// literals
///

INDICES(DIGIT_LITERAL_SPAN, DIGIT_LITERAL_TYPE, DIGIT_LITERAL_VALUE);
static const field_t kDigitLiteralFields[] = {
    [DIGIT_LITERAL_SPAN] = FIELD("span", FIELD_REFERENCE),
    [DIGIT_LITERAL_TYPE] = FIELD("type", FIELD_REFERENCE),
    [DIGIT_LITERAL_VALUE] = FIELD("value", FIELD_INT),
};
static const layout_t kDigitLiteralLayout = LAYOUT("digit-literal", kDigitLiteralFields);

INDICES(BOOL_LITERAL_SPAN, BOOL_LITERAL_TYPE, BOOL_LITERAL_VALUE);
static const field_t kBoolLiteralFields[] = {
    [BOOL_LITERAL_SPAN] = FIELD("span", FIELD_REFERENCE),
    [BOOL_LITERAL_TYPE] = FIELD("type", FIELD_REFERENCE),
    [BOOL_LITERAL_VALUE] = FIELD("value", FIELD_BOOL),
};
static const layout_t kBoolLiteralLayout = LAYOUT("bool-literal", kBoolLiteralFields);

INDICES(STRING_LITERAL_SPAN, STRING_LITERAL_TYPE, STRING_LITERAL_VALUE);
static const field_t kStringLiteralFields[] = {
    [STRING_LITERAL_SPAN] = FIELD("span", FIELD_REFERENCE),
    [STRING_LITERAL_TYPE] = FIELD("type", FIELD_REFERENCE),
    [STRING_LITERAL_VALUE] = FIELD("value", FIELD_STRING),
};
static const layout_t kStringLiteralLayout = LAYOUT("string-literal", kStringLiteralFields);

///
/// expressions
///

INDICES(NAME_SPAN, NAME_EXPR);
static const field_t kNameFields[] = {
    [NAME_SPAN] = FIELD("span", FIELD_REFERENCE),
    [NAME_EXPR] = FIELD("expr", FIELD_REFERENCE),
};
static const layout_t kNameLayout = LAYOUT("name", kNameFields);

INDICES(UNARY_SPAN, UNARY_TYPE, UNARY_OP, UNARY_EXPR);
static const field_t kUnaryFields[] = {
    [UNARY_SPAN] = FIELD("span", FIELD_REFERENCE),
    [UNARY_TYPE] = FIELD("type", FIELD_REFERENCE),
    [UNARY_OP] = FIELD("op", FIELD_INT),
    [UNARY_EXPR] = FIELD("expr", FIELD_REFERENCE),
};
static const layout_t kUnaryLayout = LAYOUT("unary", kUnaryFields);

INDICES(BINARY_SPAN, BINARY_TYPE, BINARY_OP, BINARY_LHS, BINARY_RHS);
static const field_t kBinaryFields[] = {
    [BINARY_SPAN] = FIELD("span", FIELD_REFERENCE), [BINARY_TYPE] = FIELD("type", FIELD_REFERENCE),
    [BINARY_OP] = FIELD("op", FIELD_INT),           [BINARY_LHS] = FIELD("lhs", FIELD_REFERENCE),
    [BINARY_RHS] = FIELD("rhs", FIELD_REFERENCE),
};
static const layout_t kBinaryLayout = LAYOUT("binary", kBinaryFields);

INDICES(COMPARE_SPAN, COMPARE_TYPE, COMPARE_OP, COMPARE_LHS, COMPARE_RHS);
static const field_t kCompareFields[] = {
    [COMPARE_SPAN] = FIELD("span", FIELD_REFERENCE), [COMPARE_TYPE] = FIELD("type", FIELD_REFERENCE),
    [COMPARE_OP] = FIELD("op", FIELD_INT),           [COMPARE_LHS] = FIELD("lhs", FIELD_REFERENCE),
    [COMPARE_RHS] = FIELD("rhs", FIELD_REFERENCE),
};
static const layout_t kCompareLayout = LAYOUT("compare", kCompareFields);

INDICES(CALL_SPAN, CALL_EXPR, CALL_ARGS);
static const field_t kCallFields[] = {
    [CALL_SPAN] = FIELD("span", FIELD_REFERENCE),
    [CALL_EXPR] = FIELD("expr", FIELD_REFERENCE),
    [CALL_ARGS] = FIELD("args", FIELD_ARRAY),
};
static const layout_t kCallLayout = LAYOUT("call", kCallFields);

///
/// statements
///
INDICES(STMTS_SPAN, STMTS_OPS);
static const field_t kStmtsFields[] = {
    [STMTS_SPAN] = FIELD("span", FIELD_REFERENCE),
    [STMTS_OPS] = FIELD("ops", FIELD_ARRAY),
};
static const layout_t kStmtsLayout = LAYOUT("stmts", kStmtsFields);

INDICES(BRANCH_SPAN, BRANCH_COND, BRANCH_THEN, BRANCH_ELSE);
static const field_t kBranchFields[] = {
    [BRANCH_SPAN] = FIELD("span", FIELD_REFERENCE),
    [BRANCH_COND] = FIELD("cond", FIELD_REFERENCE),
    [BRANCH_THEN] = FIELD("then", FIELD_REFERENCE),
    [BRANCH_ELSE] = FIELD("else", FIELD_REFERENCE),
};
static const layout_t kBranchLayout = LAYOUT("branch", kBranchFields);

INDICES(LOOP_SPAN, LOOP_COND, LOOP_BODY, LOOP_ELSE);
static const field_t kLoopFields[] = {
    [LOOP_SPAN] = FIELD("span", FIELD_REFERENCE),
    [LOOP_COND] = FIELD("cond", FIELD_REFERENCE),
    [LOOP_BODY] = FIELD("body", FIELD_REFERENCE),
    [LOOP_ELSE] = FIELD("else", FIELD_REFERENCE),
};
static const layout_t kLoopLayout = LAYOUT("loop", kLoopFields);

INDICES(ASSIGN_SPAN, ASSIGN_DST, ASSIGN_SRC);
static const field_t kAssignFields[] = {
    [ASSIGN_SPAN] = FIELD("span", FIELD_REFERENCE),
    [ASSIGN_DST] = FIELD("dst", FIELD_REFERENCE),
    [ASSIGN_SRC] = FIELD("src", FIELD_REFERENCE),
};
static const layout_t kAssignLayout = LAYOUT("assign", kAssignFields);

///
/// types
///

INDICES(DIGIT_SPAN, DIGIT_NAME, DIGIT_SIGN, DIGIT_WIDTH);
static const field_t kDigitFields[] = {
    [DIGIT_SPAN] = FIELD("span", FIELD_REFERENCE),
    [DIGIT_NAME] = FIELD("name", FIELD_STRING),
    [DIGIT_SIGN] = FIELD("sign", FIELD_INT),
    [DIGIT_WIDTH] = FIELD("width", FIELD_INT),
};
static const layout_t kDigitLayout = LAYOUT("digit", kDigitFields);

INDICES(BOOL_SPAN, BOOL_TYPE, BOOL_NAME);
static const field_t kBoolFields[] = {
    [BOOL_SPAN] = FIELD("span", FIELD_REFERENCE),
    [BOOL_NAME] = FIELD("name", FIELD_STRING),
};
static const layout_t kBoolLayout = LAYOUT("bool", kBoolFields);

INDICES(STRING_SPAN, STRING_NAME);
static const field_t kStringFields[] = {
    [STRING_SPAN] = FIELD("span", FIELD_REFERENCE),
    [STRING_NAME] = FIELD("name", FIELD_STRING),
};
static const layout_t kStringLayout = LAYOUT("string", kStringFields);

INDICES(VOID_SPAN, VOID_NAME);
static const field_t kVoidFields[] = {
    [VOID_SPAN] = FIELD("span", FIELD_REFERENCE),
    [VOID_NAME] = FIELD("name", FIELD_STRING),
};
static const layout_t kVoidLayout = LAYOUT("void", kVoidFields);

INDICES(CLOSURE_SPAN, CLOSURE_NAME, CLOSURE_ARGS, CLOSURE_RESULT, CLOSURE_VARIADIC);
static const field_t kClosureFields[] = {
    [CLOSURE_SPAN] = FIELD("span", FIELD_REFERENCE),    [CLOSURE_NAME] = FIELD("name", FIELD_STRING),
    [CLOSURE_ARGS] = FIELD("args", FIELD_ARRAY),        [CLOSURE_RESULT] = FIELD("result", FIELD_REFERENCE),
    [CLOSURE_VARIADIC] = FIELD("variadic", FIELD_BOOL),
};
static const layout_t kClosureLayout = LAYOUT("closure", kClosureFields);

INDICES(POINTER_SPAN, POINTER_NAME, POINTER_TYPE, POINTER_INDEXABLE);
static const field_t kPointerFields[] = {
    [POINTER_SPAN] = FIELD("span", FIELD_REFERENCE),
    [POINTER_NAME] = FIELD("name", FIELD_STRING),
    [POINTER_TYPE] = FIELD("type", FIELD_REFERENCE),
    [POINTER_INDEXABLE] = FIELD("indexable", FIELD_BOOL),
};
static const layout_t kPointerLayout = LAYOUT("pointer", kPointerFields);

INDICES(ARRAY_SPAN, ARRAY_NAME, ARRAY_TYPE, ARRAY_LENGTH);
static const field_t kArrayFields[] = {
    [ARRAY_SPAN] = FIELD("span", FIELD_REFERENCE),
    [ARRAY_NAME] = FIELD("name", FIELD_STRING),
    [ARRAY_TYPE] = FIELD("type", FIELD_REFERENCE),
    [ARRAY_LENGTH] = FIELD("length", FIELD_INT),
};
static const layout_t kArrayLayout = LAYOUT("array", kArrayFields);

// HLIR_TYPE is omitted

///
/// declarations
///

INDICES(LOCAL_SPAN, LOCAL_ATTRIBS, LOCAL_NAME, LOCAL_TYPE, LOCAL_INDEX);
static const field_t kLocalFields[] = {
    [LOCAL_SPAN] = FIELD("span", FIELD_REFERENCE), [LOCAL_ATTRIBS] = FIELD("attribs", FIELD_REFERENCE),
    [LOCAL_NAME] = FIELD("name", FIELD_STRING),    [LOCAL_TYPE] = FIELD("type", FIELD_REFERENCE),
    [LOCAL_INDEX] = FIELD("index", FIELD_INT),
};
static const layout_t kLocalLayout = LAYOUT("local", kLocalFields);

INDICES(GLOBAL_SPAN, GLOBAL_ATTRIBS, GLOBAL_NAME, GLOBAL_TYPE, GLOBAL_INIT);
static const field_t kGlobalFields[] = {
    [GLOBAL_SPAN] = FIELD("span", FIELD_REFERENCE), [GLOBAL_ATTRIBS] = FIELD("attribs", FIELD_REFERENCE),
    [GLOBAL_NAME] = FIELD("name", FIELD_STRING),    [GLOBAL_TYPE] = FIELD("type", FIELD_REFERENCE),
    [GLOBAL_INIT] = FIELD("init", FIELD_REFERENCE),
};
static const layout_t kGlobalLayout = LAYOUT("global", kGlobalFields);

INDICES(FUNCTION_SPAN, FUNCTION_ATTRIBS, FUNCTION_NAME, FUNCTION_PARAMS, FUNCTION_RESULT, FUNCTION_VARIADIC,
        FUNCTION_LOCALS, FUNCTION_BODY);
static const field_t kFunctionFields[] = {
    [FUNCTION_SPAN] = FIELD("span", FIELD_REFERENCE),     [FUNCTION_ATTRIBS] = FIELD("attribs", FIELD_REFERENCE),
    [FUNCTION_NAME] = FIELD("name", FIELD_STRING),        [FUNCTION_PARAMS] = FIELD("params", FIELD_ARRAY),
    [FUNCTION_RESULT] = FIELD("result", FIELD_REFERENCE), [FUNCTION_VARIADIC] = FIELD("variadic", FIELD_BOOL),
    [FUNCTION_LOCALS] = FIELD("locals", FIELD_ARRAY),     [FUNCTION_BODY] = FIELD("body", FIELD_REFERENCE),
};
static const layout_t kFunctionLayout = LAYOUT("function", kFunctionFields);

INDICES(STRUCT_SPAN, STRUCT_ATTRIBS, STRUCT_NAME, STRUCT_ITEMS);
static const field_t kStructFields[] = {
    [STRUCT_SPAN] = FIELD("span", FIELD_REFERENCE),
    [STRUCT_ATTRIBS] = FIELD("attribs", FIELD_REFERENCE),
    [STRUCT_NAME] = FIELD("name", FIELD_STRING),
    [STRUCT_ITEMS] = FIELD("items", FIELD_ARRAY),
};
static const layout_t kStructLayout = LAYOUT("struct", kStructFields);

INDICES(UNION_SPAN, UNION_ATTRIBS, UNION_NAME, UNION_ITEMS);
static const field_t kUnionFields[] = {
    [UNION_SPAN] = FIELD("span", FIELD_REFERENCE),
    [UNION_ATTRIBS] = FIELD("attribs", FIELD_REFERENCE),
    [UNION_NAME] = FIELD("name", FIELD_STRING),
    [UNION_ITEMS] = FIELD("items", FIELD_ARRAY),
};
static const layout_t kUnionLayout = LAYOUT("union", kUnionFields);

INDICES(ALIAS_SPAN, ALIAS_ATTRIBS, ALIAS_NAME, ALIAS_TYPE, ALIAS_NEWTYPE);
static const field_t kAliasFields[] = {
    [ALIAS_SPAN] = FIELD("span", FIELD_REFERENCE),  [ALIAS_ATTRIBS] = FIELD("attribs", FIELD_REFERENCE),
    [ALIAS_NAME] = FIELD("name", FIELD_STRING),     [ALIAS_TYPE] = FIELD("type", FIELD_REFERENCE),
    [ALIAS_NEWTYPE] = FIELD("newtype", FIELD_BOOL),
};
static const layout_t kAliasLayout = LAYOUT("alias", kAliasFields);

INDICES(FIELD_SPAN, FIELD_NAME, FIELD_TYPE);
static const field_t kFieldFields[] = {
    [FIELD_SPAN] = FIELD("span", FIELD_REFERENCE),
    [FIELD_NAME] = FIELD("name", FIELD_STRING),
    [FIELD_TYPE] = FIELD("type", FIELD_REFERENCE),
};
static const layout_t kFieldLayout = LAYOUT("field", kFieldFields);

INDICES(MODULE_SPAN, MODULE_NAME, MODULE_TYPES, MODULE_GLOBALS, MODULE_FUNCTIONS);
static const field_t kModuleFields[] = {
    [MODULE_SPAN] = FIELD("span", FIELD_REFERENCE),       [MODULE_NAME] = FIELD("name", FIELD_STRING),
    [MODULE_TYPES] = FIELD("types", FIELD_ARRAY),         [MODULE_GLOBALS] = FIELD("globals", FIELD_ARRAY),
    [MODULE_FUNCTIONS] = FIELD("functions", FIELD_ARRAY),
};
static const layout_t kModuleLayout = LAYOUT("module", kModuleFields);

///
/// final type table
///

#ifndef _WIN32
static const layout_t ALL_TYPES[LAYOUTS_TOTAL] = {
    [SPAN_INDEX] = kSpanLayout,
    [ATTRIBUTE_INDEX] = kAttribLayout,

    [HLIR_DIGIT_LITERAL] = kDigitLiteralLayout,
    [HLIR_BOOL_LITERAL] = kBoolLiteralLayout,
    [HLIR_STRING_LITERAL] = kStringLiteralLayout,

    [HLIR_NAME] = kNameLayout,
    [HLIR_UNARY] = kUnaryLayout,
    [HLIR_BINARY] = kBinaryLayout,
    [HLIR_COMPARE] = kCompareLayout,
    [HLIR_CALL] = kCallLayout,

    [HLIR_STMTS] = kStmtsLayout,
    [HLIR_BRANCH] = kBranchLayout,
    [HLIR_LOOP] = kLoopLayout,
    [HLIR_ASSIGN] = kAssignLayout,

    [HLIR_DIGIT] = kDigitLayout,
    [HLIR_BOOL] = kBoolLayout,
    [HLIR_STRING] = kStringLayout,
    [HLIR_VOID] = kVoidLayout,
    [HLIR_CLOSURE] = kClosureLayout,
    [HLIR_POINTER] = kPointerLayout,
    [HLIR_ARRAY] = kArrayLayout,

    [HLIR_STRUCT] = kStructLayout,
    [HLIR_UNION] = kUnionLayout,
    [HLIR_ALIAS] = kAliasLayout,
    [HLIR_FIELD] = kFieldLayout,

    [HLIR_LOCAL] = kLocalLayout,
    [HLIR_GLOBAL] = kGlobalLayout,
    [HLIR_FUNCTION] = kFunctionLayout,

    [HLIR_MODULE] = kModuleLayout,
};

static const format_t GLOBAL = FORMAT(&kHeaderLayout, ALL_TYPES);

static const format_t *get_format(void) {
    return &GLOBAL;
}

#else

static const format_t *get_format(void) {
    layout_t *allTypes = ctu_malloc(sizeof(layout_t) * LAYOUTS_TOTAL);

    allTypes[SPAN_INDEX] = kSpanLayout;
    allTypes[ATTRIBUTE_INDEX] = kAttribLayout;

    allTypes[HLIR_DIGIT_LITERAL] = kDigitLiteralLayout;
    allTypes[HLIR_BOOL_LITERAL] = kBoolLiteralLayout;
    allTypes[HLIR_STRING_LITERAL] = kStringLiteralLayout;

    allTypes[HLIR_NAME] = kNameLayout;
    allTypes[HLIR_UNARY] = kUnaryLayout;
    allTypes[HLIR_BINARY] = kBinaryLayout;
    allTypes[HLIR_COMPARE] = kCompareLayout;
    allTypes[HLIR_CALL] = kCallLayout;

    allTypes[HLIR_STMTS] = kStmtsLayout;
    allTypes[HLIR_BRANCH] = kBranchLayout;
    allTypes[HLIR_LOOP] = kLoopLayout;
    allTypes[HLIR_ASSIGN] = kAssignLayout;

    allTypes[HLIR_DIGIT] = kDigitLayout;
    allTypes[HLIR_BOOL] = kBoolLayout;
    allTypes[HLIR_STRING] = kStringLayout;
    allTypes[HLIR_VOID] = kVoidLayout;
    allTypes[HLIR_CLOSURE] = kClosureLayout;
    allTypes[HLIR_POINTER] = kPointerLayout;
    allTypes[HLIR_ARRAY] = kArrayLayout;

    allTypes[HLIR_STRUCT] = kStructLayout;
    allTypes[HLIR_UNION] = kUnionLayout;
    allTypes[HLIR_ALIAS] = kAliasLayout;
    allTypes[HLIR_FIELD] = kFieldLayout;

    allTypes[HLIR_LOCAL] = kLocalLayout;
    allTypes[HLIR_GLOBAL] = kGlobalLayout;
    allTypes[HLIR_FUNCTION] = kFunctionLayout;

    allTypes[HLIR_MODULE] = kModuleLayout;

    format_t fmt = {
        .header = &kHeaderLayout,
        .types = LAYOUTS_TOTAL,
        .layouts = allTypes,
    };

    return BOX(fmt);
}
#endif

#define HLIR_SUBMAGIC 0x484C4952 // 'HLIR'
#define HLIR_VERSION  NEW_VERSION(CTHULHU_MAJOR, CTHULHU_MINOR, CTHULHU_PATCH)

///
/// hlir loading
///

#define READ_OR_RETURN(data, index, values)            \
    do {                                               \
        bool result = read_entry(data, index, values); \
        if (!result) return false;                     \
    } while (0)

typedef struct {
    reports_t *reports;
    scan_t *scan;
    data_t *data;
} load_t;

static const node_t *load_span(load_t *load, index_t index) {
    value_t values[FIELDLEN(kSpanFields)];
    bool ok = read_entry(load->data, index, values);
    if (!ok) {
        return node_builtin();
    }

    where_t where = {
        .firstLine = get_int(values[SPAN_FIRST_LINE]),
        .firstColumn = get_int(values[SPAN_FIRST_COLUMN]),
        .lastLine = get_int(values[SPAN_LAST_LINE]),
        .lastColumn = get_int(values[SPAN_LAST_COLUMN]),
    };

    return node_new(load->scan, where);
}

static hlir_attributes_t *load_attributes(load_t *load, value_t value) {
    value_t values[FIELDLEN(kAttribFields)];
    READ_OR_RETURN(load->data, get_reference(value), values);

    return hlir_attributes(get_int(values[ATTRIB_LINKAGE]), get_int(values[ATTRIB_TAGS]));
}

static hlir_t *load_node(load_t *load, index_t index, const char *trace);

static hlir_t *load_opt_node(load_t *load, index_t index, const char *trace) {
    if (index.type == UINT32_MAX) {
        return NULL;
    }

    return load_node(load, index, trace);
}

static const node_t *get_span(load_t *load, value_t *values) {
    return load_span(load,
                     get_reference(values[0])); // the ENUM_FIELDS macro makes
                                                // sure that values[0] is a span
}

#define GET_REF(load, values, name)     load_node(load, get_reference((values)[name]), "loading " #name)
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
    value_t values[FIELDLEN(kDigitLiteralFields)];
    READ_OR_RETURN(load->data, index, values);

    mpz_t digit;
    get_digit(digit, values[DIGIT_LITERAL_VALUE]);

    return hlir_digit_literal(get_span(load, values), GET_REF(load, values, DIGIT_LITERAL_TYPE), digit);
}

static hlir_t *load_bool_literal_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(kBoolLiteralFields)];
    READ_OR_RETURN(load->data, index, values);

    return hlir_bool_literal(get_span(load, values), GET_REF(load, values, BOOL_LITERAL_TYPE),
                             get_bool(values[BOOL_LITERAL_VALUE]));
}

static hlir_t *load_string_literal_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(kStringLiteralFields)];
    READ_OR_RETURN(load->data, index, values);

    return hlir_string_literal(get_span(load, values), GET_REF(load, values, STRING_LITERAL_TYPE),
                               get_string(values[STRING_LITERAL_VALUE]));
}

static hlir_t *load_name_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(kNameFields)];
    READ_OR_RETURN(load->data, index, values);

    return hlir_name(get_span(load, values), GET_REF(load, values, NAME_EXPR));
}

static hlir_t *load_unary_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(kUnaryFields)];
    READ_OR_RETURN(load->data, index, values);

    return hlir_unary(get_span(load, values), GET_REF(load, values, UNARY_TYPE), GET_REF(load, values, UNARY_EXPR),
                      get_int(values[UNARY_OP]));
}

static hlir_t *load_binary_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(kBinaryFields)];
    READ_OR_RETURN(load->data, index, values);

    return hlir_binary(get_span(load, values), GET_REF(load, values, BINARY_TYPE), get_int(values[BINARY_OP]),
                       GET_REF(load, values, BINARY_LHS), GET_REF(load, values, BINARY_RHS));
}

static hlir_t *load_compare_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(kCompareFields)];
    READ_OR_RETURN(load->data, index, values);

    return hlir_compare(get_span(load, values), GET_REF(load, values, COMPARE_TYPE), get_int(values[COMPARE_OP]),
                        GET_REF(load, values, COMPARE_LHS), GET_REF(load, values, COMPARE_RHS));
}

static hlir_t *load_call_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(kCallFields)];
    READ_OR_RETURN(load->data, index, values);

    return hlir_call(get_span(load, values), GET_REF(load, values, CALL_EXPR), get_arr(load, values[CALL_ARGS]));
}

// statements

static hlir_t *load_stmts_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(kStmtsFields)];
    READ_OR_RETURN(load->data, index, values);

    return hlir_stmts(get_span(load, values), get_arr(load, values[STMTS_OPS]));
}

static hlir_t *load_branch_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(kBranchFields)];
    READ_OR_RETURN(load->data, index, values);

    return hlir_branch(get_span(load, values), GET_REF(load, values, BRANCH_COND), GET_REF(load, values, BRANCH_THEN),
                       GET_REF_OPT(load, values, BRANCH_ELSE));
}

static hlir_t *load_loop_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(kLoopFields)];
    READ_OR_RETURN(load->data, index, values);

    return hlir_loop(get_span(load, values), GET_REF(load, values, LOOP_COND), GET_REF(load, values, LOOP_BODY),
                     GET_REF_OPT(load, values, LOOP_ELSE));
}

static hlir_t *load_assign_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(kAssignFields)];
    READ_OR_RETURN(load->data, index, values);

    return hlir_assign(get_span(load, values), GET_REF(load, values, ASSIGN_DST), GET_REF(load, values, ASSIGN_SRC));
}

// types

static hlir_t *load_digit_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(kDigitFields)];
    READ_OR_RETURN(load->data, index, values);

    return hlir_digit(get_span(load, values), get_string(values[DIGIT_NAME]), get_int(values[DIGIT_SIGN]),
                      get_int(values[DIGIT_WIDTH]));
}

static hlir_t *load_bool_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(kBoolFields)];
    READ_OR_RETURN(load->data, index, values);

    return hlir_bool(get_span(load, values), get_string(values[BOOL_NAME]));
}

static hlir_t *load_string_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(kStringFields)];
    READ_OR_RETURN(load->data, index, values);

    return hlir_string(get_span(load, values), get_string(values[STRING_NAME]));
}

static hlir_t *load_void_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(kVoidFields)];
    READ_OR_RETURN(load->data, index, values);

    return hlir_void(get_span(load, values), get_string(values[VOID_NAME]));
}

static hlir_t *load_closure_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(kClosureFields)];
    READ_OR_RETURN(load->data, index, values);

    return hlir_closure(get_span(load, values), get_string(values[CLOSURE_NAME]), get_arr(load, values[CLOSURE_ARGS]),
                        GET_REF(load, values, CLOSURE_RESULT), get_bool(values[CLOSURE_VARIADIC]));
}

static hlir_t *load_pointer_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(kPointerFields)];
    READ_OR_RETURN(load->data, index, values);

    return hlir_pointer(get_span(load, values), get_string(values[POINTER_NAME]), GET_REF(load, values, POINTER_TYPE),
                        get_bool(values[POINTER_INDEXABLE]));
}

static hlir_t *load_array_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(kArrayFields)];
    READ_OR_RETURN(load->data, index, values);

    return hlir_array(get_span(load, values), get_string(values[ARRAY_NAME]), GET_REF(load, values, ARRAY_TYPE),
                      GET_REF(load, values, ARRAY_LENGTH));
}

static hlir_t *load_local_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(kLocalFields)];
    READ_OR_RETURN(load->data, index, values);

    return hlir_indexed_local(get_span(load, values), get_string(values[LOCAL_NAME]), get_int(values[LOCAL_INDEX]),
                              GET_REF(load, values, LOCAL_TYPE));
}

static hlir_t *load_global_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(kGlobalFields)];
    READ_OR_RETURN(load->data, index, values);

    hlir_t *hlir = hlir_global(get_span(load, values), get_string(values[GLOBAL_NAME]),
                               GET_REF(load, values, GLOBAL_TYPE), GET_REF_OPT(load, values, GLOBAL_INIT));

    hlir_set_attributes(hlir, load_attributes(load, values[GLOBAL_ATTRIBS]));

    return hlir;
}

static hlir_t *load_function_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(kFunctionFields)];
    READ_OR_RETURN(load->data, index, values);

    signature_t signature = {
        .params = get_arr(load, values[FUNCTION_PARAMS]),
        .result = GET_REF(load, values, FUNCTION_RESULT),
        .variadic = get_bool(values[FUNCTION_VARIADIC]),
    };

    hlir_t *hlir = hlir_function(get_span(load, values), get_string(values[FUNCTION_NAME]), signature,
                                 get_arr(load, values[FUNCTION_LOCALS]), GET_REF_OPT(load, values, FUNCTION_BODY));

    hlir_set_attributes(hlir, load_attributes(load, values[FUNCTION_ATTRIBS]));

    return hlir;
}

static hlir_t *load_struct_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(kStructFields)];
    READ_OR_RETURN(load->data, index, values);

    hlir_t *hlir =
        hlir_struct(get_span(load, values), get_string(values[STRUCT_NAME]), get_arr(load, values[STRUCT_ITEMS]));

    hlir_set_attributes(hlir, load_attributes(load, values[STRUCT_ATTRIBS]));

    return hlir;
}

static hlir_t *load_union_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(kUnionFields)];
    READ_OR_RETURN(load->data, index, values);

    hlir_t *hlir =
        hlir_union(get_span(load, values), get_string(values[UNION_NAME]), get_arr(load, values[UNION_ITEMS]));

    hlir_set_attributes(hlir, load_attributes(load, values[UNION_ATTRIBS]));

    return hlir;
}

static hlir_t *load_alias_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(kAliasFields)];
    READ_OR_RETURN(load->data, index, values);

    hlir_t *hlir = hlir_alias(get_span(load, values), get_string(values[ALIAS_NAME]), GET_REF(load, values, ALIAS_TYPE),
                              get_bool(values[ALIAS_NEWTYPE]));

    hlir_set_attributes(hlir, load_attributes(load, values[ALIAS_ATTRIBS]));

    return hlir;
}

static hlir_t *load_field_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(kFieldFields)];
    READ_OR_RETURN(load->data, index, values);

    return hlir_field(get_span(load, values), GET_REF(load, values, FIELD_TYPE), get_string(values[FIELD_NAME]));
}

static hlir_t *load_module_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(kModuleFields)];
    READ_OR_RETURN(load->data, index, values);

    return hlir_module(get_span(load, values), get_string(values[MODULE_NAME]), get_arr(load, values[MODULE_TYPES]),
                       get_arr(load, values[MODULE_GLOBALS]), get_arr(load, values[MODULE_FUNCTIONS]));
}

static hlir_t *load_node(load_t *load, index_t index, const char *trace) {
    switch (index.type) {
    /* literals */
    case HLIR_DIGIT_LITERAL:
        return load_digit_literal_node(load, index);
    case HLIR_BOOL_LITERAL:
        return load_bool_literal_node(load, index);
    case HLIR_STRING_LITERAL:
        return load_string_literal_node(load, index);

    /* expressions */
    case HLIR_NAME:
        return load_name_node(load, index);
    case HLIR_UNARY:
        return load_unary_node(load, index);
    case HLIR_BINARY:
        return load_binary_node(load, index);
    case HLIR_COMPARE:
        return load_compare_node(load, index);
    case HLIR_CALL:
        return load_call_node(load, index);

    /* statements */
    case HLIR_STMTS:
        return load_stmts_node(load, index);
    case HLIR_BRANCH:
        return load_branch_node(load, index);
    case HLIR_LOOP:
        return load_loop_node(load, index);
    case HLIR_ASSIGN:
        return load_assign_node(load, index);

    /* types */
    case HLIR_DIGIT:
        return load_digit_node(load, index);
    case HLIR_BOOL:
        return load_bool_node(load, index);
    case HLIR_STRING:
        return load_string_node(load, index);
    case HLIR_VOID:
        return load_void_node(load, index);
    case HLIR_CLOSURE:
        return load_closure_node(load, index);
    case HLIR_POINTER:
        return load_pointer_node(load, index);
    case HLIR_ARRAY:
        return load_array_node(load, index);

    /* declarations */
    case HLIR_LOCAL:
        return load_local_node(load, index);
    case HLIR_GLOBAL:
        return load_global_node(load, index);
    case HLIR_FUNCTION:
        return load_function_node(load, index);

    case HLIR_STRUCT:
        return load_struct_node(load, index);
    case HLIR_UNION:
        return load_union_node(load, index);
    case HLIR_ALIAS:
        return load_alias_node(load, index);
    case HLIR_FIELD:
        return load_field_node(load, index);

    case HLIR_MODULE:
        return load_module_node(load, index);

    default:
        ctu_assert(load->reports, "loading unknown node type %d due to %s", index.type, trace);
        return NULL;
    }
}

static scan_t make_scanner(reports_t *reports, const char *lang, const char *path, const char *source) {
    if (source != NULL) {
        return scan_string(reports, lang, path, source);
    }

    file_t *file = file_new(path, TEXT, READ);
    return scan_file(reports, lang, file);
}

hlir_t *load_module(reports_t *reports, const char *path) {
    value_t values[FIELDLEN(kHeaderFields)];
    record_t record = {.layout = &kHeaderLayout, .values = values};

    header_t header = {.reports = reports,
                       .format = get_format(),
                       .path = path,
                       .header = record,
                       .submagic = HLIR_SUBMAGIC,
                       .semver = HLIR_VERSION};

    data_t data;
    bool ok = begin_load(&data, header);
    if (!ok) {
        return NULL;
    }

    const char *language = get_string(data.header.header.values[HEADER_LANGUAGE]);
    const char *where = get_string(data.header.header.values[HEADER_PATH]);
    const char *source = get_string(data.header.header.values[HEADER_SOURCE]);

    logverbose("loading(lang=%s, path=%s, source=%s)", language, where, source != NULL ? "embedded" : "external");

    scan_t scan = make_scanner(reports, language, where, source);

    load_t load = {.reports = reports, .scan = BOX(scan), .data = &data};
    index_t index = {.type = HLIR_MODULE};
    hlir_t *hlir = load_node(&load, index, "root load");

    end_load(&data);

    return hlir;
}

///
/// hlir saving
///

static index_t save_span(data_t *data, const node_t *node) {
    if (node == NULL) {
        return NULL_INDEX;
    }

    value_t values[FIELDLEN(kSpanFields)] = {
        [SPAN_FIRST_LINE] = int_value((long)node->where.firstLine),
        [SPAN_FIRST_COLUMN] = int_value((long)node->where.firstColumn),
        [SPAN_LAST_LINE] = int_value((long)node->where.lastLine),
        [SPAN_LAST_COLUMN] = int_value((long)node->where.lastColumn),
    };

    return write_entry(data, SPAN_INDEX, values);
}

static index_t save_attributes(data_t *data, const hlir_attributes_t *attributes) {
    value_t values[FIELDLEN(kAttribFields)] = {
        [ATTRIB_LINKAGE] = int_value(attributes->linkage),
        [ATTRIB_TAGS] = int_value(attributes->tags),
    };

    return write_entry(data, ATTRIBUTE_INDEX, values);
}

static index_t save_node(data_t *data, const hlir_t *hlir);

static value_t make_ref(data_t *data, const hlir_t *hlir) {
    index_t index = save_node(data, hlir);
    return reference_value(index);
}

static value_t make_ref_opt(data_t *data, const hlir_t *hlir) {
    if (hlir == NULL) {
        return reference_value(NULL_INDEX);
    }

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
        [DIGIT_LITERAL_VALUE] = digit_value(hlir->digit),
    };

    return write_entry(data, HLIR_DIGIT_LITERAL, values);
}

static index_t save_bool_literal_node(data_t *data, const hlir_t *hlir) {
    value_t values[] = {
        [BOOL_LITERAL_SPAN] = span_ref(data, hlir),
        [BOOL_LITERAL_TYPE] = make_ref(data, get_hlir_type(hlir)),
        [BOOL_LITERAL_VALUE] = bool_value(hlir->boolean),
    };

    return write_entry(data, HLIR_BOOL_LITERAL, values);
}

static index_t save_string_literal_node(data_t *data, const hlir_t *hlir) {
    value_t values[] = {
        [STRING_LITERAL_SPAN] = span_ref(data, hlir),
        [STRING_LITERAL_TYPE] = make_ref(data, get_hlir_type(hlir)),
        [STRING_LITERAL_VALUE] = string_value(hlir->string),
    };

    return write_entry(data, HLIR_STRING_LITERAL, values);
}

static index_t save_name_node(data_t *data, const hlir_t *hlir) {
    value_t values[] = {
        [NAME_SPAN] = span_ref(data, hlir),
        [NAME_EXPR] = make_ref(data, hlir->read),
    };

    return write_entry(data, HLIR_NAME, values);
}

static index_t save_unary_node(data_t *data, const hlir_t *hlir) {
    value_t values[] = {
        [UNARY_SPAN] = span_ref(data, hlir),
        [UNARY_TYPE] = make_ref(data, get_hlir_type(hlir)),
        [UNARY_OP] = int_value(hlir->unary),
        [UNARY_EXPR] = make_ref(data, hlir->operand),
    };

    return write_entry(data, HLIR_UNARY, values);
}

static index_t save_binary_node(data_t *data, const hlir_t *hlir) {
    value_t values[] = {
        [BINARY_SPAN] = span_ref(data, hlir),     [BINARY_TYPE] = make_ref(data, get_hlir_type(hlir)),
        [BINARY_OP] = int_value(hlir->binary),    [BINARY_LHS] = make_ref(data, hlir->lhs),
        [BINARY_RHS] = make_ref(data, hlir->rhs),
    };

    return write_entry(data, HLIR_BINARY, values);
}

static index_t save_compare_node(data_t *data, const hlir_t *hlir) {
    value_t values[] = {
        [COMPARE_SPAN] = span_ref(data, hlir),     [COMPARE_TYPE] = make_ref(data, get_hlir_type(hlir)),
        [COMPARE_OP] = int_value(hlir->compare),   [COMPARE_LHS] = make_ref(data, hlir->lhs),
        [COMPARE_RHS] = make_ref(data, hlir->rhs),
    };

    return write_entry(data, HLIR_COMPARE, values);
}

static index_t save_call_node(data_t *data, const hlir_t *hlir) {
    array_t args = save_array(data, hlir->args);

    value_t values[] = {
        [CALL_SPAN] = span_ref(data, hlir),
        [CALL_EXPR] = make_ref(data, hlir->call),
        [CALL_ARGS] = array_value(args),
    };

    return write_entry(data, HLIR_CALL, values);
}

// statements

static index_t save_stmts_node(data_t *data, const hlir_t *hlir) {
    array_t stmts = save_array(data, hlir->stmts);

    value_t values[] = {
        [STMTS_SPAN] = span_ref(data, hlir),
        [STMTS_OPS] = array_value(stmts),
    };

    return write_entry(data, HLIR_STMTS, values);
}

static index_t save_branch_node(data_t *data, const hlir_t *hlir) {
    value_t values[] = {
        [BRANCH_SPAN] = span_ref(data, hlir),
        [BRANCH_COND] = make_ref(data, hlir->cond),
        [BRANCH_THEN] = make_ref(data, hlir->then),
        [BRANCH_ELSE] = make_ref_opt(data, hlir->other),
    };

    return write_entry(data, HLIR_BRANCH, values);
}

static index_t save_loop_node(data_t *data, const hlir_t *hlir) {
    value_t values[] = {
        [LOOP_SPAN] = span_ref(data, hlir),
        [LOOP_COND] = make_ref(data, hlir->cond),
        [LOOP_BODY] = make_ref(data, hlir->then),
        [LOOP_ELSE] = make_ref_opt(data, hlir->other),
    };

    return write_entry(data, HLIR_LOOP, values);
}

static index_t save_assign_node(data_t *data, const hlir_t *hlir) {
    value_t values[] = {
        [ASSIGN_SPAN] = span_ref(data, hlir),
        [ASSIGN_DST] = make_ref(data, hlir->dst),
        [ASSIGN_SRC] = make_ref(data, hlir->src),
    };

    return write_entry(data, HLIR_ASSIGN, values);
}

// types

static index_t save_digit_node(data_t *data, const hlir_t *hlir) {
    value_t values[] = {
        [DIGIT_SPAN] = span_ref(data, hlir),
        [DIGIT_NAME] = string_value(hlir->name),
        [DIGIT_SIGN] = int_value(hlir->sign),
        [DIGIT_WIDTH] = int_value(hlir->width),
    };

    return write_entry(data, HLIR_DIGIT, values);
}

static index_t save_bool_node(data_t *data, const hlir_t *hlir) {
    value_t values[] = {
        [BOOL_SPAN] = span_ref(data, hlir),
        [BOOL_NAME] = string_value(hlir->name),
    };

    return write_entry(data, HLIR_BOOL, values);
}

static index_t save_string_node(data_t *data, const hlir_t *hlir) {
    value_t values[] = {
        [STRING_SPAN] = span_ref(data, hlir),
        [STRING_NAME] = string_value(hlir->name),
    };

    return write_entry(data, HLIR_STRING, values);
}

static index_t save_void_node(data_t *data, const hlir_t *hlir) {
    value_t values[] = {
        [VOID_SPAN] = span_ref(data, hlir),
        [VOID_NAME] = string_value(hlir->name),
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
        [CLOSURE_VARIADIC] = bool_value(hlir->variadic),
    };

    return write_entry(data, HLIR_CLOSURE, values);
}

static index_t save_pointer_node(data_t *data, const hlir_t *hlir) {
    value_t values[] = {
        [POINTER_SPAN] = span_ref(data, hlir),
        [POINTER_NAME] = string_value(hlir->name),
        [POINTER_TYPE] = make_ref(data, hlir->ptr),
        [POINTER_INDEXABLE] = bool_value(hlir->indexable),
    };

    return write_entry(data, HLIR_POINTER, values);
}

static index_t save_array_node(data_t *data, const hlir_t *hlir) {
    value_t values[] = {
        [ARRAY_SPAN] = span_ref(data, hlir),
        [ARRAY_NAME] = string_value(hlir->name),
        [ARRAY_TYPE] = make_ref(data, hlir->element),
        [ARRAY_LENGTH] = make_ref(data, hlir->length),
    };

    return write_entry(data, HLIR_ARRAY, values);
}

/// declarations

static index_t save_local_node(data_t *data, const hlir_t *hlir) {
    value_t values[] = {
        [LOCAL_SPAN] = span_ref(data, hlir),          [LOCAL_ATTRIBS] = attrib_ref(data, hlir),
        [LOCAL_NAME] = string_value(hlir->name),      [LOCAL_TYPE] = make_ref(data, get_hlir_type(hlir)),
        [LOCAL_INDEX] = int_value((long)hlir->index),
    };

    return write_entry(data, HLIR_LOCAL, values);
}

static index_t save_global_node(data_t *data, const hlir_t *hlir) {
    value_t values[] = {
        [GLOBAL_SPAN] = span_ref(data, hlir),
        [GLOBAL_ATTRIBS] = attrib_ref(data, hlir),
        [GLOBAL_NAME] = string_value(hlir->name),
        [GLOBAL_TYPE] = make_ref(data, get_hlir_type(hlir)),
        [GLOBAL_INIT] = make_ref_opt(data, hlir->value),
    };

    return write_entry(data, HLIR_GLOBAL, values);
}

static index_t save_function_node(data_t *data, const hlir_t *hlir) {
    array_t locals = save_array(data, hlir->locals);
    array_t params = save_array(data, hlir->params);

    value_t values[] = {
        [FUNCTION_SPAN] = span_ref(data, hlir),           [FUNCTION_ATTRIBS] = attrib_ref(data, hlir),
        [FUNCTION_NAME] = string_value(hlir->name),       [FUNCTION_PARAMS] = array_value(params),
        [FUNCTION_RESULT] = make_ref(data, hlir->result), [FUNCTION_VARIADIC] = bool_value(hlir->variadic),
        [FUNCTION_LOCALS] = array_value(locals),          [FUNCTION_BODY] = make_ref_opt(data, hlir->body),
    };

    return write_entry(data, HLIR_FUNCTION, values);
}

static index_t save_struct_node(data_t *data, const hlir_t *hlir) {
    array_t fields = save_array(data, hlir->fields);

    value_t values[] = {
        [STRUCT_SPAN] = span_ref(data, hlir),
        [STRUCT_ATTRIBS] = attrib_ref(data, hlir),
        [STRUCT_NAME] = string_value(hlir->name),
        [STRUCT_ITEMS] = array_value(fields),
    };

    return write_entry(data, HLIR_STRUCT, values);
}

static index_t save_union_node(data_t *data, const hlir_t *hlir) {
    array_t fields = save_array(data, hlir->fields);

    value_t values[] = {
        [UNION_SPAN] = span_ref(data, hlir),
        [UNION_ATTRIBS] = attrib_ref(data, hlir),
        [UNION_NAME] = string_value(hlir->name),
        [UNION_ITEMS] = array_value(fields),
    };

    return write_entry(data, HLIR_UNION, values);
}

static index_t save_alias_node(data_t *data, const hlir_t *hlir) {
    value_t values[] = {
        [ALIAS_SPAN] = span_ref(data, hlir),
        [ALIAS_ATTRIBS] = attrib_ref(data, hlir),
        [ALIAS_NAME] = string_value(hlir->name),
        [ALIAS_TYPE] = make_ref(data, hlir->alias),
    };

    return write_entry(data, HLIR_ALIAS, values);
}

static index_t save_field_node(data_t *data, const hlir_t *hlir) {
    value_t values[] = {
        [FIELD_SPAN] = span_ref(data, hlir),
        [FIELD_NAME] = string_value(hlir->name),
        [FIELD_TYPE] = make_ref(data, get_hlir_type(hlir)),
    };

    return write_entry(data, HLIR_FIELD, values);
}

static index_t save_module_node(data_t *data, const hlir_t *hlir) {
    array_t types = save_array(data, hlir->types);
    array_t globals = save_array(data, hlir->globals);
    array_t functions = save_array(data, hlir->functions);

    value_t values[] = {
        [MODULE_SPAN] = span_ref(data, hlir),        [MODULE_NAME] = string_value(hlir->name),
        [MODULE_TYPES] = array_value(types),         [MODULE_GLOBALS] = array_value(globals),
        [MODULE_FUNCTIONS] = array_value(functions),
    };

    return write_entry(data, HLIR_MODULE, values);
}

static index_t save_node(data_t *data, const hlir_t *hlir) {
    switch (hlir->type) {
    /* literals */
    case HLIR_DIGIT_LITERAL:
        return save_digit_literal_node(data, hlir);
    case HLIR_BOOL_LITERAL:
        return save_bool_literal_node(data, hlir);
    case HLIR_STRING_LITERAL:
        return save_string_literal_node(data, hlir);

    /* expressions */
    case HLIR_NAME:
        return save_name_node(data, hlir);
    case HLIR_UNARY:
        return save_unary_node(data, hlir);
    case HLIR_BINARY:
        return save_binary_node(data, hlir);
    case HLIR_COMPARE:
        return save_compare_node(data, hlir);
    case HLIR_CALL:
        return save_call_node(data, hlir);

    /* statements */
    case HLIR_STMTS:
        return save_stmts_node(data, hlir);
    case HLIR_BRANCH:
        return save_branch_node(data, hlir);
    case HLIR_LOOP:
        return save_loop_node(data, hlir);
    case HLIR_ASSIGN:
        return save_assign_node(data, hlir);

    /* types */
    case HLIR_DIGIT:
        return save_digit_node(data, hlir);
    case HLIR_BOOL:
        return save_bool_node(data, hlir);
    case HLIR_STRING:
        return save_string_node(data, hlir);
    case HLIR_VOID:
        return save_void_node(data, hlir);
    case HLIR_CLOSURE:
        return save_closure_node(data, hlir);
    case HLIR_POINTER:
        return save_pointer_node(data, hlir);
    case HLIR_ARRAY:
        return save_array_node(data, hlir);
    case HLIR_TYPE:
        return NULL_INDEX;

    /* declarations */
    case HLIR_LOCAL:
        return save_local_node(data, hlir);
    case HLIR_GLOBAL:
        return save_global_node(data, hlir);
    case HLIR_FUNCTION:
        return save_function_node(data, hlir);

    case HLIR_STRUCT:
        return save_struct_node(data, hlir);
    case HLIR_UNION:
        return save_union_node(data, hlir);
    case HLIR_ALIAS:
        return save_alias_node(data, hlir);
    case HLIR_FIELD:
        return save_field_node(data, hlir);

    case HLIR_MODULE:
        return save_module_node(data, hlir);

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
        [HEADER_SOURCE] = string_value(NULL),
    };

    if (settings->embedSource) {
        values[HEADER_SOURCE] = string_value(scan_text(scan));
    }

    record_t record = {.layout = &kHeaderLayout, .values = values};

    header_t header = {
        .reports = reports,
        .format = get_format(),
        .path = path,
        .header = record,
        .submagic = HLIR_SUBMAGIC,
        .semver = HLIR_VERSION,
    };

    data_t data;
    begin_save(&data, header);

    save_node(&data, module);

    end_save(&data);
}

bool is_hlir_module(const char *path) {
    return is_loadable(path, HLIR_SUBMAGIC, HLIR_VERSION);
}
