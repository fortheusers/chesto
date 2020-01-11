#pragma once

#include "ImageElement.hpp"
#include "TextElement.hpp"

class Button : public Element
{
public:
	Button(const char* text, int button, bool dark = false, int size = 20, int width = 0);

	bool process(InputEvents* event);
	void render(Element* parent);
	void position(int x, int y);

private:
	static CST_Color colors[2];
	const char* getUnicode(int button);

	// the physical button to activate this button
	int physical = -1;

	// original x and y coordinates of this button before add in the parent
	int ox = 0, oy = 0;

	// whether the button is dark or light themed
	bool dark = false;

	TextElement icon;
	TextElement text;
};
