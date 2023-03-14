#include "editor/editor.h"

#include "base/version-def.h"
#include "base/macros.h"
#include "base/memory.h"
#include "base/panic.h"

#include "std/str.h"

#include "report/report.h"

#include "cthulhu/hlir/query.h"
#include "cthulhu/interface/interface.h"

#include "imgui/imgui_internal.h"
#include "imgui/misc/cpp/imgui_stdlib.h"

#include <stdio.h>
#include <unordered_map>
#include <unordered_set>

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
        return strcmp(inputSlot, other.inputSlot) == 0
            && strcmp(outputSlot, other.outputSlot) == 0
            && inputNode == other.inputNode
            && outputNode == other.outputNode;
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

    ImVec2 position = ImVec2(100, 100);
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

namespace hlir {
    struct DigitLiteral : HlirNode 
    {
        DigitLiteral(const char *text)
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

    struct BoolLiteral : HlirNode 
    {
        BoolLiteral(bool value)
            : HlirNode("Bool Literal", {}, { SlotInfo{"Value", eSlotDigit} })
            , value(value)
        { }

        void drawContent() override
        {
            ImGui::SetNextItemWidth(64.f);
            ImGui::Checkbox("Value", &value);
        }

    private:
        bool value;
    };

    struct StringLiteral : HlirNode 
    {
        StringLiteral(const char *text)
            : HlirNode("String Literal", {}, { SlotInfo{"Value", eSlotDigit} })
            , str(text)
        { }

        void drawContent() override
        {
            ImGui::SetNextItemWidth(64.f);
            ImGui::InputText("Value", &str, ImGuiInputTextFlags_CharsDecimal);
        }
    private:
        std::string str;
    };  

    #define BINARY_OP(op, name, symbol) name "\0"
    const char *kBinaryNames = 
    #include "cthulhu/hlir/hlir-def.inc"
        "\0\0"
        ;

    #define UNARY_OP(op, name, symbol) name "\0"
    const char *kUnaryNames =
    #include "cthulhu/hlir/hlir-def.inc"
        "\0\0"
        ;

    struct Binary : HlirNode
    {
        Binary(binary_t op)
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

    struct Unary : HlirNode
    {
        Unary(unary_t op)
            : HlirNode("Unary", { SlotInfo{"Value", eSlotDigit} }, { SlotInfo{"Value", eSlotDigit} })
            , unary(op)
        { }

        void drawContent() override
        {
            ImGui::SetNextItemWidth(64.f);
            ImGui::Combo("Op", &unary, kUnaryNames);
        }

    private:
        int unary;
    };
}

int main()
{
    Editor editor(get_driver());

    bool memoryView = false;
    bool perfView = false;
    bool hlirView = false;
    bool ssaView = false;
    bool logView = false;

    std::vector<HlirNode*> hlirNodes = {
        new hlir::DigitLiteral("0"),
        new hlir::Binary(eBinaryAdd)
    };

    while (editor.begin()) {
        editor.beginDock();

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Open"))
                {
                    editor.openDialog();
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Metrics"))
            {
                ImGui::MenuItem("Memory Stats", nullptr, &memoryView);
                ImGui::MenuItem("Performance Stats", nullptr, &perfView);
                ImGui::MenuItem("HLIR Debug View", nullptr, &hlirView);
                ImGui::MenuItem("SSA Debug View", nullptr, &ssaView);
                ImGui::MenuItem("Logs", nullptr, &logView);
                ImGui::EndMenu();
            }

            ImGuiStyle& style = ImGui::GetStyle();
            ImVec2 closeButtonPos(ImGui::GetWindowWidth() - (style.FramePadding.x * 2) - ImGui::GetFontSize(), 0.f);

            if (ImGui::CloseButton(ImGui::GetID("CloseEditor"), closeButtonPos)) {
                editor.quit();
            }

            ImGui::EndMenuBar();
        }

        editor.endDock();

        std::string name, path;
        if (editor.showDialog(name, path)) {

        }

        if (ImGui::Begin("Performance Stats", &perfView))
        {
        }
        ImGui::End();

        editor.memoryStats(&memoryView);
        editor.nodeEditor(&hlirView);

        if (ImGui::Begin("SSA Debug View", &ssaView))
        {
        }
        ImGui::End();

        if (ImGui::Begin("Compiler Log View", &logView))
        {
        }
        ImGui::End();

        ImGui::ShowDemoWindow();

        editor.end();
    }

    return EXIT_OK;
}
