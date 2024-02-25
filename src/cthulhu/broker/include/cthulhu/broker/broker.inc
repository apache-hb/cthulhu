// SPDX-License-Identifier: LGPL-3.0-only

#ifndef BROKER_PASS
#   define BROKER_PASS(ID, STR)
#endif

BROKER_PASS(ePassForwardDecls, "forward_decls")
BROKER_PASS(ePassImportModules, "import_modules")
BROKER_PASS(ePassCompileTypes, "compile_types")
BROKER_PASS(ePassCompileDecls, "compile_decls")

#undef BROKER_PASS

#ifndef BROKER_STAGE
#   define BROKER_STAGE(ID, STR)
#endif

// before language initialization
BROKER_STAGE(eStageInit, "init")

// before parsing files
BROKER_STAGE(eStageParse, "parse")

// before semantic analysis
BROKER_STAGE(eStageSema, "sema")

// before final resolve
BROKER_STAGE(eStageResolve, "resolve")

// before tree validation
BROKER_STAGE(eStageCheck, "check")

// before emitting tree IR (may not happen depending on targets)
BROKER_STAGE(eStageEmitTree, "emit_tree")

// before lowering to SSA
BROKER_STAGE(eStageLower, "lower")

// before ssa optimization
BROKER_STAGE(eStageOptimize, "optimize")

// before emitting ssa IR (may not happen depending on targets)
BROKER_STAGE(eStageEmitSsa, "emit_ssa")

// before deinitialization
BROKER_STAGE(eStageDeinit, "deinit")

#undef BROKER_STAGE

#ifndef BROKER_EVENT
#   define BROKER_EVENT(ID, STR)
#endif

// an attribute has been matched
BROKER_EVENT(eEventMatchAttrib, "match_attribute")

#undef BROKER_EVENT

#ifndef BROKER_ARENA
#   define BROKER_ARENA(ID, STR)
#endif

// arena for language specific asts
BROKER_ARENA(eArenaAst, "ast")

// arena for the tree IR produced by a language
BROKER_ARENA(eArenaTree, "tree")

// arena for string allocations
BROKER_ARENA(eArenaString, "string")

// arena for transient allocations that
// do not need to be persisted between passes
BROKER_ARENA(eArenaTransient, "transient")

#undef BROKER_ARENA

#ifndef FILE_LAYOUT
#   define FILE_LAYOUT(id, str)
#endif

// output files are organized in a tree
FILE_LAYOUT(eFileLayoutTree, "tree")

// output files are organized in a flat structure
FILE_LAYOUT(eFileLayoutFlat, "flat")

// all output is in a single file
FILE_LAYOUT(eFileLayoutSingle, "single")

// output in a source + header pair
FILE_LAYOUT(eFileLayoutPair, "pair")

#undef FILE_LAYOUT
