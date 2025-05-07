#include "gamepadmapping.hpp"
#include "RootDisplay.hpp"
#include <map>
#include <iostream>

// if true, don't count key inputs (PC/usb keyboard) as button events for us
bool InputEvents::bypassKeyEvents = false;

auto defaultKeyName = "WiiU Gamepad";
std::string InputEvents::lastGamepadKey = defaultKeyName;
unsigned int* currentButtons = nintendo_buttons;
std::string* currentButtonNames = nintendoButtonNames;

// map of controller name to buttons, names, prefix, and controller type
std::map<std::string, GamepadInfo> gamepadMap = {
	/* Non-controller types, like keyboard */
	{ "Keyboard", GamepadInfo(nintendo_buttons, keyButtonNames, "keyboard", "key") },
	/* These 5 names are returned by the wiiu SDL2 port */
	{ "WiiU Gamepad", GamepadInfo(nintendo_buttons, nintendoButtonNames, "wiiu_button", "gamepad") },
	{ "WiiU Pro Controller", { nintendo_buttons, nintendoButtonNames, "wiiu_button", "pro" } },
	{ "Wii Remote", { wii_buttons, wiiButtonNames, "wii_button", "remote" } },
	{ "Wii Remote and Nunchuk", { nunchuk_buttons, nunchukButtonNames, "wii_button", "nunchuk" } },
	{ "Wii Classic Controller", { nintendo_buttons, nintendoButtonNames, "wii_button", "classic"} },
	/* The switch SDL2 port only returns this string for all controller types*/
	{ "Switch Controller", { nintendo_buttons, nintendoButtonNames, "switch_button", "pro" } },
	/* The XBOX 360 controller is used as a generic input device on some platforms */
	{ "X360 Controller", { xbox_buttons, xboxButtonNames, "wiiu_button", "pro" } } // use wiiu button icon assets, which are similar, TODO: get xbox ones
};

InputEvents::InputEvents()
{
#if defined(__WIIU__) && defined(USE_KEYBOARD)
	// hook up keyboard events for wiiu and SDL (TODO: have these fired by SDL2 port itself)
	KBWrapper* kbdwrapper = new KBWrapper(true, true);
#endif
}

