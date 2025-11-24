#pragma once

#include "Element.hpp"
#include <vector>

namespace Chesto {

class Grid : public Element
{
public:
	Grid(int columns, int width, int cellPadding = 0, int rowPadding = 0);
	
	/// Recalculate positions for all child elements
	void refresh();
	bool process(InputEvents* event) override;
	
	int columns;
	int width;
	int cellPadding;
	int rowPadding;
	
	/// Cursor state
	int highlighted = -1;  // Ccrrently highlighted element index (-1 = none)
	bool touchMode = true;  // whether we're in touch or cursor mode
};

} // namespace Chesto
