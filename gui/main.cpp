#include "app/GuiApp.hpp"
#include "config/CliConfig.hpp"

int main(int argc, char **argv)
{
    const auto config = CliConfig::parse(argc, argv);
    if (!config)
    {
        CliConfig::printUsage(argv[0]);
        return 84;
    }

    return GuiApp(*config).run();
}
