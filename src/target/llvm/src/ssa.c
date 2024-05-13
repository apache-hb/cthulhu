// SPDX-License-Identifier: LGPL-3.0-or-later

#include "base/panic.h"
#include "llvm-target/target.h"

#include "cthulhu/ssa/ssa.h"

#include "core/macros.h"
#include "notify/notify.h"
#include "std/map.h"
#include "std/str.h"
#include "std/typed/vector.h"
#include "std/vector.h"

#include <llvm-c/Core.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>
#include <llvm-c/Analysis.h>

typedef struct llvm_target_t
{
    arena_t *arena;

    map_t *types; // map_t<ssa_type_t*, LLVMTypeRef>
} llvm_target_t;

static LLVMContextRef gContext = NULL;
static char *gTargetTriple = NULL;
static LLVMTargetRef gTarget = NULL;
static LLVMTargetMachineRef gMachine = NULL;
static LLVMTargetDataRef gDataLayout = NULL;

// ^(?!(typedef|\*)).*LLVM.*([\S\s]|[,])\)

static const diagnostic_t *get_diagnostic_level(LLVMDiagnosticSeverity severity)
{
    switch (severity)
    {
    case LLVMDSError: return &kEvent_LLVMFatalEvent;
    case LLVMDSWarning: return &kEvent_LLVMWarnEvent;
    case LLVMDSRemark: return &kEvent_LLVMRemarkEvent;
    case LLVMDSNote: return &kEvent_LLVMNoteEvent;
    }

    return &kEvent_LLVMFatalEvent;
}

static void llvm_report_callback(LLVMDiagnosticInfoRef info, void *ctx)
{
    target_runtime_t *self = ctx;

    char *message = LLVMGetDiagInfoDescription(info);
    const diagnostic_t *event = get_diagnostic_level(LLVMGetDiagInfoSeverity(info));
    const node_t *node = broker_get_node(self->broker);

    msg_notify(self->logger, event, node, "%s", message);

    LLVMDisposeMessage(message);
}

void llvm_create(target_runtime_t *runtime)
{
    LLVMInitializeAllTargetInfos();
    LLVMInitializeAllTargets();
    LLVMInitializeAllTargetMCs();
    LLVMInitializeAllAsmPrinters();
    LLVMInitializeAllAsmParsers();

    gContext = LLVMContextCreate();
    if (!gContext)
        CT_PANIC("Failed to create LLVM context");

    LLVMContextSetDiagnosticHandler(gContext, llvm_report_callback, runtime);

    gTargetTriple = LLVMGetDefaultTargetTriple();

    char *error = NULL;
    if (LLVMGetTargetFromTriple(gTargetTriple, &gTarget, &error))
        CT_PANIC("Failed to get target %s: %s", gTargetTriple, error);

    gMachine = LLVMCreateTargetMachine(gTarget, gTargetTriple, "generic", "", LLVMCodeGenLevelDefault, LLVMRelocStatic, LLVMCodeModelDefault);

    gDataLayout = LLVMCreateTargetDataLayout(gMachine);
}

void llvm_destroy(target_runtime_t *runtime)
{
    // empty
    CT_UNUSED(runtime);

    LLVMDisposeTargetData(gDataLayout);
    LLVMDisposeTargetMachine(gMachine);
    LLVMDisposeMessage(gTargetTriple);
    LLVMContextDispose(gContext);

    LLVMShutdown();
}

static LLVMTypeRef get_llvm_type(llvm_target_t *self, const ssa_type_t *type);

// only uses the width as llvm digit types sign depends on instructions used
// rather than the type itself
static LLVMTypeRef get_llvm_digit_type(LLVMContextRef ctx, ssa_type_digit_t digit)
{
    // TODO: this may depend on target abi
    // currently copies what stdint.h does on windows
    digit_t width = digit.digit;
    switch (width)
    {
    case eDigit8: case eDigitChar:
    case eDigitLeast8: case eDigitFast8:
        return LLVMInt8TypeInContext(ctx);
    case eDigit16: case eDigitShort:
    case eDigitLeast16:
        return LLVMInt16TypeInContext(ctx);
    case eDigit32: case eDigitInt:
    case eDigitLong: case eDigitLeast32:
    case eDigitFast16: case eDigitFast32:
        return LLVMInt32TypeInContext(ctx);
    case eDigit64: case eDigitLongLong:
    case eDigitLeast64: case eDigitFast64:
    case eDigitMax: case eDigitSize: case eDigitPtr: // TODO: this assumes 64 bit always
        return LLVMInt64TypeInContext(ctx);

        // TODO: is this how we want to handle floats?
    case eDigitHalf:
        return LLVMHalfTypeInContext(ctx);
    case eDigitFloat:
        return LLVMFloatTypeInContext(ctx);
    case eDigitDouble:
        return LLVMDoubleTypeInContext(ctx);

    default:
        CT_NEVER("Invalid digit width %s", digit_name(width));
    }
}

static LLVMTypeRef get_llvm_pointer_type(llvm_target_t *self, ssa_type_pointer_t ptr)
{
    LLVMTypeRef inner = get_llvm_type(self, ptr.pointer);
    if (ptr.length == 0)
    {
        return LLVMPointerType(inner, 0);
    }

    return LLVMArrayType2(inner, ptr.length);
}

static LLVMTypeRef get_llvm_closure_type(llvm_target_t *self, ssa_type_closure_t closure)
{
    size_t len = typevec_len(closure.params);
    LLVMTypeRef *params = ARENA_MALLOC(len * sizeof(LLVMTypeRef), "llvm_closure_params", NULL, self->arena);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_type_t *param = typevec_offset(closure.params, i);
        params[i] = get_llvm_type(self, param);
    }

    LLVMTypeRef ret = get_llvm_type(self, closure.result);

    return LLVMFunctionType(ret, params, len, closure.variadic);
}

