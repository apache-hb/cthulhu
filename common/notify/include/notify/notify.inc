#ifndef SEVERITY
#   define SEVERITY(ID, NAME)
#endif

// ordered from highest severity to lowest
// to allow for comparisons
SEVERITY(eSeveritySorry, "sorry")
SEVERITY(eSeverityInternal, "panic")
SEVERITY(eSeverityFatal, "fatal")
SEVERITY(eSeverityWarn, "warn")
SEVERITY(eSeverityInfo, "info")
SEVERITY(eSeverityDebug, "debug")

#undef SEVERITY
