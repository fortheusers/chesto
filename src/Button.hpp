#pragma once

#include "ImageElement.hpp"
#include "TextElement.hpp"

class Button : public Element
{
public:
	Button(const char* text, int button, bool dark = false, int size = 20, int width = 0);

	bool process(InputEvents* event);

	void updateBounds();
	void updateText(const char* inc_text);
	const std::string getText();

	TextElement text;

private:
	static CST_Color colors[2];
	const char* getUnicode(int button);

	/// the physical button to activate this button
	int physical = -1;

	// a width we set the button to regardless of inner text
	int fixedWidth = 0;

	// whether the button is dark or light themed
	bool dark = false;

	TextElement icon;
};
