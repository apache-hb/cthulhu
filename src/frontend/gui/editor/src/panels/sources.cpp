// SPDX-License-Identifier: GPL-3.0-only
#include "stdafx.hpp"

#include "editor/panels/sources.hpp"

using namespace ed;

SourceView::SourceView(const fs::path& ospath)
    : directory(ospath.parent_path().string())
    , basename(ospath.filename().string())
    , io(ctu::Io::file(ospath.string().c_str()))
    , error(io.error())
{
    if (error.success())
    {
        source = io.text();
        build_line_offsets();
    }
}

void SourceView::build_line_offsets()
{
    line_offsets.clear();
    line_offsets.push_back(0);

    for (size_t i = 0; i < source.size(); i++)
    {
        if (source[i] == '\n')
        {
            line_offsets.push_back(i + 1);
        }
    }
}

static constexpr ImGuiWindowFlags kSourceFlags
    = ImGuiWindowFlags_HorizontalScrollbar;
static constexpr ImGuiChildFlags kChildFlags
    = ImGuiChildFlags_AutoResizeY
    | ImGuiChildFlags_AutoResizeX;

void SourceView::draw_content()
{
    if (ImGui::BeginChild(get_path(), ImVec2(0, 0), kChildFlags, kSourceFlags))
    {
        if (error.failed())
        {
            ImGui::Text("Failed to open file: %s", error.what());
        }
        else
        {
            ImGui::TextUnformatted(source.data(), source.data() + source.size());
        }
        ImGui::EndChild();
    }
}
