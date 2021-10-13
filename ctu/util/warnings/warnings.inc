#ifndef WARN
#   define WARN(id, str)
#endif

WARN(WARN_UNKNOWN_ATTRIBUTE, "unknown-attribute")
WARN(WARN_UNUSED_EXPRESSION, "unused-expression")
WARN(WARN_REDUNDANT_MUTABLE, "redundant-mutable")

#undef WARN
