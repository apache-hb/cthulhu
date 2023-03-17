#include <stdbool.h>
#include <stddef.h>
static signed int doThing(signed int i);
static signed int doThing(signed int param0) {
  entry: {
    signed int reg0 = *param0;
    return reg0;
  }
}
