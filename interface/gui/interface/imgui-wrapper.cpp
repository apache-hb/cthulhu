#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "widgets/file-dialog.h"
#include "widgets/text-editor.h"

#include "imgui-wrapper.h"

#include <array>

struct WrapperContext
{
    ImGui::FileBrowser fileDialog{ImGuiFileBrowserFlags_MultipleSelection};
    TextEditor editor;

    bool showDemoWindow = true;
    bool shouldQuit = false;
};

void *imgui_init(GLFWwindow *window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    auto *wrapper = new WrapperContext();

    wrapper->fileDialog.SetTitle("Open File");

    wrapper->editor.SetText("Hello, World!");
    wrapper->editor.SetColorizerEnable(true);
    wrapper->editor.SetPalette(TextEditor::GetDarkPalette());

    return wrapper;
}

void imgui_cleanup(void *ctx)
{
    (void)ctx;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void imgui_new_frame(void *ctx)
{
    auto *wrapper = reinterpret_cast<WrapperContext*>(ctx);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (wrapper->showDemoWindow)
    {
        ImGui::ShowDemoWindow(&wrapper->showDemoWindow);
    }

    static ImGuiDockNodeFlags dockFlags = ImGuiDockNodeFlags_None;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    windowFlags |= ImGuiWindowFlags_NoTitleBar 
                | ImGuiWindowFlags_NoCollapse 
                | ImGuiWindowFlags_NoResize 
                | ImGuiWindowFlags_NoMove
                | ImGuiWindowFlags_NoBringToFrontOnFocus 
                | ImGuiWindowFlags_NoNavFocus;

    if (dockFlags & ImGuiDockNodeFlags_PassthruCentralNode)
    {
        windowFlags |= ImGuiWindowFlags_NoBackground;
    }

    ImGui::Begin("DockSpace Demo", &wrapper->shouldQuit, windowFlags);

    ImGui::PopStyleVar(2);

    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID dockId = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockId, ImVec2(0.0f, 0.0f), dockFlags);
    }

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New"))
            {

            }

            if (ImGui::MenuItem("Open"))
            {
                wrapper->fileDialog.Open();
            }

            if (ImGui::MenuItem("Save"))
            {

            }

            if (ImGui::MenuItem("Quit", NULL, false, true))
            {
                wrapper->shouldQuit = true;
            }

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    ImGui::End();

    wrapper->fileDialog.Display();
    wrapper->editor.Render("Text Editor");
}

void imgui_render(void *ctx)
{
    (void)ctx;

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool imgui_should_quit(void *ctx)
{
    auto *wrapper = reinterpret_cast<WrapperContext*>(ctx);
    return wrapper->shouldQuit;
}
