#include <algorithm>
#include "Container.hpp"

Container::Container(int layout, int padding)
{
	// these values only affect adds into this container
	this->layout = layout;
	this->padding = padding;
}

Element* Container::add(Element* elem)
{
	int newPosX = (layout == ROW_LAYOUT) ? this->width + padding : this->x;
	int newPosY = (layout == COL_LAYOUT) ? this->height + padding : this->y;

	if (elements.size() == 0) {
		// first element, goes at 0, 0 always
		newPosX = newPosY = 0;
	}

	child(elem->setPosition(newPosX, newPosY));

	this->width = (layout == ROW_LAYOUT) ? this->width + elem->width + padding : std::max(this->width, elem->width);
	this->height = (layout == COL_LAYOUT) ? this->height + elem->height + padding :  std::max(this->height, elem->height);
}
