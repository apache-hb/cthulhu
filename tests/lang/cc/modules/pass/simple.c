#include <ext_modules.h>

module;

int do_inner(int i) { return i * 2; }

export module simple;

export int do_stuff(int i) { return do_inner(i + 1); }
