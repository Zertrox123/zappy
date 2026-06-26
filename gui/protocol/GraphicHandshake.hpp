#pragma once

#include <chrono>

class NetworkClient;
class ReceiveBuffer;

enum class HandshakeResult
{
    Ok,
    Timeout,
    Disconnected,
    SendFailed,
};

class GraphicHandshake
{
  public:
    GraphicHandshake(NetworkClient &client, ReceiveBuffer &buffer);

    HandshakeResult run(std::chrono::milliseconds welcomeTimeout);

  private:
    NetworkClient &_client;
    ReceiveBuffer &_buffer;

    void pullAvailable();
    bool waitForWelcome(std::chrono::milliseconds welcomeTimeout);
    bool waitForMapSize(std::chrono::milliseconds timeout);
};
