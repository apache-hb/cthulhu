// SPDX-License-Identifier: LGPL-3.0-or-later
#include "cthulhu/cinterop/cinterop.h"

#include "base/panic.h"

void cinterop_emit_header(io_t *io, const tree_t *tree)
{
    CTASSERT(io != NULL);
    CTASSERT(tree != NULL);
}
