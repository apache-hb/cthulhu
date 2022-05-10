#pragma once

#include "cthulhu/hlir/hlir.h"
#include "cthulhu/report/report.h"
#include "cthulhu/util/stream.h"

stream_t *c89_emit_modules(reports_t *reports, vector_t *modules);
