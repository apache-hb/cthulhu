#include "common.h"
#include "const.h"

#include "std/str.h"
#include "std/vector.h"

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

const char *jvm_access_string(jvm_access_t access)
{
    vector_t *parts = vector_new(8);
#define JVM_ACCESS(id, str, bits) if (access & (bits)) { vector_push(&parts, str); }

#include "jvm.inc"

    if (vector_len(parts) > 0)
    {
        return str_join("|", parts);
    }

    return format("none (0x%x)", access);
}
