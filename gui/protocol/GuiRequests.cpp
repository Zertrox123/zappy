#include "protocol/GuiRequests.hpp"

#include <sstream>

namespace
{
bool sendLine(NetworkClient &client, const std::string &line)
{
    return client.isConnected() && client.sendRaw(line);
}
} // namespace

namespace GuiRequests
{
bool sendMsz(NetworkClient &client) { return sendLine(client, "msz\n"); }

bool sendMct(NetworkClient &client) { return sendLine(client, "mct\n"); }

bool sendTna(NetworkClient &client) { return sendLine(client, "tna\n"); }

bool sendBct(NetworkClient &client, int x, int y)
{
    std::ostringstream out;
    out << "bct " << x << ' ' << y << '\n';
    return sendLine(client, out.str());
}

bool sendPpo(NetworkClient &client, int playerId)
{
    std::ostringstream out;
    out << "ppo #" << playerId << '\n';
    return sendLine(client, out.str());
}

bool sendPlv(NetworkClient &client, int playerId)
{
    std::ostringstream out;
    out << "plv #" << playerId << '\n';
    return sendLine(client, out.str());
}

bool sendPin(NetworkClient &client, int playerId)
{
    std::ostringstream out;
    out << "pin #" << playerId << '\n';
    return sendLine(client, out.str());
}

bool sendSgt(NetworkClient &client) { return sendLine(client, "sgt\n"); }

bool sendSst(NetworkClient &client, int timeUnit)
{
    std::ostringstream out;
    out << "sst " << timeUnit << '\n';
    return sendLine(client, out.str());
}

bool requestMapSync(NetworkClient &client)
{
    return sendMct(client) && sendTna(client) && sendSgt(client);
}

bool requestPlayerInfo(NetworkClient &client, int playerId)
{
    return sendPpo(client, playerId) && sendPlv(client, playerId) &&
           sendPin(client, playerId);
}
} // namespace GuiRequests
