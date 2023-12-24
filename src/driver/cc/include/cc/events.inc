#ifndef NEW_EVENT
#   define NEW_EVENT(id, ...)
#endif

NEW_EVENT(DriverUnimplemented, {
    .severity = eSeveritySorry,
    .id = "C0001",
    .brief = "C language driver unimplemented",
    .description = "The C language driver is not yet implemented, sorry."
})

#undef NEW_EVENT
