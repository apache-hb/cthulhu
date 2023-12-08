#include "editor/config.hpp"

#include "imgui/imgui.h"

// for some reason the ImU64 overload of ImGui::CheckboxFlags is not included in imgui.h
#include "imgui/imgui_internal.h" // IWYU pragma: keep

#include "std/vector.h"

#include <algorithm>

#include <stdio.h>

/// drawing

static const ImGuiTableFlags kTableFlags
    = ImGuiTableFlags_BordersV
    | ImGuiTableFlags_BordersOuterH
    | ImGuiTableFlags_Resizable
    | ImGuiTableFlags_RowBg
    | ImGuiTableFlags_NoHostExtendX
    | ImGuiTableFlags_NoBordersInBody;

static const ImGuiTreeNodeFlags kGroupNodeFlags
    = ImGuiTreeNodeFlags_SpanAllColumns
    | ImGuiTreeNodeFlags_AllowOverlap;

static const ImGuiTreeNodeFlags kValueNodeFlags
    = kGroupNodeFlags
    | ImGuiTreeNodeFlags_Leaf
    | ImGuiTreeNodeFlags_Bullet
    | ImGuiTreeNodeFlags_NoTreePushOnOpen;

const char *get_name(const cfg_field_t *field)
{
    const cfg_info_t *info = cfg_get_info(field);
    return info->name;
}

void get_label(char *buf, size_t size, const cfg_field_t *field)
{
    snprintf(buf, size, "##%s", get_name(field));
}

void draw_info_preamble(const cfg_info_t *info)
{
    const char *brief = info->brief != nullptr ? info->brief : "no brief";
    ImGui::Text("brief: %s", brief);

    if (info->description != nullptr)
    {
        ImGui::Separator();
        ImGui::Text("description: %s", info->description);
    }
}

void draw_field_info(const cfg_field_t *field)
{
    const cfg_info_t *info = cfg_get_info(field);
    ImGui::TextDisabled("%s", info->name);
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.f);
        draw_info_preamble(info);

        if (info->arg_long != nullptr && info->arg_short != nullptr)
        {
            ImGui::Separator();
            ImGui::Text("args: --%s / -%s", info->arg_long, info->arg_short);
        }
        else if (info->arg_long != nullptr)
        {
            ImGui::Separator();
            ImGui::Text("long: --%s", info->arg_long);
        }
        else if (info->arg_short != nullptr)
        {
            ImGui::Separator();
            ImGui::Text("short: -%s", info->arg_short);
        }

        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void draw_group_info(const config_t *group)
{
    const cfg_info_t *info = cfg_group_info(group);

    ImGui::TextDisabled("%s", info->name);
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.f);
        draw_info_preamble(info);

        vector_t *child_groups = cfg_get_groups(group);
        vector_t *child_fields = cfg_get_fields(group);

        size_t group_count = vector_len(child_groups);
        size_t field_count = vector_len(child_fields);

        ImGui::SeparatorText("Children");
        ImGui::Text("%zu groups, %zu fields", group_count, field_count);

        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void draw_bool(cfg_field_t *field)
{
    char label[64];
    get_label(label, std::size(label), field);

    bool value = cfg_bool_value(field);
    if (ImGui::Checkbox(label, &value))
    {
        cfg_set_bool(field, value);
    }
}

void draw_int(cfg_field_t *field)
{
    char label[64];
    get_label(label, std::size(label), field);

    const cfg_int_t *cfg = cfg_int_info(field);
    int value = cfg_int_value(field);
    if (ImGui::DragInt(label, &value, 1.f, cfg->min, cfg->max))
    {
        cfg_set_int(field, value);
    }
}

void draw_string(cfg_field_t *field)
{
    char label[64];
    get_label(label, std::size(label), field);

    const char *value = cfg_string_value(field);
    char buffer[256] = { 0 };
    strncpy_s(buffer, value, std::size(buffer));
    if (ImGui::InputText(label, buffer, std::size(buffer)))
    {
        cfg_set_string(field, buffer);
    }
}

