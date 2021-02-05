#ifndef KEY
#   define KEY(id, str)
#endif

#ifndef OP
#   define OP(id, str)
#endif

KEY(DEF, "def")
KEY(STRUCT, "struct")
KEY(UNION, "union")
KEY(ENUM, "enum")

#undef KEY
#undef OP
