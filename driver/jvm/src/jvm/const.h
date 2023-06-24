#pragma once

#include <stdint.h>

typedef struct io_t io_t;

typedef enum jvm_const_tag_t {
#define JVM_CONST(id, name, v) id = (v),
#include "jvm.inc"
} jvm_const_tag_t;

typedef struct jvm_class_info_t {
    uint16_t nameIndex;
} jvm_class_info_t;

typedef struct jvm_field_info_t {
    uint16_t classIndex;
    uint16_t nameAndTypeIndex;
} jvm_field_info_t;

typedef struct jvm_string_info_t {
    uint16_t stringIndex;
} jvm_string_info_t;

typedef struct jvm_int_info {
    uint32_t value;
} jvm_int_info_t;

typedef struct jvm_float_info {
    uint32_t value;
} jvm_float_info_t;

typedef struct jvm_long_info {
    uint32_t hiValue;
    uint32_t loValue;
} jvm_long_info_t;

typedef struct jvm_double_info {
    uint32_t hiValue;
    uint32_t loValue;
} jvm_double_info_t;

typedef struct jvm_name_and_type_info_t {
    uint16_t nameIndex;
    uint16_t descriptorIndex;
} jvm_name_and_type_info_t;

typedef struct jvm_utf8_info_t {
    uint16_t length;
    uint8_t *bytes;
} jvm_utf8_info_t;

typedef struct jvm_method_type_info_t {
    uint16_t descriptorIndex;
} jvm_method_type_info_t;

typedef struct jvm_method_handle_info_t {
    uint8_t referenceKind;
    uint16_t referenceIndex;
} jvm_method_handle_info_t;

typedef struct jvm_invoke_dynamic_info_t {
    uint16_t bootstrapMethodAttrIndex;
    uint16_t nameAndTypeIndex;
} jvm_invoke_dynamic_info_t;

typedef struct jvm_const_t {
    jvm_const_tag_t tag;
    union {
        jvm_class_info_t classInfo;
        jvm_field_info_t fieldInfo;
        jvm_string_info_t stringInfo;
        jvm_int_info_t intInfo;
        jvm_float_info_t floatInfo;
        jvm_long_info_t longInfo;
        jvm_double_info_t doubleInfo;
        jvm_name_and_type_info_t nameAndTypeInfo;
        jvm_utf8_info_t utf8Info;
        jvm_method_type_info_t methodTypeInfo;
        jvm_method_handle_info_t methodHandleInfo;
        jvm_invoke_dynamic_info_t invokeDynamicInfo;
    };
} jvm_const_t;

jvm_const_t jvm_parse_const(io_t *io);
const char *jvm_const_tag_string(jvm_const_tag_t tag);
