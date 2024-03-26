#pragma once

namespace draw
{
    struct config_t
    {
        const wchar_t *title = L"draw";
        bool hardware_acceleration:1 = true;
    };

    bool create(const config_t& config);
    void destroy();

    bool begin_frame();
    void end_frame();
}
