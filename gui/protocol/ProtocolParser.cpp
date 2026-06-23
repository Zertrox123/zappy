#include "protocol/ProtocolParser.hpp"

#include "protocol/Commands.hpp"

#include <sstream>

ProtocolParser::ProtocolParser()
{
    _commands.push_back(std::make_unique<MszCommand>());
    _commands.push_back(std::make_unique<SgtCommand>());
    _commands.push_back(std::make_unique<TnaCommand>());
    _commands.push_back(std::make_unique<BctCommand>());
    _commands.push_back(std::make_unique<PnwCommand>());
    _commands.push_back(std::make_unique<PpoCommand>());
    _commands.push_back(std::make_unique<PlvCommand>());
    _commands.push_back(std::make_unique<PinCommand>());
    _commands.push_back(std::make_unique<PdiCommand>());
    _commands.push_back(std::make_unique<EnwCommand>());
    _commands.push_back(std::make_unique<EboCommand>());
    _commands.push_back(std::make_unique<EdiCommand>());
    _commands.push_back(std::make_unique<SegCommand>());
    _commands.push_back(std::make_unique<MctCommand>());
    _commands.push_back(std::make_unique<PexCommand>());
    _commands.push_back(std::make_unique<PbcCommand>());
    _commands.push_back(std::make_unique<PicCommand>());
    _commands.push_back(std::make_unique<PieCommand>());
    _commands.push_back(std::make_unique<SmgCommand>());
}

void ProtocolParser::consume(ReceiveBuffer &buffer, GameState &state)
{
    while (buffer.hasLine())
        drainLine(buffer.popLine(), state);
}

void ProtocolParser::parseLine(const std::string &line, GameState &state)
{
    drainLine(line, state);
}

void ProtocolParser::drainLine(const std::string &line, GameState &state)
{
    if (line.empty())
        return;

    std::istringstream iss(line);
    std::string cmd;
    iss >> cmd;

    for (const std::unique_ptr<ICommand> &command : _commands)
    {
        if (command->keyword() == cmd)
        {
            command->execute(iss, state);
            return;
        }
    }
}
