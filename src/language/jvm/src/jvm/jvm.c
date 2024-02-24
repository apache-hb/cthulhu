#include "jvm/jvm.h"

#include "std/str.h"
#include "std/vector.h"

#include "base/panic.h"
#include "base/endian.h"

#include "io/io.h"

#include <string.h>

const char *jvm_version_string(jvm_version_t version)
{
#define JVM_VERSION(id, str, it) case id: return str " (" #it ")";
    switch (version) {
#include "jvm/jvm.inc"
    default: return format("unknown (0x%x)", version);
    }
}

const char *jvm_const_tag_string(jvm_const_tag_t tag)
{
#define JVM_CONST(id, str, items) case id: return str " (" #items ")";
    switch (tag) {
#include "jvm/jvm.inc"
    default: return format("unknown (0x%x)", tag);
    }
}

const char *jvm_attrib_tag_string(jvm_attrib_tag_t tag)
{
#define JVM_ATTRIB(id, str) case id: return str;
    switch (tag) {
#include "jvm/jvm.inc"
    default: return format("unknown (0x%x)", tag);
    }
}

const char *jvm_access_string(jvm_access_t access)
{
    arena_t *arena = get_global_arena();
    vector_t *parts = vector_new(8, arena);
#define JVM_ACCESS(id, str, bits) if (access & (bits)) { vector_push(&parts, str); }

#include "jvm/jvm.inc"

    if (vector_len(parts) > 0)
    {
        return str_join("|", parts, arena);
    }

    return str_format(arena, "none (0x%x)", access);
}

#define FN_READ_IO(name, type, err) \
    type name(io_t *io) \
    { \
        type value = (err); \
        CTASSERT(io_read(io, &value, sizeof(type)) == sizeof(type)); \
        return value; \
    }


FN_READ_IO(read_io8, uint8_t, UINT8_MAX)
FN_READ_IO(read_io16, uint16_t, UINT16_MAX)
FN_READ_IO(read_io32, uint32_t, UINT32_MAX)

uint8_t read_iobe8(io_t *io)
{
    return read_io8(io);
}

uint16_t read_iobe16(io_t *io)
{
    return native_order16(read_io16(io), eEndianBig);
}

uint32_t read_iobe32(io_t *io)
{
    return native_order32(read_io32(io), eEndianBig);
}
