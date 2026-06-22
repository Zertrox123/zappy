#include "app/GuiApp.hpp"

#include "config/Headless.hpp"
#include "protocol/GraphicHandshake.hpp"
#include "render/GuiWindow.hpp"

#include <chrono>
#include <cstdio>
#include <utility>

namespace
{
void logInfo(const char *message)
{
    std::fprintf(stderr, "[zappy_gui] %s\n", message);
}

void logError(const char *message)
{
    std::fprintf(stderr, "[zappy_gui] error: %s\n", message);
}

const char *handshakeError(HandshakeResult result)
{
    switch (result)
    {
    case HandshakeResult::Ok:
        return "ok";
    case HandshakeResult::Timeout:
        return "timed out waiting for server data";
    case HandshakeResult::Disconnected:
        return "server closed the connection";
    case HandshakeResult::SendFailed:
        return "failed to send GRAPHIC";
    }
    return "unknown handshake failure";
}

void printBufferedLines(const ReceiveBuffer &buffer)
{
    std::string pending = buffer.drain();
    std::size_t pos = 0;
    int count = 0;
    while (pos < pending.size() && count < 5)
    {
        const std::size_t end = pending.find('\n', pos);
        const std::size_t lineEnd =
            end == std::string::npos ? pending.size() : end;
        std::fprintf(stderr, "[zappy_gui] server: %.*s\n",
                     static_cast<int>(lineEnd - pos), pending.data() + pos);
        if (end == std::string::npos)
            break;
        pos = end + 1;
        ++count;
    }
}
} // namespace

GuiApp::GuiApp(CliConfig config) : _config(std::move(config)) {}

int GuiApp::run()
{
    std::fprintf(stderr, "[zappy_gui] connecting to %s:%d...\n",
                 _config.host.c_str(), _config.port);

    if (!_client.connect(_config.host, _config.port))
    {
        logError("could not connect to server");
        return kExitUsage;
    }

    logInfo("connected");

    const HandshakeResult result =
        GraphicHandshake(_client, _buffer).run(std::chrono::milliseconds(3000));

    if (result != HandshakeResult::Ok)
    {
        std::fprintf(stderr, "[zappy_gui] error: handshake %s\n",
                     handshakeError(result));
        return kExitUsage;
    }

    logInfo("handshake complete (WELCOME + GRAPHIC + msz)");
    printBufferedLines(_buffer);

    if (isHeadlessMode())
    {
        logInfo("headless mode: skipping window");
        return kExitOk;
    }

    logInfo("opening map window (close or Escape to quit)");
    return GuiWindow(_client, _buffer, _config.host, _config.port).run();
}
