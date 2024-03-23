#include "TextElement.hpp"
#include "RootDisplay.hpp"
#include <fstream>

const char *TextElement::fontPaths[] = {
	RAMFS "./res/fonts/OpenSans-Regular.ttf", // 0 = NORMAL
	RAMFS "./res/fonts/UbuntuMono-Regular.ttf", // 1 = MONOSPACED
	RAMFS "./res/fonts/oldmono.ttf", // 2 = OLD_MONOSPACED
	RAMFS "./res/fonts/PTSerif-Regular.ttf", // 3 = SERIF
	RAMFS "./res/fonts/NotoSansSC-Regular.ttf", // 4 = SIMPLIFIED_CHINESE
};

std::unordered_map<std::string, std::string> TextElement::i18nCache = {};

bool TextElement::useSimplifiedChineseFont = false;

TextElement::TextElement()
{
}

// static method to load i18n cache
void TextElement::loadI18nCache(std::string locale) {
	// en-us, zh-cn
	std::string localePath = RAMFS "res/i18n/" + locale + ".ini";
	std::ifstream file(localePath);
	printf("Loading i18n cache from %s\n", localePath.c_str());
	if (file.is_open()) {
		std::string line;
		while (std::getline(file, line)) {
			size_t pos = line.find(" =");
			if (pos == std::string::npos) {
				continue; // bad format
			}
			std::string key = line.substr(0, pos);
			pos = line.find("= ");
			if (pos == std::string::npos) {
				continue;
			}
			std::string value = line.substr(pos + 2);
			TextElement::i18nCache[key] = value;
			printf("Loaded i18n key %s with value %s\n", key.c_str(), value.c_str());
		}
		file.close();

		// if locale is zh-cn, we need to force the simple chinese font
		if (locale == "zh-cn") {
			printf("Overriding font choice\n");
			TextElement::useSimplifiedChineseFont = true;
		}
	}
}

TextElement::TextElement(std::string text, int size, CST_Color* color, int font_type, int wrapped_width)
{
	std::string sText = text;
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
		if (TextElement::useSimplifiedChineseFont && textFont == NORMAL) {
			textFont = SIMPLIFIED_CHINESE;
		}
		auto fontPath = fontPaths[textFont % 5];
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

std::string& i18n(std::string key) {
	if (TextElement::i18nCache.find(key) != TextElement::i18nCache.end()) {
		return TextElement::i18nCache[key];
	}
	return key;
}