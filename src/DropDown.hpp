#include "Button.hpp"
#include <map>
#include <functional>
#include <string>
#include "ListElement.hpp"

class DropDownChoices;

class DropDownControllerElement : public Element
{
public:
	DropDownChoices* curDropDown = nullptr;
};

class DropDown : public Button
{
public:
	DropDown(
		DropDownControllerElement* parentView,
		int physicalButton,
		std::unordered_map<std::string, std::string> choices,
		std::function<void(std::string)> onSelect,
		int textSize,
		std::string defaultChoice = ""
	);
	std::unordered_map<std::string, std::string> choices;
	std::function<void(std::string)> onSelect;
	std::string selectedChoiceIndex = "";
	DropDownChoices* dropDownScreen = nullptr;
	DropDownControllerElement* controller = nullptr;

	// element overriden functions
	bool process(InputEvents* event);
};

class DropDownChoices : public ListElement
{
public:
	DropDownChoices(std::unordered_map<std::string, std::string> choices, DropDown* dropdown);
};
