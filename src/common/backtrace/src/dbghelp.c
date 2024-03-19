// SPDX-License-Identifier: LGPL-3.0-only

#include "backtrace/backtrace.h"

#include "core/macros.h"
#include "core/win32.h" // IWYU pragma: keep

#if CTU_WIN32_TRICKERY
#   include <windef.h>
#   include <winbase.h>
#   include <verrsrc.h>
#endif

#include <dbghelp.h>

#define MAX_NAME_SIZE 512

union disp_t {
    DWORD disp;
    DWORD64 disp64;
};

USE_DECL
const char *bt_backend(void)
{
    return "dbghelp";
}

static BOOL walk_stack(STACKFRAME *frame, CONTEXT *ctx, HANDLE process, HANDLE thread)
{
    return StackWalk(
        /* MachineType = */ IMAGE_FILE_MACHINE_AMD64,
        /* hProcess = */ process,
        /* kThread = */ thread,
        /* StackFrame = */ frame,
        /* Context = */ ctx,
        /* ReadMemoryRoutine = */ NULL,
        /* FunctionTableAccessRoutine = */ SymFunctionTableAccess64,
        /* GetModuleBaseRoutine = */ SymGetModuleBase64,
        /* TranslateAddress = */ NULL
    );
}

// split this out as a function so we can use it in the exception handler
// exception handlers provide a CONTEXT we can use to walk the stack
static void read_context_stack(CONTEXT *ctx, bt_trace_t callback, void *user)
{
    HANDLE thread = GetCurrentThread();
    HANDLE process = GetCurrentProcess();

    STACKFRAME stackframe = {
        .AddrPC = {
            .Offset = ctx->Rip,
            .Mode = AddrModeFlat
        },
        .AddrFrame = {
            .Offset = ctx->Rbp,
            .Mode = AddrModeFlat
        },
        .AddrStack = {
            .Offset = ctx->Rsp,
            .Mode = AddrModeFlat
        }
    };

    while (walk_stack(&stackframe, ctx, process, thread))
    {
        bt_address_t frame = stackframe.AddrPC.Offset;

        callback(frame, user);
    }
}

void bt_read_inner(bt_trace_t callback, void *user)
{
    CONTEXT ctx = { 0 };
    RtlCaptureContext(&ctx);

    read_context_stack(&ctx, callback, user);
}

frame_resolve_t bt_resolve_inner(bt_address_t frame, bt_symbol_t *symbol)
{
    union disp_t disp = { 0 };
    IMAGEHLP_LINE64 line = { 0 };
    HANDLE process = GetCurrentProcess();
    text_t name = symbol->name;
    text_t path = symbol->path;

    symbol->line = 0;
    strcpy_s(name.text, name.length, "<unknown>");

    // cap the name size to a known maximum so we can put it on the stack rather than call malloc
    // we do this because this may be called in a signal handler and we don't want to allocate
    ULONG name_size = CT_MIN((ULONG)name.length, MAX_NAME_SIZE);
    char buffer[sizeof(SYMBOL_INFO) + (MAX_NAME_SIZE - 1) * sizeof(TCHAR)];
    memset(buffer, 0, sizeof(SYMBOL_INFO)); // only zero the symbol info struct

    PSYMBOL_INFO info = (PSYMBOL_INFO)buffer;
    info->SizeOfStruct = sizeof(SYMBOL_INFO);
    info->MaxNameLen = name_size;

    frame_resolve_t resolve = eResolveNothing;

    if (SymFromAddr(process, frame, &disp.disp64, info))
    {
        resolve |= eResolveName;

        if (SymGetLineFromAddr64(process, frame, &disp.disp, &line))
        {
            // subtract 1 from the line number because dbghelp is 1-indexed
            symbol->line = line.LineNumber - 1;
            strcpy_s(path.text, path.length, line.FileName);

            resolve |= eResolveLine | eResolveFile;
        }

        // even though we're a C codebase still run the demangler to catch C++ code
        // that people consuming this library might be using
        if (UnDecorateSymbolName(info->Name, name.text, name_size, UNDNAME_COMPLETE))
        {
            resolve |= eResolveDemangledName;
        }
        else
        {
            // copy the mangled name if we can't demangle it
            strcpy_s(name.text, name_size, info->Name);
        }
    }

    return resolve;
}

static LONG WINAPI bt_exception_handler(EXCEPTION_POINTERS *exception)
{
    gSystemError.begin(exception->ExceptionRecord->ExceptionCode, gSystemError.user);

    read_context_stack(exception->ContextRecord, gSystemError.next, gSystemError.user);

    gSystemError.end(gSystemError.user);

    return EXCEPTION_EXECUTE_HANDLER;
}

void bt_init(void)
{
    SymInitialize(GetCurrentProcess(), NULL, TRUE);
    SymSetOptions(SYMOPT_LOAD_LINES);

    SetUnhandledExceptionFilter(bt_exception_handler);
}

void bt_update(void)
{
    SymRefreshModuleList(GetCurrentProcess());
}
