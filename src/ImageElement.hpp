#pragma once

#include "Texture.hpp"
#include <string>

class ImageElement : public Texture
{
public:
	/// Creates a new image element, loading the image
	/// from the specified filesystem path
	ImageElement(const char* path);
};
