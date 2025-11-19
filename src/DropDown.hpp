#include "Button.hpp"
#include <vector>
#include <utility>
#include <functional>
#include <string>
#include "ListElement.hpp"
#include "Container.hpp"

#ifndef DROPDOWN_HPP_
#define DROPDOWN_HPP_

class DropDownChoices;

class DropDownControllerElement : public Element
{
public:
	DropDownChoices* curDropDown = nullptr;
	bool process(InputEvents* event) override;
};

class DropDown : public Button
{
public:
	DropDown(
		DropDownControllerElement* parentView,
		int physicalButton,
		std::vector<std::pair<std::string, std::string>> choices,
		std::function<void(std::string)> onSelect,
		int textSize,
		std::string defaultChoice = "",
		bool isDarkMode = false
	);
	std::vector<std::pair<std::string, std::string>> choices;
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
	DropDownChoices(std::vector<std::pair<std::string, std::string>> choices, DropDown* dropdown, bool isDarkMode);
	bool process(InputEvents* event) override;
	void render(Element* parent) override;

	int curHighlighted = -1;
	Container* container = nullptr; // the container that contains the actual choice elements (used for navigation)
};

#endif