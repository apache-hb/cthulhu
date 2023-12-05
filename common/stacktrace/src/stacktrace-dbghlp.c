#include "stacktrace/stacktrace.h"

#include <windows.h>
#include <dbghelp.h>

void stacktrace_init(void)
{
    SymInitialize(GetCurrentProcess(), NULL, TRUE);
    SymSetOptions(SYMOPT_LOAD_LINES);
}

USE_DECL
const char *stacktrace_backend(void)
{
    return "dbghelp";
}

static BOOL get_frame(STACKFRAME *frame, CONTEXT *ctx, HANDLE process, HANDLE thread)
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

static void demangle_symbol(HANDLE process, CHAR *name, DWORD size, DWORD64 offset, SYMBOL_INFO *symbol, DWORD disp)
{
    DWORD64 disp2 = disp;
    if (SymFromAddr(process, offset, &disp2, symbol))
    {
        DWORD len = UnDecorateSymbolName(symbol->Name, name, size, UNDNAME_COMPLETE);
        if (len > 0)
        {
            strcpy_s(name, STACKTRACE_NAME_LENGTH, name);
        }
        else
        {
            strcpy_s(name, STACKTRACE_NAME_LENGTH, symbol->Name);
        }
    }
    else
    {
        sprintf_s(name, STACKTRACE_NAME_LENGTH, "0x%llX", offset);
    }
}

union disp_t {
    DWORD disp;
    DWORD64 disp64;
};

USE_DECL
size_t stacktrace_get(frame_t *frames, size_t size)
{
    if (frames == NULL) return 0;
    if (size == 0) return 0;

    char buffer[sizeof(SYMBOL_INFO) + (STACKTRACE_NAME_LENGTH - 1) * sizeof(TCHAR)] = { 0 };
    HANDLE thread = GetCurrentThread();
    HANDLE process = GetCurrentProcess();

    // allocate a symbol from the stack rather than the heap
    PSYMBOL_INFO symbol = (PSYMBOL_INFO)buffer;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    symbol->MaxNameLen = STACKTRACE_NAME_LENGTH;

    IMAGEHLP_LINE64 line = { 0 };

    CONTEXT ctx = { 0 };
    RtlCaptureContext(&ctx);

    union disp_t disp = { 0 };

    STACKFRAME frame = {
        .AddrPC = {
            .Offset = ctx.Rip,
            .Mode = AddrModeFlat
        },
        .AddrFrame = {
            .Offset = ctx.Rbp,
            .Mode = AddrModeFlat
        },
        .AddrStack = {
            .Offset = ctx.Rsp,
            .Mode = AddrModeFlat
        }
    };

    size_t used = 0;
    while (get_frame(&frame, &ctx, process, thread) && used < size)
    {
        frame_t result_frame = {
            .line = 0,
            .name = { 0 },
            .path = { 0 }
        };

        BOOL has_line = SymGetLineFromAddr64(process, frame.AddrPC.Offset, &disp.disp, &line);
        if (has_line)
        {
            result_frame.line = line.LineNumber;
            strcpy_s(result_frame.path, STACKTRACE_PATH_LENGTH, line.FileName);
        }

        demangle_symbol(process, result_frame.name, STACKTRACE_NAME_LENGTH, frame.AddrPC.Offset, symbol, disp.disp);

        frames[used] = result_frame;

        used += 1;
    }

    return used;
}
