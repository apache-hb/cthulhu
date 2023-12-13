#include "stacktrace/stacktrace.h"

void recurse(int i) {
    if (i > 0) recurse(i - 1);
    else stacktrace_print(stdout);
}

int main() {
    stacktrace_init();
    recurse(10);
    return 0;
}
