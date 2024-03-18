// SPDX-License-Identifier: GPL-3.0-only
#include "stdafx.hpp"

#include "editor/panic.hpp"

#include "backtrace/backtrace.h"

#include "cthulhu/broker/broker.h"
#include "memory/memory.h"
#include "std/str.h"

using namespace ed;

static std::jmp_buf gPanicEnv = {};
static ed::PanicInfo gPanicInfo = {};

void ed::install_panic_handler()
{
    gPanicHandler = [](source_info_t location, const char *fmt, va_list args) {
        gPanicInfo.capture_trace(location, fmt, args);
        std::longjmp(gPanicEnv, 1); // NOLINT
    };
}

CompileError ed::run_compile(Broker& info)
{
    if (std::setjmp(gPanicEnv)) // NOLINT
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

    if (!info.check_reports())
    {
        CompileError error = {
            .code = eCompileError,
        };
        return error;
    }

    for (size_t stage = 0; stage < ePassCount; stage++)
    {
        broker_run_pass(info.broker, broker_pass_t(stage));
        if (!info.check_reports())
        {
            CompileError error = {
                .code = eCompileError,
            };
            return error;
        }
    }

    broker_resolve(info.broker);
    if (!info.check_reports())
    {
        CompileError error = {
            .code = eCompileError,
        };
        return error;
    }

    CompileError error = {
        .code = eCompileOk,
    };
    return error;
}

struct trace_capture_t
{
    bt_symbol_t symbol = {};
    PanicInfo *info = nullptr;
};

static void trace_callback(const bt_frame_t *frame, void *user)
{
    auto& [symbol, info] = *reinterpret_cast<trace_capture_t*>(user);

    bt_resolve_symbol(frame, &symbol);

    text_t path = symbol.path;
    text_t name = symbol.name;

    stack_frame_t stack_frame = {
        .address = frame->address,
        .line = symbol.line,
        .symbol = std::string(name.text, name.length),
        .file = std::string(path.text, path.length),
    };

    info->frames.push_back(stack_frame);
}

void PanicInfo::capture_trace(source_info_t location, const char *fmt, va_list args)
{
    arena_t *arena = get_global_arena();
    trace_capture_t capture = {
        .info = this,
    };

    frames.clear();
    frames.reserve(64);

    bt_read(trace_callback, &capture);

    info = location;
    message = str_vformat(arena, fmt, args);
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

        for (const stack_frame_t& frame : frames)
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
