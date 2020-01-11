#include "ImageElement.hpp"

ImageElement::ImageElement(const char* path)
{
	std::string key = std::string(path);
	if (!loadFromCache(key))
	{
		CST_Surface *surface = IMG_Load(path);
		loadFromSurfaceSaveToCache(key, surface);
		CST_FreeSurface(surface);
	}
}
