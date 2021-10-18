#include "ops.h"

const char *binary_name(binary_t op) {
    switch (op) {
    case BINARY_ADD: return "add";
    case BINARY_SUB: return "sub";
    case BINARY_MUL: return "mul";
    case BINARY_DIV: return "div";
    case BINARY_REM: return "rem";

    case BINARY_EQ: return "eq";
    case BINARY_NEQ: return "neq";
    case BINARY_LT: return "lt";
    case BINARY_LTE: return "lte";
    case BINARY_GT: return "gt";
    case BINARY_GTE: return "gte";

    case BINARY_SHL: return "shl";
    case BINARY_SHR: return "shr";
    case BINARY_AND: return "and";
    case BINARY_OR: return "or";
    case BINARY_XOR: return "xor";

    case BINARY_BITAND: return "bitand";
    case BINARY_BITOR: return "bitor";

    default: return "???";
    }
}

const char *unary_name(unary_t op) {
    switch (op) {
    case UNARY_ABS: return "abs";
    case UNARY_NEG: return "neg";
    case UNARY_ADDR: return "addr";
    case UNARY_DEREF: return "deref";
    
    default: return "???";
    }
}
