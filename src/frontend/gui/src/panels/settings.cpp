#include "editor/panels/settings.hpp"
#include "editor/utils.hpp"

#include "std/str.h"

using namespace ed;

static int gMemoryFormat = eIEC;
static char gThousandsSeparator = ',';

static constexpr const char *kMemoryFormatNames[eFormatCount] = {
    "IEC (base 10)",
    "SI (base 2)",
};

void SettingsPanel::draw_content()
{
    ImGui::Text("Display");

    ImGui::Combo("Memory Format", &gMemoryFormat, kMemoryFormatNames, eFormatCount);
}

SettingsPanel::SettingsPanel()
    : IEditorPanel("Settings")
{ }

char ed::get_thousands_separator()
{
    return gThousandsSeparator;
}

memory_format_t ed::get_memory_format()
{
    return (memory_format_t)gMemoryFormat;
}

void ed::format_memory(uintmax_t value, char *buffer)
{
    ed::memory_to_chars(value, buffer, get_memory_format());
}

size_t ed::format_large_number(intmax_t value, char *buffer, char seperator)
{
    // format a string, inserting the thousands separator every 3 digits
    size_t len = str_sprintf(buffer, 64, "%jd", value);
    size_t offset = len % 3;

    for (size_t i = 0; i < len; i++)
    {
        if (i > 0 && (i - offset) % 3 == 0)
        {
            buffer[i] = seperator;
            offset--;
        }
        else
        {
            buffer[i] = buffer[i - offset];
        }
    }

    return len + (len / 3);
}