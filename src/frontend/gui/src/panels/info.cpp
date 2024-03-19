#include "stdafx.hpp"

#include "editor/utils.hpp"

#include "editor/panels/info.hpp"

#include "core/macros.h"

#include "cthulhu/broker/broker.h"
#include "interop/compile.h"

using namespace ed;

// helpers

static std::string_view from_text_view(const text_view_t& view)
{
    return std::string_view(view.text, view.length);
}

static void draw_version(const char *id, version_t version)
{
    int major = CT_VERSION_MAJOR(version);
    int minor = CT_VERSION_MINOR(version);
    int patch = CT_VERSION_PATCH(version);

    ImGui::Text("%s: %d.%d.%d", id, major, minor, patch);
}

static void draw_diagnostics(diagnostic_list_t list)
{
    if (ImGui::TreeNode((void*)&list, "Diagnostics: %zu", list.count))
    {
        for (size_t i = 0; i < list.count; i++)
        {
            const diagnostic_t *diag = list.diagnostics[i];
            char label[128];
            (void)std::snprintf(label, std::size(label), "Diagnostic %s | %s", diag->id, severity_name(diag->severity));
            ImGui::SeparatorText(label);
            ImGui::TextWrapped("%s", diag->brief ? diag->brief : "no brief");
            ImGui::TextWrapped("%s", diag->description ? diag->description : "no description");
        }
        ImGui::TreePop();
    }
}

static void draw_feature(const char *name, bool supported)
{
    ImGui::TextUnformatted(name);
    ImGui::SameLine();
    if (supported)
    {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Available");
    }
    else
    {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Not Available");
    }
}

static void text_memory(uintmax_t value)
{
    char buffer[64];
    ed::format_memory(value, buffer);
    ImGui::Text("%s", buffer);
    ImGui::SetItemTooltip("%ju bytes", value);
}

static void draw_memory(const char *label, uintmax_t value)
{
    ImGui::Text("%s: ", label);
    ImGui::SameLine();

    text_memory(value);
}

// module info

ModuleInfoPanel::ModuleInfoPanel(const module_info_t& info, panel_info_t setup)
    : IEditorPanel(info.name, setup)
    , info(info)
{ }

void ModuleInfoPanel::draw_info()
{
    ImGui::Text("ID: %s", info.id);

    version_info_t version = info.version;
    ImGui::Text("License: %s", version.license);
    ImGui::Text("Description: %s", version.desc);
    ImGui::Text("Author: %s", version.author);
    draw_version("Version", version.version);
    draw_diagnostics(info.diagnostics);
}

// frontend info

FrontendInfoPanel::FrontendInfoPanel(const frontend_t& frontend, panel_info_t setup)
    : ModuleInfoPanel(frontend.info, setup)
    , frontend(frontend)
{ }

void FrontendInfoPanel::draw_content()
{
    ModuleInfoPanel::draw_info();
    CT_UNUSED(frontend); // TODO: once theres more info to display, use it here
}

// language info

LanguageInfoPanel::LanguageInfoPanel(const language_t& lang, panel_info_t setup)
    : ModuleInfoPanel(lang.info, setup)
    , lang(lang)
{
    builtin = from_text_view(lang.builtin.name);
    for (char& c : builtin) if (c == '\0') c = '/';

    for (size_t i = 0; lang.exts[i]; i++)
    {
        if (i > 0) extensions += ", ";
        extensions += lang.exts[i];
    }
}

static const ImGuiTableFlags kCallbackTableFlags
    = ImGuiTableFlags_BordersV
    | ImGuiTableFlags_BordersOuterH
    | ImGuiTableFlags_Resizable
    | ImGuiTableFlags_RowBg
    | ImGuiTableFlags_NoHostExtendX
    | ImGuiTableFlags_NoBordersInBody;

static float get_tooltip_width()
{
    ImVec2 ttsize = ImGui::CalcTextSize("(?)");
    return ttsize.x * 2;
}

static void draw_function_address(const void *pfn)
{
    if (pfn == nullptr)
    {
        ImVec4 orange = ImVec4(1.0f, 0.5f, 0.0f, 1.0f);
        ImGui::TextColored(orange, "Not provided");
    }
    else
    {
        ImGui::Text("0x%p", pfn);
    }
}

static void draw_row(const char *name, const void *pfn, const char *tooltip)
{
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::TextDisabled("(?)");
    ImGui::SetItemTooltip("%s", tooltip);

    ImGui::TableNextColumn();
    ImGui::TextUnformatted(name);

    ImGui::TableNextColumn();
    draw_function_address(pfn);
}

static bool begin_table_node(const void *ptr_id, const char *label, int columns, ImGuiTableFlags flags)
{
    if (ImGui::TreeNode(ptr_id, label))
    {
        if (ImGui::BeginTable(label, columns, flags))
        {
            return true;
        }
        else
        {
            ImGui::TreePop();
        }
    }

    return false;
}

