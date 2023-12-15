#include "editor/panic.hpp"

#include "stacktrace/stacktrace.h"

#include "std/str.h"

#include "imgui/imgui.h"

using namespace ed;

void RuntimePanic::init(size_t frame_count)
{
    bt_count = frame_count;
    bt_data = std::make_unique<frame_t[]>(frame_count);
}

void RuntimePanic::capture_trace(panic_t info, const char *fmt, va_list args)
{
    size_t bt_capture = stacktrace_get(bt_data.get(), bt_count);
    symbol_t symbol = {};
    for (size_t i = 0; i < bt_capture; i++)
    {
        frame_resolve(&bt_data[i], &symbol);

        StackFrame frame = {
            .address = bt_data[i].address,
            .line = symbol.line,
            .symbol = symbol.name,
            .file = symbol.file,
        };

        frames.push_back(frame);
    }

    panic = info;
    message = vformat(fmt, args);
}

void RuntimePanic::draw_error()
{
    if (ImGui::BeginTable("Backtrace", 4))
    {
        ImGui::EndTable();
    }
}
