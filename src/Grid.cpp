#include "Grid.hpp"
#include "InputEvents.hpp"

namespace Chesto {

/**
 * The Grid component is similar to Container in that it has to deal
 * with positioning its child elements, but unlike container, it is designed
 * to take in the full list of elements, and then arrange them
 * based on some fixed width.
 * 
 * Calling refresh rebuilds the grid, again according to the width and its elements, which allows for reflowing when container sizes change.
 */
Grid::Grid(int columns, int width, int cellPadding, int rowPadding)
	: columns(columns)
	, width(width)
	, cellPadding(cellPadding)
	, rowPadding(rowPadding)
{
	this->width = width;
	this->height = 0; // calculated on refresh()
}

void Grid::refresh()
{
	if (elements.empty()) {
		this->height = 0;
		return;
	}
	
	// calculate cell width: (total width - padding between cells) / columns
	int cellWidth = (this->width - (cellPadding * (columns - 1))) / columns;
	
	int xPos = 0;
	int yPos = 0;
	int currentRowHeight = 0;
	int col = 0;
	
	for (auto& elem : elements) {
		// actually position the element
		elem->x = xPos;
		elem->y = yPos;
		
		// the tallest element in this row
		if (elem->height > currentRowHeight) {
			currentRowHeight = elem->height;
		}
		
		// advance column
		col++;
		xPos += cellWidth + cellPadding;
		
		// do we need to start a new row?
		if (col >= columns) {
			col = 0;
			xPos = 0;
			yPos += currentRowHeight + rowPadding;
			currentRowHeight = 0;
		}
	}
	
	// final grid height
	if (col > 0) {
		yPos += currentRowHeight; // commit last row height
	} else if (yPos > 0) {
		yPos -= rowPadding;
	}
	
	this->height = yPos;
}

bool Grid::process(InputEvents* event)
{
	// This method lets the children elements handle their
	// touch events, but manages grid-like cursor navigation	
	bool ret = false;
	
	// normal event handling
	ret |= Element::process(event);
	
	if (elements.empty()) {
		return ret;
	}
	
	if (touchMode) {
		return ret;
	}
	
	// cursor logic from HBAS's AppList
	// int origHighlighted = highlighted;
	
	// start highlight if none
	if (highlighted < 0 && !elements.empty()) {
		highlighted = 0;
	}
	
	// ensure highlighted is in bounds
	if (highlighted >= (int)elements.size()) {
		highlighted = elements.size() - 1;
	}

	// TODO: fiinsh grid navigation logic
	ret = true;
	
	return ret;
}

} // namespace Chesto
