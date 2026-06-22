#pragma once

#include <cstdlib>

inline bool isHeadlessMode()
{
    const char *value = std::getenv("ZAPPY_GUI_HEADLESS");
    return value != nullptr && value[0] == '1';
}
