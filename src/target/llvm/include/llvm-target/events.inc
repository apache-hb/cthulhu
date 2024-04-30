#ifndef NEW_EVENT
#   define NEW_EVENT(id, ...)
#endif

NEW_EVENT(LLVMWarnEvent, {
    .severity = eSeverityWarn,
    .id = "LLVM0001",
    .brief = "LLVM warning",
    .description = "An LLVM warning was encountered",
})

NEW_EVENT(LLVMFatalEvent, {
    .severity = eSeverityFatal,
    .id = "LLVM0002",
    .brief = "LLVM fatal error",
    .description = "An LLVM fatal error was encountered",
})

NEW_EVENT(LLVMRemarkEvent, {
    .severity = eSeverityInfo,
    .id = "LLVM0003",
    .brief = "LLVM remark",
    .description = "An LLVM remark was encountered",
})

NEW_EVENT(LLVMNoteEvent, {
    .severity = eSeverityInfo,
    .id = "LLVM0004",
    .brief = "LLVM note",
    .description = "An LLVM note was encountered",
})

#undef NEW_EVENT
