#ifndef NEW_EVENT
#   define NEW_EVENT(id, ...)
#endif

NEW_EVENT(DuplicateDefaultCases, {
    .severity = eSeverityFatal,
    .id = "CTU0001",
    .brief = "duplicate default cases",
    .description =
        "enum has multiple default cases.\n"
        "Enums are only allowed to have one or zero default cases.\n",
})

NEW_EVENT(InvalidDereference, {
    .severity = eSeverityFatal,
    .id = "CTU0002",
    .brief = "invalid dereference",
    .description =
        "invalid dereference.\n"
        "You can only dereference pointers.\n",
})

#undef NEW_EVENT

#ifndef DECL_TAG
#   define DECL_TAG(id, val, name)
#endif

DECL_TAG(eCtuTagValues, = eSemaValues, "values")
DECL_TAG(eCtuTagTypes, = eSemaTypes, "types")
DECL_TAG(eCtuTagFunctions, = eSemaProcs, "functions")
DECL_TAG(eCtuTagModules, = eSemaModules, "modules")
DECL_TAG(eCtuTagImports, /* empty */, "imports")
DECL_TAG(eCtuTagAttribs, /* empty */, "attributes")
DECL_TAG(eCtuTagSuffixes, /* empty */, "suffixes")
DECL_TAG(eCtuTagLabels, /* empty */, "labels")

#undef DECL_TAG
