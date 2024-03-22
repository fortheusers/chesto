#include "TextElement.hpp"
#include "RootDisplay.hpp"

const char *TextElement::fontPaths[] = {
	RAMFS "./res/opensans.ttf", // 0 = NORMAL
	RAMFS "./res/mono.ttf", // 1 = MONOSPACED
	RAMFS "./res/nxicons.ttf", // 2 = ICON
};

TextElement::TextElement()
{
}

TextElement::TextElement(const char* text, int size, CST_Color* color, int font_type, int wrapped_width)
{
	std::string sText = std::string(text);
	setText(sText);
	setSize(size);
	if (color) setColor(*color);
	setFont(font_type);
	setWrappedWidth(wrapped_width);
	update();
}

void TextElement::setText(const std::string& text)
{
	this->text = text;
}

void TextElement::setSize(int size)
{
	this->textSize = size;
}

void TextElement::setColor(const CST_Color& color)
{
	this->textColor = color;
}

void TextElement::setFont(int font_type)
{
	this->textFont = font_type;
}

void TextElement::setWrappedWidth(int wrapped_width)
{
	this->textWrappedWidth = wrapped_width;
}

void TextElement::update(bool forceUpdate)
{
	std::string key = text + std::to_string(textSize);

	clear();

	if (!loadFromCache(key) || forceUpdate)
	{
		auto fontPath = fontPaths[textFont % 3];
		if (customFontPath != "") {
			fontPath = customFontPath.c_str();
		}
		TTF_Font* font = TTF_OpenFont(fontPath, textSize);

		CST_Surface *textSurface = ((textFont == ICON) || (textWrappedWidth == 0)) ?
			TTF_RenderUTF8_Blended(font, text.c_str(), textColor) :
			TTF_RenderUTF8_Blended_Wrapped(font, text.c_str(), textColor, textWrappedWidth);
		if(textSurface==NULL) printf("TTF_GetError: %s\n", TTF_GetError());

		loadFromSurfaceSaveToCache(key, textSurface);

		CST_FreeSurface(textSurface);
		TTF_CloseFont(font);
	}

	getTextureSize(&width, &height);
}
