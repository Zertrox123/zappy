#include "protocol/GraphicHandshake.hpp"

#include "net/NetworkClient.hpp"
#include "net/ReceiveBuffer.hpp"

#include <chrono>
#include <string>
#include <string_view>
#include <thread>

GraphicHandshake::GraphicHandshake(NetworkClient &client, ReceiveBuffer &buffer)
    : _client(client), _buffer(buffer)
{
}

void GraphicHandshake::pullAvailable()
{
    char chunk[4096];

    while (_client.isConnected())
    {
        const int n = _client.recvRaw(chunk, sizeof(chunk));
        if (n < 0)
            break;
        if (n == 0)
            return;
        _buffer.append(std::string_view(chunk, static_cast<std::size_t>(n)));
    }
}

bool GraphicHandshake::waitForWelcome(std::chrono::milliseconds welcomeTimeout)
{
    const auto deadline = std::chrono::steady_clock::now() + welcomeTimeout;

    while (std::chrono::steady_clock::now() < deadline)
    {
        pullAvailable();

        while (_buffer.hasLine())
        {
            const std::string line = _buffer.popLine();
            if (line == "WELCOME")
                return true;
        }

        if (!_client.isConnected())
            return false;

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return false;
}

bool GraphicHandshake::waitForMapSize(std::chrono::milliseconds timeout)
{
    const auto deadline = std::chrono::steady_clock::now() + timeout;

    while (std::chrono::steady_clock::now() < deadline)
    {
        pullAvailable();

        if (_buffer.containsLineStartingWith("msz"))
            return true;

        if (!_client.isConnected())
            return false;

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return false;
}

HandshakeResult GraphicHandshake::run(std::chrono::milliseconds welcomeTimeout)
{
    if (!waitForWelcome(welcomeTimeout))
        return _client.isConnected() ? HandshakeResult::Timeout
                                     : HandshakeResult::Disconnected;

    if (!_client.sendRaw("GRAPHIC\n"))
        return HandshakeResult::SendFailed;

    if (!waitForMapSize(std::chrono::milliseconds(3000)))
        return _client.isConnected() ? HandshakeResult::Timeout
                                     : HandshakeResult::Disconnected;

    pullAvailable();

    return HandshakeResult::Ok;
}
