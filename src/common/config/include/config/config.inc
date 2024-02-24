// SPDX-License-Identifier: LGPL-3.0-only

#ifndef CFG_TYPE
#   define CFG_TYPE(id, name)
#endif

CFG_TYPE(eConfigInt,     "int")
CFG_TYPE(eConfigBool,    "bool")
CFG_TYPE(eConfigString,  "string")
CFG_TYPE(eConfigEnum,    "enum")
CFG_TYPE(eConfigFlags,   "flags")
CFG_TYPE(eConfigVector,  "list")

#undef CFG_TYPE
