#include "tools.h"

#include <cthulhu.h>

#include <string>
#include <fstream>

std::string name = "input";
std::string text = "";

auto interning = std::make_shared<cthulhu::pool>();

ImGuiTableFlags table_flags 
    = ImGuiTableFlags_BordersInnerH 
    | ImGuiTableFlags_BordersInnerV 
    | ImGuiTableFlags_None 
    | ImGuiTableFlags_SizingFixedSame;

int main(int argc, const char** argv) {
    if (argc > 1) {
        std::ifstream in(argv[1]);
        text = std::string(std::istreambuf_iterator<char>{in}, {});
        name = argv[1];
    }

    GUI("tokens", {
        cthulhu::text_stream stream(text);
        cthulhu::lexer lexer(&stream, interning);

        if (ImGui::Begin("source")) {
            ImGui::Text("%s", text.c_str());
        }
        ImGui::End();

        if (ImGui::Begin("tokens")) {
            if (ImGui::BeginTable("tokens", 2, table_flags)) {
                while (true) {
                    auto tok = lexer.read();
                    auto src = lexer.slice(tok.range);

                    ImGui::TableNextColumn();
                    ImGui::Text("%s", tok.repr());
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", src.c_str());
                    ImGui::TableNextRow();

                    if (!tok.valid()) {
                        break;
                    }
                }

                ImGui::EndTable();
            }
        }
        ImGui::End();

        ImGui::ShowDemoWindow();
    })
}
