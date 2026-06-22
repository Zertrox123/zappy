#include "net/NetworkClient.hpp"

#include <cerrno>
#include <fcntl.h>
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

NetworkClient::NetworkClient() : _fd(-1), _connected(false) {}

NetworkClient::~NetworkClient() { disconnect(); }

bool NetworkClient::connect(const std::string &host, int port)
{
    addrinfo hints{};
    addrinfo *result = nullptr;

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    const std::string portStr = std::to_string(port);
    if (getaddrinfo(host.c_str(), portStr.c_str(), &hints, &result) != 0)
        return false;

    _fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (_fd < 0)
    {
        freeaddrinfo(result);
        return false;
    }

    if (::connect(_fd, result->ai_addr, result->ai_addrlen) < 0)
    {
        freeaddrinfo(result);
        close(_fd);
        _fd = -1;
        return false;
    }

    freeaddrinfo(result);

    const int flags = fcntl(_fd, F_GETFL, 0);
    if (flags < 0 || fcntl(_fd, F_SETFL, flags | O_NONBLOCK) < 0)
    {
        close(_fd);
        _fd = -1;
        return false;
    }

    _connected = true;
    return true;
}

void NetworkClient::disconnect()
{
    if (_fd >= 0)
    {
        close(_fd);
        _fd = -1;
    }
    _connected = false;
}

bool NetworkClient::isConnected() const { return _connected && _fd >= 0; }

bool NetworkClient::sendRaw(std::string_view data)
{
    if (!isConnected() || data.empty())
        return false;

    std::size_t sent = 0;
    while (sent < data.size())
    {
        const ssize_t n =
            ::send(_fd, data.data() + sent, data.size() - sent, 0);
        if (n < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                continue;
            _connected = false;
            return false;
        }
        if (n == 0)
        {
            _connected = false;
            return false;
        }
        sent += static_cast<std::size_t>(n);
    }

    return true;
}

int NetworkClient::recvRaw(char *buffer, std::size_t maxSize)
{
    if (!isConnected() || maxSize == 0)
        return -1;

    const ssize_t n = ::recv(_fd, buffer, maxSize, 0);
    if (n < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return -1;
        _connected = false;
        return 0;
    }
    if (n == 0)
    {
        _connected = false;
        return 0;
    }

    return static_cast<int>(n);
}
