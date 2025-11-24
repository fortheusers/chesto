#include <algorithm>
#include "Container.hpp"

namespace Chesto {

Container::Container(int layout, int padding)
{
	// these values only affect adds into this container
	this->layout = layout;
	this->padding = padding;
}

Element* Container::add(std::unique_ptr<Element> elem)
{
	int newPosX = (layout == ROW_LAYOUT) ? this->width + padding : this->x;
	int newPosY = (layout == COL_LAYOUT) ? this->height + padding : this->y;

	Element* rawPtr = elem.get();
	rawPtr->setPosition(newPosX, newPosY);
	addNode(std::move(elem)); // transfers ownership

	this->width = (layout == ROW_LAYOUT) ? this->width + rawPtr->width + padding : std::max(this->width, rawPtr->width);
	this->height = (layout == COL_LAYOUT) ? this->height + rawPtr->height + padding :  std::max(this->height, rawPtr->height);

	return rawPtr;
}

} // namespace Chesto