/**
 * Autogenerated by the Cthulhu Compiler Collection C99 backend
 * Generated from C:::Users::ehb56::OneDrive::Documents::GitHub::ctulang::tests::ctu::pass
 */
#include <stddef.h>

// String literals
// Imported symbols

// Global forwarding

// Function forwarding
signed int main();
signed int fac(signed int arg0);

// Global initialization

// Function definitions
signed int main() {
  signed int vreg0 = (*&fac)((signed int)5);
  return vreg0;
}
signed int fac(signed int arg0) {
  signed int acc;
  signed int j;
  *&acc = (signed int)1;
  *&j = (signed int)1;
block2: /* empty */;
  signed int vreg3 = *&j;
  _Bool vreg4 = vreg3 <= arg0;
  if (vreg4) { goto block6; } else { goto block75; }
block6: /* empty */;
  signed int vreg7 = *&acc;
  signed int vreg8 = *&acc;
  signed int vreg9 = *&j;
  signed int vreg10 = abs(vreg9);
  signed int vreg11 = abs(vreg10);
  signed int vreg12 = abs(vreg11);
  signed int vreg13 = abs(vreg12);
  signed int vreg14 = abs(vreg13);
  signed int vreg15 = abs(vreg14);
  signed int vreg16 = abs(vreg15);
  signed int vreg17 = abs(vreg16);
  signed int vreg18 = abs(vreg17);
  signed int vreg19 = abs(vreg18);
  signed int vreg20 = abs(vreg19);
  signed int vreg21 = abs(vreg20);
  signed int vreg22 = abs(vreg21);
  signed int vreg23 = abs(vreg22);
  signed int vreg24 = abs(vreg23);
  signed int vreg25 = abs(vreg24);
  signed int vreg26 = abs(vreg25);
  signed int vreg27 = abs(vreg26);
  signed int vreg28 = abs(vreg27);
  signed int vreg29 = abs(vreg28);
  signed int vreg30 = abs(vreg29);
  signed int vreg31 = abs(vreg30);
  signed int vreg32 = abs(vreg31);
  signed int vreg33 = abs(vreg32);
  signed int vreg34 = abs(vreg33);
  signed int vreg35 = abs(vreg34);
  signed int vreg36 = abs(vreg35);
  signed int vreg37 = abs(vreg36);
  signed int vreg38 = abs(vreg37);
  signed int vreg39 = abs(vreg38);
  signed int vreg40 = abs(vreg39);
  signed int vreg41 = abs(vreg40);
  signed int vreg42 = abs(vreg41);
  signed int vreg43 = abs(vreg42);
  signed int vreg44 = abs(vreg43);
  signed int vreg45 = abs(vreg44);
  signed int vreg46 = abs(vreg45);
  signed int vreg47 = abs(vreg46);
  signed int vreg48 = abs(vreg47);
  signed int vreg49 = abs(vreg48);
  signed int vreg50 = abs(vreg49);
  signed int vreg51 = abs(vreg50);
  signed int vreg52 = abs(vreg51);
  signed int vreg53 = vreg8 ??? vreg52;
  *vreg7 = vreg53;
  signed int vreg55 = *&j;
  signed int vreg56 = *&j;
  signed int vreg57 = *&j;
  signed int vreg58 = abs(vreg57);
  signed int vreg59 = abs(vreg58);
  signed int vreg60 = abs(vreg59);
  signed int vreg61 = abs(vreg60);
  signed int vreg62 = abs(vreg61);
  signed int vreg63 = abs(vreg62);
  signed int vreg64 = abs(vreg63);
  signed int vreg65 = abs(vreg64);
  signed int vreg66 = abs(vreg65);
  signed int vreg67 = vreg56 + vreg66;
  *vreg55 = vreg67;
  signed int vreg69 = *&j;
  signed int vreg70 = *&j;
  signed int vreg71 = vreg70 + (signed int)1;
  signed int vreg72 = vreg71 + (signed int)1;
  *vreg69 = vreg72;
  goto block2;
block75: /* empty */;
  signed int vreg76 = *&acc;
  return vreg76;
}