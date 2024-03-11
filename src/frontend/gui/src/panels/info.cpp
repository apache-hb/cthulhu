#include "editor/panels/info.hpp"

#include "core/macros.h"
#include "imgui/imgui.h"

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
    if (supported) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Available");
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Not Available");
    }
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

void LanguageInfoPanel::draw_content()
{
    ModuleInfoPanel::draw_info();
    ImGui::Text("Default extensions: %s", extensions.c_str());
    ImGui::Text("Context size: %zu", lang.context_size);
    ImGui::Text("AST size: %zu", lang.ast_size);

    if (ImGui::TreeNode((void*)&lang.builtin, "Builtin"))
    {
        ImGui::BulletText("Builtin module: %s", builtin.c_str());
        language_info_t mod = lang.builtin;
        for (size_t i = 0; i < mod.length; i++)
        {
            const char *id = mod.names ? mod.names[i] : "unknown";
            ImGui::BulletText("Initial size %s: %zu", id, mod.decls[i]);
        }
        ImGui::TreePop();
    }

    if (ImGui::TreeNode((void*)&lang, "Callbacks"))
    {
        ImGui::BulletText("Create %p", lang.fn_create);
        ImGui::BulletText("Destroy %p", lang.fn_destroy);

        ImGui::BulletText("Preparse %p", lang.fn_preparse);
        ImGui::BulletText("Postparse %p", lang.fn_postparse);
        ImGui::TreePop();
    }

    const scan_callbacks_t *scan = lang.scanner;
    ImGui::BeginDisabled(scan == nullptr);
    if (ImGui::TreeNode((void*)scan, "Scanner (0x%p)", scan))
    {
        ImGui::BulletText("Init: %p", scan->init);
        ImGui::BulletText("Parse: %p", scan->parse);
        ImGui::BulletText("Scan: %p", scan->scan);
        ImGui::BulletText("Destroy Buffer: %p", scan->destroy);
        ImGui::BulletText("Destroy: %p", scan->destroy);
    }
    ImGui::TreePop();
    if (scan == nullptr)
    {
        ImGui::SetItemTooltip("This language does not provide a scanner");
    }

    if (ImGui::TreeNode((void*)&lang.fn_passes, "Passes"))
    {
        for (size_t i = 0; i < ePassCount; i++)
        {
            broker_pass_t pass = static_cast<broker_pass_t>(i);
            ImGui::BulletText("Pass %s: %p", broker_pass_name(pass), lang.fn_passes[i]);
        }
        ImGui::TreePop();
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
