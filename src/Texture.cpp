#include "Texture.hpp"

std::unordered_map<std::string, TextureData> Texture::texCache;

Texture::~Texture()
{
}

void Texture::clear(void)
{
	mTexture = nullptr;
	texW = 0;
	texH = 0;
	texFirstPixel = (CST_Color){0,0,0,0};
}

bool Texture::loadFromSurface(CST_Surface *surface)
{
	if (!surface)
		return false;

	// try to create a texture from the surface
	CST_Texture *texture = CST_CreateTextureFromSurface(this->renderer, surface);
	if (!texture)
		return false;

	// load first pixel color
	Uint32 pixelcolor = *(Uint32*)surface->pixels;
	Uint32 emptybits = 8 * (4 - surface->format->BytesPerPixel);
	pixelcolor >>= emptybits * (SDL_BYTEORDER == SDL_BIG_ENDIAN);
	pixelcolor &= 0xffffffff >> emptybits;
	SDL_GetRGB(pixelcolor, surface->format, &texFirstPixel.r, &texFirstPixel.g, &texFirstPixel.b);

	// load texture size
	CST_QueryTexture(texture, &texW, &texH);

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
		CST_QueryTexture(mTexture, &texW, &texH);
		return true;
	}

	return false;
}

bool Texture::loadFromSurfaceSaveToCache(std::string &key, CST_Surface *surface)
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

	// update xAbs and yAbs
	this->recalcPosition(parent);

	// rect of element's size
	CST_Rect rect;
	rect.x = this->xAbs;
	rect.y = this->yAbs;
	rect.w = this->width;
	rect.h = this->height;

	if (CST_isRectOffscreen(&rect))
		return;

	if (texScaleMode == SCALE_PROPORTIONAL_WITH_BG)
	{
		// draw colored background
		CST_SetDrawColor(this->renderer, texFirstPixel);
		CST_FillRect(this->renderer, &rect);

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

	if ((texScaleMode == SCALE_STRETCH) && angle!=0)
	{
		// render the texture with a rotation

		// only supported for SCALE_STRETCH textures,
		// as the colored background wouldn't get rotated

		CST_SetQualityHint("best");
		CST_RenderCopyRotate(this->renderer, mTexture, NULL, &rect, this->angle);
	}
	else
	{
		// render the texture normally
		CST_RenderCopy(this->renderer, mTexture, NULL, &rect);
	}
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
