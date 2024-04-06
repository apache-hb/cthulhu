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

#ifndef CFG_SPELLING
#   define CFG_SPELLING(id, v, name, prefix)
#endif

CFG_SPELLING(eSpellShort, (1 << 0), "short", "-")
CFG_SPELLING(eSpellLong,  (1 << 1), "long", "--")
CFG_SPELLING(eSpellMicrosoft,    (1 << 2), "microsoft", "/")

#undef CFG_SPELLING
