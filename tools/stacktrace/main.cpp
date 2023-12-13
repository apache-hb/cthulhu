#include "stacktrace/stacktrace.h"

template<int I> void recurse() {
    recurse<I - 1>();
}

template<>
void recurse<0>() {
    stacktrace_print(stdout);
}

int main() {
    stacktrace_init();
    recurse<10>();
    return 0;
}
