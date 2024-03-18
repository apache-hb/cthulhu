// SPDX-License-Identifier: GPL-3.0-only
#include "stdafx.hpp"

#include "editor/panels/arena.hpp"

using namespace ed;

TraceArenaPanel::TraceArenaPanel(ed::TraceArena& arena, panel_info_t setup)
    : IEditorPanel(arena.get_name(), setup)
    , arena(arena)
{ }

void TraceArenaPanel::draw_content()
{
    arena.draw_info();
}
