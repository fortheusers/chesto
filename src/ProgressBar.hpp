#pragma once

#include "Element.hpp"

namespace Chesto {

class ProgressBar : public Element
{
public:
	ProgressBar();
	void render(Element* parent);
	float percent = 0;

	int color;
	bool dimBg = false;

};

} // namespace Chesto
