#pragma once

#if 0

#include <stdint.h>

typedef enum {
    CT_EXPR_INT_LITERAL,
    CT_EXPR_FLOAT_LITERAL,
    CT_EXPR_NULL_LITERAL,
    CT_EXPR_CHAR_LITERAL,
    CT_EXPR_STRING_LITERAL,
    CT_EXPR_TYPE_UNARY,
    CT_EXPR_TYPE_BINARY,
    CT_EXPR_TYPE_TERNARY,
    CT_EXPR_TYPE_SUBSCRIPT
} CtExprType;

typedef enum {
    CT_UNARY_OP_ADD,
    CT_UNARY_OP_SUB,
    CT_UNARY_OP_NOT,
    CT_UNARY_OP_FLIP
} CtUnaryOp;

typedef enum {
    CT_BINARY_OP_ADD,
    CT_BINARY_OP_ADDEQ,
    CT_BINARY_OP_SUB,
    CT_BINARY_OP_SUBEQ,
    CT_BINARY_OP_DIV,
    CT_BINARY_OP_DIVEQ,
    CT_BINARY_OP_MUL,
    CT_BINARY_OP_MULEQ,
    CT_BINARY_OP_MOD,
    CT_BINARY_OP_MODEQ,

    CT_BINARY_OP_XOR,
    CT_BINARY_OP_XOREQ,

    CT_BINARY_OP_SHL,
    CT_BINARY_OP_SHLEQ,

    CT_BINARY_OP_SHR,
    CT_BINARY_OP_SHREQ,

    CT_BINARY_OP_BITAND,
    CT_BINARY_OP_BITANDEQ,

    CT_BINARY_OP_BITOR,
    CT_BINARY_OP_BITOREQ,

    CT_BINARY_OP_AND,
    CT_BINARY_OP_OR,

    CT_BINARY_OP_EQ,
    CT_BINARY_OP_NEQ
} CtBinaryOp;

typedef struct {
    CtUnaryOp op;
    struct CtExpr *expr;
} CtUnaryExpr;

typedef struct {
    CtBinaryOp op;
    struct CtExpr *lhs;
    struct CtExpr *rhs;
} CtBinaryExpr;

typedef struct {
    struct CtExpr *cond;
    struct CtExpr *truthy;
    struct CtExpr *falsey;
} CtTernaryExpr;

typedef struct {
    struct CtExpr *expr;
    struct CtExpr *index;
} CtSubscriptExpr;

typedef union {
    uint64_t integer;
    double number;
    uint8_t character;
    char *string;

    CtUnaryExpr unary;
    CtBinaryExpr binary;
    CtTernaryExpr ternary;
    CtSubscriptExpr subscript;
} CtExprBody;

typedef struct {
    CtExprType type;
    CtExprBody expr;
} CtExpr;

typedef enum {
    CT_BUILTIN_TYPE_U8,
    CT_BUILTIN_TYPE_U16,
    CT_BUILTIN_TYPE_U32,
    CT_BUILTIN_TYPE_U64,
    CT_BUILTIN_TYPE_I8,
    CT_BUILTIN_TYPE_I16,
    CT_BUILTIN_TYPE_I32,
    CT_BUILTIN_TYPE_I64,
    CT_BUILTIN_TYPE_F32,
    CT_BUILTIN_TYPE_F64,
    CT_BUILTIN_TYPE_VOID
} CtBuiltinType;

typedef enum {
    CT_TYPE_BUILTIN,
    CT_TYPE_CONST,
    CT_TYPE_POINTER,
    CT_TYPE_REFERENCE,
    CT_TYPE_UNIQUE,
    CT_TYPE_STRUCT,
    CT_TYPE_UNION,
    CT_TYPE_VARIANT,
    CT_TYPE_ENUM,
    CT_TYPE_ARRAY,
    CT_TYPE_CLOSURE
} CtTypeType;

typedef struct {
    int fields;
    char *names;
    struct CtType *fields;
} CtStruct;

typedef struct {
    int fields;
    char *names;
    struct CtType *fields;
} CtUnion;

typedef struct {
    struct CtType *backing;

    int fields;
    char *names;
    struct CtType *fields;
    struct CtExpr *exprs;
} CtVariant;

typedef struct {
    struct CtType *backing;

    int fields;
    char *names;
    struct CtExpr *values;
} CtEnum;

typedef struct {
    struct CtType *type;
    struct CtExpr *size;
} CtArray;

typedef struct {
    struct CtType *result;

    int count;
    struct CtType *args;
} CtClosure;

typedef union {
    CtBuiltinType _builtin;
    struct CtType *_const;
    struct CtType *_ptr;
    struct CtType *_ref;
    struct CtType *_unique;

    CtStruct _struct;
    CtUnion _union;
    CtVariant _variant;
    CtEnum _enum;
    CtArray _array;
    CtClosure _closure;
} CtTypeBody;

typedef struct {
    CtTypeType type;
    CtTypeBody data;
} CtType;

typedef struct {
    int count;
    char *path;
} CtPath;

#endif
