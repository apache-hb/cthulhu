#pragma once

#include "notify/diagnostic.h" // IWYU pragma: export

BEGIN_API

#define CTU_EVENT(name, ...) extern const diagnostic_t kEvent_##name;
#include "events.def"

END_API