static LLVMTypeRef get_llvm_enum_type(llvm_target_t *self, ssa_type_enum_t it)
{
    return get_llvm_type(self, it.underlying);
}

static LLVMTypeRef get_llvm_struct_type(llvm_target_t *self, ssa_type_record_t it)
{
    size_t len = typevec_len(it.fields);
    LLVMTypeRef *fields = ARENA_MALLOC(len * sizeof(LLVMTypeRef), "llvm_struct_fields", NULL, self->arena);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_field_t *field = typevec_offset(it.fields, i);
        fields[i] = get_llvm_type(self, field->type);
    }

    // TODO: support packed structs
    return LLVMStructTypeInContext(gContext, fields, len, false);
}

static LLVMTypeRef get_llvm_union_type(llvm_target_t *self, ssa_type_record_t it)
{
    // get the largest field size
    size_t len = typevec_len(it.fields);
    size_t largest = 0;
    for (size_t i = 0; i < len; i++)
    {
        const ssa_field_t *field = typevec_offset(it.fields, i);
        LLVMTypeRef type = get_llvm_type(self, field->type);
        size_t size = LLVMStoreSizeOfType(gDataLayout, type);
        if (size > largest)
            largest = size;
    }

    return LLVMArrayType2(LLVMInt8TypeInContext(gContext), largest);
}

static LLVMTypeRef get_llvm_type_inner(llvm_target_t *self, const ssa_type_t *type)
{
    switch (type->kind)
    {
    case eTypeUnit:
    case eTypeEmpty: // TODO: empty is not the same as void/unit
        return LLVMVoidTypeInContext(gContext);
    case eTypeBool:
        return LLVMInt1TypeInContext(gContext);
    case eTypeDigit:
        return get_llvm_digit_type(gContext, type->digit);
    case eTypeClosure:
        return get_llvm_closure_type(self, type->closure);
    case eTypePointer:
        return LLVMPointerType(get_llvm_pointer_type(self, type->pointer), 0);
    case eTypeEnum:
        return get_llvm_enum_type(self, type->sum);
    case eTypeOpaque:
        return LLVMPointerTypeInContext(gContext, 0);
    case eTypeStruct:
        return get_llvm_struct_type(self, type->record);
    case eTypeUnion:
        return get_llvm_union_type(self, type->record);

    default:
        CT_NEVER("Invalid type kind %s", ssa_type_name(type->kind));
    }
}

static LLVMTypeRef get_llvm_type(llvm_target_t *self, const ssa_type_t *type)
{
    LLVMTypeRef ref = map_get(self->types, type);
    if (ref != NULL)
        return ref;

    ref = get_llvm_type_inner(self, type);
    map_set(self->types, type, ref);

    return ref;
}

static LLVMValueRef get_digit_value(LLVMTypeRef type, const mpz_t value, ssa_type_digit_t digit)
{
    return LLVMConstIntOfString(type, mpz_get_str(NULL, 10, value), digit.sign == eSignSigned);
}

static LLVMValueRef get_llvm_value(LLVMTypeRef type, const ssa_value_t *value)
{
    if (!value->init)
        return LLVMGetUndef(type);

    switch (value->type->kind)
    {
    case eTypeDigit:
        return get_digit_value(type, value->digit_value, value->type->digit);
    case eTypeBool:
        return LLVMConstInt(type, value->bool_value, false);
    case eTypeOpaque: case eTypePointer: // TODO: need to reconstruct strings from pointer values...
        return LLVMConstPointerNull(type);

    default:
        CT_NEVER("Invalid value type %s", ssa_type_name(value->type->kind));
    }
}

static void llvm_build_module(llvm_target_t *self, const ssa_module_t *mod)
{
    LLVMModuleRef modref = LLVMModuleCreateWithNameInContext(mod->name, gContext);
    LLVMSetModuleDataLayout(modref, gDataLayout);
    LLVMSetTarget(modref, gTargetTriple);

    size_t vars = vector_len(mod->globals);
    for (size_t i = 0; i < vars; i++)
    {
        const ssa_symbol_t *global = vector_get(mod->globals, i);
        ssa_storage_t storage = global->storage;
        LLVMTypeRef type = get_llvm_type(self, global->type);

        if (storage.size > 1)
            type = LLVMArrayType2(type, storage.size);

        LLVMValueRef ref = LLVMAddGlobal(modref, type, global->name);
        LLVMSetGlobalConstant(ref, storage.quals & eQualConst);

        if (global->value != NULL)
        {
            LLVMValueRef init = get_llvm_value(type, global->value);
            LLVMSetInitializer(ref, init);
        }
    }

    char *message;
    if (LLVMVerifyModule(modref, LLVMReturnStatusAction, &message))
        CT_NEVER("Failed to verify module %s: %s", mod->name, message);

    LLVMDisposeMessage(message);
}

emit_result_t llvm_ssa(target_runtime_t *runtime, const ssa_result_t *ssa, target_emit_t *emit)
{
    vector_t *mods = ssa->modules;

    CT_UNUSED(runtime);
    CT_UNUSED(mods);
    CT_UNUSED(emit);

    llvm_target_t info = {
        .arena = runtime->arena,
        .types = map_new(64, kTypeInfoPtr, runtime->arena)
    };

    size_t len = vector_len(mods);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_module_t *mod = vector_get(mods, i);
        llvm_build_module(&info, mod);
    }

    emit_result_t result = {
        .files = vector_new(0, runtime->arena)
    };

    return result;
}
