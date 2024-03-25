#pragma once

#include "ImageElement.hpp"
#include "TextElement.hpp"

class Button : public Element
{
public:
	Button(std::string text, int button, bool dark = false, int size = 20, int width = 0);

	bool process(InputEvents* event);

	void updateBounds();
	void updateText(const char* inc_text);
	const std::string getText();

	std::string myLastSeenGamepad = "";

	TextElement text;
	static std::string getControllerButtonImageForPlatform(int button, bool isGray, bool isOutline);

private:
	static CST_Color colors[2];
	/// the physical button to activate this button
	int physical = -1;

	// a width we set the button to regardless of inner text
	int fixedWidth = 0;

	// whether the button is dark or light themed
	bool dark = false;

	ImageElement icon;
};
