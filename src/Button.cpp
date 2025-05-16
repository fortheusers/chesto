#include "Button.hpp"
#include <iostream>

CST_Color Button::colors[2] = {
	{ 0x00, 0x00, 0x00, 0xff }, // light
	{ 0xff, 0xff, 0xff, 0xff }, // dark
};

Button::Button(std::string message, int button, bool dark, int size, int width)
	: physical(button)
	, dark(dark)
	, icon(getControllerButtonImageForPlatform(button, !dark, false))
	, text(message, (size / SCALER), &colors[dark])
{

	super::append(&text);
	super::append(&icon);

	// on initialization, store the last gamepad info
	myLastSeenGamepad = "";

	icon.resize(text.height*1.5, text.height*1.5);
	
	if (shouldRenderGlossy()) {
		icon.hide();
	}

	fixedWidth = width;

	updateBounds();

	this->touchable = true;
	this->hasBackground = true;

	// protect "stack" children
	text.isProtected = icon.isProtected = true;

	if (dark)
	{
		backgroundColor = RootDisplay::mainDisplay->backgroundColor;
		backgroundColor.r += 0x25/255.0;
		backgroundColor.g += 0x25/255.0;
		backgroundColor.b += 0x25/255.0;
		// backgroundColor.r = fmin(backgroundColor.r, 1.0);
	}
	else
		backgroundColor = (rgb){ 0xee/255.0, 0xee/255.0, 0xee/255.0 };
}

void Button::updateBounds()
{
	int PADDING = 10 / SCALER;

	int bWidth = PADDING * 0.5 * (icon.width != 0); // gap space between button

	if (shouldRenderGlossy()) {
		icon.position(0, 0); // the icon is the overlay, same size as the button
 	} else {
		text.position(PADDING * 2 + bWidth + icon.width, PADDING);
		icon.position(PADDING * 1.7, PADDING + (text.height - icon.height) / 2);
	}

	this->width = (fixedWidth > 0) ? fixedWidth : text.width + PADDING * 4 + bWidth + (icon.hidden ? 0 : icon.width);
	this->height = text.height + PADDING * 2;

	if (shouldRenderGlossy()) {
		text.position(width / 2 - text.width / 2, PADDING); // y is already centered
	}
}

void Button::updateText(const char* inc_text)
{
	this->text.setText(inc_text);
	this->text.update();
	updateBounds();
}

std::string Button::getControllerButtonImageForPlatform(int button, bool isGray, bool isOutline)
{
	// grab the current gamepad info
	GamepadInfo& gamepad = InputEvents::getLastGamepadInfo();

	// find the button index in the gamepad.buttons array
	// TODO: use a hashmap instead of an array
	if (gamepad.buttons != nullptr) {
		for (int i = 0; i < TOTAL_BUTTONS; i++)
		{
			if (gamepad.buttons[i] == button)
			{
				auto outlineSuffix = isOutline ? "_outline" : "";
				auto graySuffix = isGray ? "_gray" : "";
				auto retVal = RAMFS "res/controllers/buttons/" + gamepad.prefix + "_" + gamepad.names[i] + outlineSuffix + graySuffix + ".svg";
				return retVal;
			}
		}
	}
	// if we didn't find it, return an empty string
	printf("Button %d not found in gamepad, returning empty string\n", button);
	return "";
}

void Button::render(Element* parent)
{
	if (hidden) return;

	CST_SetDrawBlend(RootDisplay::renderer, true);

	this->recalcPosition(parent);

	if (shouldRenderGlossy()) {
		// draw a glossy button
		auto r = backgroundColor.r * 0xff;
		auto g = backgroundColor.g * 0xff;
		auto b = backgroundColor.b * 0xff;

		CST_roundedBoxRGBA(
			RootDisplay::renderer,
			xAbs, yAbs, xAbs + width, yAbs + height, cornerRadius,
			r, g, b, 0xff
		);
		// slightly darker overlay for the second half of the button
		auto darkenOffset = dark ? 0x10 : 0x00; // use a deeper color for dark buttons
		CST_roundedBoxRGBA(
			RootDisplay::renderer,
			xAbs, yAbs + height / 2, xAbs + width, yAbs + height, cornerRadius,
			0x00, 0x00, 0x00, 0x15 + darkenOffset
		);
		// inner white outline around the whole thing
		auto p = 0.5;
		CST_roundedRectangleRGBA(
			RootDisplay::renderer,
			xAbs+2*p, yAbs+2*p, xAbs + width - p, yAbs + height - p, cornerRadius,
			0xff, 0xff, 0xff, 0x40
		);
		// thicker outer gray outline
		CST_roundedRectangleRGBA(
			RootDisplay::renderer,
			xAbs, yAbs, xAbs + width, yAbs + height, cornerRadius,
			0x00, 0x00, 0x00, 0x80
		);
	}

	// render text/icon and bg if needed
	super::render(parent);

	CST_SetDrawBlend(RootDisplay::renderer, false);
}

bool Button::process(InputEvents* event)
{
	if (event->isKeyDown() && this->physical != 0 && event->held(this->physical))
	{
		// invoke our action, since we saw a physical button press that matches!
		this->action();
		return true;
	}

	bool ret = false;

	// if the last gamepad is different from the current one, update the button image
	if (myLastSeenGamepad != InputEvents::lastGamepadKey)
	{
		if (shouldRenderGlossy()) {
			icon.hide();
			hasBackground = false; // we'll handle our own two-toned and outlined background drawing
			cornerRadius = fmin(25, height / 4); // bound the corner radius to the height of our darker highlight
		} else {
			auto newPath = getControllerButtonImageForPlatform(this->physical, !dark, false);
			icon.unhide();
			hasBackground = true;
			icon.loadPath(newPath);
			icon.resize(text.height*1.5, text.height*1.5);
		}
		myLastSeenGamepad = InputEvents::lastGamepadKey;
	}

	return super::process(event) || ret;
}
  
const std::string Button::getText()
{
	return this->text.text;
}

bool Button::shouldRenderGlossy()
{
#ifdef WII_MOCK
	return true;
#endif

	return InputEvents::lastGamepadKey == "Wii Remote" || InputEvents::lastGamepadKey == "Wii Remote and Nunchuk";
}