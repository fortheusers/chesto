#include "TextElement.hpp"
#include "RootDisplay.hpp"

const char *TextElement::fontPaths[] = {
	RAMFS "./res/opensans.ttf", // 0 = NORMAL
	RAMFS "./res/mono.ttf", // 1 = MONOSPACED
	RAMFS "./res/nxicons.ttf", // 2 = ICON
};

TextElement::TextElement(const char* text, int size, SDL_Color* color, int font_type, int wrapped_width)
{
	std::string key = std::string(text) + std::to_string(size);

	if (!loadFromCache(key))
	{
		TTF_Font* font = TTF_OpenFont(fontPaths[font_type % 3],	size);

		SDL_Color textColor = (color) ? *color : (SDL_Color){ 0xff, 0xff, 0xff };

		SDL_Surface *textSurface = ((font_type == ICON) || (wrapped_width == 0)) ?
			TTF_RenderUTF8_Blended(font, text, textColor) :
			TTF_RenderText_Blended_Wrapped(font, text, textColor, wrapped_width);

		loadFromSurfaceSaveToCache(key, textSurface);

		SDL_FreeSurface(textSurface);
		TTF_CloseFont(font);
	}

	getTextureSize(&width, &height);
}
