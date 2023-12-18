#include "editor/panic.hpp"

#include "stacktrace/stacktrace.h"

#include "std/str.h"

#include "imgui/imgui.h"

#include <csetjmp>

using namespace ed;

static std::jmp_buf gPanicEnv = {};
static ed::PanicInfo gPanicInfo = {};

void ed::install_panic_handler()
{
    gPanicHandler = [](panic_t panic, const char *fmt, va_list args) {
        gPanicInfo.capture_trace(panic, fmt, args);
        std::longjmp(gPanicEnv, 1);
    };
}

CompileError ed::run_compile(CompileInfo& info)
{
    if (std::setjmp(gPanicEnv))
    {
        CompileError error = {
            .code = eCompilePanic,
            .panic = gPanicInfo,
        };

        gPanicInfo.reset();
        return error;
    }

    info.init();

    if (!info.check_reports())
    {
        CompileError error = {
            .code = eCompileError,
        };
        return error;
    }

    for (size_t i = 0; i < info.sources.count(); i++)
    {
        char *message = info.parse_source(i);
        if (message)
        {
            CompileError error = {
                .code = eCompileError,
                .error = message,
            };
            return error;
        }
    }

    CompileError error = {
        .code = eCompileOk,
    };
    return error;
}

struct TraceCapture
{
    symbol_t symbol = {};
    PanicInfo *info = nullptr;
};

static void trace_callback(void *user, const frame_t *frame)
{
    auto& [symbol, info] = *reinterpret_cast<TraceCapture*>(user);

    bt_resolve_symbol(frame, &symbol);

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

    bt_read(trace_callback, &capture);

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

static const ImGuiTableFlags kTableFlags
    = ImGuiTableFlags_BordersV
    | ImGuiTableFlags_BordersOuterH
    | ImGuiTableFlags_Resizable
    | ImGuiTableFlags_RowBg
    | ImGuiTableFlags_NoHostExtendX
    | ImGuiTableFlags_NoBordersInBody
    | ImGuiTableFlags_SizingStretchProp;

void PanicInfo::draw()
{
    if (ImGui::BeginTable("Backtrace", 4, kTableFlags))
    {
        ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthFixed, 150.0f);
        ImGui::TableSetupColumn("Symbol");
        ImGui::TableSetupColumn("File");
        ImGui::TableSetupColumn("Line");

        ImGui::TableHeadersRow();

        for (const StackFrame& frame : frames)
        {
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::Text("0x%p", reinterpret_cast<void*>(frame.address));

            ImGui::TableNextColumn();
            ImGui::Text("%s", frame.symbol.c_str());

            ImGui::TableNextColumn();
            ImGui::Text("%s", frame.file.c_str());

            ImGui::TableNextColumn();
            ImGui::Text("%zu", frame.line);
        }

        ImGui::EndTable();
    }
}
