#pragma once

#include "editor/panels/panel.hpp"

namespace ed
{
    class SettingsPanel final : IEditorPanel
    {
        // IEditorPanel
        void draw_content() override;

    public:
        SettingsPanel();
    };
}
