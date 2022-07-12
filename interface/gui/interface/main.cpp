#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include <stdio.h>

#include "report/report.h"

static void framebuffer_size(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

static void error_callback(int error, const char *description)
{
    fprintf(stderr, "GLFW error: %s", description);
}

int main()
{
    if (!glfwInit())
    {
        return EXIT_INTERNAL;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

    const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    GLFWwindow *window = glfwCreateWindow(mode->width, mode->height, "Cthulhu", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        return EXIT_INTERNAL;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size);
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


    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::ShowDemoWindow(nullptr);

        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOUR_BUFFER_BIT);

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
