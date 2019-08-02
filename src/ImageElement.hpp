#pragma once

#include "Element.hpp"
#include <SDL2/SDL_image.h>
#include <unordered_map>

class ImageElement : public Element
{
public:
	ImageElement(const char* path, bool calcFirstPixel = false);
	void render(Element* parent);

	SDL_Texture* imgSurface = NULL;
	const char* path;

	SDL_Color* firstPixel = NULL;

	void resize(int width, int height);

	// a map of all SDL surfaces that have been displayed
	static std::unordered_map<std::string, SDL_Texture*> cache;
};

Uint32 getpixel(SDL_Surface* surface, int x, int y);
