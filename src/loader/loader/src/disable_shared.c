// SPDX-License-Identifier: LGPL-3.0-only

#include "common.h"

#include "core/macros.h"

STA_DECL
loaded_module_t load_shared_module(loader_t *loader, module_type_t mask, const char *name)
{
    CT_UNUSED(loader);
    CT_UNUSED(mask);
    CT_UNUSED(name);

    return load_error(eLoadErrorDisabled, 0);
}
