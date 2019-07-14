#include "RootDisplay.hpp"
#include <SDL2/SDL2_rotozoom.h>
#include <string.h>

ImageElement::ImageElement(const char* incoming, bool calcFirstPixel)
{
	this->path = incoming;

	std::string key = std::string(this->path);

	// try to find it in the cache first
	if (ImageCache::cache.count(key))
	{
		this->imgSurface = ImageCache::cache[key];

		// don't go through cache if we are trying to calculate the pixel on this element
		if (!calcFirstPixel)
			return;
	}

	// not found, create it

	if (this->imgSurface != NULL && !calcFirstPixel)
		SDL_DestroyTexture(this->imgSurface);

	SDL_Surface* surface = IMG_Load(this->path);

	if (!calcFirstPixel)
		this->imgSurface = SDL_CreateTextureFromSurface(RootDisplay::mainRenderer, surface);

	this->width = 0;  //surface->w;
	this->height = 0; //surface->h;

	if (surface != NULL && imgSurface != NULL && calcFirstPixel)
	{
		this->firstPixel = new SDL_Color();
		Uint32 value = getpixel(surface, 0, 0);
		SDL_GetRGB(value, surface->format, &(this->firstPixel->r), &(this->firstPixel->g), &(this->firstPixel->b));
	}

	SDL_FreeSurface(surface);

	// add to cache for next time
	if (this->imgSurface != NULL)
		ImageCache::cache[key] = (this->imgSurface);
}

void ImageElement::render(Element* parent)
{
	SDL_Rect imgLocation;
	imgLocation.x = this->x + parent->x;
	imgLocation.y = this->y + parent->y;
	imgLocation.w = this->width;
	imgLocation.h = this->height;

	SDL_RenderCopy(RootDisplay::mainRenderer, this->imgSurface, NULL, &imgLocation);
}

void ImageElement::resize(int width, int height)
{
	// don't resize for null image surfaces
	if (this->imgSurface == NULL)
		return;

	this->width = width;
	this->height = height;
}

Uint32 getpixel(SDL_Surface* surface, int x, int y)
{
	int bpp = surface->format->BytesPerPixel;
	/* Here p is the address to the pixel we want to retrieve */
	Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;

	switch (bpp)
	{
	case 1:
		return *p;
		break;

	case 2:
		return *(Uint16*)p;
		break;

	case 3:
		if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
			return p[0] << 16 | p[1] << 8 | p[2];
		else
			return p[0] | p[1] << 8 | p[2] << 16;
		break;

	case 4:
		return *(Uint32*)p;
		break;

	default:
		return 0; /* shouldn't happen, but avoids warnings */
	}
}