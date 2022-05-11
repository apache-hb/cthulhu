#pragma once

#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

    void *imgui_init(GLFWwindow *window);
    void imgui_cleanup(void *ctx);

    void imgui_new_frame(void *ctx);
    void imgui_render(void *ctx);

    bool imgui_should_quit(void *ctx);

#ifdef __cplusplus
}
#endif
