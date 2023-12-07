#include "editor/draw.h"

#include "core/macros.h"

#include "imgui.h"

static const ImGuiDockNodeFlags kDockFlags = ImGuiDockNodeFlags_PassthruCentralNode;
static const ImGuiWindowFlags kDockWindowFlags = ImGuiWindowFlags_MenuBar
                                           | ImGuiWindowFlags_NoCollapse
                                           | ImGuiWindowFlags_NoMove
                                           | ImGuiWindowFlags_NoResize
                                           | ImGuiWindowFlags_NoTitleBar
                                           | ImGuiWindowFlags_NoBackground
                                           | ImGuiWindowFlags_NoBringToFrontOnFocus
                                           | ImGuiWindowFlags_NoNavFocus
                                           | ImGuiWindowFlags_NoDocking;

struct EditorUi
{
    bool show_demo_window = false;

    void dock_space()
    {
        const ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));

        ImGui::Begin("Editor", nullptr, kDockWindowFlags);

        ImGui::PopStyleVar(3);

        ImGuiID id = ImGui::GetID("EditorDock");
        ImGui::DockSpace(id, ImVec2(0.f, 0.f), kDockFlags);

        if (ImGui::BeginMenuBar())
        {
            ImGui::Text("Game2005");
            ImGui::Separator();

            if (ImGui::BeginMenu("Style"))
            {
                if (ImGui::MenuItem("Classic"))
                {
                    ImGui::StyleColorsClassic();
                }

                if (ImGui::MenuItem("Dark"))
                {
                    ImGui::StyleColorsDark();
                }

                if (ImGui::MenuItem("Light"))
                {
                    ImGui::StyleColorsLight();
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Windows"))
            {
                ImGui::SeparatorText("ImGui");
                ImGui::MenuItem("Dear ImGui Demo", nullptr, &show_demo_window);
                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        ImGui::End();
    }

    void demo_window()
    {
        if (show_demo_window)
        {
            ImGui::ShowDemoWindow(&show_demo_window);
        }
    }
};

int main(int argc, const char **argv)
{
    CTU_UNUSED(argc);
    CTU_UNUSED(argv);

    if (!draw::create())
    {
        return 1;
    }

    EditorUi ui;

    while (draw::begin_frame())
    {
        ui.dock_space();
        ui.demo_window();

        draw::end_frame();
    }

    draw::destroy();
}
