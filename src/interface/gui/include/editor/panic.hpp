#pragma once

#include "editor/compile.hpp"

#include "base/panic.h"

#include "backtrace/backtrace.h"

#include <string>
#include <vector>

typedef struct frame_t frame_t;

namespace ed
{
    void install_panic_handler();

    struct stack_frame_t
    {
        bt_address_t address;
        size_t line;
        std::string symbol;
        std::string file;
    };

    class PanicInfo
    {
    public:
        bool has_error = false;

        panic_t info = {};
        std::string message;
        std::vector<stack_frame_t> frames;

        bool has_info() const { return has_error; }
        void reset();

        void capture_trace(panic_t panic, const char *fmt, va_list args);
        void draw();
    };

    enum compile_code_t
    {
        eCompileOk,
        eCompileError,
        eCompilePanic,

        eCompileTotal
    };

    class CompileError
    {
    public:
        bool has_error() const { return code != eCompileOk; }

        compile_code_t code = eCompileOk;

        ed::PanicInfo panic;
        std::string error;
    };

    CompileError run_compile(CompileInfo& info);
}
