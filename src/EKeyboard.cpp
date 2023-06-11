#include "EKeyboard.hpp"

using namespace std;

EKeyboard::EKeyboard() : EKeyboard::EKeyboard(NULL)
{
	storeOwnText = true;
}

EKeyboard::EKeyboard(std::function<void(char)> typeAction)
{
	this->x = 30;
	this->y = SCREEN_HEIGHT - 420;

	this->width = SCREEN_WIDTH - 380;

	this->typeAction = typeAction;

	curRow = index = -1;
	this->isAbsolute = true;

	// position the EKeyboard based on this x and y
	updateSize();
}

void EKeyboard::render(Element* parent)
{
	if (hidden || immersiveMode)
		return;

	CST_Rect dimens = { this->x, this->y, this->width + 305, this->height + 140 };

	auto renderer = getRenderer();

	CST_SetDrawColor(renderer, { 0xf9, 0xf9, 0xf9, 0xFF });
	if (hasRoundedKeys) {
		CST_SetDrawColor(renderer, { 0xdd, 0xdd, 0xdd, 0xFF });

		// rounded keys keyboard is full screen width
		dimens.x = 0;
		dimens.y -= 15;
		dimens.w = RootDisplay::screenWidth;
	}

	CST_FillRect(renderer, &dimens);

	for (int y = 0; y < rowCount(); y++)
		for (int x = 0; x < rowLength(y) + 1; x++)
		{
			CST_Rect dimens2 = { this->x + kXPad + x * kXOff + y * yYOff, this->y + kYPad + y * ySpacing, keyWidth, keyWidth };
			if (this->hasRoundedKeys) {
				CST_roundedBoxRGBA(renderer, dimens2.x, dimens2.y, dimens2.x + dimens2.w, dimens2.y + dimens2.h, 20, 0xee, 0xee, 0xee, 0xff);
			} else {
				CST_SetDrawColor(renderer, { 0xf4, 0xf4, 0xf4, 0xff });
				CST_FillRect(renderer, &dimens2);
			}

			// draw the letters with fontcache, for rounded keys
			if (hasRoundedKeys) {
				char curChar = rows[y]->at(x*2);
				auto curCharStr = std::string(1, curChar);
				int fHeight = CST_GetFontHeight(roundKeyFont, curCharStr.c_str());
				int fWidth = CST_GetFontWidth(roundKeyFont, curCharStr.c_str());
				CST_DrawFont(
					roundKeyFont,
					renderer,
					dimens2.x + dimens2.w/2 - fWidth/2,
					dimens2.y + dimens2.h/2 - fHeight/2,
					curCharStr.c_str()
				);
			}
		}

	CST_Rect dimensSpace = { this->x + sPos, this->y + dHeight, sWidth, textSize };
	CST_Rect dimensEnter = { this->x + enterPos, this->y + enterHeight, enterWidth, (int)(1.5 * textSize) };
	CST_Rect dimensTab = { this->x + dPos, this->y + enterHeight, enterWidth, (int)(1.5 * textSize) };

	// if there's a highlighted piece set, color it in
	if (curRow >= 0 || index >= 0)
	{
		CST_Rect dimens2 = { this->x + kXPad + index * kXOff + curRow * yYOff, this->y + kYPad + curRow * ySpacing, keyWidth, keyWidth };

		if (curRow >= rowCount())
		{
			switch (index)
			{
				case 0:
					dimens2 = dimensTab;
					break;
				case 1:
					// if we're on SPACE, expand the dimens width of the highlighted button
					dimens2 = dimensSpace;
					break;
				case 2:
					// if we're on ENTER, expand the dimens width of the highlighted button
					dimens2 = dimensEnter;
					break;
				default:
					break;
			}
		}

		// draw the currently selected tile if these index things are set
		if (touchMode)
		{
			if (hasRoundedKeys) {
				CST_roundedBoxRGBA(renderer, dimens2.x, dimens2.y, dimens2.x + dimens2.w, dimens2.y + dimens2.h, 20, 0xff, 0xff, 0xff, 0x90);
			} else {
				CST_SetDrawColor(renderer, { 0xad, 0xd8, 0xe6, 0x90 }); // TODO: matches the DEEP_HIGHLIGHT color
				CST_FillRect(renderer, &dimens2);
			}
		}
		else if (hasRoundedKeys) {
			CST_roundedBoxRGBA(renderer, dimens2.x, dimens2.y, dimens2.x + dimens2.w, dimens2.y + dimens2.h, 20, 0xff, 0xff, 0xff, 0x90);
		}
		else {
			CST_SetDrawColor(renderer, { 0xff, 0xff, 0xff, 0xff }); // TODO: matches the DEEP_HIGHLIGHT color
			CST_FillRect(renderer, &dimens2);

			// border
			for (int z = 4; z >= 0; z--)
			{
				dimens2.x--;
				dimens2.y--;
				dimens2.w += 2;
				dimens2.h += 2;

				CST_SetDrawColor(renderer, { 0x66 - z * 10, 0x7c + z * 20, 0x89 + z * 10, 0xFF });
				CST_DrawRect(renderer, &dimens2);
			}
		}
	}

	if (hasRoundedKeys) {
		CST_roundedBoxRGBA(renderer, dimensSpace.x, dimensSpace.y, dimensSpace.x + dimensSpace.w, dimensSpace.y + dimensSpace.h, 20, 0xee, 0xee, 0xee, 0xff);
		CST_roundedBoxRGBA(renderer, dimensEnter.x, dimensEnter.y, dimensEnter.x + dimensEnter.w, dimensEnter.y + dimensEnter.h, 20, 0xee, 0xee, 0xee, 0xff);
		CST_roundedBoxRGBA(renderer, dimensTab.x, dimensTab.y, dimensTab.x + dimensTab.w, dimensTab.y + dimensTab.h, 20, 0xee, 0xee, 0xee, 0xff);
	} else {
		CST_SetDrawColor(renderer, { 0xf4, 0xf4, 0xf4, 0xff });
		CST_FillRect(renderer, &dimensSpace);

		if (!preventEnterAndTab) {
			CST_FillRect(renderer, &dimensEnter);
			CST_FillRect(renderer, &dimensTab);
		}
	}

	super::render(this);
}

