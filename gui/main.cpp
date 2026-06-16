#include <cstdio>
#include <cstring>
#include <string>

static void printUsage(const char *bin)
{
    std::fprintf(stderr, "USAGE: %s -p port -h machine\n", bin);
    std::fprintf(stderr, "  -p port     port number\n");
    std::fprintf(stderr, "  -h machine  hostname of the server\n");
}

static bool parseArgs(int argc, char **argv, int &port, std::string &host)
{
    for (int i = 1; i < argc - 1; i++) {
        if (std::strcmp(argv[i], "-p") == 0)
            port = std::stoi(argv[i + 1]);
        else if (std::strcmp(argv[i], "-h") == 0)
            host = argv[i + 1];
    }
    return port > 0 && !host.empty();
}

int main(int argc, char **argv)
{
    int port = -1;
    std::string host;

    if (argc < 5 || !parseArgs(argc, argv, port, host)) {
        printUsage(argv[0]);
        return 84;
    }

    std::printf("zappy_gui: OK\n");
    return 0;
}
