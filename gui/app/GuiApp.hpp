#pragma once

#include "config/CliConfig.hpp"
#include "net/NetworkClient.hpp"
#include "net/ReceiveBuffer.hpp"

class GuiApp
{
  public:
    explicit GuiApp(CliConfig config);

    int run();

    const ReceiveBuffer &buffer() const { return _buffer; }

  private:
    static constexpr int kExitUsage = 84;
    static constexpr int kExitOk = 0;

    CliConfig _config;
    NetworkClient _client;
    ReceiveBuffer _buffer;
};
