#include "base/result.h"

#include <stdint.h>

// we only use the first 32 bits of the pointer to store the result

#define STATUS_BITS 8
#define CODE_BITS 24

#define STATUS_MASK ((1 << STATUS_BITS) - 1)
#define CODE_MASK ((1 << CODE_BITS) - 1)

#define TYPE_OK 0
#define TYPE_ERROR 1

#define MAKE_RESULT(status, code) (((status) << CODE_BITS) | (code))

#define GET_STATUS(value) ((value) >> CODE_BITS)
#define GET_CODE(value) ((value) & CODE_MASK)

static void *set_value(uintptr_t value)
{
    return (void *)value;
}

static uintptr_t get_value(void *ptr)
{
    return (uintptr_t)ptr;
}

result_t result_ok(int code)
{
    return set_value(MAKE_RESULT(TYPE_OK, code));
}

result_t result_error(int code)
{
    return set_value(MAKE_RESULT(TYPE_ERROR, code));
}

bool should_exit(result_t result)
{
    return GET_STATUS(get_value(result)) == TYPE_ERROR;
}

int get_exit_code(result_t result) 
{
    return GET_CODE(get_value(result));
}
