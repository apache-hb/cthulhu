#pragma once

namespace draw
{
    struct config_t
    {
        const wchar_t *title = L"draw";
        bool hwaccel = true;
    };

    bool create(const config_t& config);
    void destroy();
    void close();

    bool begin_frame();
    void end_frame();
}
