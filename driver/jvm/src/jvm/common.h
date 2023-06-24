#pragma once

typedef enum jvm_access_t {
#define JVM_ACCESS(id, name, v) id = (v),
#include "jvm.inc"
} jvm_access_t;

typedef enum jvm_version_t {
#define JVM_VERSION(id, name, v) id = (v),
#include "jvm.inc"
} jvm_version_t;

const char *jvm_version_string(jvm_version_t version);
const char *jvm_access_string(jvm_access_t access);
