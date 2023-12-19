#pragma once

#include "core/compiler.h"

#include "notify/notify.h"

BEGIN_API

// general events
extern const diagnostic_t kEvent_NoSourceFiles;
extern const diagnostic_t kEvent_FailedToOpenSourceFile;
extern const diagnostic_t kEvent_FailedToIdentifyLanguage;
extern const diagnostic_t kEvent_NoFileExtension;
extern const diagnostic_t kEvent_FailedToCreateOutputDirectory;
extern const diagnostic_t kEvent_FailedToCreateOutputFile;
extern const diagnostic_t kEvent_FailedToWriteOutputFile;
extern const diagnostic_t kEvent_ExtensionConflict;

// scan events
extern const diagnostic_t kEvent_ParseInitFailed;
extern const diagnostic_t kEvent_ScanFailed;
extern const diagnostic_t kEvent_ParseFailed;
extern const diagnostic_t kEvent_ParseInternalError;

// ssa events
extern const diagnostic_t kEvent_UninitializedValueUsed;
extern const diagnostic_t kEvent_DivideByZero;
extern const diagnostic_t kEvent_ModuloByZero;

// shared language parsing events
extern const diagnostic_t kEvent_InvalidStringEscape;
extern const diagnostic_t kEvent_InvalidIntegerLiteral;

// shared language events
extern const diagnostic_t kEvent_SymbolShadowed;
extern const diagnostic_t kEvent_VariableNotFound;
extern const diagnostic_t kEvent_FunctionNotFound;
extern const diagnostic_t kEvent_TypeNotFound;
extern const diagnostic_t kEvent_FieldNotFound;
extern const diagnostic_t kEvent_InvalidIndirection;
extern const diagnostic_t kEvent_InvalidIndex;
extern const diagnostic_t kEvent_SymbolNotVisible;
extern const diagnostic_t kEvent_SymbolNotFound;
extern const diagnostic_t kEvent_MalformedTypeName;
extern const diagnostic_t kEvent_MutationOfConst;
extern const diagnostic_t kEvent_ExpressionHasNoEffect;

extern const diagnostic_t kEvent_InvalidLiteralType;
extern const diagnostic_t kEvent_InvalidBinaryOperation;
extern const diagnostic_t kEvent_InvalidUnaryOperation;
extern const diagnostic_t kEvent_InvalidInitializer;
extern const diagnostic_t kEvent_InvalidControlFlow;

extern const diagnostic_t kEvent_CirclularImport;
extern const diagnostic_t kEvent_ImportNotFound;
extern const diagnostic_t kEvent_ImportShadowed;

extern const diagnostic_t kEvent_InvalidEnumUnderlyingType;
extern const diagnostic_t kEvent_DuplicateField;
extern const diagnostic_t kEvent_InvalidFunctionSignature;
extern const diagnostic_t kEvent_InvalidArraySize;
extern const diagnostic_t kEvent_InvalidVariableType;

// tree events
extern const diagnostic_t kEvent_CyclicDependency;

extern const diagnostic_t kEvent_ImportedWithImpl;
extern const diagnostic_t kEvent_IgnoredMangling;
extern const diagnostic_t kEvent_EntryNotFunction;
extern const diagnostic_t kEvent_EntryMissingBody;
extern const diagnostic_t kEvent_MultipleEntryPoints;
extern const diagnostic_t kEvent_ReturnTypeMismatch;
extern const diagnostic_t kEvent_Deprecated;
extern const diagnostic_t kEvent_MayNotReturn;
extern const diagnostic_t kEvent_RecursiveEval;
extern const diagnostic_t kEvent_InfiniteSizedType;
extern const diagnostic_t kEvent_InvalidType;
extern const diagnostic_t kEvent_InvalidCast;

// emit events
extern const diagnostic_t kEvent_EmitStateError;

void evt_scan_error(logger_t *logger, node_t *node, const char *msg);
void evt_scan_unknown(logger_t *logger, node_t *node, const char *msg);

event_t *evt_symbol_shadowed(logger_t *logger, const char *name, const node_t *prev, const node_t *next);

END_API
