#pragma once

#include "model/GameState.hpp"
#include "net/NetworkClient.hpp"
#include "net/ReceiveBuffer.hpp"
#include "protocol/ProtocolParser.hpp"

#include <chrono>

class MapSync
{
  public:
    static void request(NetworkClient &client);
    static void flush(NetworkClient &client, ReceiveBuffer &buffer,
                      ProtocolParser &parser, GameState &state,
                      std::chrono::milliseconds timeout);
};
