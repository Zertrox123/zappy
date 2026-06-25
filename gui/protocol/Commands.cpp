#include "protocol/Commands.hpp"

#include "protocol/ProtocolParse.hpp"

std::string MszCommand::keyword() const { return "msz"; }

void MszCommand::execute(std::istringstream &iss, GameState &state)
{
    int w = 0;
    int h = 0;
    iss >> w >> h;
    state.resize(w, h);
}

std::string SgtCommand::keyword() const { return "sgt"; }

void SgtCommand::execute(std::istringstream &iss, GameState &state)
{
    iss >> state.timeUnit;
}

std::string TnaCommand::keyword() const { return "tna"; }

void TnaCommand::execute(std::istringstream &iss, GameState &state)
{
    std::string team;
    iss >> team;
    if (!team.empty())
        state.addTeam(team);
}

std::string BctCommand::keyword() const { return "bct"; }

void BctCommand::execute(std::istringstream &iss, GameState &state)
{
    int x = 0;
    int y = 0;
    if (!(iss >> x >> y))
        return;
    Tile tile{};
    protocol::readResources(iss, tile.resources);
    state.setTile(x, y, tile);
    state.noteTileKnown();
}

std::string PnwCommand::keyword() const { return "pnw"; }

void PnwCommand::execute(std::istringstream &iss, GameState &state)
{
    Player player;
    std::string playerNum;
    iss >> playerNum >> player.id >> player.x >> player.y >>
        player.orientation >> player.level >> player.team;
    state.setPlayer(player);
}

std::string PpoCommand::keyword() const { return "ppo"; }

void PpoCommand::execute(std::istringstream &iss, GameState &state)
{
    int id = 0;
    std::string playerNum;
    iss >> playerNum >> id;
    Player &player = state.playerOrCreate(id);
    player.id = id;
    iss >> player.x >> player.y >> player.orientation;
}

std::string PlvCommand::keyword() const { return "plv"; }

void PlvCommand::execute(std::istringstream &iss, GameState &state)
{
    int id = 0;
    int level = 0;
    std::string playerNum;
    iss >> playerNum >> id >> level;
    state.setPlayerLevel(id, level);
}

std::string PinCommand::keyword() const { return "pin"; }

void PinCommand::execute(std::istringstream &iss, GameState &state)
{
    int id = 0;
    std::string playerNum;
    iss >> playerNum >> id;
    Player &player = state.playerOrCreate(id);
    player.id = id;
    iss >> player.x >> player.y;
    protocol::readResources(iss, player.inventory);
}

std::string PdiCommand::keyword() const { return "pdi"; }

void PdiCommand::execute(std::istringstream &iss, GameState &state)
{
    int id = 0;
    if (!protocol::readPlayerId(iss, id))
        return;
    const Player *player = state.findPlayer(id);
    if (player != nullptr)
    {
        WorldEffect effect{};
        effect.kind = EffectKind::Death;
        effect.playerId = id;
        effect.x = player->x;
        effect.y = player->y;
        state.pushEffect(std::move(effect));
    }
    state.removePlayer(id);
}

std::string EnwCommand::keyword() const { return "enw"; }

void EnwCommand::execute(std::istringstream &iss, GameState &state)
{
    Egg egg;
    std::string eggNum;
    std::string playerNum;
    iss >> eggNum >> egg.id >> playerNum >> egg.playerId >> egg.x >> egg.y;
    state.setEgg(egg);
}

std::string EboCommand::keyword() const { return "ebo"; }

void EboCommand::execute(std::istringstream &iss, GameState &state)
{
    int id = 0;
    std::string eggNum;
    iss >> eggNum >> id;
    state.removeEgg(id);
}

std::string EdiCommand::keyword() const { return "edi"; }

void EdiCommand::execute(std::istringstream &iss, GameState &state)
{
    int id = 0;
    std::string eggNum;
    iss >> eggNum >> id;
    state.removeEgg(id);
}

std::string SegCommand::keyword() const { return "seg"; }

void SegCommand::execute(std::istringstream &iss, GameState &state)
{
    std::string winner;
    iss >> winner;
    state.setWinner(std::move(winner));
}

std::string MctCommand::keyword() const { return "mct"; }

void MctCommand::execute(std::istringstream &, GameState &state)
{
    state.resetKnownTiles();
}

std::string PexCommand::keyword() const { return "pex"; }

