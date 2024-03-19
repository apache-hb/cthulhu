#include "editor/panels/settings.hpp"

using namespace ed;

static int gMemoryFormat = eIEC;

static constexpr const char *kMemoryFormatNames[eFormatCount] = {
    "IEC",
    "SI",
};

void SettingsPanel::draw_content()
{
    ImGui::Text("Display");

    ImGui::Combo("Memory Format", &gMemoryFormat, kMemoryFormatNames, eFormatCount);
}

memory_format_t ed::get_memory_format()
{
    return (memory_format_t)gMemoryFormat;
}

void ed::format_memory(uintmax_t value, char *buffer)
{
    ed::memory_to_chars(value, buffer, get_memory_format());
}
