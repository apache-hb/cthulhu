#include "backtrace/backtrace.h"
#include "base/panic.h"
#include "defaults/defaults.h"

int main(void)
{
    default_init();

    //CTASSERT(false);

    volatile int *ptr = NULL;
    *ptr = 0;
}
