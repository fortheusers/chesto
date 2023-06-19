#include "ImageElement.hpp"

ImageElement::ImageElement(const char* path)
{
	std::string key = std::string(path);
	loadPath(key);
}
