// SPDX-License-Identifier: GPL-3.0-only

#include "editor/panels/sources.hpp"

#include "imgui/imgui.h"
#include "imgui_stdlib.h"

#include "memory/memory.h"
#include "base/util.h"
#include "io/io.h"
#include "std/str.h"

using namespace ed;

Source::Source(std::string_view str, panel_info_t setup)
    : IEditorPanel(str, setup)
    , path(str)
    , io(io_file(path.c_str(), eOsAccessRead, get_global_arena()))
{
    basename = str_filename(path.c_str(), get_global_arena());

    if (os_error_t err = io_error(io))
    {
        error_string = os_error_string(err, get_global_arena());
    }
    else
    {
        const void *data = io_map(io, eOsProtectRead);
        size_t size = io_size(io);

        if (data == nullptr)
        {
            error_string = os_error_string(io_error(io), get_global_arena());
        }
        else
        {
            source = text_view_make((const char *)data, size);
        }
    }
}

static constexpr ImGuiChildFlags kSourceFlags = ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY;

void Source::draw_content()
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

SourceList::SourceList(panel_info_t setup)
    : IEditorPanel("Source List", setup)
{ }

void SourceList::draw_content()
{
    bool open = ImGui::InputText("SourceList", &buffer, ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::SameLine();
    bool add = ImGui::Button("Add");

    if (open || add)
    {
        paths.emplace_back(buffer);
        buffer[0] = '\0';
    }

    size_t idx = SIZE_MAX;

    char label[1024] = {};

    for (size_t i = 0; i < paths.size(); i++)
    {
        Source& src = paths[i];
        if (ImGui::CollapsingHeader(src.get_title(), ImGuiTreeNodeFlags_DefaultOpen))
        {
            (void)snprintf(label, std::size(label), "Remove##%s", src.get_path());
            if (ImGui::Button(label))
            {
                idx = i;
            }

            ImGui::SameLine();
            ImGui::Text("%s | size: %zu", src.get_path(), src.get_size());

            //src.draw_section();
        }
    }

    if (idx != SIZE_MAX)
    {
        paths.erase(paths.begin() + (ptrdiff_t)idx);
    }
}
