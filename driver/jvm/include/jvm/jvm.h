#pragma once

#include <stdint.h>

typedef struct io_t io_t;

typedef enum jvm_version_t {
#define JVM_VERSION(id, name, v) id = (v),
#include "jvm.inc"
} jvm_version_t;

typedef enum jvm_const_tag_t {
#define JVM_CONST(id, name, v) id = (v),
#include "jvm.inc"
} jvm_const_tag_t;

typedef enum jvm_attrib_tag_t {
#define JVM_ATTRIB(id, name) id,
#include "jvm.inc"
    eAttribTotal
} jvm_attrib_tag_t;

typedef enum jvm_access_t {
#define JVM_ACCESS(id, name, v) id = (v),
#include "jvm.inc"
} jvm_access_t;

const char *jvm_version_string(jvm_version_t version);
const char *jvm_const_tag_string(jvm_const_tag_t tag);
const char *jvm_attrib_tag_string(jvm_attrib_tag_t tag);
const char *jvm_access_string(jvm_access_t access);

// jvm loading

uint8_t read_io8(io_t *io);
uint16_t read_io16(io_t *io);
uint32_t read_io32(io_t *io);

uint8_t read_iobe8(io_t *io);
uint16_t read_iobe16(io_t *io);
uint32_t read_iobe32(io_t *io);
