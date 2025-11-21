#include "Texture.hpp"

namespace Chesto {

std::unordered_map<std::string, TextureData> Texture::texCache;

const std::string Texture::textElemPrefix = "(TextElement):";

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

// https://stackoverflow.com/a/53067795
Uint32 getpixel(CST_Surface *surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

switch (bpp)
{
    case 1:
        return *p;
        break;

    case 2:
        return *(Uint16 *)p;
        break;

    case 3:
        if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
            return p[0] << 16 | p[1] << 8 | p[2];
        else
            return p[0] | p[1] << 8 | p[2] << 16;
		break;

	case 4:
		return *(Uint32 *)p;
		break;

	default:
		return 0;       /* shouldn't happen, but avoids warnings */
	}
}

bool Texture::loadFromSurface(CST_Surface *surface)
{
	if (!surface)
		return false;

	// will default MainDisplay's renderer if we don't have one in this->renderer
	CST_Renderer* renderer = getRenderer();

	// try to create a texture from the surface
	CST_Texture *texture = CST_CreateTextureFromSurface(renderer, surface, true);
	SDL_SetTextureBlendMode(texture, blendMode);
	if (!texture)
		return false;

	// load first pixel color
	auto pixelcolor = getpixel(surface, 0, 0);
	CST_GetRGBA(pixelcolor, surface->format, &texFirstPixel);

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

void Texture::wipeEntireCache()
{
	texCache.clear();
}

void Texture::wipeTextCache()
{
	for (auto it = texCache.begin(); it != texCache.end(); )
	{
		std::string key = it->first;
		if (key.find(Texture::textElemPrefix) == 0) { // does the key start with our text prefix?
			it = texCache.erase(it);
		} else {
			++it;
		}
	}
}

void Texture::render(Element* parent)
{
	// update xAbs and yAbs
	super::render(parent);

	if (!mTexture)
		return;

	if (hidden)
		return;

	// rect of element's size
	CST_Rect rect;
	rect.x = this->xAbs;
	rect.y = this->yAbs;
	rect.w = this->width;
	rect.h = this->height;

	if (CST_isRectOffscreen(&rect))
		return;

	CST_Renderer* renderer = getRenderer();

	if (texScaleMode == SCALE_PROPORTIONAL_WITH_BG || texScaleMode == SCALE_PROPORTIONAL_NO_BG)
	{
		CST_SetDrawBlend(RootDisplay::renderer, false);
	
		// draw colored background
		if (texScaleMode == SCALE_PROPORTIONAL_WITH_BG)
		{
			CST_SetDrawColor(renderer, texFirstPixel);
			auto color = (CST_Color){ texFirstPixel.r, texFirstPixel.g, texFirstPixel.b, 0xFF };

			// if the first pixel is transparent, use the background color
			if (texFirstPixel.a == 0) {
				auto r = backgroundColor.r * 0xff;
				auto g = backgroundColor.g * 0xff;
				auto b = backgroundColor.b * 0xff;
				color = (CST_Color){ r, g, b, 0xFF };
			}

			CST_roundedBoxRGBA(renderer, rect.x, rect.y, rect.x + rect.w, rect.y + rect.h, cornerRadius,
				color.r, color.g, color.b, 0xFF);
		}

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

	if (angle != 0) {
		// render the texture with a rotation
		CST_SetQualityHint("best");
		CST_RenderCopyRotate(renderer, mTexture, NULL, &rect, this->angle);
	}
	else if (useColorMask) {
		// render the texture with a mask color (only can darken the texture)
		SDL_SetTextureColorMod(mTexture, maskColor.r, maskColor.g, maskColor.b);
		CST_RenderCopy(renderer, mTexture, NULL, &rect);
		SDL_SetTextureColorMod(mTexture, 0xFF, 0xFF, 0xFF);
	} else {
		// render the texture normally
		CST_RenderCopy(renderer, mTexture, NULL, &rect);
	}
}

void Texture::resize(int w, int h)
{
	width = w;
	height = h;
}

Texture* Texture::setSize(int w, int h)
{
	this->resize(w, h);
	return this;
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

bool Texture::saveTo(std::string &path)
{
	if (!mTexture)
		return false;

	// render the texture to one that can be saved (TARGET ACCESS)
	CST_Texture* target = SDL_CreateTexture(getRenderer(), SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, texW, texH);
	if (!target)
		return false;
	
	// set the target texture
	SDL_SetRenderTarget(getRenderer(), target);

	// render the texture
	SDL_RenderCopy(getRenderer(), mTexture, NULL, NULL);

	// reset the target texture
	SDL_SetRenderTarget(getRenderer(), NULL);


	// save the surface to the path
	return CST_SavePNG(target, path.c_str());
}

void Texture::loadPath(std::string& path, bool forceReload) {
	// Guard against empty paths
	if (path.empty()) {
		width = 0;
		height = 0;
		return;
	}
	
	if (forceReload || !loadFromCache(path))
	{
		CST_Surface *surface = IMG_Load(path.c_str());
		loadFromSurfaceSaveToCache(path, surface);
		CST_FreeSurface(surface);
	}

	width = texW;
	height = texH;
}
} // namespace Chesto
