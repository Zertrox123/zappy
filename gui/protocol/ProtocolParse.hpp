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

inline bool readPlayerId(std::istringstream &iss, int &id)
{
    std::string token;
    if (!(iss >> token))
        return false;
    if (!token.empty() && token.front() == '#')
        id = std::stoi(token.substr(1));
    else
        id = std::stoi(token);
    return true;
}
} // namespace protocol
