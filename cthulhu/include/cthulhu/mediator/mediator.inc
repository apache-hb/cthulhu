#ifndef STAGE
#   define STAGE(ID, STR) 
#endif

STAGE(eStageForwardSymbols, "forward-symbols")
STAGE(eStageCompileImports, "compile-imports")
STAGE(eStageCompileTypes, "compile-types")
STAGE(eStageCompileSymbols, "compile-symbols")

#undef STAGE