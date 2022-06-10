#include "platform/segfault.h"

#include <stddef.h>

volatile int *kDanger = NULL;

int main() 
{
    install_segfault();

    *kDanger = 1;
}
