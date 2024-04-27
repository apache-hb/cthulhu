// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef MS_DEMANGLE_NODE
#   define MS_DEMANGLE_NODE(id, name)
#endif

MS_DEMANGLE_NODE(eMsNodeSymbol, "symbol")
MS_DEMANGLE_NODE(eMsNodePointerType, "pointer_type")
MS_DEMANGLE_NODE(eMsNodeQualifiedName, "qualified_name")

#undef MS_DEMANGLE_NODE

#ifndef ITANIUM_DEMANGLE_NODE
#   define ITANIUM_DEMANGLE_NODE(id, name)
#endif

#undef ITANIUM_DEMANGLE_NODE
