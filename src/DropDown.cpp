#include "DropDown.hpp"
#include "Button.hpp"
#include "Container.hpp"
#include "Constraint.hpp"

DropDown::DropDown(
	DropDownControllerElement* parentView,
	int physicalButton,
	std::unordered_map<std::string, std::string> choices,
	std::function<void(std::string)> onSelect,
	int textSize,
	std::string defaultChoice
) : Button(defaultChoice.empty() ? "Select..." : (choices.find(defaultChoice) != choices.end() ? choices[defaultChoice] : defaultChoice), physicalButton, false, textSize, 0)
{
	this->choices = choices;
	this->onSelect = onSelect;
	this->controller = parentView;
	
	this->setAction([this, parentView]() {
		// when clicked, open the dropdown choices subscreen
		dropDownScreen = new DropDownChoices(this->choices, this);
		dropDownScreen->constrain(ALIGN_CENTER_HORIZONTAL);
		parentView->curDropDown = dropDownScreen;
		parentView->append(dropDownScreen);
	});
}

bool DropDown::process(InputEvents* event) {
	// if (dropDownScreen) {
	// 	// if the dropdown choices screen is open, pass events to it first
	// 	return dropDownScreen->process(event);
	// }
	super::process(event);
}

// The subscreen element that gets created when we want to display the dropdown
DropDownChoices::DropDownChoices(std::unordered_map<std::string, std::string> choices, DropDown* dropdown) {
	// create a vertical container
	auto container = new Container(COL_LAYOUT, 20);
	container->backgroundColor = dropdown->backgroundColor;
	container->hasBackground = true;
	
	// populate the dropdown choices
	int widestWidth = 0;
	for (const auto& choice : dropdown->choices) {
		auto choiceElement = new Button(choice.second, -1, false, 20, 0);
		widestWidth = std::max(widestWidth, choiceElement->width);
		choiceElement->setAction([this, choice, dropdown]() {
			// when a choice is selected, update the button text and invoke the onSelect callback
			dropdown->selectedChoiceIndex = choice.first;
			dropdown->updateText(choice.second);
			if (dropdown->onSelect) {
				dropdown->onSelect(dropdown->selectedChoiceIndex);
			}
			// close the dropdown screen (deletes us)
			dropdown->controller->remove(this);
			dropdown->controller->curDropDown = nullptr; // TODO: probably this should be a method
			dropdown->dropDownScreen = nullptr;
		});
		container->add(choiceElement);
	}

	this->width = widestWidth; // our container dropdown also needs this width

	// for each choice, make its width the widest 
	for (const auto& child : container->elements) {
		if (auto buttonChild = dynamic_cast<Button*>(child)) {
			buttonChild->fixedWidth = widestWidth;
			buttonChild->updateBounds();
		}
	}

	super::append(container);
}