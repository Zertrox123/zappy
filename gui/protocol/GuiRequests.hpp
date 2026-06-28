#pragma once

#include "net/NetworkClient.hpp"

namespace GuiRequests
{
bool sendMsz(NetworkClient &client);
bool sendMct(NetworkClient &client);
bool sendTna(NetworkClient &client);
bool sendBct(NetworkClient &client, int x, int y);
bool sendPpo(NetworkClient &client, int playerId);
bool sendPlv(NetworkClient &client, int playerId);
bool sendPin(NetworkClient &client, int playerId);
bool sendSgt(NetworkClient &client);
bool sendSst(NetworkClient &client, int timeUnit);

bool requestMapSync(NetworkClient &client);
bool requestPlayerInfo(NetworkClient &client, int playerId);
} // namespace GuiRequests
