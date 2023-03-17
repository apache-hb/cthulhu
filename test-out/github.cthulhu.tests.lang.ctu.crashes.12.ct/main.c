#include <stdbool.h>
#include <stddef.h>
extern signed int puts(const char *s);
extern void exit(signed int code);
static void _start(void);
static void _start(void) {
  signed int(*local0[1])(const char *);
  entry: {
    *local0 = puts;
  }
}
