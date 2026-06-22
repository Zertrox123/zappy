#include "net/ReceiveBuffer.hpp"

#include <string_view>

void ReceiveBuffer::append(std::string_view data) { _buffer.append(data); }

bool ReceiveBuffer::hasLine() const
{
    return _buffer.find('\n') != std::string::npos;
}

std::string ReceiveBuffer::popLine()
{
    const std::size_t pos = _buffer.find('\n');
    if (pos == std::string::npos)
        return {};

    std::string line = _buffer.substr(0, pos);
    _buffer.erase(0, pos + 1);
    return line;
}

std::string ReceiveBuffer::drain() const { return _buffer; }

bool ReceiveBuffer::containsLineStartingWith(std::string_view prefix) const
{
    std::size_t pos = 0;
    while (pos < _buffer.size())
    {
        const std::size_t end = _buffer.find('\n', pos);
        const std::size_t lineEnd =
            end == std::string::npos ? _buffer.size() : end;
        const std::string_view line(_buffer.data() + pos, lineEnd - pos);
        if (line.rfind(prefix, 0) == 0)
            return true;
        if (end == std::string::npos)
            break;
        pos = end + 1;
    }
    return false;
}