bool EKeyboard::process(InputEvents* event)
{
	InputEvents::bypassKeyEvents = false;

	// don't do anything if we're hidden, or there's a sidebar and it's active
	if (hidden)
		return false;

	// our keyboard will be processing its own key events (not button events)
	InputEvents::bypassKeyEvents = true;

	if (event->type == SDL_KEYDOWN)
		return listenForPhysicalKeys(event);
	if (event->type == SDL_KEYUP &&	(event->keyCode == SDLK_LSHIFT ||
	                                 event->keyCode == SDLK_RSHIFT ||
		                             event->keyCode == SDLK_CAPSLOCK)) {
		shiftOn = false;
		updateSize();
		return true;
	}

	// immersive view doesn't use any keyboard touch events
	if (immersiveMode) return false;

	if (event->isTouchDown())
	{
		curRow = index = -1;
		touchMode = true;
	}

	if (event->isKeyDown())
		touchMode = false;

	bool ret = false;

	if (!touchMode)
	{
		if (curRow < 0 && index < 0)
		{
			// switched into EKeyboard, set to 0 and return
			curRow = 1;
			index = 0;
			return true;
		}

		if (event->isKeyDown())
		{
			int lastRow = curRow;
			curRow += (event->held(DOWN_BUTTON) - event->held(UP_BUTTON));
			index += (event->held(RIGHT_BUTTON) - event->held(LEFT_BUTTON));

			if (curRow < 0) curRow = 0;
			if (index < 0) index = 0;
			if (curRow >= rowCount() + 1) curRow = rowCount(); // +1 for bottom "row" (tab, space, enter)
			if (curRow == rowCount())
			{
				// go to space key if last index is in the middle of row
				if (lastRow < curRow && index > 0 && index < rowLength(lastRow))
				{
 					index = 1;
				}

				// tab key
				if (index < 0) index = 0;

				// enter key
				if (index > 2) index = 2;
			}
			else
			{
				if (index > rowLength(curRow))
					index = rowLength(curRow);

				if (lastRow == rowCount()) {
					switch (index)
					{
						case 0: //tab
							index = 0;
							break;
						case 1: // space
							// go to middle of current row
							index = rowLength(curRow) / 2;
							break;
						case 2: // enter
							// go to end of current row
							index = rowLength(curRow);
							break;
						default:
							break;
					}
				}
			}

			if (event->held(A_BUTTON))
			{
				// space bar and enter key
				if (curRow >= rowCount())
				{
					switch (index)
					{
						case 0:
							just_type('\t');
							break;
						case 1:
							just_type(' ');
							break;
						case 2:
							just_type('\n');
							break;
						default:
							break;
					}
					return true;
				}

				type(curRow, index);
			}

			// updateView();
			return true;
		}

		return false;
	}

	int extWidth = width + 305;

	if (event->isTouchDown() && event->touchIn(this->x, this->y, extWidth, height + 200))
	{
		for (int y = 0; y < rowCount(); y++)
			for (int x = 0; x < rowLength(y) + 1; x++)
				if (event->touchIn(this->x + kXPad + x * kXOff + y * yYOff, this->y + kYPad + y * ySpacing, keyWidth, keyWidth))
				{
					ret |= true;
					curRow = y;
					index = x;
				}
		return true;
	}

	if (event->isTouchUp())
	{
		// only proceed if we've been touchdown'd
		// reset current row and info
		curRow = -1;
		index = -1;

		if (event->touchIn(this->x, this->y, extWidth, height + 200))
		{

			for (int y = 0; y < rowCount(); y++)
				for (int x = 0; x < rowLength(y) + 1; x++)
					if (event->touchIn(this->x + kXPad + x * kXOff + y * yYOff, this->y + kYPad + y * ySpacing, keyWidth, keyWidth))
					{
						ret |= true;
						type(y, x);
					}

			if (event->touchIn(this->x + dPos, this->y + enterHeight, enterWidth, textSize))
			{
				ret |= true;
				just_type('\t');
			}

			if (event->touchIn(this->x + sPos, this->y + dHeight, sWidth, textSize))
			{
				ret |= true;
				just_type(' ');
			}

			if (event->touchIn(this->x + enterPos, this->y + enterHeight, enterWidth, textSize))
			{
				ret |= true;
				just_type('\n');
			}

			// if (ret)
			// 	updateView();

			return ret;
		}

		return false;
	}

	return false;
}

