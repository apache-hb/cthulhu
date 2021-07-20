#include "type.h"

#include <stdio.h>
#include <stdint.h>

static const char *integer_name(type_t *type) {
    bool sign = is_signed(type);
    switch (type->integer) {
    case INTEGER_CHAR: return sign ? "char" : "uchar";
    case INTEGER_SHORT: return sign ? "short" : "ushort";
    case INTEGER_INT: return sign ? "int" : "uint";
    case INTEGER_LONG: return sign ? "long" : "ulong";
    case INTEGER_SIZE: return sign ? "isize" : "usize";
    case INTEGER_INTPTR: return sign ? "intptr" : "uintptr";
    case INTEGER_INTMAX: return sign ? "intmax" : "uintmax";
    default: return "unknown";
    }
}

void debug_type(type_t *type);

static void debug_callable(type_t *type) {
    types_t *args = type->args;
    printf("(");
    for (size_t i = 0; i < typelist_len(args); i++) {
        if (i != 0) {
            printf(", ");
        }
        debug_type(typelist_at(args, i));
    }
    printf(") -> ");
    debug_type(type->result);
}

static void debug_struct(record_t fields) {
    printf("{");
    for (size_t i = 0; i < fields.size; i++) {
        if (i != 0) {
            printf(", ");
        }
        field_t field = fields.fields[i];
        printf("%s: ", field.name);
        debug_type(field.type);
    }
    printf("}");
}

static void debug_type_internal(type_t *type, bool verbose) {
    switch (type->kind) {
    case TYPE_INTEGER: printf("%s", integer_name(type)); break;
    case TYPE_BOOLEAN: printf("bool"); break;
    case TYPE_VOID: printf("void"); break;
    case TYPE_CALLABLE: debug_callable(type); break;
    case TYPE_POINTER: printf("*"); debug_type(type->ptr); break;
    case TYPE_POISON: printf("poison(%s)", type->text); break;
    case TYPE_UNRESOLVED: printf("unresolved"); break;
    
    case TYPE_STRUCT: 
        printf("struct(%s)", type->name); 
        if (verbose) { 
            debug_struct(type->fields);
        } 
        break;

    case TYPE_FIELD: break;
    }

    if (type->index != SIZE_MAX) {
        printf(" @%zu", type->index);
    }
}

void debug_type(type_t *type) {
    debug_type_internal(type, false);
}

void debug_type_verbose(type_t *type) {
    debug_type_internal(type, true);
}