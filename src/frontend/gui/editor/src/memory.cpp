#include "stdafx.hpp"

#include "editor/memory.hpp"

std::string Memory::to_string() const
{
    if (mBytes == 0) { return "0b"; }

    std::stringstream ss;
    size_t total = mBytes;

    // seperate each part with a +

    for (int fmt = eLimit - 1; fmt >= 0; fmt--)
    {
        size_t size = total / kSizes[fmt];
        if (size > 0)
        {
            ss << size << kNames[fmt];
            total %= kSizes[fmt];

            if (total > 0)
                ss << "+";
        }
    }

    return ss.str();
}
