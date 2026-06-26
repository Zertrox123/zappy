#pragma once

#include "model/GameState.hpp"
#include "net/ReceiveBuffer.hpp"

class ProtocolParser
{
  public:
    void consume(ReceiveBuffer &buffer, GameState &state);

  private:
    void drainLine(const std::string &line, GameState &state);
};
