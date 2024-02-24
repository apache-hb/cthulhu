// SPDX-License-Identifier: GPL-3.0-only

#ifndef DECL_TAG
#   define DECL_TAG(id, init, str)
#endif

DECL_TAG(ePl0TagValues, = eSemaValues, "values")
DECL_TAG(ePl0TagTypes, = eSemaTypes, "types")
DECL_TAG(ePl0TagProcs, = eSemaProcs, "procedures")
DECL_TAG(ePl0TagModules, = eSemaModules, "modules")

DECL_TAG(ePl0TagImportedValues, /* empty */, "imported values")
DECL_TAG(ePl0TagImportedProcs, /* empty */, "imported procedures")

#undef DECL_TAG
