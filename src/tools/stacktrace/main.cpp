#include "backtrace/backtrace.h"

template<int I> void recurse() {
    recurse<I - 1>();
}

template<>
void recurse<0>() {
    bt_print_trace(stdout);
}

int main() {
    bt_init();
    recurse<10>();
    return 0;
}
