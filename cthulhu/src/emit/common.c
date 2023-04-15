#include "common.h"

#include "base/panic.h"
#include "io/io.h"

#include <string.h>

static const char *kIndent = "  ";

void write_string(emit_t *emit, const char *str)
{
    if (emit->io == NULL)
    {
        return;
    }
    
    size_t indentLen = strlen(kIndent);
    for (size_t i = 0; i < emit->indent; i++)
    {
        io_write(emit->io, kIndent, indentLen);
    }

    io_write(emit->io, str, strlen(str));
}

void emit_indent(emit_t *emit)
{
    emit->indent++;
}

void emit_dedent(emit_t *emit)
{
    CTASSERT(emit->indent > 0);
    emit->indent--;
}
