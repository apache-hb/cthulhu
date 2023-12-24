#include "cc/driver.h" // IWYU pragma: keep

#define NEW_EVENT(name, ...) const diagnostic_t kEvent_##name = __VA_ARGS__;

#include "cc/events.inc"
