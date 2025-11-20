#include "TextElement.hpp"
#include "RootDisplay.hpp"
#include <fstream>
#include <ctime>   // std::time
#include <dirent.h> // for directory reading
#include <map>
#include <algorithm>

const char *TextElement::fontPaths[] = {
	RAMFS "./res/fonts/OpenSans-Regular.ttf", // 0 = NORMAL
	RAMFS "./res/fonts/UbuntuMono-Regular.ttf", // 1 = MONOSPACED
	RAMFS "./res/fonts/oldmono.ttf", // 2 = OLD_MONOSPACED
	RAMFS "./res/fonts/PTSerif-Regular.ttf", // 3 = SERIF
	RAMFS "./res/fonts/NotoSansSC-Regular.ttf", // 4 = SIMPLIFIED_CHINESE
	RAMFS "./res/fonts/NotoSansKR-Regular.ttf", // 5 = KOREAN
	RAMFS "./res/fonts/NotoSansJP-Regular.ttf", // 6 = JAPANESE
};

std::map<std::string, std::string> TextElement::i18nCache = {};
std::string TextElement::curLang = "en-us";

bool TextElement::useSimplifiedChineseFont = false;
bool TextElement::useKoreanFont = false;
bool TextElement::useJapaneseFont = false;

// map of specific text strings to force a specific font type
std::map<std::string, int> TextElement::forcedLangFonts = {};

TextElement::TextElement()
{
}

// static method to enumerate all languages into a vector of pairs
std::vector<std::pair<std::string, std::string>> TextElement::getAvailableLanguages() {
	std::vector<std::pair<std::string, std::string>> languages;
	// read all files in RAMFS res/i18n and their 'meta.lang.name' entry
	std::string i18nPath = RAMFS "res/i18n/";
	DIR* dir = opendir(i18nPath.c_str());
	if (dir) {
		struct dirent* entry;
		while ((entry = readdir(dir)) != NULL) {
			std::string fileName = entry->d_name;;
			if (fileName.length() > 4 && fileName.substr(fileName.length() - 4) == ".ini") {
				std::string locale = fileName.substr(0, fileName.length() - 4);
				// to lower case
				std::transform(locale.begin(), locale.end(), locale.begin(), ::tolower);
				// read the file to find meta.lang.name
				std::ifstream file(i18nPath + fileName);
				if (file.is_open()) {
					std::string line;
					while (std::getline(file, line)) {
						if (line.find("meta.lang.name = ") == 0) {
							std::string langName = line.substr(strlen("meta.lang.name = "));
							languages.push_back({locale, langName});

							// also store the language name with its forced font face
							if (locale == "zh-cn") {
								forcedLangFonts[langName] = SIMPLIFIED_CHINESE;
							} else if (locale == "ko-kr") {
								forcedLangFonts[langName] = KOREAN;
							} else if (locale == "ja-jp") {
								forcedLangFonts[langName] = JAPANESE;
							} else {
								forcedLangFonts[langName] = NORMAL;
							}
							break;
						}
					}
					file.close();
				}
			}
		}
		closedir(dir);
	}

	return languages;
}

// static method to load i18n cache
void TextElement::loadI18nCache(std::string locale) {
	// en-us, zh-cn
	std::string localePath = RAMFS "res/i18n/" + locale + ".ini";
	std::transform(locale.begin(), locale.end(), locale.begin(), ::tolower);
	TextElement::curLang = locale;
	std::ifstream file(localePath);
	// printf("Loading i18n cache from %s\n", localePath.c_str());
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
			// printf("Loaded i18n key %s with value %s\n", key.c_str(), value.c_str());
		}
		file.close();

		TextElement::useSimplifiedChineseFont = false;
		TextElement::useKoreanFont = false;
		TextElement::useJapaneseFont = false;

		// if locale is zh-cn, we need to force the simple chinese font
		if (locale == "zh-cn") {
			printf("Overriding font choice\n");
			TextElement::useSimplifiedChineseFont = true;
		}
		if (locale == "ko-kr") {
			printf("Overriding font choice for Korean\n");
			TextElement::useKoreanFont = true;
		}
		if (locale == "ja-jp") {
			printf("Overriding font choice for Japanese\n");
			TextElement::useJapaneseFont = true;
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
	std::string key = Texture::textElemPrefix + text + std::to_string(textSize);

	clear();

	if (!loadFromCache(key) || forceUpdate)
	{
		// Use a local variable to determine which font to use, without modifying the instance variable
		int actualFont = textFont;
		if (TextElement::useSimplifiedChineseFont && textFont == NORMAL) {
			actualFont = SIMPLIFIED_CHINESE;
		}
		if (TextElement::useKoreanFont && textFont == NORMAL) {
			actualFont = KOREAN;
		}
		if (TextElement::useJapaneseFont && textFont == NORMAL) {
			actualFont = JAPANESE;
		}

		// also, if the specific text string is in the map of force-lang strings, always use that font instead
		if (forcedLangFonts.find(text) != forcedLangFonts.end()) {
			actualFont = forcedLangFonts[text];
		}

		auto fontPath = fontPaths[actualFont % 7];
		if (customFontPath != "") {
			fontPath = customFontPath.c_str();
		}
		TTF_Font* font = TTF_OpenFont(fontPath, textSize);

		CST_Surface *textSurface = ((actualFont == ICON) || (textWrappedWidth == 0)) ?
			TTF_RenderUTF8_Blended(font, text.c_str(), textColor) :
			TTF_RenderUTF8_Blended_Wrapped(font, text.c_str(), textColor, textWrappedWidth);
		if(textSurface==NULL) printf("TTF_GetError: %s\n", TTF_GetError());

		loadFromSurfaceSaveToCache(key, textSurface);

		CST_FreeSurface(textSurface);
		TTF_CloseFont(font);
	}

	getTextureSize(&width, &height);
}

std::string i18n(std::string key) {
    if (const auto& keyItr = TextElement::i18nCache.find(key); keyItr != TextElement::i18nCache.end()) {
        return keyItr->second;
    }
    return key;
}

std::string i18n_number(int number) {
	std::string decimalSeparator = i18n("number.decimal");
	std::string thousandsSeparator = i18n("number.thousands");
	if (decimalSeparator.empty()) {
		decimalSeparator = ".";
	}
	if (thousandsSeparator.empty()) {
		thousandsSeparator = ",";
	}
	std::string numberString = std::to_string(number);
	size_t decimalPos = numberString.find(".");
	if (decimalPos == std::string::npos) {
		decimalPos = numberString.length();
	}
	std::string integerPart = numberString.substr(0, decimalPos);
	std::string decimalPart = numberString.substr(decimalPos);
	if (decimalPart.length() > 0) {
		decimalPart = decimalSeparator + decimalPart.substr(1);
	}
	for (int i = integerPart.length() - 3; i > 0; i -= 3) {
		integerPart.insert(i, thousandsSeparator);
	}
	return integerPart + decimalPart;
}

std::string i18n_date(int timestamp) {
	std::string dateFormatString = i18n("date.format");
	if (dateFormatString.empty()) {
		dateFormatString = "%Y-%m-%d";
	}
	// convert int to time_t
	time_t timestamp2 = static_cast<time_t>(timestamp);
	
	struct tm* timeinfo = localtime(&timestamp2);
	char buffer[256];
	strftime(buffer, sizeof(buffer), dateFormatString.c_str(), timeinfo);
	return std::string(buffer);
}