bool InputEvents::processSDLEvents()
{
	// get an event from SDL
	if (!SDL_PollEvent(&event))
		return false;

	// update our variables
	this->type = event.type;
	this->noop = false;

	// process joystick hotplugging events
	processJoystickHotplugging(&event);

	std::string curControllerName= lastGamepadKey;

	// get the controller name
	if (this->type == SDL_KEYDOWN || this->type == SDL_KEYUP) {
		lastGamepadKey = "Keyboard";
	} else if (this->type == SDL_CONTROLLERBUTTONDOWN || this->type == SDL_CONTROLLERBUTTONUP) {
		auto controllerId = SDL_GameControllerFromInstanceID(event.cbutton.which);
		if (controllerId != NULL) {
			std::string controllerName = SDL_GameControllerName(controllerId);
			lastGamepadKey = defaultKeyName; // default in case no match is found
			if (!controllerName.empty() && gamepadMap.find(controllerName) != gamepadMap.end()){
				lastGamepadKey = controllerName;
			}
			// std::cout << "Controller name: " << lastGamepadKey << std::endl;
		}
	}
	if (curControllerName != lastGamepadKey) {
		printf("Switched to controller profile: %s\n", lastGamepadKey.c_str());
		GamepadInfo& gamepadInfo = gamepadMap[lastGamepadKey];
		if (gamepadInfo.buttons != nullptr) {
			currentButtons = gamepadInfo.buttons;
		}
		// keyButtonNames = gamepadInfo.names;
		// TODO: callback to update all buttons on the UI
	}

	this->isScrolling = false;

#ifdef PC
	this->allowTouch = false;
	if (event.type == SDL_MOUSEWHEEL) {
		this->wheelScroll = event.wheel.y;
		this->isScrolling = true;
	}
#endif

	if (this->type == SDL_QUIT)
	{
#if !defined(__WIIU__)
		RootDisplay::mainDisplay->requestQuit(); // on wiiu we don't want to respond to these events in case SDL fires one
#endif
		return false; //Quitting overrides all other events.
	}
	else if (event.key.repeat == 0 && (this->type == SDL_KEYDOWN || this->type == SDL_KEYUP))
	{
		this->keyCode = event.key.keysym.sym;
		this->mod = event.key.keysym.mod;
	}
	else if (this->type == SDL_CONTROLLERBUTTONDOWN || this->type == SDL_CONTROLLERBUTTONUP)
	{
		this->keyCode = event.cbutton.button;

		auto instanceId = event.cbutton.which;
		auto timestamp = event.cbutton.timestamp;
		auto button = event.cbutton.button;

		// if our current timestamp and button matches the last one, but is a different instance id, ignore the event
		// (may indicate one controller was detected as two different ones)
		if (this->lastGamepadTimestamp == timestamp && this->lastGamepadButton == button && this->lastGamepadInstanceId != instanceId) {
			return false;
		}

		this->lastGamepadInstanceId = instanceId;
		this->lastGamepadTimestamp = timestamp;
		this->lastGamepadButton = button;
	}
	else if (this->type == SDL_CONTROLLERAXISMOTION)
	{
		// firstly, pass through raw axis data, in case the implementing app wants it
		this->axisCode = event.caxis.axis;
		this->axisValue = ((float)event.caxis.value) / SDL_JOYSTICK_AXIS_MAX;

		// std::cout << "Axis: " << int(axisCode) << " Value: " << this->axisValue << std::endl;

		// also, map ZL/ZR and optionally up/down/left/right to digital events
		for (auto idx = 0; idx < TOTAL_AXIS_COUNT; idx++)
		{
			auto myAxis = this->axisValue;
			// if we're one of the first 4*2 axes (the directions) we need to clip out either the negative or positive
			if (idx < 8) {
				// if we're not using emulated input, skip these directional checks
				if (!this->useEmulatedDigitalInputs) {
					continue;
				}
				// odd directions can only be negative, even can only be positive
				if (idx % 2 == 0) {
					myAxis = fmin(0.0f, myAxis);
				} else {
					myAxis = fmax(0.0f, myAxis);
				}
				// if it's less than the deadzone, unset it in our hold button array,
				// but skip further checks (we will never fire a button up event for a joystick direction)
				if (abs(myAxis) <= this->deadZone) {
					this->held_buttons[idx/2] = false; // divide by 2 because two sticks -> one direction
					continue;
				}
			}
			auto btnIdx = idx/2 + (idx==TOTAL_AXIS_COUNT-1); // +1 if it's the last axis (ZR)
			if (int(event.caxis.axis) == int(axis_values[idx]))
			{
				auto prevValue = this->held_buttons[btnIdx];
				auto newValue = abs(this->axisValue) > this->deadZone; // if we're over the deadzone, hold the button
				if (prevValue != newValue) {
					// state changed, so fire a release or press event
					this->type = newValue ? SDL_CONTROLLERBUTTONDOWN : SDL_CONTROLLERBUTTONUP;
					this->keyCode = axis_buttons[idx];
				}
				this->held_buttons[btnIdx] = newValue;
			}
		}
	}
	else if (this->type == SDL_MOUSEMOTION || this->type == SDL_MOUSEBUTTONUP || this->type == SDL_MOUSEBUTTONDOWN)
	{
		bool isMotion = this->type == SDL_MOUSEMOTION;

		this->yPos = isMotion ? event.motion.y : event.button.y;
		this->xPos = isMotion ? event.motion.x : event.button.x;
	}
	else if (allowTouch && (this->type == SDL_FINGERMOTION || this->type == SDL_FINGERUP || this->type == SDL_FINGERDOWN))
	{
		this->yPos = event.tfinger.y * SCREEN_HEIGHT;
		this->xPos = event.tfinger.x * SCREEN_WIDTH;
	}

	if (this->type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
		printf("Window resized to %dx%d\n", event.window.data1, event.window.data2);
		RootDisplay::mainDisplay->setScreenResolution(
			event.window.data1 * RootDisplay::dpiScale,
			event.window.data2 * RootDisplay::dpiScale
		);

		// callback to alert the app that the window changed resolution
		if (RootDisplay::mainDisplay->windowResizeCallback)
			RootDisplay::mainDisplay->windowResizeCallback();

		RootDisplay::mainDisplay->needsRedraw = true;
	}

	// offset the x, y positions by the dpi scale
	this->xPos = (int)(this->xPos * RootDisplay::dpiScale);
	this->yPos = (int)(this->yPos * RootDisplay::dpiScale);

	toggleHeldButtons();

	return true;
}

bool InputEvents::update()
{
	this->type = 0;
	this->keyCode = -1;
	this->axisCode = -1;
	this->noop = true;

	// process SDL or directional events
	return processSDLEvents() || processDirectionalButtons();
}

void InputEvents::toggleHeldButtons()
{
	int directionCode = directionForKeycode();

	if (directionCode >= 0)
	{
		if (isKeyDown())
		{
			// make sure it's not already down
			if (!held_buttons[directionCode])
			{
				// on key down, set the corresponding held boolean to true
				held_buttons[directionCode] = true;
				held_type = this->type;

				// reset the frame counter so we don't fire on this frame
				// (initial reset is lower to add a slight delay when they first start holding)
				curFrame = -25;
			}
		}

		if (isKeyUp())
		{
			// release the corresponding key too
			held_buttons[directionCode] = false;
		}
	}
}

