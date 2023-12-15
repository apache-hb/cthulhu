#pragma once

#include "stacktrace/stacktrace.h"

void frame_resolve_inner(const frame_t *frame, symbol_t *symbol);

void stacktrace_read_inner(bt_frame_t callback, void *user);
