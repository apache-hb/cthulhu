// SPDX-License-Identifier: LGPL-3.0-or-later
#include "meta/meta.h"
#include "meta/ast.h"

#include "io/io.h"
#include "base/panic.h"

void meta_emit_cmdline(IN_NOTNULL const meta_ast_t *ast, IN_NOTNULL io_t *header, IN_NOTNULL io_t *source)
{
    CTASSERT(ast != NULL);
    CTASSERT(header != NULL);
    CTASSERT(source != NULL);
}
