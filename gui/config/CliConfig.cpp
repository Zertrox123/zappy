#include "config/CliConfig.hpp"

#include <cstdio>
#include <string_view>

namespace
{
bool parsePort(std::string_view value, int &port)
{
    if (value.empty())
        return false;
    try
    {
        std::size_t consumed = 0;
        const long parsed = std::stol(std::string(value), &consumed);
        if (consumed != value.size() || parsed <= 0 || parsed > 65535)
            return false;
        port = static_cast<int>(parsed);
        return true;
    }
    catch (...)
    {
        return false;
    }
}
} // namespace

std::optional<CliConfig> CliConfig::parse(int argc, char **argv)
{
    CliConfig config;

    for (int i = 1; i < argc; ++i)
    {
        const std::string_view flag(argv[i]);
        if (flag == "-p")
        {
            if (i + 1 >= argc || !parsePort(argv[i + 1], config.port))
                return std::nullopt;
            ++i;
        }
        else if (flag == "-h")
        {
            if (i + 1 >= argc || argv[i + 1][0] == '\0')
                return std::nullopt;
            config.host = argv[i + 1];
            ++i;
        }
        else
        {
            return std::nullopt;
        }
    }

    if (config.port <= 0 || config.host.empty())
        return std::nullopt;

    return config;
}

void CliConfig::printUsage(std::string_view program)
{
    std::fprintf(stderr, "USAGE: %.*s -p port -h machine\n",
                 static_cast<int>(program.size()), program.data());
    std::fprintf(stderr, "  -p port     port number\n");
    std::fprintf(stderr, "  -h machine  hostname of the server\n");
}
