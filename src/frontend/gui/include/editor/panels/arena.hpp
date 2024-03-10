#pragma once

#include "editor/panels/panel.hpp"

#include "editor/trace.hpp"

namespace ed
{
    class TraceArenaPanel final : public IEditorPanel
    {
        ed::TraceArena& arena;

        // IEditorPanel
        void draw_content() override;

    public:
        TraceArenaPanel(ed::TraceArena& arena, panel_info_t setup = {});
    };
}
