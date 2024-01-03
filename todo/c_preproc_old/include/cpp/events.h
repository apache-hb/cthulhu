#pragma once

#include "notify/diagnostic.h" // IWYU pragma: export

#define NEW_EVENT(id, ...) extern const diagnostic_t kEvent_##id;
#include "events.inc"
