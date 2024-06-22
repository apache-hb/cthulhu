#pragma once

typedef void *marshall_float_t;

marshall_float_t toFloat(int numerator, int demoninator);
float fromFloat(marshall_float_t value);

void putf(marshall_float_t value);