bool EKeyboard::listenForPhysicalKeys(InputEvents* e)
{
	int curBreak = 0;
	int offset = 0;

	auto keyCode = e->keyCode;
	auto mod = e->mod;

	// special keys
	if (keyCode == SDLK_LSHIFT || keyCode == SDLK_RSHIFT || keyCode == SDLK_CAPSLOCK) {
		shiftOn = true;
		updateSize();
		return true;
	}

	if (keyCode == SDLK_TAB) {
		just_type('\t');
		return true;
	}
	if (keyCode == SDLK_SPACE) {
		just_type(' ');
		return true;
	}
	if (keyCode == SDLK_RETURN && !preventEnterAndTab) {
		just_type('\n');
		return true;
	}

	// alt key will toggle the keyboard, but still allow inputs
	if (keyCode == SDLK_LALT || keyCode == SDLK_RALT) {
		immersiveMode = !immersiveMode;
		return true;
	}

	// primary typing loop, handles all the keycodes on our keyboard
	auto roundedOffset = hasRoundedKeys ? 1 : 0;
	for (int x=roundedOffset; x<KEYCODE_COUNT; x++)
	{
		int xx = x - offset;
		int topRowOffset = curBreak == 0 ? roundedOffset : 0;

		if (keyCode == usbKeys[x])
		{
			// we got a key down for this code, type our current position
			// and update cursor
			type(curBreak, xx - topRowOffset);
			curRow = curBreak;
			index = xx - topRowOffset;
			return true;
		}

		if (xx + 1 >= breaks[curBreak]) {
			curBreak++;
			offset = x + 1;
		}
	}

	// below are progammatic invocations of certain features we expect to be there
	// in vgedit
	// TODO: either make these callbacks or internal calls rather than going through SDL event engine

	if (keyCode == SDLK_BACKSPACE) {
		// programatically invoke the B button event
		backspace();
		return true;
	}

	if (keyCode == SDLK_RETURN && preventEnterAndTab) {
		// if don't allow enter, let's consider RETURN as submitting the type action (hardcoded to X button)
		// TODO: make it its own callback
		SDL_Event sdlevent;
		sdlevent.type = SDL_JOYBUTTONDOWN;
		sdlevent.jbutton.button = SDL_X;
		SDL_PushEvent(&sdlevent);
		return true;
	}

	if (keyCode == SDLK_ESCAPE) {
		// dismiss keyboard, also programmatically
		SDL_Event sdlevent;
		sdlevent.type = SDL_JOYBUTTONDOWN;
		sdlevent.jbutton.button = SDL_Y;
		SDL_PushEvent(&sdlevent);
		return true;
	}

	return false;
}

