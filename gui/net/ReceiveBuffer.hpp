#pragma once

#include <string>
#include <string_view>

class ReceiveBuffer
{
  public:
    void append(std::string_view data);
    bool hasLine() const;
    std::string popLine();
    std::string drain() const;
    bool containsLineStartingWith(std::string_view prefix) const;

  private:
    std::string _buffer;
};
