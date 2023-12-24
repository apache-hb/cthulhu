#include "ctu/sema/decl/resolve.h"

#include "cthulhu/util/util.h"

#include "base/panic.h"

ctu_t *begin_resolve(tree_t *sema, tree_t *self, void *user, ctu_kind_t kind)
{
    ctu_t *decl = user;
    CTASSERTF(decl->kind == kind, "decl %s is not a %d", decl->name, kind);

    util_set_current_module(sema, sema);
    util_set_current_symbol(sema, self);

    return decl;
}
