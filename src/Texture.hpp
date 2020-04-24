#pragma once

#include <unordered_map>
#include <string>
#include "RootDisplay.hpp"
#include "Element.hpp"

enum TextureScaleMode
{
	/// Stretch the texture to fill the size
	SCALE_STRETCH,

	/// Keep texture proportions and fill the background with first pixel's color
	SCALE_PROPORTIONAL_WITH_BG,
};

struct TextureData
{
	CST_Texture* texture;
	CST_Color firstPixel;
};

class Texture : public Element
{
public:
	virtual ~Texture();

	/// Reinitialize Texture
	/// Resets texture content, size and color
	void clear(void);

	/// Loads the texture from a surface
	/// Returns true if successful
	bool loadFromSurface(CST_Surface *surface);

	/// Loads the texture from caches
	/// Returns true if successful
	bool loadFromCache(std::string &key);

	/// Loads the texture from a surface and saves the results in caches
	/// Returns true if successful
	bool loadFromSurfaceSaveToCache(std::string &key, CST_Surface *surface);

	/// Renders the texture
	void render(Element* parent);

	/// Resizes the texture
	void resize(int w, int h);

	/// Sets texture scaling mode
	void setScaleMode(TextureScaleMode mode);

	/// Return texture's original size
	void getTextureSize(int *w, int *h);

protected:
	/// Cache previously displayed textures
	static std::unordered_map<std::string, TextureData> texCache;

	/// The actual texture
	CST_Texture* mTexture = nullptr;

	/// The size of the texture
	int texW = 0, texH = 0;

	/// The color of the first pixel
	CST_Color texFirstPixel = {0,0,0,0};

	/// Texture's scaling mode
	TextureScaleMode texScaleMode = SCALE_STRETCH;
};
