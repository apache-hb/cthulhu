#include "stacktrace/stacktrace.h"

#include <windows.h>
#include <dbghelp.h>

void stacktrace_init(void)
{
    SymInitialize(GetCurrentProcess(), NULL, TRUE);
}

const char *stacktrace_backend(void)
{
    return "win32-dbghelp";
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

size_t stacktrace_get(frame_t *frames, size_t size)
{
    HANDLE thread = GetCurrentThread();
    HANDLE process = GetCurrentProcess();

    // TODO: allocating here is probably not a good idea
    IMAGEHLP_SYMBOL *symbol = malloc(sizeof(IMAGEHLP_SYMBOL) + STACKTRACE_NAME_LENGTH);
    symbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
    symbol->Address = 0;
    symbol->Size = 0;
    symbol->Flags = SYMF_FUNCTION;
    symbol->MaxNameLength = STACKTRACE_NAME_LENGTH;

    DWORD64 disp = 0;

    CONTEXT ctx = { 0 };
    RtlCaptureContext(&ctx);

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

    char name[STACKTRACE_NAME_LENGTH] = { 0 };

    size_t used = 0;
    while (get_frame(&frame, &ctx, process, thread) && used < size)
    {
        memset(name, 0, STACKTRACE_NAME_LENGTH);

        SymGetSymFromAddr(process, frame.AddrPC.Offset, &disp, symbol);
        UnDecorateSymbolName(symbol->Name, name, STACKTRACE_NAME_LENGTH, UNDNAME_COMPLETE);

        memcpy(frames[used].name, name, STACKTRACE_NAME_LENGTH);

        used += 1;
    }

    return used;
}
