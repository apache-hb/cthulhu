// SPDX-License-Identifier: GPL-3.0-only

#include "c/driver.h" // IWYU pragma: keep

#define NEW_EVENT(name, ...) const diagnostic_t kEvent_##name = __VA_ARGS__;

#include "c/events.inc"