void draw_enum(cfg_field_t *field)
{
    char label[64];
    get_label(label, std::size(label), field);

    const cfg_enum_t *cfg = cfg_enum_info(field);
    size_t value = cfg_enum_value(field);

    size_t current = SIZE_MAX;
    for (size_t i = 0; i < cfg->count; i++)
    {
        if (cfg->options[i].value == value)
        {
            current = i;
            break;
        }
    }

    CTASSERTF(current != SIZE_MAX, "invalid enum value %zu for field %s", value, label);

    if (ImGui::BeginCombo(label, cfg->options[current].text))
    {
        for (size_t i = 0; i < cfg->count; i++)
        {
            bool is_selected = (current == i);
            if (ImGui::Selectable(cfg->options[i].text, is_selected))
            {
                cfg_set_enum(field, cfg->options[i].text);
            }

            if (is_selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndCombo();
    }
}

void draw_flags(cfg_field_t *field)
{
    const char *edit_flags_popup = "Edit Flags";
    char label[64];
    get_label(label, std::size(label), field);

    const cfg_flags_t *cfg = cfg_flags_info(field);
    ImU64 value = cfg_flags_value(field);

    if (ImGui::Button("Edit"))
    {
        ImGui::OpenPopup(edit_flags_popup);
    }

    if (ImGui::BeginPopup(edit_flags_popup))
    {
        bool changed = false;
        for (size_t i = 0; i < cfg->count; i++)
        {
            const cfg_choice_t *choice = &cfg->options[i];
            ImU64 flag = choice->value;
            if (ImGui::CheckboxFlags(choice->text, &value, flag))
            {
                changed = true;
            }
        }

        if (changed)
        {
            cfg_set_flag_value(field, value);
        }

        ImGui::EndPopup();
    }
}

void draw_value(cfg_field_t *field)
{
    cfg_type_t type = cfg_get_type(field);
    switch (type)
    {
    case eConfigBool:
        draw_bool(field);
        break;

    case eConfigInt:
        draw_int(field);
        break;

    case eConfigString:
        draw_string(field);
        break;

    case eConfigEnum:
        draw_enum(field);
        break;

    case eConfigFlags:
        draw_flags(field);
        break;

    default:
        ImGui::TextDisabled("Unknown type");
        break;
    }
}

void draw_int_constraints(const cfg_field_t *field)
{
    const cfg_int_t *cfg = cfg_int_info(field);

    if (cfg->min == INT_MIN && cfg->max == INT_MAX)
    {
        ImGui::TextDisabled("no constraints");
        return;
    }
    else if (cfg->min == INT_MIN)
    {
        ImGui::Text("min (%d)", cfg->max);
        return;
    }
    else if (cfg->max == INT_MAX)
    {
        ImGui::Text("max (%d)", cfg->min);
        return;
    }
    else
    {
        ImGui::Text("range (%d, %d)", cfg->min, cfg->max);
    }
}

void draw_constraints(const cfg_field_t *field)
{
    cfg_type_t type = cfg_get_type(field);
    switch (type)
    {
    case eConfigBool:
    case eConfigString:
    case eConfigEnum:
    case eConfigFlags:
        break;

    case eConfigInt:
        draw_int_constraints(field);
        break;

    default:
        ImGui::TextDisabled("Unknown type");
        break;
    }
}

void draw_config_entry(cfg_field_t *field)
{
    const cfg_info_t *info = cfg_get_info(field);
    cfg_type_t type = cfg_get_type(field);

    ImGui::TableNextColumn();
    ImGui::AlignTextToFramePadding();
    ImGui::TreeNodeEx(info->name, kValueNodeFlags);

    ImGui::TableNextColumn();
    ImGui::TextUnformatted(cfg_type_name(type));

    ImGui::TableNextColumn();
    draw_value(field);

    ImGui::TableNextColumn();
    draw_constraints(field);

    ImGui::TableNextColumn();
    draw_field_info(field);
}

void draw_config_group(config_t *group);

void draw_config_group_children(config_t *group)
{
    vector_t *children = cfg_get_groups(group);
    size_t child_count = vector_len(children);
    for (size_t i = 0; i < child_count; ++i)
    {
        ImGui::TableNextRow();
        config_t *child = reinterpret_cast<config_t*>(vector_get(children, i));
        draw_config_group(child);
    }
}

void draw_config_group_fields(config_t *group)
{
    vector_t *fields = cfg_get_fields(group);
    size_t field_count = vector_len(fields);
    for (size_t i = 0; i < field_count; ++i)
    {
        ImGui::TableNextRow();
        cfg_field_t *field = reinterpret_cast<cfg_field_t*>(vector_get(fields, i));
        draw_config_entry(field);
    }
}

void draw_config_group(config_t *group)
{
    ImGui::PushID(group);

    ImGui::TableNextColumn();

    const cfg_info_t *info = cfg_group_info(group);
    bool is_group_open = ImGui::TreeNodeEx(info->name, kGroupNodeFlags);

    ImGui::TableNextColumn();
    ImGui::TextDisabled("--");

    ImGui::TableNextColumn();
    ImGui::TableNextColumn();
    ImGui::TableNextColumn();
    draw_group_info(group);

    if (is_group_open)
    {
        draw_config_group_children(group);
        draw_config_group_fields(group);

        ImGui::TreePop();
    }

    ImGui::PopID();
}

void ed::draw_config_panel(config_t *config)
{
    if (ImGui::BeginTable("Config", 5, kTableFlags))
    {
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Type");
        ImGui::TableSetupColumn("Value");
        ImGui::TableSetupColumn("Constraints");
        ImGui::TableSetupColumn("Info");

        ImGui::TableHeadersRow();

        draw_config_group_children(config);

        ImGui::EndTable();
    }
}
