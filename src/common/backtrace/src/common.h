#pragma once

#include "backtrace/backtrace.h"

frame_resolve_t bt_resolve_inner(const frame_t *frame, symbol_t *symbol);

void bt_read_inner(bt_frame_t callback, void *user);
