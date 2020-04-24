#include "Button.hpp"

CST_Color Button::colors[2] = {
	{ 0x00, 0x00, 0x00, 0xff }, // light
	{ 0xff, 0xff, 0xff, 0xff }, // dark
};

Button::Button(const char* message, int button, bool dark, int size, int width)
	: dark(dark)
	, physical(button)
	, icon(getUnicode(button), size * 1.25, &colors[dark], ICON)
	, text(message, size, &colors[dark])
{
	int PADDING = 10;

	int bWidth = PADDING * 0.5 * (icon.width != 0); // gap space between button

	text.position(PADDING * 2 + bWidth + icon.width, PADDING);
	super::append(&text);

	icon.position(PADDING * 1.7, PADDING + (text.height - icon.height) / 2);
	super::append(&icon);

	this->width = (width > 0) ? width : text.width + PADDING * 4 + bWidth + icon.width;
	this->height = text.height + PADDING * 2;

	this->touchable = true;
}

const char* Button::getUnicode(int button)
{
	switch (button)
	{
		case A_BUTTON:
			return "\ue0a0";
		case B_BUTTON:
			return "\ue0a1";
		case Y_BUTTON:
			return "\ue0a2";
		case X_BUTTON:
			return "\ue0a3";
		case START_BUTTON:
			return "\ue0a4";
		case SELECT_BUTTON:
			return "\ue0a5";
		default:
			break;
	}
	return "";
}

bool Button::process(InputEvents* event)
{
	if (event->isKeyDown() && this->physical != 0 && event->held(this->physical))
	{
		// invoke our action, since we saw a physical button press that matches!
		this->action();
		return true;
	}

	return super::process(event);
}

void Button::render(Element* parent)
{
	// update our x and y according to our parent
	this->recalcPosition(parent);

	// draw bg for button
	CST_Rect dimens = { xAbs, yAbs, width, height };

	if (dark)
	{
		CST_SetDrawColor(this->renderer, (CST_Color){ 0x67, 0x6a, 0x6d, 0xFF } );
#if defined(__WIIU__)
		CST_SetDrawColor(this->renderer, (CST_Color){ 0x3b, 0x3c, 0x4e, 0xFF } );
#endif
	}
	else
		CST_SetDrawColor(this->renderer, (CST_Color){ 0xee, 0xee, 0xee, 0xFF } );

	CST_FillRect(this->renderer, &dimens);

	super::render(this);
}
