#pragma once

#include "editor/compile.hpp"

#include "base/panic.h"

#include <memory>
#include <string>
#include <vector>

typedef struct frame_t frame_t;

namespace ed
{
    void install_panic_handler();

    struct StackFrame
    {
        uintptr_t address;
        size_t line;
        std::string symbol;
        std::string file;
    };

    struct PanicInfo
    {
        bool has_error = false;

        panic_t info = {};
        std::string message;
        std::vector<StackFrame> frames;

        bool has_info() const { return has_error; }
        void reset();

        void capture_trace(panic_t panic, const char *fmt, va_list args);
        void draw();
    };

    enum CompileCode
    {
        eCompileOk,
        eCompileError,
        eCompilePanic,

        eCompileTotal
    };

    struct CompileError
    {
        bool has_error() const { return code != eCompileOk; }

        CompileCode code = eCompileOk;

        ed::PanicInfo panic;
        std::string error;
    };

    CompileError run_compile(CompileInfo& info);
}
