#include "base/version-def.h"
#include "cthulhu/interface/interface.h"
#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include "base/macros.h"
#include "base/memory.h"
#include "base/panic.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <stdio.h>
#include <unordered_map>
#include <unordered_set>

#include "imgui_internal.h"
#include "report/report.h"
#include "std/str.h"

// TODO: use this once the compiler is allocator aware
#if 0
struct Alloc {
    Alloc(const char *name) {
        alloc.name = name;
        alloc.data = this;
        alloc.arenaMalloc = [](auto *self, auto size, auto name) {
            return reinterpret_cast<Alloc*>(self)->doMalloc(size, name);
        };
        alloc.arenaRealloc = [](auto *self, auto ptr, auto newSize, auto oldSize) {
            return reinterpret_cast<Alloc*>(self)->doRealloc(ptr, newSize, oldSize);
        };
        alloc.arenaFree = [](auto *self, auto ptr, auto size) {
            reinterpret_cast<Alloc*>(self)->doFree(ptr, size);
        };
    }

    operator alloc_t() { return alloc; }

    virtual ~Alloc() = default;

protected:
    virtual void *doMalloc(size_t size, const char *name) = 0;
    virtual void *doRealloc(void *ptr, size_t newSize, size_t oldSize) = 0;
    virtual void doFree(void *ptr, size_t size) = 0;

private:
    alloc_t alloc;
};

struct StatsAlloc : Alloc {
    virtual void *doMalloc(size_t size, const char *name) override {
        UNUSED(name);

        adjustMemoryUsage(size);
        totalAllocs += 1;

        return malloc(size);
    }

    virtual void *doRealloc(void *ptr, size_t newSize, size_t oldSize) override {
        adjustMemoryUsage(newSize - oldSize);
        totalReallocs += 1;

        return realloc(ptr, newSize);
    }

    virtual void doFree(void *ptr, size_t size) override {
        adjustMemoryUsage(-size);
        totalFrees += 1;

        free(ptr);
    }

private:
    void adjustMemoryUsage(ssize_t size) {
        currentUsedMemory += size;
        peakUsedMemory = MAX(peakUsedMemory, currentUsedMemory);
    }

    size_t currentUsedMemory;
    size_t peakUsedMemory;

    size_t totalAllocs;
    size_t totalReallocs;
    size_t totalFrees;
};
#endif

int main()
{
    common_init();
    driver_t driver = get_driver();
    char *title = format("GUI Editor (%s | %lu.%lu.%lu)", driver.name, VERSION_MAJOR(driver.version),
                         VERSION_MINOR(driver.version), VERSION_PATCH(driver.version));

    if (!glfwInit())
    {
        return EXIT_INTERNAL;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwSetErrorCallback([](auto error, auto desc) { fprintf(stderr, "GLFW error(%d): %s", error, desc); });

    auto *primary = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(primary);

    GLFWwindow *window = glfwCreateWindow(mode->width, mode->height, title, primary, NULL);

    if (!window)
    {
        glfwTerminate();
        return EXIT_INTERNAL;
    }

    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, [](auto, auto width, auto height) { glViewport(0, 0, width, height); });

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

    bool openMemoryView = false;
    bool openPerfView = false;
    bool openHlirView = false;
    bool openSsaView = false;
    bool openLogView = false;

    while (!glfwWindowShouldClose(window) && !shouldExit)
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        constexpr auto windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking |
                                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                     ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        constexpr auto dockFlags = ImGuiDockNodeFlags_None;

        const ImGuiViewport *viewport = ImGui::GetMainViewport();
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
                ImGui::MenuItem("Memory Stats", nullptr, &openMemoryView);
                ImGui::MenuItem("Performance Stats", nullptr, &openPerfView);
                ImGui::MenuItem("HLIR Debug View", nullptr, &openHlirView);
                ImGui::MenuItem("SSA Debug View", nullptr, &openSsaView);
                ImGui::MenuItem("Logs", nullptr, &openLogView);
                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        ImGui::End();

        if (openMemoryView)
        {
            if (ImGui::Begin("Memory Stats", &openMemoryView))
            {
            }
            ImGui::End();
        }

        if (openPerfView)
        {
            if (ImGui::Begin("Performance Stats", &openPerfView))
            {
            }
            ImGui::End();
        }

        if (openHlirView)
        {
            if (ImGui::Begin("HLIR Debug View", &openHlirView))
            {
            }
            ImGui::End();
        }

        if (openSsaView)
        {
            if (ImGui::Begin("SSA Debug View", &openSsaView))
            {
            }
            ImGui::End();
        }

        if (openLogView)
        {
            if (ImGui::Begin("Compiler Log View", &openLogView))
            {
            }
            ImGui::End();
        }

        ImGui::ShowDemoWindow();

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
