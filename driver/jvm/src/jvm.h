#pragma once

#include <stdint.h>

typedef enum jvm_access_t {
    eAccessPublic = 0x0001,
    eAccessFinal = 0x0010,
    eAccessSuper = 0x0020,
    eAccessInterface = 0x0200,
    eAccessAbstract = 0x0400
} jvm_access_t;

typedef enum jvm_version_t {
#define JVM_VERSION(id, name, v) id = (v),
#include "jvm.inc"
} jvm_version_t;

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

typedef struct jvm_header_t {
    uint16_t minorVersion;
    uint16_t majorVersion;

    uint16_t constPoolCount;
    // TODO: pool data

    uint16_t accessFlags;
    uint16_t thisClass;
    uint16_t superClass;

    uint16_t interfacesCount;
    // TODO: interfaces

    uint16_t fieldsCount;
    // TODO: fields

    uint16_t methodsCount;
    // TODO: methods

    uint16_t attributesCount;
    // TODO: attributes
} jvm_header_t;

const char *jvm_version_string(jvm_version_t version);
const char *jvm_const_tag_string(jvm_const_tag_t tag);