void PexCommand::execute(std::istringstream &iss, GameState &state)
{
    int id = 0;
    if (!protocol::readPlayerId(iss, id))
        return;
    const Player *player = state.findPlayer(id);
    if (player == nullptr)
        return;
    WorldEffect effect{};
    effect.kind = EffectKind::Expulsion;
    effect.playerId = id;
    effect.x = player->x;
    effect.y = player->y;
    state.pushEffect(std::move(effect));
}

std::string PbcCommand::keyword() const { return "pbc"; }

void PbcCommand::execute(std::istringstream &iss, GameState &state)
{
    int id = 0;
    if (!protocol::readPlayerId(iss, id))
        return;
    std::string message;
    std::getline(iss, message);
    if (!message.empty() && message.front() == ' ')
        message.erase(message.begin());
    const Player *player = state.findPlayer(id);
    if (player == nullptr)
        return;
    WorldEffect effect{};
    effect.kind = EffectKind::Broadcast;
    effect.playerId = id;
    effect.x = player->x;
    effect.y = player->y;
    effect.message = std::move(message);
    state.pushEffect(std::move(effect));
}

std::string PicCommand::keyword() const { return "pic"; }

void PicCommand::execute(std::istringstream &iss, GameState &state)
{
    int x = 0;
    int y = 0;
    int level = 0;
    std::string playerNum;
    iss >> x >> y >> level;
    WorldEffect effect{};
    effect.kind = EffectKind::Incantation;
    effect.x = x;
    effect.y = y;
    effect.level = level;
    while (iss >> playerNum)
    {
        if (playerNum.empty() || playerNum.front() != '#')
            continue;
        effect.participants.push_back(std::stoi(playerNum.substr(1)));
    }
    if (!effect.participants.empty())
        effect.playerId = effect.participants.front();
    state.pushEffect(std::move(effect));
}

std::string PieCommand::keyword() const { return "pie"; }

void PieCommand::execute(std::istringstream &iss, GameState &state)
{
    int x = 0;
    int y = 0;
    int result = 0;
    iss >> x >> y >> result;
    state.clearIncantationsAt(x, y);
    WorldEffect effect{};
    effect.kind = EffectKind::IncantationEnd;
    effect.x = x;
    effect.y = y;
    effect.success = result != 0;
    state.pushEffect(std::move(effect));
}

std::string SmgCommand::keyword() const { return "smg"; }

void SmgCommand::execute(std::istringstream &iss, GameState &state)
{
    std::string message;
    std::getline(iss, message);
    if (!message.empty() && message.front() == ' ')
        message.erase(message.begin());
    if (message.find("Paused") != std::string::npos)
        state.setPaused(true);
    if (message.find("Resumed") != std::string::npos)
        state.setPaused(false);
    state.pushServerMessage(std::move(message));
}

std::string PfkCommand::keyword() const { return "pfk"; }

void PfkCommand::execute(std::istringstream &iss, GameState &state)
{
    int id = 0;
    if (!protocol::readPlayerId(iss, id))
        return;
    const Player *player = state.findPlayer(id);
    if (player == nullptr)
        return;
    WorldEffect effect{};
    effect.kind = EffectKind::Fork;
    effect.playerId = id;
    effect.x = player->x;
    effect.y = player->y;
    state.pushEffect(std::move(effect));
}

std::string PdrCommand::keyword() const { return "pdr"; }

void PdrCommand::execute(std::istringstream &iss, GameState &state)
{
    int id = 0;
    int resource = 0;
    if (!protocol::readPlayerId(iss, id))
        return;
    iss >> resource;
    const Player *player = state.findPlayer(id);
    if (player == nullptr)
        return;
    WorldEffect effect{};
    effect.kind = EffectKind::ResourceDrop;
    effect.playerId = id;
    effect.x = player->x;
    effect.y = player->y;
    effect.resource = resource;
    state.pushEffect(std::move(effect));
}

std::string PgtCommand::keyword() const { return "pgt"; }

void PgtCommand::execute(std::istringstream &iss, GameState &state)
{
    int id = 0;
    int resource = 0;
    if (!protocol::readPlayerId(iss, id))
        return;
    iss >> resource;
    const Player *player = state.findPlayer(id);
    if (player == nullptr)
        return;
    WorldEffect effect{};
    effect.kind = EffectKind::ResourceTake;
    effect.playerId = id;
    effect.x = player->x;
    effect.y = player->y;
    effect.resource = resource;
    state.pushEffect(std::move(effect));
}
