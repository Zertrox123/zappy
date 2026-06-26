#pragma once

#include <optional>
#include <string>
#include <string_view>

struct CliConfig
{
    int port = -1;
    std::string host;

    static std::optional<CliConfig> parse(int argc, char **argv);
    static void printUsage(std::string_view program);
};
