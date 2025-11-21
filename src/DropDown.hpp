#include "Button.hpp"
#include <vector>
#include <utility>
#include <functional>
#include <string>
#include "Container.hpp"
#include "Screen.hpp"
#include "ListElement.hpp"

#ifndef DROPDOWN_HPP_
#define DROPDOWN_HPP_

namespace Chesto {

class DropDownChoices;

class DropDownControllerElement : public Screen
{
public:
	void rebuildUI() override {}
};

class DropDown : public Button
{
public:
	DropDown(
		int physicalButton,
		std::vector<std::pair<std::string, std::string>> choices,
		std::function<void(std::string)> onSelect,
		int textSize,
		std::string defaultChoice = "",
		bool isDarkMode = false
	);
	std::vector<std::pair<std::string, std::string>> choices;
	std::function<void(std::string)> onSelect;
	std::string selectedChoice; // Current selection

	// element overriden functions
	bool process(InputEvents* event);
};

// DropDownChoices is now a Screen that gets pushed onto the screen stack
class DropDownChoices : public Screen
{
public:
	DropDownChoices(
		std::vector<std::pair<std::string, std::string>> choices,
		std::function<void(std::string)> onSelect,
		bool isDarkMode,
		std::string header = ""
	);
	bool process(InputEvents* event) override;
	void render(Element* parent) override;
	void rebuildUI() override; // Required by Screen

	int curHighlighted = -1;
	Container* container = nullptr; // the container that contains the actual choice elements (used for navigation)
	ListElement* scrollList = nullptr; // the scrollable wrapper for everything
	
private:
	std::vector<std::pair<std::string, std::string>> choices;
	std::function<void(std::string)> onSelectCallback;
	bool isDarkMode;
	std::string header;
};

} // namespace Chesto

#endif