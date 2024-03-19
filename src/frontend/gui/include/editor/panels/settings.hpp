#pragma once

#include "editor/panels/panel.hpp"

#include "editor/units.hpp"

namespace ed
{
    class SettingsPanel final : IEditorPanel
    {
        // IEditorPanel
        void draw_content() override;

    public:
        SettingsPanel();
    };

    memory_format_t get_memory_format();
    void format_memory(uintmax_t value, char *buffer);
}
