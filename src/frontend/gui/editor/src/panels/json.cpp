// SPDX-License-Identifier: GPL-3.0-or-later
#include "editor/panels/arena.hpp"
#include "stdafx.hpp"

#include "editor/panels/json.hpp"
#include "editor/utils/io.hpp"

#include "json/json.hpp"

namespace fs = std::filesystem;
namespace json = ctu::json;

struct JsonInfo
{
    fs::path path;
    ctu::Io io;

    ctu::json::Json value;

    JsonInfo(json::JsonParser& parser, const fs::path& path)
        : path(path)
        , io(ctu::Io::file(path.string().c_str()))
        , value(parser.parse(io.get()))
    { }
};

class JsonPanel : public ed::IEditorPanel
{
public:
    TraceArena arena{"json", TraceArena::eCollectStackTrace};
    json::JsonParser parser{arena.get_arena()};

    ImGui::FileBrowser dialog { ImGuiFileBrowserFlags_ConfirmOnEnter | ImGuiFileBrowserFlags_MultipleSelection | ImGuiFileBrowserFlags_CloseOnEsc };
    std::vector<JsonInfo> documents;

    void draw_content() override
    {
        ImGui::Text("JSON Panel");
    }

    JsonPanel()
        : ed::IEditorPanel("JSON")
    { }

    void update() override
    {
        dialog.Display();

        if (dialog.HasSelected())
        {
            for (const auto& path : dialog.GetMultiSelected())
            {
                documents.push_back({ parser, path });
            }

            dialog.ClearSelected();
        }

        for (auto& doc : documents)
        {
            if (ImGui::Begin(doc.io.name()))
            {
                ImGui::Text("JSON Panel");
            }
            ImGui::End();
        }
    }
};

ed::IEditorPanel *ed::create_json_panel()
{
    return new JsonPanel();
}
