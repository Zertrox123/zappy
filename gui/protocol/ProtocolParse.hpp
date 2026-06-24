#pragma once

#include <sstream>

namespace protocol
{
inline bool readResources(std::istringstream &iss, int (&out)[7])
{
    for (int i = 0; i < 7; ++i)
    {
        if (!(iss >> out[i]))
            return false;
    }
    return true;
}
} // namespace protocol
