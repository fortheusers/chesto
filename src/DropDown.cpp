#include "DropDown.hpp"
#include "Button.hpp"
#include "Constraint.hpp"
#include <iostream>

DropDown::DropDown(
	DropDownControllerElement* parentView,
	int physicalButton,
	std::map<std::string, std::string> choices,
	std::function<void(std::string)> onSelect,
	int textSize,
	std::string defaultChoice,
	bool isDarkMode
) : Button(defaultChoice.empty() ? "Select..." : (choices.find(defaultChoice) != choices.end() ? choices[defaultChoice] : defaultChoice), physicalButton, isDarkMode, textSize, 0)
{
	this->choices = choices;
	this->onSelect = onSelect;
	this->controller = parentView;
	
	this->setAction([this, parentView, isDarkMode]() {
		// when clicked, open the dropdown choices subscreen
		dropDownScreen = new DropDownChoices(this->choices, this, isDarkMode);
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
DropDownChoices::DropDownChoices(std::map<std::string, std::string> choices, DropDown* dropdown, bool isDarkMode) {
	this->width = SCREEN_WIDTH;
	this->height = SCREEN_HEIGHT;
	
	// overlay and shade bg color TODO: do this directly in DropDownControllerElement render method, without an overlay Element?
	Element* overlay = new Element();
    overlay->width = this->width;
    overlay->height = this->height;
    overlay->backgroundColor = fromRGB(0, 0, 0);
    overlay->backgroundOpacity = 0x70;
    overlay->cornerRadius = 1; // forces transparency to render properly (via sdl_gfx)
    overlay->hasBackground = true;
	overlay->isAbsolute = true;
    this->child(overlay); // first element is this full screen barrier
	
	// create a vertical container
	container = new Container(COL_LAYOUT, 20);
	container->backgroundColor = dropdown->backgroundColor;
	container->hasBackground = true;

	// center the container horizontally within our full screen overlay
	container->constrain(ALIGN_CENTER_HORIZONTAL);
	// position the start of the dropdown in the upper fifth of the screen
	// TODO: auto scroll towards the default selection
	container->y = SCREEN_HEIGHT / 5;
	
	// populate the dropdown choices
	int widestWidth = 0;
	for (const auto& choice : dropdown->choices) {
		auto choiceElement = new Button(choice.second, 0, isDarkMode, 20, 0);
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
			dropdown->controller->curDropDown = nullptr; // TODO: probably this cleanup should be a method
			dropdown->dropDownScreen = nullptr;
			this->container = nullptr;
		});
		container->add(choiceElement);
	}

	// this->width = widestWidth; // our container dropdown also needs this width

	// for each choice, make its width the widest 
	for (const auto& child : container->elements) {
		if (auto buttonChild = dynamic_cast<Button*>(child)) {
			buttonChild->fixedWidth = widestWidth;
			buttonChild->updateBounds();
		}
	}

	super::append(container);
}

void DropDownChoices::render(Element* parent) {
	super::render(parent);
}

bool DropDownChoices::process(InputEvents* event) {
	// if we have an up or down button event, instead of scrolling the list element, move cursor selection within the container
	if (event->held(A_BUTTON) && this->curHighlighted >= 0) {
		this->container->elements[this->curHighlighted]->action(); // fire that button's action
		return true;
	}

	if (event->held(B_BUTTON)) {
		// just dismiss without a selection
		// TODO: that
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
			if (curHighlighted >= 0 && container->elements[curHighlighted])
				container->elements[curHighlighted]->elasticCounter = NO_HIGHLIGHT;

			// adjust it by R for up and down
			this->curHighlighted += -1 * (event->held(UP_BUTTON)) + (event->held(DOWN_BUTTON));

			// don't let the cursor go out of bounds
			if (curHighlighted >= (int)container->elements.size()) curHighlighted = container->elements.size() - 1;
			if (curHighlighted < 0) curHighlighted = 0;

			// highlight the new element
			if (container->elements[curHighlighted])
				container->elements[curHighlighted]->elasticCounter = THICK_HIGHLIGHT;

			return true;
		}
	}

	// Auto-scroll to keep highlighted element on screen (similar to AppList logic)
	if (curHighlighted >= 0 && curHighlighted < (int)container->elements.size() && container->elements[curHighlighted]) {
		Element* curElement = container->elements[curHighlighted];
		
		// Calculate the y-position of the currently highlighted element on screen (accounting for scroll)
		// this->y is the scroll offset, container->y is the initial position, curElement->y is relative to container
		int normalizedY = container->y + curElement->y + this->y;
		
		// If element is going off the top of the screen, scroll down to keep it visible
		if (normalizedY < 100) {
			event->wheelScroll = 1;
		}
		// If element is going off the bottom of the screen, scroll up to keep it visible
		else if (normalizedY + curElement->height > SCREEN_HEIGHT - 100) {
			event->wheelScroll = -1;
		}
	}

	// pass remaining processing (touch events mostly) to the container as usual
	bool ret = ListElement::process(event); // ListElement's, which handles scrolling

	// If container was deleted (from above process) stop!
	if (!container) {
		return ret;
	}

	// Calculate the absolute bottom position of the container
	// container->yAbs accounts for both the container's initial y position and any scrolling (this->y)
	int containerBottom = container->yAbs + container->height;
	
	// Define minimum allowed bottom position (keep 50% of container visible)
	int minBottomPosition = container->height / 2;
	
	// If container bottom has scrolled too far up (less than minBottomPosition), snap back
	if ((!event->isScrolling || event->isTouchUp()) && containerBottom < minBottomPosition) {
		// Calculate what this->y should be to put the bottom at minBottomPosition
		int adjustment = minBottomPosition - containerBottom;
		this->y += adjustment;
		// Zero out elastic counter to prevent jittery re-snapping
		this->elasticCounter = 0;
	}

	return ret;
}

bool DropDownControllerElement::process(InputEvents* event) {
	// if we have a current dropdown open, pass events to it instead
	if (curDropDown) {
		return curDropDown->process(event);
	}
	return super::process(event);
}