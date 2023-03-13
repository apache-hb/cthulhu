#include "editor/editor.h"

#include "base/panic.h"

#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <format>

using namespace ed;

using ssize_t = std::make_signed_t<size_t>;

Alloc::Alloc(const char *name) {
    alloc.name = name;
    alloc.data = this;
    alloc.arenaMalloc = [](alloc_t *self, size_t size, const char *name) {
        CTASSERT(self != nullptr);
        return reinterpret_cast<Alloc*>(self->data)->doMalloc(size, name);
    };

    alloc.arenaRealloc = [](alloc_t *self, void *ptr, size_t newSize, size_t oldSize) {
        CTASSERT(self != nullptr);
        return reinterpret_cast<Alloc*>(self->data)->doRealloc(ptr, newSize, oldSize);
    };

    alloc.arenaFree = [](alloc_t *self, void *ptr, size_t size) {
        CTASSERT(self != nullptr);
        reinterpret_cast<Alloc*>(self->data)->doFree(ptr, size);
    };
}

alloc_t Alloc::get() const { return alloc; }

void *StatsAlloc::doMalloc(size_t size, const char *name) {
    UNUSED(name);

    adjustMemoryUsage(size);
    totalAllocs += 1;
    currentAllocations += 1;

    return malloc(size);
}

void *StatsAlloc::doRealloc(void *ptr, size_t newSize, size_t oldSize) {
    adjustMemoryUsage(newSize - oldSize);
    totalReallocs += 1;

    return realloc(ptr, newSize);
}

void StatsAlloc::doFree(void *ptr, size_t size) {
    adjustMemoryUsage(-ssize_t(size));
    totalFrees += 1;
    currentAllocations -= 1;

    free(ptr);
}

void StatsAlloc::adjustMemoryUsage(size_t size) {
    currentUsedMemory += size;
    peakUsedMemory.store(MAX(peakUsedMemory, currentUsedMemory));
}

namespace {
    std::string newTitle(const driver_t &driver) {
        return std::format(
            "GUI Editor ({} | {}.{}.{})", 
            driver.name, 
            VERSION_MAJOR(driver.version), VERSION_MINOR(driver.version), VERSION_PATCH(driver.version)
        );
    }
}

Editor::Editor(driver_t driver) 
    : driver(driver)
    , alloc("editor") 
    , title(newTitle(driver))
{
    GLOBAL_INIT();
    globalAlloc = alloc.get();

    common_init();

    initGl();
    initImGui();
}

Editor::~Editor() {
    deinitImGui();
    deinitGl();
}

void Editor::initGl() {
    int init = glfwInit();
    CTASSERT(init);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwSetErrorCallback([](int error, const char* desc) { 
        (void)fprintf(stderr, "GLFW error(%d): %s\n", error, desc); 
    });

    GLFWmonitor *primary = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(primary);

    window = glfwCreateWindow(mode->width, mode->height, title.c_str(), primary, NULL);
    CTASSERT(window != nullptr);
    
    glfwMakeContextCurrent(window);
    
    glfwSetFramebufferSizeCallback(window, [](auto, auto width, auto height) { 
        glViewport(0, 0, width, height); 
    });

    glfwSwapInterval(0);

    int load = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    CTASSERT(load);
}

void Editor::initImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    ctx = ImNodes::Ez::CreateContext();
}

void Editor::deinitImGui() {
    ImNodes::Ez::FreeContext(ctx);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Editor::deinitGl() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Editor::beginDock() {
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
}

void Editor::endDock() {
    ImGui::End();
}

void Editor::openDialog() {
    dialog.OpenDialog("ChooseFile", "Open File", driver.exts, ".");
}

bool Editor::showDialog(std::string& name, std::string& path) {
    bool result = false;

    if (dialog.Display("ChooseFile")) {
        if (dialog.IsOk()) {
            name = dialog.GetFilePathName();
            path = dialog.GetCurrentPath();
            result = true;
        }

        dialog.Close();
    }

    return result;
}

bool Editor::begin() {
    if (glfwWindowShouldClose(window) || shouldQuit)
        return false;

    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    return true;
}

void Editor::end() {
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
}

void Editor::memoryStats(bool *open) {
    if (ImGui::Begin("Memory Stats", open)) {
        ImGui::Text("Current Used Memory: %zu", alloc.currentUsedMemory.load());
        ImGui::Text("Current Allocations: %zu", alloc.currentAllocations.load());

        ImGui::Text("Peak Used Memory: %zu", alloc.peakUsedMemory.load());

        ImGui::Text("Total Allocs: %zu", alloc.totalAllocs.load());
        ImGui::Text("Total Reallocs: %zu", alloc.totalReallocs.load());
        ImGui::Text("Total Frees: %zu", alloc.totalFrees.load());
    }
    ImGui::End();
}

void Editor::nodeEditor(bool *open) {
    if (ImGui::Begin("HLIR Debug View", open, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
        ImNodes::Ez::BeginCanvas();

        if (ImGui::IsMouseReleased(ImGuiMouseButton_Right) && ImGui::IsWindowHovered() && !ImGui::IsMouseDragging(ImGuiMouseButton_Right))
        {
            ImGui::FocusWindow(ImGui::GetCurrentWindow());
            ImGui::OpenPopup("NodeContextMenu");
        }

        if (ImGui::BeginPopup("NodeContextMenu"))
        {
            ImGui::EndPopup();
        }

        ImNodes::Ez::EndCanvas();
    }
    ImGui::End();
}
