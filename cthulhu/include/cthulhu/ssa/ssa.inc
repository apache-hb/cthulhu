#ifndef SSA_OPCODE
#   define SSA_OPCODE(op, name)
#endif

SSA_OPCODE(eOpReturn, "ret")
SSA_OPCODE(eOpJmp, "jmp")
SSA_OPCODE(eOpBranch, "branch")

SSA_OPCODE(eOpStore, "store")
SSA_OPCODE(eOpLoad, "load")
SSA_OPCODE(eOpAddr, "addr")

SSA_OPCODE(eOpImm, "imm")
SSA_OPCODE(eOpUnary, "unary")
SSA_OPCODE(eOpBinary, "binary")
SSA_OPCODE(eOpCompare, "compare")
SSA_OPCODE(eOpCast, "cast")
SSA_OPCODE(eOpCall, "call")

SSA_OPCODE(eOpIndex, "index") // index into an array
SSA_OPCODE(eOpOffset, "offset") // offset into a struct

SSA_OPCODE(eOpSizeOf, "sizeof")
SSA_OPCODE(eOpNop, "nop")

#undef SSA_OPCODE

#ifndef SSA_OPERAND
#   define SSA_OPERAND(op, name)
#endif

SSA_OPERAND(eOperandEmpty, "empty")
SSA_OPERAND(eOperandBlock, "block")
SSA_OPERAND(eOperandReg, "reg")
SSA_OPERAND(eOperandLocal, "local")
SSA_OPERAND(eOperandParam, "param")
SSA_OPERAND(eOperandGlobal, "global")
SSA_OPERAND(eOperandFunction, "function")
SSA_OPERAND(eOperandImm, "imm")

#undef SSA_OPERAND

#ifndef SSA_TYPE
#   define SSA_TYPE(op, name)
#endif

SSA_TYPE(eTypeDigit, "digit")
SSA_TYPE(eTypeDecimal, "decimal")
SSA_TYPE(eTypeBool, "bool")
SSA_TYPE(eTypeString, "string")

SSA_TYPE(eTypeUnit, "unit")
SSA_TYPE(eTypeEmpty, "empty")

SSA_TYPE(eTypePointer, "pointer")
SSA_TYPE(eTypeArray, "array")
SSA_TYPE(eTypeOpaque, "opaque")

SSA_TYPE(eTypeSignature, "signature")
SSA_TYPE(eTypeVarArgs, "varargs")
SSA_TYPE(eTypeVaList, "valist")

SSA_TYPE(eTypeStruct, "struct")
SSA_TYPE(eTypeUnion, "union")

#undef SSA_TYPE
