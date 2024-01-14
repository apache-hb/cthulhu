#include "editor/sources.hpp"

#include "imgui/imgui.h"

#include "io/io.h"
#include "std/str.h"

using namespace ed;

static constexpr os_access_t kAccess = os_access_t(eAccessRead | eAccessText);

Source::Source(const char *str, arena_t *arena)
    : path(str)
    , io(io_file(path.c_str(), kAccess, arena))
{
    basename = str_filename(path.c_str(), arena);

    os_error_t err = io_error(io);
    if (err == 0)
    {
        source.size = io_size(io);
        source.text = (const char*)io_map(io);
    }
    else
    {
        error_string = os_error_string(err, arena);
    }
}

static constexpr ImGuiChildFlags kSourceFlags = ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY;

void Source::draw()
{
    if (io_error(io) != 0)
    {
        ImGui::TextUnformatted(error_string);
        return;
    }

    if (ImGui::BeginChild(get_path(), ImVec2(0, 0), kSourceFlags))
    {
        ImGui::TextUnformatted(source.text, source.text + source.size);
    }

    ImGui::EndChild();
}

void SourceList::draw()
{
    bool open = ImGui::InputText("SourceList", buffer, std::size(buffer), ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::SameLine();
    bool add = ImGui::Button("Add");

    if (open || add)
    {
        paths.emplace_back(buffer, arena);
        buffer[0] = '\0';
    }

    size_t idx = SIZE_MAX;

    char label[1024] = {};

    for (size_t i = 0; i < paths.size(); i++)
    {
        Source *src = &paths[i];
        if (ImGui::CollapsingHeader(src->get_title(), ImGuiTreeNodeFlags_DefaultOpen))
        {
            (void)snprintf(label, std::size(label), "Remove##%s", src->get_path());
            if (ImGui::Button(label))
            {
                idx = i;
            }

            ImGui::SameLine();
            ImGui::Text("%s | size: %zu", src->get_path(), src->get_size());

            src->draw();
        }
    }

    if (idx != SIZE_MAX)
    {
        paths.erase(paths.begin() + idx);
    }
}
