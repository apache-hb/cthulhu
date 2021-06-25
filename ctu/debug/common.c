#include "common.h"

const char *unary_name(unary_t unary) {
    switch (unary) {
    case UNARY_ABS: return "abs";
    case UNARY_NEG: return "neg";
    case UNARY_TRY: return "try";
    default: return "error";
    }
}

const char *binary_name(binary_t binary) {
    switch (binary) {
    case BINARY_ADD: return "add";
    case BINARY_SUB: return "sub";
    case BINARY_MUL: return "mul";
    case BINARY_DIV: return "div";
    case BINARY_REM: return "rem";
    case BINARY_GT: return "gt";
    case BINARY_GTE: return "gte";
    case BINARY_LT: return "lt";
    case BINARY_LTE: return "lte";
    case BINARY_EQ: return "eq";
    case BINARY_NEQ: return "neq";
    default: return "error";
    }
}
