/**
 * Autogenerated by the Cthulhu Compiler Collection C99 backend
 * Generated from C:::Users::ehb56::OneDrive::Documents::GitHub::ctulang::tests::pl0
 */
#include <stddef.h>

// String literals
const char *strtab0 = "%d
";
// Imported symbols
extern signed int printf(const char *arg0, ...);

// Global forwarding
signed long x;

// Function forwarding
void hello();
void main();

// Global initialization
signed long x = (signed long)0;

// Function definitions
void hello() {
  signed long x;
  *&x = (signed long)1;
  signed long vreg1 = *&x;
  signed long vreg2 = vreg1 + (signed long)1;
  *&x = vreg2;
  signed long vreg4 = *&x;
  signed int vreg5 = (*&printf)(strtab0, vreg4);
  return;
}
void main() {
  (*&hello)();
  return;
}