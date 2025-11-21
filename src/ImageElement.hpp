#pragma once

#include "Texture.hpp"
#include <string>

namespace Chesto {

class ImageElement : public Texture
{
public:
	/// Creates a new image element, loading the image
	/// from the specified filesystem path
	ImageElement(std::string path);
};

} // namespace Chesto
