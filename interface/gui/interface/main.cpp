#include "base/version-def.h"
#include "cthulhu/interface/interface.h"
#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "base/macros.h"

#include <stdio.h>

#include "imgui_internal.h"
#include "std/str.h"
#include "report/report.h"

int main()
{
    common_init();
    driver_t driver = get_driver();
    char *title = format("GUI Editor (%s | %lu.%lu.%lu)", driver.name, VERSION_MAJOR(driver.version), VERSION_MINOR(driver.version), VERSION_PATCH(driver.version));

    if (!glfwInit())
    {
        return EXIT_INTERNAL;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwSetErrorCallback([](auto error, auto desc) {
        fprintf(stderr, "GLFW error(%d): %s", error, desc);
    });

    auto *primary = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(primary);

    GLFWwindow *window = glfwCreateWindow(mode->width, mode->height, title, primary, NULL);

    if (!window)
    {
        glfwTerminate();
        return EXIT_INTERNAL;
    }

    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, [](auto, auto width, auto height) {
        glViewport(0, 0, width, height);
    });

    glfwSwapInterval(0);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        glfwTerminate();
        return EXIT_INTERNAL;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    bool shouldExit = false;

    constexpr auto kEditorSize = 0x4000;
    char *textInput = new char[kEditorSize]();

    while (!glfwWindowShouldClose(window) && !shouldExit)
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        constexpr auto windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        constexpr auto dockFlags = ImGuiDockNodeFlags_NoResize;

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        
        ImGui::Begin("DockSpace", nullptr, windowFlags);

        ImGui::PopStyleVar(3);

        auto id = ImGui::GetID("Dock");
        ImGui::DockSpace(id, ImVec2(0.f, 0.f), dockFlags);

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                ImGui::MenuItem("Open");
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Metrics"))
            {
                ImGui::MenuItem("Memory Stats");
                ImGui::MenuItem("Performance Stats");
                ImGui::MenuItem("HLIR Debug View");
                ImGui::MenuItem("SSA Debug View");
                ImGui::MenuItem("Logs");
                ImGui::EndMenu();
            }
            
            ImGui::EndMenuBar();
        }

        constexpr auto *helloId = "Hello World!";

        ImGui::Begin(helloId);
            ImGui::InputTextMultiline("##", textInput, kEditorSize); // TODO: make this scale properly
        ImGui::End();

        ImGui::End();

        ImGui::ShowDemoWindow(nullptr);

        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();

    return EXIT_OK;
}