static void end_table_node()
{
    ImGui::EndTable();
    ImGui::TreePop();
}

void LanguageInfoPanel::draw_content()
{
    float ttwidth = get_tooltip_width();
    ModuleInfoPanel::draw_info();
    ImGui::Text("Default extensions: %s", extensions.c_str());

    draw_memory("Context struct size", lang.context_size);
    draw_memory("AST struct size", lang.ast_size);

    if (begin_table_node((void*)&lang.builtin, "Builtin", 2, kCallbackTableFlags))
    {
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Size");
        ImGui::TableHeadersRow();

        for (size_t i = 0; i < lang.builtin.length; i++)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(lang.builtin.names[i]);
            ImGui::TableNextColumn();
            text_memory(lang.builtin.decls[i]);
        }
        end_table_node();
    }

    if (begin_table_node((void*)&lang.fn_passes, "Callbacks", 3, kCallbackTableFlags))
    {
        ImGui::TableSetupColumn("Info", ImGuiTableColumnFlags_WidthFixed, ttwidth);
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Address");
        ImGui::TableHeadersRow();

        draw_row("Create", (const void*)lang.fn_create,
            "Called during initial creation of the language driver.\n"
            "Responsible for setting up any resources required for the language.");

        draw_row("Destroy", (const void*)lang.fn_destroy,
            "Called during destruction of the language driver.\n"
            "Responsible for cleaning up any resources allocated during creation or runtime.");

        draw_row("Preparse", (const void*)lang.fn_preparse,
            "Called once before parsing each source file.\n"
            "Configures parser and scanner local values for the source file.");

        draw_row("Postparse", (const void*)lang.fn_postparse,
            "Called once after parsing each source file.\n"
            "Responsible for producing translation units that will later be analyzed.");

        end_table_node();
    }

    const scan_callbacks_t *scan = lang.scanner;
    ImGui::BeginDisabled(scan == nullptr);

    if (begin_table_node((void*)scan, ed::strfmt<64>("Scanner (0x%p)", scan), 3, kCallbackTableFlags))
    {
        ImGui::TableSetupColumn("Info", ImGuiTableColumnFlags_WidthFixed, ttwidth);
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Address");
        ImGui::TableHeadersRow();

        draw_row("Init", (const void*)scan->init,
            "Called once during the creation of the scanner.\n"
            "Responsible for setting up any resources required for the scanner.");

        draw_row("Scan", (const void*)scan->scan,
            "Scans a source file and produces a token stream.\n"
            "This token stream is required for parsing.");

        draw_row("Parse", (const void*)scan->parse,
            "Parses a token stream and produces an ast.");

        draw_row("Destroy Buffer", (const void*)scan->destroy,
            "Called during destruction of the scanner.\n"
            "Responsible for cleaning up any resources allocated during creation or runtime.");

        draw_row("Destroy", (const void*)scan->destroy,
            "Called during destruction of the scanner.\n"
            "Responsible for cleaning up any resources allocated during creation or runtime.");

        end_table_node();
    }
    ImGui::EndDisabled();

    if (scan == nullptr)
    {
        ImGui::SetItemTooltip("This language does not provide a scanner");
    }

    if (begin_table_node((void*)&lang.fn_passes, "Passes", 2, kCallbackTableFlags))
    {
        ImGui::TableSetupColumn("Index", ImGuiTableColumnFlags_WidthFixed, 30.f);
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Address");

        for (size_t i = 0; i < ePassCount; i++)
        {
            broker_pass_t pass = static_cast<broker_pass_t>(i);
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%zu", i);
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(broker_pass_name(pass));
            ImGui::TableNextColumn();
            draw_function_address((const void*)lang.fn_passes[i]);
        }
        end_table_node();
    }
}

// plugin info

PluginInfoPanel::PluginInfoPanel(const plugin_t& plugin, panel_info_t setup)
    : ModuleInfoPanel(plugin.info, setup)
    , plugin(plugin)
{ }

void PluginInfoPanel::draw_content()
{
    ModuleInfoPanel::draw_info();
    ImGui::Text("Create: %p", plugin.fn_create);
    ImGui::Text("Destroy: %p", plugin.fn_destroy);

    event_list_t events = plugin.events;
    if (ImGui::TreeNode((void*)&events, "Events"))
    {
        for (size_t i = 0; i < events.count; i++)
        {
            ImGui::BulletText("Event %zu: %d", i, events.events[i].event);
        }
    }
}

// target info

TargetInfoPanel::TargetInfoPanel(const target_t& target, panel_info_t setup)
    : ModuleInfoPanel(target.info, setup)
    , target(target)
{ }

void TargetInfoPanel::draw_content()
{
    ModuleInfoPanel::draw_info();
    ImGui::Text("Create: %p", target.fn_create);
    ImGui::Text("Destroy: %p", target.fn_destroy);
    draw_feature("Tree output", target.fn_tree != nullptr);
    draw_feature("SSA output", target.fn_ssa != nullptr);
}
