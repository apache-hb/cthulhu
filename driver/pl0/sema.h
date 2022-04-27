#pragma once

#include "ast.h"
#include "cthulhu/driver/driver.h"
#include "cthulhu/hlir/hlir.h"
#include "cthulhu/util/report.h"

void pl0_init();
hlir_t *pl0_sema(runtime_t *runtime, void *node);
