#ifndef SEVERITY
#   define SEVERITY(ID, NAME)
#endif

SEVERITY(eSorry, "sorry")
SEVERITY(eInternal, "internal")
SEVERITY(eFatal, "fatal")
SEVERITY(eWarn, "warn")
SEVERITY(eInfo, "info")
SEVERITY(eDebug, "debug")

#undef SEVERITY
