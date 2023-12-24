#include "editor/sources.hpp"

#include "imgui/imgui.h"

using namespace ed;

void SourceList::draw()
{
    bool open = ImGui::InputText("SourceList", buffer, std::size(buffer), ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::SameLine();
    bool add = ImGui::Button("Add");

    if (open || add)
    {
        paths.push_back(buffer);
        buffer[0] = '\0';
    }

    size_t idx = SIZE_MAX;

    for (size_t i = 0; i < paths.size(); i++)
    {
        ImGui::BulletText("%s", paths[i].c_str());
        ImGui::SameLine();
        if (ImGui::Button("Remove"))
        {
            idx = i;
            break;
        }
    }

    if (idx != SIZE_MAX)
    {
        paths.erase(paths.begin() + idx);
    }
}
