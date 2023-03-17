#include <stdbool.h>
#include <stddef.h>
signed int result = ((signed int)(0));
signed int lhs = ((signed int)(0));
signed int rhs = ((signed int)(0));
void entry(void);
void add(void);
extern signed int printf(const char *string, ...);
void entry(void) {
  entry: {
    *lhs = ((signed int)(25));
    *rhs = ((signed int)(50));
    add();
    signed int reg3 = *result;
    signed int reg4 = printf("%d\n", reg3);
  }
}
void add(void) {
  signed int local0[1];
  entry: {
    signed int reg0 = *lhs;
    signed int reg1 = *rhs;
    signed int reg2 = (reg0 + reg1);
    *local0 = reg2;
    signed int reg4 = *local0;
    *result = reg4;
  }
}
int main(void) {
  entry: {
    entry();
  }
}
