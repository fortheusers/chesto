#include "Texture.hpp"

std::unordered_map<std::string, TextureData> Texture::texCache;

Texture::~Texture()
{
}

bool Texture::loadFromSurface(SDL_Surface *surface)
{
	if (!surface)
		return false;

	// try to create a texture from the surface
	SDL_Texture *texture = SDL_CreateTextureFromSurface(RootDisplay::mainRenderer, surface);
	if (!texture)
		return false;

	// load first pixel color
	Uint32 pixelcolor = 0;
	for (int i = 0; i < surface->format->BytesPerPixel; i++)
		pixelcolor = (pixelcolor << 8) + *((Uint8*)surface->pixels + i);
	SDL_GetRGB(pixelcolor, surface->format, &texFirstPixel.r, &texFirstPixel.g, &texFirstPixel.b);

	// load texture size
	SDL_QueryTexture(texture, nullptr, nullptr, &texW, &texH);

	// load texture
	mTexture = texture;

	return true;
}

bool Texture::loadFromCache(std::string &key)
{
	// check if the texture is cached
	if (texCache.count(key))
	{
		TextureData *texData = &texCache[key];
		mTexture = texData->texture;
		texFirstPixel = texData->firstPixel;
		SDL_QueryTexture(mTexture, nullptr, nullptr, &texW, &texH);
		return true;
	}

	return false;
}

bool Texture::loadFromSurfaceSaveToCache(std::string &key, SDL_Surface *surface)
{
	bool success = loadFromSurface(surface);

	// only save to caches if loading was successful
	// and the texture isn't already cached
	if (success && !texCache.count(key))
	{
		TextureData texData;
		texData.texture = mTexture;
		texData.firstPixel = texFirstPixel;
		texCache[key] = texData;
	}

	return success;
}

void Texture::render(Element* parent)
{
	if (!mTexture)
		return;

	if (hidden)
		return;

	// rect of element's size
	SDL_Rect rect;
	rect.x = x + parent->x;
	rect.y = y + parent->y;
	rect.w = width;
	rect.h = height;

	if (texScaleMode == SCALE_PROPORTIONAL_WITH_BG)
	{
		// draw colored background
		SDL_SetRenderDrawColor(RootDisplay::mainRenderer, texFirstPixel.r, texFirstPixel.g, texFirstPixel.b, 0xFF);
		SDL_RenderFillRect(RootDisplay::mainRenderer, &rect);

		// recompute drawing rect
		if ((width * texH) > (height * texW))
		{
			// keep height, scale width
			rect.h = height;
			rect.w = (texW * rect.h) / texH;
		}
		else
		{
			// keep width, scale height
			rect.w = width;
			rect.h = (texH * rect.w) / texW;
		}

		// center the texture
		rect.x += (width - rect.w) / 2;
		rect.y += (height - rect.h) / 2;
	}

	// render the texture
	SDL_RenderCopy(RootDisplay::mainRenderer, mTexture, NULL, &rect);
}

void Texture::resize(int w, int h)
{
	width = w;
	height = h;
}

void Texture::setScaleMode(TextureScaleMode mode)
{
	texScaleMode = mode;
}

void Texture::getTextureSize(int *w, int *h)
{
	if (w)
		*w = texW;
	if (h)
		*h = texH;
}
