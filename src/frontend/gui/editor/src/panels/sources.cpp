// SPDX-License-Identifier: GPL-3.0-only
#include "editor/utils.hpp"
#include "stdafx.hpp"

#include "editor/panels/sources.hpp"

#include "memory/memory.h"
#include "base/util.h"
#include "io/io.h"
#include "os/os.h"
#include "std/str.h"

using namespace ed;

bool OsError::success() const
{
    return error == eOsSuccess;
}

bool OsError::failed() const
{
    return error != eOsSuccess;
}

const char *OsError::what() const
{
    if (!string)
    {
        string = os_error_string(error, get_global_arena());
    }

    return string;
}

Io Io::file(const char *path)
{
    return io_file(path, eOsAccessRead, get_global_arena());
}

OsError Io::error() const
{
    return io_error(io.get());
}

size_t Io::size() const
{
    return io_size(io.get());
}

const void *Io::map() const
{
    return io_map(io.get(), eOsProtectRead);
}

std::string_view Io::text() const
{
    const char *data = static_cast<const char*>(map());
    return std::string_view{data, size()};
}

const char *Io::name() const
{
    return io_name(io.get());
}

SourceView::SourceView(const fs::path& ospath, panel_info_t setup)
    : IEditorPanel(ospath.filename().string(), setup)
    , directory(ospath.parent_path().string())
    , basename(ospath.filename().string())
    , io(Io::file(ospath.string().c_str()))
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

void SourceView::draw_content()
{
    if (error.failed())
    {
        ImGui::Text("Failed to open file: %s", error.what());
        return;
    }

    if (ImGui::BeginChild(get_path(), ImVec2(0, 0), ImGuiWindowFlags_HorizontalScrollbar))
    {
        ImGuiListClipper clipper;
        clipper.Begin((int)line_offsets.size());

        while (clipper.Step())
        {
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
            {
                ImGui::TextUnformatted(source.data() + line_offsets[i], source.data() + line_offsets[i + 1]);
            }
        }
    }
}

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
        sources.emplace_back(buffer);
        buffer[0] = '\0';
    }

    size_t idx = SIZE_MAX;

    for (size_t i = 0; i < sources.size(); i++)
    {
        Source& src = sources[i];
        if (ImGui::CollapsingHeader(src.get_title(), ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::Button(ed::strfmt<1024>("Remove##%s", src.get_path())))
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
        sources.erase(sources.begin() + (ptrdiff_t)idx);
    }
}
