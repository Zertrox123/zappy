#include "render/UiFont.hpp"

#include <cstdlib>
#include <iostream>

int main()
{
    if (!UiFont::available())
    {
        std::cerr << "[FAIL] bundled UI font must load\n";
        return EXIT_FAILURE;
    }

    std::cout << "[OK] ui font tests passed\n";
    return EXIT_SUCCESS;
}
