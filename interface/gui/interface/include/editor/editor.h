#pragma once

#include "base/memory.h"

#include "cthulhu/interface/interface.h"

#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include "imgui/imgui.h"
#include "imnodes/ImNodesEz.h"
#include "file-dialog/ImGuiFileDialog.h"

#include <atomic>
#include <string>

namespace ed {
    extern "C" struct Alloc {
        Alloc(const char *name);

        alloc_t get() const;

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

        virtual void *doMalloc(size_t size, const char *name) override;

        virtual void *doRealloc(void *ptr, size_t newSize, size_t oldSize) override;

        virtual void doFree(void *ptr, size_t size) override;

    private:
        void adjustMemoryUsage(size_t size);

    public:
        std::atomic_size_t currentUsedMemory{0};
        std::atomic_size_t peakUsedMemory{0};

        std::atomic_size_t totalAllocs{0};
        std::atomic_size_t totalReallocs{0};
        std::atomic_size_t totalFrees{0};

        std::atomic_size_t currentAllocations{0};
    };
}

struct Editor {
    Editor(driver_t driver);
    ~Editor();

    void beginDock();
    void endDock();

    bool begin();
    void end();

    void openDialog();
    bool showDialog(std::string& name, std::string& path);

    void memoryStats(bool *open);
    void nodeEditor(bool *open);

    void quit() { shouldQuit = true; }

private:
    driver_t driver;
    ed::StatsAlloc alloc;

    void initGl();
    void initImGui();

    void deinitGl();
    void deinitImGui();

    std::string title;
    GLFWwindow *window;

    ImNodes::Ez::Context *ctx;
    ImGuiFileDialog dialog;

    bool shouldQuit = false;
};
