#pragma once

#include "Texture.hpp"
#include <string>
#include <map>

#define NORMAL 0
#define MONOSPACED 1
#define ICON 2 // old icon font, no longer used
#define OLD_MONOSPACED 2
#define SERIF 3
#define SIMPLIFIED_CHINESE 4
#define KOREAN 5
#define JAPANESE 6

namespace Chesto {

std::string i18n(std::string key);
std::string i18n_number(int number);
std::string i18n_date(int timestamp);

class TextElement : public Texture
{
public:
	// constructors
	TextElement();
	TextElement(std::string text, int size, CST_Color* color = 0, int font_type = NORMAL, int wrapped_width = 0);

	// change TextElement
	void setText(const std::string& text);
	void setSize(int size);
	void setColor(const CST_Color& color);
	void setFont(int font_type);
	void setWrappedWidth(int wrapped_width);

	/// update TextElement with changes
	void update(bool forceUpdate = false);
	std::string text = "";

	// if specified, will override any font_type setting
	std::string customFontPath = "";

	static std::map<std::string, std::string> i18nCache;
	static std::vector<std::pair<std::string, std::string>> getAvailableLanguages();
	static void loadI18nCache(std::string locale);
	static std::string curLang;

	// if true, replaces all NORMAL fonts with SIMPLIFIED_CHINESE
	static bool useSimplifiedChineseFont;
	static bool useKoreanFont;
	static bool useJapaneseFont;

	static std::map<std::string, int> forcedLangFonts;

private:
	// default values
	int textSize = 16;
	CST_Color textColor = (CST_Color){ 0xff, 0xff, 0xff, 0xff };
	int textFont = NORMAL;
	int textWrappedWidth = 0;

	// font ttf files path
	static const char *fontPaths[];
};

} // namespace Chesto
