
#include <string.h>

void *toFloat(int numerator, int denominator) {
    float result = (float)numerator / (float)denominator;
    void *pun = NULL;
    memcpy(&pun, &result, sizeof(float));
    return pun;
}

float fromFloat(void *pun) {
    float result = 0.0f;
    memcpy(&result, &pun, sizeof(float));
    return result;
}
