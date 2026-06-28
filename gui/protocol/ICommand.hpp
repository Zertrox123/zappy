#pragma once

#include "model/GameState.hpp"

#include <sstream>
#include <string>

class ICommand
{
  public:
    virtual ~ICommand() = default;

    virtual std::string keyword() const = 0;
    virtual void execute(std::istringstream &iss, GameState &state) = 0;
};
