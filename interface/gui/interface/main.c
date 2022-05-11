#include "imgui-wrapper.h"

#include <stdio.h>

#include "cthulhu/report/report.h"

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

    void *ctx = imgui_init(window);

    while (!glfwWindowShouldClose(window) && !imgui_should_quit(ctx))
    {
        glfwPollEvents();

        imgui_new_frame(ctx);

        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        imgui_render(ctx);

        glfwSwapBuffers(window);
    }

    imgui_cleanup(ctx);

    glfwTerminate();

    return EXIT_OK;
}
