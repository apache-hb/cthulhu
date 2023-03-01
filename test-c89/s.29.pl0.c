#include <stddef.h>
#include <stdint.h>
void main(void);
extern signed int printf(const char *string, ...);
void main(void)
{
  (printf("%d\n", 25));
}
