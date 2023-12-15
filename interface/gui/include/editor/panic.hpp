#pragma once

#include "base/panic.h"

#include <memory>
#include <string>
#include <vector>

typedef struct frame_t frame_t;

namespace ed
{
    struct StackFrame
    {
        uintptr_t address;
        size_t line;
        std::string symbol;
        std::string file;
    };

    struct RuntimePanic
    {
        void init(size_t frame_count);

        void capture_trace(panic_t info, const char *fmt, va_list args);
        void draw_error();

    private:
        size_t bt_count;
        std::unique_ptr<frame_t[]> bt_data;

        panic_t panic;
        std::string message;
        std::vector<StackFrame> frames;
    };
}
