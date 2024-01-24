#include "ref/events.h" // IWYU pragma: keep

#define CTU_EVENT(name, ...) const diagnostic_t kEvent_##name = __VA_ARGS__;
#include "ref/events.def"
