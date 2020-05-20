#include "Container.hpp"

Container::Container(int layout, int padding)
{
	// these values only affect adds into this container
	this->layout = layout;
	this->padding = padding;
}

void Container::add(Element* elem)
{
	int newPosX = (layout == ROW_LAYOUT) ? this->width + padding : 0;
	int newPosY = (layout == COL_LAYOUT) ? this->height + padding : 0;

	child(elem->setPosition(newPosX, newPosY));

	this->width += elem->width + padding;
	this->height += elem->height + padding;
}
