#include "cthulhu/emit/jvm.h"

#include "base/macros.h"

#define CLS_MAGIC 0xCAFEBABE

typedef struct cls_emit_t
{
    io_t *io;
} cls_emit_t;

void jvm_emit(jvm_emit_t config, ssa_module_t *module)
{
    UNUSED(config);
    UNUSED(module);
}
