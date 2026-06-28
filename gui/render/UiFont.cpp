#include "render/UiFont.hpp"

namespace
{
const char *kFontPaths[] = {
    "gui/assets/UiFont.ttf",
    "assets/UiFont.ttf",
    "../gui/assets/UiFont.ttf",
    "/System/Library/Fonts/Supplemental/Arial.ttf",
    "/Library/Fonts/Arial.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
};
} // namespace

UiFont::UiFont()
{
    for (const char *path : kFontPaths)
    {
        if (_font.loadFromFile(path))
        {
            _loaded = true;
            break;
        }
    }
}

UiFont &UiFont::instance()
{
    static UiFont font;
    return font;
}

bool UiFont::available() { return instance()._loaded; }

const sf::Font &UiFont::get() { return instance()._font; }
