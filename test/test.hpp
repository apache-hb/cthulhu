#pragma once

#include <stdlib.h>

#define ASSERT(expr) if (!(expr)) { printf(#expr "\n"); exit(1); }
