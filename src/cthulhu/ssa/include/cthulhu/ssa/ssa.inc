// SPDX-License-Identifier: LGPL-3.0-only

#ifndef SSA_KIND
#   define SSA_KIND(ID, NAME)
#endif

SSA_KIND(eTypeEmpty, "empty")
SSA_KIND(eTypeUnit, "unit")
SSA_KIND(eTypeBool, "bool")
SSA_KIND(eTypeDigit, "digit")
SSA_KIND(eTypeClosure, "closure")
SSA_KIND(eTypePointer, "pointer")
SSA_KIND(eTypeEnum, "enum")
SSA_KIND(eTypeOpaque, "opaque")
SSA_KIND(eTypeStruct, "struct")
SSA_KIND(eTypeUnion, "union")

#undef SSA_KIND

#ifndef SSA_OPERAND
#   define SSA_OPERAND(ID, NAME)
#endif

SSA_OPERAND(eOperandEmpty, "empty")
SSA_OPERAND(eOperandUnit, "unit")

SSA_OPERAND(eOperandBlock, "block")
SSA_OPERAND(eOperandParam, "param")
SSA_OPERAND(eOperandReg, "reg")
SSA_OPERAND(eOperandImm, "imm")

SSA_OPERAND(eOperandLocal, "local")
SSA_OPERAND(eOperandGlobal, "global")
SSA_OPERAND(eOperandFunction, "function")

#undef SSA_OPERAND

#ifndef SSA_OPCODE
#   define SSA_OPCODE(ID, NAME)
#endif

SSA_OPCODE(eOpValue,   "value")
SSA_OPCODE(eOpNop,     "nop")
SSA_OPCODE(eOpStore,   "store")
SSA_OPCODE(eOpLoad,    "load")
SSA_OPCODE(eOpAddress, "address")

SSA_OPCODE(eOpUnary,   "unary")
SSA_OPCODE(eOpBinary,  "binary")
SSA_OPCODE(eOpCompare, "compare")

SSA_OPCODE(eOpCast, "cast")
SSA_OPCODE(eOpCall, "call")

SSA_OPCODE(eOpOffset, "offset") ///< get the address of an element in an array
SSA_OPCODE(eOpMember, "member") ///< get the address of a member in a struct

SSA_OPCODE(eOpReturn, "return")
SSA_OPCODE(eOpBranch, "branch")
SSA_OPCODE(eOpJump,   "jump")

#undef SSA_OPCODE