void EKeyboard::updateSize()
{
	this->elements.clear();
	rows.clear();

	this->height = (304 / 900.0) * width;

	// set up lots of scaling variables based on the width/height

	this->keyWidth = (int)(0.08 * width);
	this->padding = keyWidth / 2.0;

	// these field variables are for displaying the QWERTY keys (touching and displaying)
	kXPad = (int)((23 / 400.0) * width);
	kXOff = (int)((36.5 / 400.0) * width) + (900.0 / width) * ( 900.0 != width); // when scaling, adjust our key x offset TODO: probably a guesstimate
	yYOff = (int)((22 / 400.0) * width);
	kYPad = (int)((17 / 135.0) * height);
	ySpacing = (int)((33 / 400.0) * width);

	if (hasRoundedKeys) {
		// different positioning for round keyboard
		// TODO: something else
		kXOff = (int)((38 / 400.0) * width) + (900.0 / width) * ( 900.0 != width); // when scaling, adjust our key x offset TODO: probably a guesstimate
		yYOff = (int)((10 / 400.0) * width);
		kYPad = (int)((0 / 135.0) * height);
		ySpacing = (int)((37 / 400.0) * width);
	}

	// these local variables position only the text, and has nothing to do with the
	// touch. They should likely be based on the above field variables so those
	// can change quickly
	int kXPad = (int)((30 / 400.0) * width);
	int kXOff = (int)((22 / 400.0) * width);
#ifdef __WIIU__
	// cheeky positioning hack because wiiu screen dimensions are different
	// to properly fix, the above comments would need to be addressed
	int kYPad = (int)((14 / 400.0) * width);
#endif
	int kYOff = (int)((33 / 400.0) * width);

	this->textSize = 0.9375 * keyWidth;

	// delete, space and enter key dimensions
	dPos = (int)((13 / 400.0) * width);
	dHeight = (int)((85 / 135.0) * height) + 145;
	sPos = (int)((150 / 400.0) * width);
	enterPos = dPos + 1000;
	enterHeight = dHeight - 34;

	dWidth = (int)(1.4125 * textSize);
	sWidth = (int)(7.5 * textSize);
	enterWidth = (int)(2.25 * textSize);

	// set up the keys vector based on the current EKeyboard selection
	generateEKeyboard();

	CST_Color gray = { 0x52, 0x52, 0x52, 0xff };

	int targetHeight = -1;

	// go through and draw each of the three rows at the right position
	if (!hasRoundedKeys) {
		for (int x = 0; x < rowCount(); x++)
		{
			TextElement* rowText = new TextElement(rows[x]->c_str(), textSize, &gray, true);
			// rowText->customFontPath = RAMFS "res/lightsans.ttf";
			if (targetHeight < 0) {
				targetHeight = rowText->height;
			}
			// rowText->update(true);
			rowText->position(kXPad + x * kXOff, kYPad + x * kYOff + targetHeight/2 - rowText->height/2);
			this->elements.push_back(rowText);
		}
	}

	// text for space, enter, and symbols
	CST_Color grayish = { 0x55, 0x55, 0x55, 0xff };
	TextElement* spaceText = new TextElement("space", 30, &grayish);
	CST_Rect d4 = { this->x + sPos, this->y + dHeight, sWidth, textSize }; // todo: extract out hardcoded rects like this
	spaceText->position(d4.x + d4.w / 2 - spaceText->width / 2 - 15, 345);
	this->elements.push_back(spaceText);

	if (!preventEnterAndTab)
	{
		TextElement* enterText = new TextElement("enter", 30, &grayish);
		CST_Rect d3 = { this->x + enterPos, this->y + enterHeight, enterWidth, textSize }; // todo: extract out hardcoded rects like this
		enterText->position(d3.x + d3.w / 2 - enterText->width / 2 - 30, 327);
		this->elements.push_back(enterText);

		TextElement* symText = new TextElement(hasRoundedKeys ? "shift" : "tab", 30, &grayish);
		CST_Rect d5 = { this->x + dPos, this->y + enterHeight, enterWidth, textSize }; // todo: extract out hardcoded rects like this
		symText->position(d5.x + d5.w / 2 - symText->width / 2 - 30, 327);
		this->elements.push_back(symText);
	}
}

