#include "editor/panic.hpp"

#include "stacktrace/stacktrace.h"

#include "std/str.h"

#include "imgui/imgui.h"

using namespace ed;

struct TraceCapture
{
    symbol_t symbol = {};
    PanicInfo *info = nullptr;
};

static void trace_callback(void *user, const frame_t *frame)
{
    auto& [symbol, info] = *reinterpret_cast<TraceCapture*>(user);

    frame_resolve(frame, &symbol);

    StackFrame stack_frame = {
        .address = frame->address,
        .line = symbol.line,
        .symbol = symbol.name,
        .file = symbol.file,
    };

    info->frames.push_back(stack_frame);
}

void PanicInfo::capture_trace(panic_t panic, const char *fmt, va_list args)
{
    TraceCapture capture = {
        .info = this,
    };

    frames.clear();
    frames.reserve(64);

    stacktrace_read(trace_callback, &capture);

    info = panic;
    message = vformat(fmt, args);
    has_error = true;
}

void PanicInfo::reset()
{
    has_error = false;
    info = {};
    message.clear();
    frames.clear();
}

void PanicInfo::draw()
{
    if (ImGui::BeginTable("Backtrace", 4))
    {
        ImGui::EndTable();
    }
}
