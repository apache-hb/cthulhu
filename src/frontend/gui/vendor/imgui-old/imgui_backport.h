#pragma once

#include "imgui/imgui.h"

namespace ImGui
{
    bool BeginChildFrame(ImGuiID id, const ImVec2& size, ImGuiWindowFlags extra_flags = 0);
    void EndChildFrame();
}
