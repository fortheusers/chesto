#pragma once

#include "Texture.hpp"
#include <string>

#define NORMAL 0
#define MONOSPACED 1
#define ICON 2

class TextElement : public Texture
{
public:
	// constructors
	TextElement();
	TextElement(const char* text, int size, CST_Color* color = 0, int font_type = NORMAL, int wrapped_width = 0);

	// change TextElement
	void setText(const std::string& text);
	void setSize(int size);
	void setColor(const CST_Color& color);
	void setFont(int font_type);
	void setWrappedWidth(int wrapped_width);

	// update TextElement with changes
	void update(void);

private:
	// default values
	std::string text = "";
	int textSize = 16;
	CST_Color textColor = (CST_Color){ 0xff, 0xff, 0xff };
	int textFont = NORMAL;
	int textWrappedWidth = 0;

	// font ttf files path
	static const char *fontPaths[];
};