// given a (y, x) pair in the keyboard layout, type the corresponding letter
void EKeyboard::type(int y, int x)
{
	const char input = (*(rows[y]))[x * 2];
	just_type(input);
}

// call type action w/ some checks
void EKeyboard::just_type(const char input)
{
	if (preventEnterAndTab && (input == '\n' || input == '\t'))
		return;

	// store our own string if we need to
	if (storeOwnText)
		textInput.push_back(input);

	// call the underlying type action, if present
	if (typeAction != NULL) {
		typeAction(input);
		return;
	}
}

void EKeyboard::generateEKeyboard()
{
	int count = 0;
	string keys;

	if (mode == 0)
		keys = string((shiftOn || capsOn) ? upper_keys : lower_keys);
	else
	{
		// depending on the mode, grab a bunch of characters from unicode starting from
		// upside exclamation mark (U+00A1) and onward https://unicode-table.com/en/
		// TODO: don't hardcode amount here, or hardcode a better one
		int offset = 47 * (mode - 1) + 0x00a1;
		char chars[47 + (mode == 2)];
		for (int x = 0; x < 47 + (mode == 2); x++)
		{
			chars[x] = offset + x;
		}

		keys = string(chars);
	}

	breaks[3] += (mode == 2); // one more key for bottom row of mode 2

	for (int& end : breaks)
	{
		auto roundedOffset = (hasRoundedKeys && count == 0) ? 1 : 0;
		string* row = new string(keys.substr(count + roundedOffset, end - roundedOffset));
		for (int x = 1; x < (int)row->size(); x += 2)
		{
			row->insert(row->begin() + x, ' ');
		}

		rows.push_back(row);
		count += end;
	}

	if (hasRoundedKeys && roundKeyFont == NULL) {
		// load the round key font
		roundKeyFont = CST_CreateFont();
		auto fontPath = RAMFS "res/lightsans.ttf";
		auto renderer = getRenderer();
		CST_LoadFont(roundKeyFont, renderer, fontPath, 40, CST_MakeColor(0,0,0,255), TTF_STYLE_NORMAL);
	}
}

void EKeyboard::backspace()
{
	if (storeOwnText) {
		// manage our own internal string
		if (!textInput.empty())
			textInput.pop_back();
		// call out to the typing callback TODO: use a separate callback (see below)
		typeAction('\b'); // not great
		return;
	}

	// TODO: use a backspace callback instead of hardcoding a B button event
	// (B is used by vgedit to manage external backspaces)
	SDL_Event sdlevent;
	sdlevent.type = SDL_JOYBUTTONDOWN;
	sdlevent.jbutton.button = SDL_B;
	SDL_PushEvent(&sdlevent);
}

const std::string& EKeyboard::getTextInput()
{
	return textInput;
}

EKeyboard::~EKeyboard()
{
	InputEvents::bypassKeyEvents = false;
}
