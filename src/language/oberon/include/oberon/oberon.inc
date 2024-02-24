// SPDX-License-Identifier: GPL-3.0-only

#ifndef NEW_EVENT
#   define NEW_EVENT(id, ...)
#endif

NEW_EVENT(BlockMismatchEnds, {
    .severity = eSeverityWarn,
    .id = "OBR0001",
    .brief = "Mismatching BEGIN and END names",
    .description = "The names of BEGIN and END blocks should match.\n"
                   "Specified in Section [10: Procedure declarations].",
})

NEW_EVENT(ArrayLengthOverflow, {
    .severity = eSeverityFatal,
    .id = "OBR0002",
    .brief = "Array size too large",
    .description = "The size of an array must be less than 2^32.\n"
                   "This is not defined in the Oberon-2 standard, but is a limitation of this implementation.",
})

NEW_EVENT(ArrayLengthNotConstant, {
    .severity = eSeverityFatal,
    .id = "OBR0003",
    .brief = "Array length must be a constant",
    .description = "The length of an array must be a constant.\n"
                   "This is not defined in the Oberon-2 standard, but is a limitation of this implementation.",
})

// deprecated, we now support public read-only
NEW_EVENT(PublicReadOnlyNotSupported, {
    .severity = eSeverityWarn,
    .id = "OBR0004",
    .brief = "Public read-only unimplemented",
    .description = "Public read-only symbols are not yet supported",
})

NEW_EVENT(ArrayLengthZero, {
    .severity = eSeverityFatal,
    .id = "OBR0005",
    .brief = "Array size must be greater than zero",
    .description = "The size of an array must be greater than zero.\n"
                   "This is not defined in the Oberon-2 standard, but is a limitation of this implementation.",
})

NEW_EVENT(ArrayLengthNegative, {
    .severity = eSeverityFatal,
    .id = "OBR0006",
    .brief = "Array size must be greater than zero",
    .description = "The size of an array must be greater than zero.\n"
                   "This is not defined in the Oberon-2 standard, but is a limitation of this implementation.",
})

#undef NEW_EVENT

#ifndef DECL_TAG
#   define DECL_TAG(id, val, name)
#endif

DECL_TAG(eObrTagValues, = eSemaValues, "values")
DECL_TAG(eObrTagTypes, = eSemaTypes, "types")
DECL_TAG(eObrTagProcs, = eSemaProcs, "procs")
DECL_TAG(eObrTagModules, = eSemaModules, "modules")
DECL_TAG(eObrTagImports, /* empty */ , "imports")

#undef DECL_TAG

#ifndef OBR_VIS
#   define OBR_VIS(ID, NAME, STR)
#endif

OBR_VIS(eObrVisPrivate, "private", "")
OBR_VIS(eObrVisPublic, "public", "*")
OBR_VIS(eObrVisReadOnly, "read_only", "-")

#undef OBR_VIS
