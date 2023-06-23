#include "jvm.h"

#include "std/str.h"

const char *jvm_version_string(jvm_version_t version)
{
#define JVM_VERSION(id, str, it) case id: return str " (" #it ")";
    switch (version) {
#include "jvm.inc"
    default: return format("unknown (0x%x)", version);
    }
}

const char *jvm_const_tag_string(jvm_const_tag_t tag)
{
#define JVM_CONST(id, str, items) case id: return str " (" #items ")";
    switch (tag) {
#include "jvm.inc"
    default: return format("unknown (0x%x)", tag);
    }
}