// returns true if a directional event was fire (so that we know to keep consuming later)
bool InputEvents::processDirectionalButtons()
{
	// up the counter
	curFrame++;

	// if one of the four direction keys is true, fire off repeat events for it
	// (when rapidFire lines up only)
	if (curFrame > 0 && curFrame % rapidFireRate == 0)
	{
		for (int x = 0; x < 4; x++)
		{
			if (!held_buttons[x])
				continue;

			// send a corresponding directional event
			this->type = held_type;
			bool isGamepad = (this->type == SDL_CONTROLLERBUTTONDOWN || this->type == SDL_CONTROLLERBUTTONUP);
			this->keyCode = isGamepad ? pad_buttons[4 + x] : key_buttons[4 + x]; // send up through right directions
			this->noop = false;

			return true;
		}
	}

	return false;
}

int InputEvents::directionForKeycode()
{
	// this keycode overlaps with some other constants, so just return asap
	if (this->type == SDL_KEYDOWN && this->keyCode == SDLK_RETURN)
		return -1;

	// returns 0 1 2 or 3 for up down left or right
	switch (this->keyCode)
	{
	case SDL_UP:
	case SDLK_UP:
		return 0;
	case SDL_DOWN:
	case SDLK_DOWN:
		return 1;
	case SDL_LEFT:
	case SDLK_LEFT:
		return 2;
	case SDL_RIGHT:
	case SDLK_RIGHT:
		return 3;
	default:
		return -1;
	}
	return -1;
}

bool InputEvents::held(int buttons)
{
	// if it's a key event
#ifndef SIMPLE_SDL2
	if ((this->type == SDL_KEYDOWN || this->type == SDL_KEYUP) && !InputEvents::bypassKeyEvents)
	{
		for (int x = 0; x < TOTAL_BUTTONS; x++)
			if (key_buttons[x] == keyCode && (buttons & currentButtons[x])) {
				return true;
			}
	} else
#endif

	// if it's a controller event
	if (this->type == SDL_CONTROLLERBUTTONDOWN || this->type == SDL_CONTROLLERBUTTONUP)
	{
		for (int x = 0; x < TOTAL_BUTTONS; x++)
			if (pad_buttons[x] == keyCode && (buttons & currentButtons[x])) {
				return true;
			}
	}

	return false;
}

bool InputEvents::pressed(int buttons)
{
	return isKeyDown() && held(buttons);
}

bool InputEvents::released(int buttons)
{
	return isKeyUp() && held(buttons);
}

bool InputEvents::touchIn(int x, int y, int width, int height)
{
	return (this->xPos >= x && this->xPos <= x + width && this->yPos >= y && this->yPos <= y + height);
}

bool InputEvents::isTouchDown()
{
	return this->type == SDL_MOUSEBUTTONDOWN || (allowTouch && this->type == SDL_FINGERDOWN);
}

bool InputEvents::isTouchDrag()
{
	return this->type == SDL_MOUSEMOTION || (allowTouch && this->type == SDL_FINGERMOTION);
}

bool InputEvents::isTouchUp()
{
	return this->type == SDL_MOUSEBUTTONUP || (allowTouch && this->type == SDL_FINGERUP);
}

bool InputEvents::isTouch()
{
	return isTouchDown() || isTouchDrag() || isTouchUp();
}

bool InputEvents::isScroll()
{
	return this->isScrolling;
}

bool InputEvents::isKeyDown()
{
	return this->type == SDL_KEYDOWN || this->type == SDL_CONTROLLERBUTTONDOWN;
}

bool InputEvents::isKeyUp()
{
	return this->type == SDL_KEYUP || this->type == SDL_CONTROLLERBUTTONUP;
}

void InputEvents::processJoystickHotplugging(SDL_Event *event)
{
	SDL_GameController* pad;
	switch(event->type)
	{
		case SDL_CONTROLLERDEVICEADDED:
			if (SDL_IsGameController(event->cdevice.which)) {
				// see if it's an existing controller, so we don't re-open it
				pad = SDL_GameControllerFromInstanceID(event->cdevice.which);
				if (pad != NULL && SDL_GameControllerGetAttached(pad)) {
					return;
				}
				pad = SDL_GameControllerOpen(event->cdevice.which);
				if (pad != NULL) {
					printf("SDL_CONTROLLERDEVICEADDED: %s\n", SDL_GameControllerName(pad));
				}
			}
			break;
		case SDL_CONTROLLERDEVICEREMOVED:
			// close the controller
			pad = SDL_GameControllerFromInstanceID(event->cdevice.which);
			if (pad != NULL) {
				SDL_GameControllerClose(pad);
			}
			break;
		default:
			break;
	}
}

GamepadInfo& InputEvents::getLastGamepadInfo()
{
	return gamepadMap[lastGamepadKey];
}