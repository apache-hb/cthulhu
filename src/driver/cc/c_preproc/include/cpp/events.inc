#ifndef NEW_EVENT
#   define NEW_EVENT(id, ...)
#endif

NEW_EVENT(IncludeDepthExceeded, {
    .severity = eSeverityFatal,
    .id = "PP0001",
    .brief = "Include depth exceeded",
    .description =
        "The maximum include depth has been exceeded. This limit can be increased for complex include heirarchies\n"
        "Alternatively this may be a sign of a circular include",
})

#undef NEW_EVENT
