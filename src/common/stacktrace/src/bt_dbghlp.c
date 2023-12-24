#include "stacktrace/stacktrace.h"

#include "core/win32.h" // IWYU pragma: keep

#include <dbghelp.h>

void bt_init(void)
{
    SymInitialize(GetCurrentProcess(), NULL, TRUE);
    SymSetOptions(SYMOPT_LOAD_LINES);
}

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

union disp_t {
    DWORD disp;
    DWORD64 disp64;
};

void bt_read_inner(bt_frame_t callback, void *user)
{
    HANDLE thread = GetCurrentThread();
    HANDLE process = GetCurrentProcess();

    CONTEXT ctx = { 0 };
    RtlCaptureContext(&ctx);

    STACKFRAME stackframe = {
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

    while (walk_stack(&stackframe, &ctx, process, thread))
    {
        frame_t frame = {
            .address = stackframe.AddrPC.Offset,
        };

        callback(user, &frame);
    }
}

frame_resolve_t bt_resolve_inner(const frame_t *frame, symbol_t *symbol)
{
    union disp_t disp = { 0 };
    IMAGEHLP_LINE64 line = { 0 };
    HANDLE process = GetCurrentProcess();

    symbol->line = 0;
    strcpy_s(symbol->name, STACKTRACE_NAME_LENGTH, "<unknown>");

    char buffer[sizeof(SYMBOL_INFO) + (STACKTRACE_NAME_LENGTH - 1) * sizeof(TCHAR)] = { 0 };
    PSYMBOL_INFO info = (PSYMBOL_INFO)buffer;
    info->SizeOfStruct = sizeof(SYMBOL_INFO);
    info->MaxNameLen = STACKTRACE_NAME_LENGTH;

    frame_resolve_t resolve = eResolveNothing;

    if (SymFromAddr(process, frame->address, &disp.disp64, info))
    {
        resolve |= eResolveName;

        if (SymGetLineFromAddr64(process, frame->address, &disp.disp, &line))
        {
            // subtract 1 from the line number because dbghelp is 1-indexed
            symbol->line = line.LineNumber - 1;
            strcpy_s(symbol->file, STACKTRACE_PATH_LENGTH, line.FileName);

            resolve |= eResolveLine | eResolveFile;
        }

        if (UnDecorateSymbolName(info->Name, symbol->name, STACKTRACE_NAME_LENGTH, UNDNAME_COMPLETE))
        {
            resolve |= eResolveDemangledName;
        }
        else
        {
            // copy the mangled name if we can't demangle it
            strcpy_s(symbol->name, STACKTRACE_NAME_LENGTH, info->Name);
        }
    }

    return resolve;
}
