/**
 * Autogenerated by the Cthulhu Compiler Collection C99 backend
 * Generated from C:::Users::ehb56::OneDrive::Documents::GitHub::ctulang::tests::ctu::pass
 */
#include <stddef.h>

// String literals
const char *strtab0 = "hello world";
// Imported symbols
extern signed int puts(const char *arg0);

// Global forwarding

// Function forwarding
signed int main(signed int arg0, const char ** arg1);

// Global initialization

// Function definitions
signed int main(signed int arg0, const char ** arg1) {
  signed int vreg0 = (*&puts)(strtab0);
  return (signed int)0;
}
