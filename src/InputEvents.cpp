#include "InputEvents.hpp"

int TOTAL_BUTTONS = 18;

// computer key mappings
CST_Keycode key_buttons[] = { SDLK_a, SDLK_b, SDLK_x, SDLK_y, SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT, SDLK_RETURN, SDLK_l, SDLK_r, SDLK_z, SDLK_BACKSPACE, SDLK_UP, SDLK_DOWN, -1, -1, SDLK_q };

#ifndef SDL1
SDL_GameControllerButton pad_buttons[] = { SDL_A, SDL_B, SDL_X, SDL_Y, SDL_UP, SDL_DOWN, SDL_LEFT, SDL_RIGHT, SDL_PLUS, SDL_L, SDL_R, SDL_ZL, SDL_MINUS, SDL_UP_STICK, SDL_DOWN_STICK, SDL_LEFT_STICK, SDL_RIGHT_STICK, SDL_ZR };
#else
int pad_buttons[] = { 1, 2, 3, 4, 0, 0, 0, 0, 8, 5, 6, 5, 7 };
#define SDL_FINGERDOWN SDL_MOUSEBUTTONDOWN
#define SDL_FINGERUP SDL_MOUSEBUTTONUP
#define SDL_FINGERMOTION SDL_MOUSEMOTION
#endif

#if defined(__WIIU__) && defined(USE_KEYBOARD)
#include "../libs/wiiu_kbd/keybdwrapper.h"
#endif

// our own "buttons" that correspond to the above SDL ones
unsigned int ie_buttons[] = { A_BUTTON, B_BUTTON, X_BUTTON, Y_BUTTON, UP_BUTTON, DOWN_BUTTON, LEFT_BUTTON, RIGHT_BUTTON, START_BUTTON, L_BUTTON, R_BUTTON, ZL_BUTTON, SELECT_BUTTON, UP_BUTTON, DOWN_BUTTON, LEFT_BUTTON, RIGHT_BUTTON, ZR_BUTTON };

// if true, don't count key inputs (PC/usb keyboard) as button events for us
bool InputEvents::bypassKeyEvents = false;

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
	SDL_Event event;
	if (!SDL_PollEvent(&event))
		return false;

	// update our variables
	this->type = event.type;
	this->noop = false;

	// proces joystick hotplugging events
	processJoystickHotplugging(&event);

	this->isScrolling = false;

#ifdef PC
	this->allowTouch = false;
#ifndef SDL1
	if (event.type == SDL_MOUSEWHEEL) {
		this->wheelScroll = event.wheel.y;
		this->isScrolling = true;
	}
#endif
#endif

	if (this->type == SDL_QUIT)
	{
		this->quitaction();
		return false; //Quitting overrides all other events.
	}
#ifndef SDL1
// TODO: key down/up input on SDL1
	else if (event.key.repeat == 0 && (this->type == SDL_KEYDOWN || this->type == SDL_KEYUP))
	{
		this->keyCode = event.key.keysym.sym;
		this->mod = event.key.keysym.mod;
	}
#endif
	else if (this->type == SDL_JOYBUTTONDOWN || this->type == SDL_JOYBUTTONUP)
	{
	#ifndef SDL1
		this->keyCode = event.jbutton.button;
	#else
		this->keyCode = event.button.button;
	#endif
	}
	else if (this->type == SDL_MOUSEMOTION || this->type == SDL_MOUSEBUTTONUP || this->type == SDL_MOUSEBUTTONDOWN)
	{
		bool isMotion = this->type == SDL_MOUSEMOTION;

		this->yPos = isMotion ? event.motion.y : event.button.y;
		this->xPos = isMotion ? event.motion.x : event.button.x;
	}
#ifndef SDL1
	else if (allowTouch && (this->type == SDL_FINGERMOTION || this->type == SDL_FINGERUP || this->type == SDL_FINGERDOWN))
	{
		this->yPos = event.tfinger.y * 720;
		this->xPos = event.tfinger.x * 1280;
	}
#endif

	toggleHeldButtons();

	return true;
}

bool InputEvents::update()
{
	this->type = 0;
	this->keyCode = -1;
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
			if (!held_directions[directionCode])
			{
				// on key down, set the corresponding held boolean to true
				held_directions[directionCode] = true;
				held_type = this->type;

				// reset the frame counter so we don't fire on this frame
				// (initial reset is lower to add a slight delay when they first start holding)
				curFrame = -25;
			}
		}

		if (isKeyUp())
		{
			// release the corresponding key too
			held_directions[directionCode] = false;
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
			if (!held_directions[x])
				continue;

			// send a corresponding directional event
			this->type = held_type;
			bool isGamepad = (this->type == SDL_JOYBUTTONDOWN || this->type == SDL_JOYBUTTONUP);
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

#ifndef SDL1
// TODO: joysticks in SDL1
	// returns 0 1 2 or 3 for up down left or right
	switch (this->keyCode)
	{
	case SDL_UP_STICK:
	case SDL_UP:
	case SDLK_UP:
		return 0;
	case SDL_DOWN_STICK:
	case SDL_DOWN:
	case SDLK_DOWN:
		return 1;
	case SDL_LEFT_STICK:
	case SDL_LEFT:
	case SDLK_LEFT:
		return 2;
	case SDL_RIGHT_STICK:
	case SDL_RIGHT:
	case SDLK_RIGHT:
		return 3;
	default:
		return -1;
	}
#endif
	return -1;
}

bool InputEvents::held(int buttons)
{
	// if it's a key event
	if ((this->type == SDL_KEYDOWN || this->type == SDL_KEYUP) && !InputEvents::bypassKeyEvents)
	{
		for (int x = 0; x < TOTAL_BUTTONS; x++)
			if (key_buttons[x] == keyCode && (buttons & ie_buttons[x]))
				return true;
	}

	// if it's a controller event
	else if (this->type == SDL_JOYBUTTONDOWN || this->type == SDL_JOYBUTTONUP)
	{
		for (int x = 0; x < TOTAL_BUTTONS; x++)
			if (pad_buttons[x] == keyCode && (buttons & ie_buttons[x]))
				return true;
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
	return this->type == SDL_KEYDOWN || this->type == SDL_JOYBUTTONDOWN;
}

bool InputEvents::isKeyUp()
{
	return this->type == SDL_KEYUP || this->type == SDL_JOYBUTTONUP;
}

void InputEvents::processJoystickHotplugging(SDL_Event *event)
{
	#ifndef SDL1
	SDL_Joystick *j;
	switch(event->type)
	{
	case SDL_JOYDEVICEADDED:
		j = SDL_JoystickOpen(event->jdevice.which);
		if (j)
			printf("Added joystick device: %s\n", SDL_JoystickName(j));
		break;
	case SDL_JOYDEVICEREMOVED:
		j = SDL_JoystickFromInstanceID(event->jdevice.which);
		if (j && SDL_JoystickGetAttached(j))
		{
			printf("Removed joystick device: %s\n", SDL_JoystickName(j));
			SDL_JoystickClose(j);
		}
		break;
	default:
		break;
	}
	#endif
}
