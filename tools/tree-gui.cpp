#include "tools.h"

#include <cthulhu.h>

#include <string>
#include <fstream>

std::string name = "input";
std::string text = "";

auto interning = std::make_shared<cthulhu::pool>();

int main(int argc, const char** argv) {
    if (argc > 1) {
        std::ifstream in(argv[1]);
        text = std::string(std::istreambuf_iterator<char>{in}, {});
        name = argv[1];
    }

    GUI("tree", {
        cthulhu::text_stream stream(text);
        cthulhu::lexer lexer(&stream, interning);
        cthulhu::parser parser(&lexer);

        ImGui::Begin("source");

        ImGui::Text("%s", text.c_str());
        
        ImGui::End();


        ImGui::Begin("tree");
        
        try {
            auto* it = parser.unit();
            if (ImGui::TreeNode("unit")) {
                for (auto* inc : it->includes) {
                    if (ImGui::TreeNode("include")) {

                        if (ImGui::TreeNode("path")) {
                            for (auto* part : inc->path) {
                                ImGui::Text("%s", part->id->c_str());
                            }
                            ImGui::TreePop();
                        }

                        if (ImGui::TreeNode("items")) {
                            for (auto* part : inc->path) {
                                ImGui::Text("%s", part->id->c_str());
                            }
                            ImGui::TreePop();
                        }

                        ImGui::TreePop();
                    }
                }

                ImGui::TreePop();
            }

        } catch (cthulhu::error* error) {
            ImGui::Text("failed to parse");
            delete error;
        }

        ImGui::End();

        ImGui::ShowDemoWindow();
    })
}
