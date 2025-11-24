#include "DropDown.hpp"
#include "Button.hpp"
#include "Constraint.hpp"
#include "RootDisplay.hpp"
#include <iostream>

namespace Chesto {

DropDown::DropDown(
	int physicalButton,
	std::vector<std::pair<std::string, std::string>> choices,
	std::function<void(std::string)> onSelect,
	int textSize,
	std::string defaultChoice,
	bool isDarkMode
) : Button(defaultChoice.empty() ? "Select..." : ([&choices, &defaultChoice]() {
		for (const auto& choice : choices) {
			if (choice.first == defaultChoice) return choice.second;
		}
		return defaultChoice;
	})(), physicalButton, isDarkMode, textSize, 0)
{
	this->choices = choices;
	this->onSelect = onSelect;
	this->selectedChoice = defaultChoice;
	
	this->setAction([
		this,
		callback = this->onSelect,
		isDarkMode
	]() {
		RootDisplay::pushScreen(std::make_unique<DropDownChoices>(
			this->choices,
			callback,
			isDarkMode
		));
	});
}

bool DropDown::process(InputEvents* event) {
	return Button::process(event);
}

// subclass of Screen
DropDownChoices::DropDownChoices(
	std::vector<std::pair<std::string, std::string>> choices,
	std::function<void(std::string)> onSelect,
	bool isDarkMode,
	std::string header
) : choices(choices),
    onSelectCallback(onSelect),
    isDarkMode(isDarkMode),
    header(header)
{
	rebuildUI();
	
	int widestWidth = 0;
	for (const auto& choice : this->choices) {
		std::string displayText = choice.second.empty() ? "(empty)" : choice.second;
		
		auto choiceElement = std::make_unique<Button>(displayText, 0, isDarkMode, 20, 0);
		widestWidth = std::max(widestWidth, choiceElement->width);
		
		choiceElement->setAction([
			this, // Capture this to access onSelectCallback member
			choiceKey = choice.first
		]() {
			// local copy is needed here to avoid it being destroyed after popScreen
			std::string choiceKeyCopy = choiceKey;
			
			// same as above?
			auto callbackCopy = this->onSelectCallback;
			
			// pop the dropdown screen (deletes 'this'!)
			RootDisplay::popScreen();
			
			// use local copies now
			if (callbackCopy) {
				callbackCopy(choiceKeyCopy);
			}
		});
		container->add(std::move(choiceElement));
	}

	// make each choice the same width (widest)
	for (const auto& child : container->elements) {
		if (auto buttonChild = dynamic_cast<Button*>(child.get())) {
			buttonChild->fixedWidth = widestWidth;
			buttonChild->updateBounds();
		}
	}
}

void DropDownChoices::rebuildUI() {
	removeAll();
	
	// Create a full-screen scrollable list to hold everything
	// TODO: a common way for Screen's to have all their content be scrollable without making a new element each time
	auto scrollContainer = std::make_unique<ListElement>();
	scrollContainer->width = SCREEN_WIDTH;
	scrollContainer->height = SCREEN_HEIGHT;
	scrollList = scrollContainer.get(); // Keep pointer for scrolling
	
	// overlay and shade bg color
	// also TODO: a common way to have the dim background exist and fade in
	auto overlay = createNode<Element>();
    overlay->width = SCREEN_WIDTH;
    overlay->height = SCREEN_HEIGHT;
    overlay->backgroundColor = fromRGB(0, 0, 0);
    overlay->backgroundOpacity = 0x70;
    overlay->cornerRadius = 1; // needed to force transparency
    overlay->hasBackground = true;
	
	// header text if specified
	if (!header.empty()) {
		auto headerText = std::make_unique<TextElement>(header.c_str(), 28);
		headerText->constrain(ALIGN_CENTER_HORIZONTAL);
		headerText->position(0, SCREEN_HEIGHT / 5 - 60);
		scrollContainer->addNode(std::move(headerText));
	}
	
	// vertical container for the choices
	auto containerPtr = std::make_unique<Container>(COL_LAYOUT, 20);
	container = containerPtr.get();
	
	// TODO: Chesto doesn't know about light/dark themes, these are hardcoded to match HBAS themes
	if (isDarkMode) {
		container->backgroundColor = fromRGB(0x2d, 0x2c, 0x31); // match theme dark background
	} else {
		container->backgroundColor = fromRGB(0xf5, 0xf5, 0xf5); // light gray for light mode
	}
	container->hasBackground = true;

	// center the container horizontally within our full screen overlay
	container->constrain(ALIGN_CENTER_HORIZONTAL);
	// position the start of the dropdown in the upper fifth of the screen
	container->y = SCREEN_HEIGHT / 5;
	
	scrollContainer->addNode(std::move(containerPtr));

	// not a part of the scrollable area
	auto backBtn = createNode<Button>(i18n("dropdown.back"), B_BUTTON, isDarkMode, 15);
	backBtn->constrain(ALIGN_BOTTOM | ALIGN_LEFT, 10);
	backBtn->setAction([]() {
		// hides the dropdown without any callback
		RootDisplay::popScreen();
	});

	addNode(std::move(scrollContainer));
}

void DropDownChoices::render(Element* parent) {
	super::render(parent);
}

bool DropDownChoices::process(InputEvents* event) {
	// if we have an up or down button event, instead of scrolling the list element, move cursor selection within the container
	if (event->held(A_BUTTON) && this->curHighlighted >= 0 && this->curHighlighted < (int)container->elements.size()) {
		this->container->elements[this->curHighlighted]->action(); // fire that button's action
		return true;
	}

	if (event->held(B_BUTTON)) {
		// dismiss without a selection - pop this screen
		RootDisplay::popScreen();
		return true;
	}

	if (event->isTouch()) {
		// unhighlight whatever may be highlighted
		this->curHighlighted = -1;
	} else {
		if (event->isKeyDown() && event->held(UP_BUTTON | DOWN_BUTTON | LEFT_BUTTON | RIGHT_BUTTON)) {
			// Similar to HBAS's AppList navigation logic
			
			// look up whatever is currently chosen as the highlighted position
			// and remove its highlight
			if (curHighlighted >= 0 && curHighlighted < (int)container->elements.size() && container->elements[curHighlighted])
				container->elements[curHighlighted]->elasticCounter = NO_HIGHLIGHT;

			// adjust it by R for up and down
			this->curHighlighted += -1 * (event->held(UP_BUTTON)) + (event->held(DOWN_BUTTON));

			// don't let the cursor go out of bounds
			if (curHighlighted >= (int)container->elements.size()) curHighlighted = container->elements.size() - 1;
			if (curHighlighted < 0) curHighlighted = 0;

			// highlight the new element
			if (curHighlighted < (int)container->elements.size() && container->elements[curHighlighted])
				container->elements[curHighlighted]->elasticCounter = THICK_HIGHLIGHT;

			// Auto-scroll to keep highlighted element on screen (similar to AppList logic)
			if (curHighlighted >= 0 && curHighlighted < (int)container->elements.size() && container->elements[curHighlighted] && scrollList) {
				Element* curElement = container->elements[curHighlighted].get();
				
				// Calculate the y-position of the currently highlighted element on screen (accounting for scroll)
				// scrollList->y is the scroll offset, container->y is the initial position, curElement->y is relative to container
				int normalizedY = container->y + curElement->y + scrollList->y;
				
				// If element is going off the top of the screen, scroll down to keep it visible
				if (normalizedY < 50) {
					event->wheelScroll = 1;
				}
				// If element is going off the bottom of the screen, scroll up to keep it visible
				else if (normalizedY + curElement->height > SCREEN_HEIGHT - 50) {
					event->wheelScroll = -1;
				}
			}

			return true;
		}
	}

	return Screen::process(event);
}

} // namespace Chesto