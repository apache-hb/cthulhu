#ifndef CFG_TYPE
#   define CFG_TYPE(id, name)
#endif

CFG_TYPE(eConfigInt,     "int")
CFG_TYPE(eConfigBool,    "bool")
CFG_TYPE(eConfigString,  "string")
CFG_TYPE(eConfigEnum,    "enum")
CFG_TYPE(eConfigFlags,   "flags")

// TODO: merge vector and string somehow
CFG_TYPE(eConfigVector,  "vector")

#undef CFG_TYPE
