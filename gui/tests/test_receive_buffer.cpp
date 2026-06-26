#include "net/ReceiveBuffer.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

namespace
{
bool expect(bool condition, const char *message)
{
    if (!condition)
    {
        std::cerr << "[test_receive_buffer] " << message << '\n';
        return false;
    }
    return true;
}
} // namespace

int main()
{
    ReceiveBuffer buffer;

    buffer.append("hel");
    buffer.append("lo\nworld");

    if (!expect(buffer.hasLine(), "completed line must be available"))
        return EXIT_FAILURE;

    const std::string first = buffer.popLine();
    if (!expect(first == "hello", "popLine must return complete line without delimiter"))
        return EXIT_FAILURE;

    if (!expect(!buffer.hasLine(), "tail without newline must not be a full line"))
        return EXIT_FAILURE;

    if (!expect(buffer.drain() == "world", "remaining bytes must stay in buffer"))
        return EXIT_FAILURE;

    buffer.append("!\n");
    if (!expect(buffer.hasLine(), "appended newline must expose a line"))
        return EXIT_FAILURE;

    const std::string second = buffer.popLine();
    if (!expect(second == "world!", "second line must include prior partial data"))
        return EXIT_FAILURE;

    if (!expect(buffer.drain().empty(), "buffer must be empty after draining line"))
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}
