#include "ImageElement.hpp"
#include <SDL2/SDL_image.h>

ImageElement::ImageElement(const char* path)
{
	std::string key = std::string(path);
	if (!loadFromCache(key))
	{
		SDL_Surface *surface = IMG_Load(path);
		loadFromSurfaceSaveToCache(key, surface);
		SDL_FreeSurface(surface);
	}
}
