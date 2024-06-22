#include "floats.h"

#include <string.h>
#include <stdio.h>

marshall_float_t toFloat(int numerator, int denominator) {
    float result = (float)numerator / (float)denominator;
    void *pun = NULL;
    memcpy(&pun, &result, sizeof(float));
    return pun;
}

float fromFloat(marshall_float_t pun) {
    float result = 0.0f;
    memcpy(&result, &pun, sizeof(float));
    return result;
}

void putf(marshall_float_t value) {
    printf("float: %f\n", fromFloat(value));
}
