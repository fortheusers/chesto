#include "Screen.hpp"
#include "RootDisplay.hpp"

namespace Chesto {

Screen::Screen()
{
	// Set to full screen dimensions by default
	this->width = RootDisplay::screenWidth;
	this->height = RootDisplay::screenHeight;
}

Screen::~Screen()
{
	// Base destructor - unique_ptr handles cleanup automatically
}

int Screen::getScreenWidth() const
{
	return RootDisplay::screenWidth;
}

int Screen::getScreenHeight() const
{
	return RootDisplay::screenHeight;
}

} // namespace Chesto
