_Module;

static int magic_value = 10;

_Export _Module simple;

_Export int do_stuff(int i) { return magic_value * i; }
