#include "protocol/ProtocolParser.hpp"

void ProtocolParser::consume(ReceiveBuffer &buffer, GameState &state)
{
    while (buffer.hasLine())
        drainLine(buffer.popLine(), state);
}

void ProtocolParser::drainLine(const std::string &line, GameState &state)
{
    state.applyLine(line);
}
