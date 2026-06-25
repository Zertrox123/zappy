#include "protocol/MapSync.hpp"

#include <algorithm>
#include <string_view>
#include <thread>

void MapSync::request(NetworkClient &client)
{
    if (!client.isConnected())
        return;
    client.sendRaw("mct\n");
    client.sendRaw("tna\n");
    client.sendRaw("sgt\n");
}

void MapSync::flush(NetworkClient &client, ReceiveBuffer &buffer,
                    ProtocolParser &parser, GameState &state,
                    std::chrono::milliseconds timeout)
{
    request(client);

    const auto deadline = std::chrono::steady_clock::now() + timeout;
    const int target = std::max(0, state.width * state.height);

    while (std::chrono::steady_clock::now() < deadline)
    {
        char chunk[4096];
        while (client.isConnected())
        {
            const int n = client.recvRaw(chunk, sizeof(chunk));
            if (n < 0)
                break;
            if (n == 0)
                goto wait;
            buffer.append(std::string_view(chunk, static_cast<std::size_t>(n)));
        }
        parser.consume(buffer, state);

        if (target > 0 && state.knownTileCount() >= target)
            return;

    wait:
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
