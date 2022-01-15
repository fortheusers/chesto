#ifndef CONTAINER_H
#define CONTAINER_H
#include "Element.hpp"

#define ROW_LAYOUT 1
#define COL_LAYOUT 2

class Container : public Element
{
public:
	Container(int layout = 0, int padding = 10);
	Element* add(Element* elem);

	int layout = 0;
	int padding = 10;
};
#endif