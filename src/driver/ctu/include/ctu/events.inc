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
