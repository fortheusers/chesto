#pragma once

#include "Texture.hpp"
#include <string>

#define NORMAL 0
#define MONOSPACED 1
#define ICON 2

class TextElement : public Texture
{
public:
	TextElement(const char* text, int size, SDL_Color* color = 0, int font_type = NORMAL, int wrapped_width = 0);

private:
	static const char *fontPaths[];
};
