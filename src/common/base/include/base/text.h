#pragma once

#include "core/compiler.h"
#include "core/text.h"

BEGIN_API

text_t text_make(char *text, size_t size);
text_view_t text_view_make(const char *text, size_t size);

END_API
