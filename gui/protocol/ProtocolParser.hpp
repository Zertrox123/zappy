#pragma once

#include "model/GameState.hpp"
#include "net/ReceiveBuffer.hpp"
#include "protocol/ICommand.hpp"

#include <memory>
#include <string>
#include <vector>

class ProtocolParser
{
  public:
    ProtocolParser();

    void consume(ReceiveBuffer &buffer, GameState &state);
    void parseLine(const std::string &line, GameState &state);

  private:
    std::vector<std::unique_ptr<ICommand>> _commands;

    void drainLine(const std::string &line, GameState &state);
};
