#include "base/version-def.h"
#include "base/macros.h"
#include "base/memory.h"
#include "base/panic.h"

#include "std/str.h"

#include "report/report.h"

#include "cthulhu/hlir/query.h"
#include "cthulhu/interface/interface.h"

#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui/misc/cpp/imgui_stdlib.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "imnodes/ImNodesEz.h"

#include <stdio.h>
#include <unordered_map>
#include <unordered_set>
#include <atomic>

using ssize_t = std::make_signed_t<size_t>;

extern "C" struct Alloc {
    Alloc(const char *name) {
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

    alloc_t get() const { return alloc; }

    virtual ~Alloc() = default;

protected:
    virtual void *doMalloc(size_t size, const char *name) = 0;
    virtual void *doRealloc(void *ptr, size_t newSize, size_t oldSize) = 0;
    virtual void doFree(void *ptr, size_t size) = 0;

private:
    alloc_t alloc;
};

struct StatsAlloc : Alloc {
    using Alloc::Alloc;

    virtual void *doMalloc(size_t size, const char *name) override {
        UNUSED(name);

        adjustMemoryUsage(size);
        totalAllocs += 1;
        currentAllocations += 1;

        return malloc(size);
    }

    virtual void *doRealloc(void *ptr, size_t newSize, size_t oldSize) override {
        adjustMemoryUsage(newSize - oldSize);
        totalReallocs += 1;

        return realloc(ptr, newSize);
    }

    virtual void doFree(void *ptr, size_t size) override {
        adjustMemoryUsage(-ssize_t(size));
        totalFrees += 1;
        currentAllocations -= 1;

        free(ptr);
    }

private:
    void adjustMemoryUsage(size_t size) {
        currentUsedMemory += size;
        peakUsedMemory.store(MAX(peakUsedMemory, currentUsedMemory));
    }

public:
    std::atomic_size_t currentUsedMemory{0};
    std::atomic_size_t peakUsedMemory{0};

    std::atomic_size_t totalAllocs{0};
    std::atomic_size_t totalReallocs{0};
    std::atomic_size_t totalFrees{0};

    std::atomic_size_t currentAllocations{0};
};

void checkNewConnection();

using SlotInfo = ImNodes::Ez::SlotInfo;
using SlotInfoVec = std::vector<SlotInfo>;

enum SlotType : int {
    eSlotInvalid,
    eSlotDigit,
    eSlotTotal
};

struct HlirConnection
{
    void *inputNode = nullptr;
    void *outputNode = nullptr;

    const char *inputSlot = nullptr;
    const char *outputSlot = nullptr;

    bool operator==(const HlirConnection& other) const 
    {
        return inputNode == other.inputNode 
            && outputNode == other.outputNode 
            && inputSlot == other.inputSlot 
            && outputSlot == other.outputSlot;
    }

    bool operator!=(const HlirConnection& other) const 
    {
        return !(*this == other);
    }
};

struct HlirNode
{
    HlirNode(const char *title, const SlotInfoVec& inputs, const SlotInfoVec& outputs)
        : title(title)
        , inputs(inputs)
        , outputs(outputs)
    { }

    virtual ~HlirNode() = default;

    virtual void drawContent() { }

    const char *title = nullptr;

    ImVec2 position = ImVec2(0, 0);
    bool selected = false;

    std::vector<HlirConnection> connections;

    SlotInfoVec inputs;
    SlotInfoVec outputs;

    void addConnection(HlirConnection conn)
    {
        connections.push_back(conn);
    }

    void removeConnection(HlirConnection conn)
    {
        auto it = std::find(connections.begin(), connections.end(), conn);
        if (it != connections.end())
        {
            connections.erase(it);
        }
    }

    void drawConnections() 
    {
        for (const auto& conn : connections)
        {
            if (conn.outputNode != this)
                continue;

            if (!ImNodes::Connection(conn.inputNode, conn.inputSlot, conn.outputNode, conn.outputSlot))
            {
                auto *inputNode = reinterpret_cast<HlirNode*>(conn.inputNode);
                auto *outputNode = reinterpret_cast<HlirNode*>(conn.outputNode);

                inputNode->removeConnection(conn);
                outputNode->removeConnection(conn);
            }
        }
    }

    void draw()
    {
        if (ImNodes::Ez::BeginNode(this, title, &position, &selected))
        {
            ImNodes::Ez::InputSlots(inputs.data(), int(inputs.size()));

            drawContent();

            ImNodes::Ez::OutputSlots(outputs.data(), int(outputs.size()));

            checkNewConnection();
            drawConnections();

            ImNodes::Ez::EndNode();
        }
    }
};

void checkNewConnection()
{
    HlirConnection conn;
    if (ImNodes::GetNewConnection(&conn.inputNode, &conn.inputSlot, &conn.outputNode, &conn.outputSlot))
    {
        auto *inputNode = reinterpret_cast<HlirNode*>(conn.inputNode);
        auto *outputNode = reinterpret_cast<HlirNode*>(conn.outputNode);

        inputNode->addConnection(conn);
        outputNode->addConnection(conn);
    }
}

struct HlirDigitLiteralNode : HlirNode 
{
    HlirDigitLiteralNode(const char *text)
        : HlirNode("Digit Literal", {}, { SlotInfo{"Value", eSlotDigit} })
        , str(text)
    { 
        mpz_init_set_str(value, str.c_str(), 10);
    }

    void drawContent() override
    {
        ImGui::SetNextItemWidth(64.f);
        ImGui::InputText("Value", &str, ImGuiInputTextFlags_CharsDecimal);
        mpz_set_str(value, str.c_str(), 10);
    }

private:
    std::string str;
    mpz_t value;
};

#define BINARY_OP(op, name, symbol) name "\0"
const char *kBinaryNames = 
#include "cthulhu/hlir/hlir-def.inc"
    "\0\0"
    ;

struct HlirBinaryNode : HlirNode
{
    HlirBinaryNode(binary_t op)
        : HlirNode("Binary", { SlotInfo{"Left", eSlotDigit}, SlotInfo{"Right", eSlotDigit} }, { SlotInfo{"Value", eSlotDigit} })
        , binary(op)
    { }

    void drawContent() override
    {
        ImGui::SetNextItemWidth(64.f);
        ImGui::Combo("Op", &binary, kBinaryNames);
    }

private:
    int binary;
};

int main()
{
    StatsAlloc statsAlloc("stats");
    globalAlloc = statsAlloc.get();

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
    glfwSetErrorCallback([](int error, const char* desc) { 
        (void)fprintf(stderr, "GLFW error(%d): %s", error, desc); 
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

    bool openMemoryView = false;
    bool openPerfView = false;
    bool openHlirView = false;
    bool openSsaView = false;
    bool openLogView = false;

    std::vector<HlirNode*> hlirNodes = {
        new HlirDigitLiteralNode("0"),
        new HlirBinaryNode(eBinaryAdd)
    };

    ImNodes::Ez::Context *ctx = ImNodes::Ez::CreateContext();

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

            ImGuiStyle& style = ImGui::GetStyle();
            ImVec2 closeButtonPos(ImGui::GetWindowWidth() - (style.FramePadding.x * 2) - ImGui::GetFontSize(), 0.f);

            if (ImGui::CloseButton(ImGui::GetID("CloseEditor"), closeButtonPos)) {
                shouldExit = true;
            }

            ImGui::EndMenuBar();
        }

        ImGui::End();

        if (openMemoryView)
        {
            if (ImGui::Begin("Memory Stats", &openMemoryView))
            {
                ImGui::Text("Current Used Memory: %zu", statsAlloc.currentUsedMemory.load());
                ImGui::Text("Current Allocations: %zu", statsAlloc.currentAllocations.load());

                ImGui::Text("Peak Used Memory: %zu", statsAlloc.peakUsedMemory.load());

                ImGui::Text("Total Allocs: %zu", statsAlloc.totalAllocs.load());
                ImGui::Text("Total Reallocs: %zu", statsAlloc.totalReallocs.load());
                ImGui::Text("Total Frees: %zu", statsAlloc.totalFrees.load());
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
            if (ImGui::Begin("HLIR Debug View", &openHlirView, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
            {
                ImNodes::Ez::BeginCanvas();

                for (auto* node : hlirNodes) {
                    node->draw();
                }

                if (ImGui::IsMouseReleased(ImGuiMouseButton_Right) && ImGui::IsWindowHovered() && !ImGui::IsMouseDragging(ImGuiMouseButton_Right))
                {
                    ImGui::FocusWindow(ImGui::GetCurrentWindow());
                    ImGui::OpenPopup("NodeContextMenu");
                }

                if (ImGui::BeginPopup("NodeContextMenu"))
                {
                    if (ImGui::MenuItem("Digit Literal"))
                    {
                        auto *node = new HlirDigitLiteralNode("0");
                        hlirNodes.push_back(node);
                        ImNodes::AutoPositionNode(node);
                    }

                    ImGui::EndPopup();
                }

                ImNodes::Ez::EndCanvas();
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

    ImNodes::Ez::FreeContext(ctx);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();

    return EXIT_OK;
}
