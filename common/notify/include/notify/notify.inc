#ifndef SEVERITY
#   define SEVERITY(ID, NAME)
#endif

SEVERITY(eSeveritySorry, "sorry")
SEVERITY(eSeverityInternal, "internal")
SEVERITY(eSeverityFatal, "fatal")
SEVERITY(eSeverityWarn, "warn")
SEVERITY(eSeverityInfo, "info")
SEVERITY(eSeverityDebug, "debug")

#undef SEVERITY
