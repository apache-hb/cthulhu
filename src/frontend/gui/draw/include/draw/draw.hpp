#pragma once

namespace draw
{
    bool create(const wchar_t *title);
    void destroy();

    bool begin_frame();
    void end_frame();
}
