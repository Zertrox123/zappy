#pragma once

#include "protocol/ICommand.hpp"

class MszCommand : public ICommand
{
  public:
    std::string keyword() const override;
    void execute(std::istringstream &iss, GameState &state) override;
};

class SgtCommand : public ICommand
{
  public:
    std::string keyword() const override;
    void execute(std::istringstream &iss, GameState &state) override;
};

class TnaCommand : public ICommand
{
  public:
    std::string keyword() const override;
    void execute(std::istringstream &iss, GameState &state) override;
};

class BctCommand : public ICommand
{
  public:
    std::string keyword() const override;
    void execute(std::istringstream &iss, GameState &state) override;
};

class PnwCommand : public ICommand
{
  public:
    std::string keyword() const override;
    void execute(std::istringstream &iss, GameState &state) override;
};

class PpoCommand : public ICommand
{
  public:
    std::string keyword() const override;
    void execute(std::istringstream &iss, GameState &state) override;
};

class PlvCommand : public ICommand
{
  public:
    std::string keyword() const override;
    void execute(std::istringstream &iss, GameState &state) override;
};

class PinCommand : public ICommand
{
  public:
    std::string keyword() const override;
    void execute(std::istringstream &iss, GameState &state) override;
};

class PdiCommand : public ICommand
{
  public:
    std::string keyword() const override;
    void execute(std::istringstream &iss, GameState &state) override;
};

class EnwCommand : public ICommand
{
  public:
    std::string keyword() const override;
    void execute(std::istringstream &iss, GameState &state) override;
};

class EboCommand : public ICommand
{
  public:
    std::string keyword() const override;
    void execute(std::istringstream &iss, GameState &state) override;
};

class EdiCommand : public ICommand
{
  public:
    std::string keyword() const override;
    void execute(std::istringstream &iss, GameState &state) override;
};

class SegCommand : public ICommand
{
  public:
    std::string keyword() const override;
    void execute(std::istringstream &iss, GameState &state) override;
};

class MctCommand : public ICommand
{
  public:
    std::string keyword() const override;
    void execute(std::istringstream &iss, GameState &state) override;
};

class PexCommand : public ICommand
{
  public:
    std::string keyword() const override;
    void execute(std::istringstream &iss, GameState &state) override;
};

class PbcCommand : public ICommand
{
  public:
    std::string keyword() const override;
    void execute(std::istringstream &iss, GameState &state) override;
};

class PicCommand : public ICommand
{
  public:
    std::string keyword() const override;
    void execute(std::istringstream &iss, GameState &state) override;
};

class PieCommand : public ICommand
{
  public:
    std::string keyword() const override;
    void execute(std::istringstream &iss, GameState &state) override;
};

class PfkCommand : public ICommand
{
  public:
    std::string keyword() const override;
    void execute(std::istringstream &iss, GameState &state) override;
};

class PdrCommand : public ICommand
{
  public:
    std::string keyword() const override;
    void execute(std::istringstream &iss, GameState &state) override;
};

class PgtCommand : public ICommand
{
  public:
    std::string keyword() const override;
    void execute(std::istringstream &iss, GameState &state) override;
};

class SmgCommand : public ICommand
{
  public:
    std::string keyword() const override;
    void execute(std::istringstream &iss, GameState &state) override;
};

class SstCommand : public ICommand
{
  public:
    std::string keyword() const override;
    void execute(std::istringstream &iss, GameState &state) override;
};

class SucCommand : public ICommand
{
  public:
    std::string keyword() const override;
    void execute(std::istringstream &iss, GameState &state) override;
};

class SbpCommand : public ICommand
{
  public:
    std::string keyword() const override;
    void execute(std::istringstream &iss, GameState &state) override;
};
