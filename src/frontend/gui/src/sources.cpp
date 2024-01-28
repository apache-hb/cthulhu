#include "editor/sources.hpp"

#include "base/util.h"
#include "imgui/imgui.h"

#include "io/io.h"
#include "std/str.h"

using namespace ed;

Source::Source(const char *str, arena_t *arena)
    : path(str)
    , io(io_file(path.c_str(), eAccessRead, arena))
{
    basename = str_filename(path.c_str(), arena);

    os_error_t err = io_error(io);
    if (err == 0)
    {
        const void *data = io_map(io, eProtectRead);
        size_t size = io_size(io);

        if (data == nullptr)
        {
            error_string = os_error_string(io_error(io), arena);
        }
        else
        {
            source = text_view_make((const char *)data, size);
        }
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
        ImGui::TextUnformatted(source.text, source.text + source.length);
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
