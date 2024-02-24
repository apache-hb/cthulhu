// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "backtrace/backtrace.h"

frame_resolve_t bt_resolve_inner(const bt_frame_t *frame, bt_symbol_t *symbol);

void bt_read_inner(bt_trace_t callback, void *user);
