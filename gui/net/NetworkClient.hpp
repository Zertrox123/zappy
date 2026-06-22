#pragma once

#include <cstddef>
#include <string>
#include <string_view>

class NetworkClient
{
  public:
    NetworkClient();
    ~NetworkClient();

    NetworkClient(const NetworkClient &) = delete;
    NetworkClient &operator=(const NetworkClient &) = delete;

    bool connect(const std::string &host, int port);
    void disconnect();
    bool isConnected() const;

    bool sendRaw(std::string_view data);
    int recvRaw(char *buffer, std::size_t maxSize);

  private:
    int _fd;
    bool _connected;
};
