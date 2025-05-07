#pragma once

#include <SDL2/SDL.h>

#if !defined(_3DS) && !defined(_3DS_MOCK)
// 3ds has no sdl keycodes
typedef SDL_Keycode CST_Keycode;
#else
typedef int CST_Keycode;
#endif

typedef uint16_t CST_Keymod;

#include <functional>
#include <string>
#include <map>

#define TOTAL_BUTTONS 15

// clang-format off
#define A_BUTTON         0b000000000000001
#define B_BUTTON         0b000000000000010
#define X_BUTTON         0b000000000000100
#define Y_BUTTON         0b000000000001000
#define UP_BUTTON        0b000000000010000
#define DOWN_BUTTON      0b000000000100000
#define LEFT_BUTTON      0b000000001000000
#define RIGHT_BUTTON     0b000000010000000
#define L_BUTTON         0b000000100000000
#define R_BUTTON         0b000001000000000
#define ZL_BUTTON        0b000010000000000
#define ZR_BUTTON        0b000100000000000
#define START_BUTTON     0b001000000000000
#define SELECT_BUTTON    0b010000000000000
#define HOME_BUTTON      0b100000000000000

// SDL enums that line up with the common HB port controls
// uses switch+wiiu mappings, see: https://github.com/rw-r-r-0644/sdl2-wiiu/blob/master/SDL2-wiiu/src/joystick/wiiu/SDL_sysjoystick.c#L38
#define SDL_A        SDL_CONTROLLER_BUTTON_A
#define SDL_B        SDL_CONTROLLER_BUTTON_B
#define SDL_X        SDL_CONTROLLER_BUTTON_X
#define SDL_Y        SDL_CONTROLLER_BUTTON_Y

#define SDL_UP        SDL_CONTROLLER_BUTTON_DPAD_UP
#define SDL_DOWN      SDL_CONTROLLER_BUTTON_DPAD_DOWN
#define SDL_LEFT      SDL_CONTROLLER_BUTTON_DPAD_LEFT
#define SDL_RIGHT     SDL_CONTROLLER_BUTTON_DPAD_RIGHT

#define SDL_L        SDL_CONTROLLER_BUTTON_LEFTSHOULDER
#define SDL_R        SDL_CONTROLLER_BUTTON_RIGHTSHOULDER
#define SDL_ZL       SDL_CONTROLLER_BUTTON_PADDLE1
#define SDL_ZR       SDL_CONTROLLER_BUTTON_PADDLE2

#define SDL_PLUS     SDL_CONTROLLER_BUTTON_START
#define SDL_MINUS    SDL_CONTROLLER_BUTTON_BACK
#define SDL_HOME	 SDL_CONTROLLER_BUTTON_GUIDE

// same as ZL/ZR, but SDL gamepad maps them to axes
#define SDL_TRIGGER_L		SDL_CONTROLLER_AXIS_TRIGGERLEFT
#define SDL_TRIGGER_R		SDL_CONTROLLER_AXIS_TRIGGERRIGHT

#define SDL_LEFT_STICK_X	SDL_CONTROLLER_AXIS_LEFTX
#define SDL_LEFT_STICK_Y	SDL_CONTROLLER_AXIS_LEFTY
#define SDL_RIGHT_STICK_X	SDL_CONTROLLER_AXIS_RIGHTX
#define SDL_RIGHT_STICK_Y	SDL_CONTROLLER_AXIS_RIGHTY
// clang-format on

struct GamepadInfo {
    unsigned int* buttons;
    std::string* names;
    std::string prefix;
    std::string controller_type;

public:
    GamepadInfo(unsigned int* buttons, std::string* names, std::string prefix, std::string controller_type)
        : buttons(buttons), names(names), prefix(prefix), controller_type(controller_type) {}
	GamepadInfo()
        : buttons(nullptr), names(nullptr), prefix(""), controller_type("")
    {
    }
};

class InputEvents
{
public:
	InputEvents();

	/// whether or not a button is pressed during this cycle
	bool held(int buttons);
	bool pressed(int buttons);
	bool released(int buttons);

	/// whether or not a touch is detected within the specified rect in this cycle
	bool touchIn(int x, int width, int y, int height);

	/// update which buttons are pressed
	bool processSDLEvents();
	bool update();

	bool allowTouch = true;
	bool isScrolling = false;

	// whether or not the current event is one of a few known ones
	bool isTouchDown();
	bool isTouchUp();
	bool isTouchDrag();
	bool isTouch();

	bool isScroll();
	bool isKeyDown();
	bool isKeyUp();

	// additional key processing info
	bool processDirectionalButtons();
	int directionForKeycode();
	void toggleHeldButtons();

	/// joystick device events processing
	void processJoystickHotplugging(SDL_Event *event);

	// if true, sticks will fire button DOWN events for directions (but not UP)
	// does not affect the trigger buttons, which will always be translated, including UP
	bool useEmulatedDigitalInputs = true; 
	
	CST_Keycode keyCode = -1;
	CST_Keycode axisCode = -1; // separate due to constant overlap
	CST_Keymod mod = -1;
	SDL_Event event; // underlying SDL event

	// the four directional buttons and two analog shoulder buttons, need their held states tracked
	bool held_buttons[6] = { false, false, false, false, false, false }; // u/d/l/r/zl/zr
	Uint32 held_type;

	int rapidFireRate = 12; // fire duplicate events if curframe mod rapidFireRate is 0 (higher = slower)
	int curFrame = 0;

	static bool bypassKeyEvents;
	static GamepadInfo& getLastGamepadInfo();
	static std::string lastGamepadKey;

	int lastGamepadInstanceId = -1;
	int lastGamepadTimestamp = -1;
	int lastGamepadButton = -1;

	float wheelScroll = 0;
	float axisValue = 0;
	float deadZone = 0.1f;

	int yPos = 0;
	int xPos = 0;
	bool noop = false;

	Uint32 type;
};
