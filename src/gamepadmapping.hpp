#include "InputEvents.hpp"

// This file maintains a mapping of different SDL button events to the chesto ones
// the ordering of the array determines which events are mapped to our SDL buttons (defined in InputEvents.hpp)

// computer key mappings
CST_Keycode key_buttons[] = { 
    SDLK_a, SDLK_b, SDLK_x, SDLK_y,
    SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT,
    SDLK_l, SDLK_r, SDLK_z, SDLK_q,
    SDLK_RETURN, SDLK_BACKSPACE, SDLK_ESCAPE
};

// human readable lowercase keyboard buttons
std::string keyButtonNames[] = {
    "a", "b", "x", "y",
    "up", "down", "left", "right",
    "l", "r", "z", "q",
    "return", "backspace", "home"
};

SDL_GameControllerButton pad_buttons[] = {
    SDL_A, SDL_B, SDL_X, SDL_Y,
    SDL_UP, SDL_DOWN, SDL_LEFT, SDL_RIGHT,
    SDL_L, SDL_R, SDL_ZL, SDL_ZR,
    SDL_PLUS, SDL_MINUS, SDL_HOME
};

// below now is the order of our own "buttons" that correspond to the above SDL ones, for each different platform's controller
// TODO: use a map instead? or load from some kind of config file

unsigned int nintendo_buttons[] = {
    A_BUTTON, B_BUTTON, X_BUTTON, Y_BUTTON,
    UP_BUTTON, DOWN_BUTTON, LEFT_BUTTON, RIGHT_BUTTON,
    L_BUTTON, R_BUTTON, ZL_BUTTON, ZR_BUTTON,
    START_BUTTON, SELECT_BUTTON, HOME_BUTTON
};

// human readable lowercase names for the buttons (used by the UI for icon display, the order lines up with the _buttons array above it)
std::string nintendoButtonNames[] = {
    "a", "b", "x", "y",
    "up", "down", "left", "right",
    "l", "r", "zl", "zr",
    "plus", "minus", "home"
};

// xbox 360 controller buttons (aka A<->B, X<->Y)
unsigned int xbox_buttons[] = {
    B_BUTTON, A_BUTTON, Y_BUTTON, X_BUTTON,
    UP_BUTTON, DOWN_BUTTON, LEFT_BUTTON, RIGHT_BUTTON,
    L_BUTTON, R_BUTTON, ZL_BUTTON, ZR_BUTTON,
    START_BUTTON, SELECT_BUTTON, HOME_BUTTON
};

std::string xboxButtonNames[] = {
    "b", "a", "y", "x",
    "up", "down", "left", "right",
    "l", "r", "zl", "zr",
    "plus", "minus", "home"
};

// wii remote (alone) buttons, a smaller set of actions
// (buttons that aren't available will need to be pressed using IR sensor)
unsigned int wii_buttons[] = {
    START_BUTTON, SELECT_BUTTON, L_BUTTON, R_BUTTON, // for the wiimote, + and - are our A and B, and A and B are the IR buttons (aka mouse buttons)
    UP_BUTTON, DOWN_BUTTON, LEFT_BUTTON, RIGHT_BUTTON,
    0, 0, 0, 0,
    0, 0, HOME_BUTTON // and the actual start/select are unmapped
};

std::string wiiButtonNames[] = {
    "plus", "minus", "1", "2",
    "up", "down", "left", "right",
    "", "", "", "",
    "", "", "home"
};

// wii remote and nunchuk, separate and more actions (but still not all) available
unsigned int nunchuk_buttons[] = {
    START_BUTTON, SELECT_BUTTON, L_BUTTON, R_BUTTON,
    UP_BUTTON, DOWN_BUTTON, LEFT_BUTTON, RIGHT_BUTTON,
    0, 0, X_BUTTON, Y_BUTTON, // to be clear, X and Y are the internal chesto names, and they'll line up with Z and C (named, below)
    0, 0, HOME_BUTTON
};

std::string nunchukButtonNames[] = {
    "plus", "minus", "1", "2",
    "up", "down", "left", "right",
    "", "", "z", "c",
    "", "", "home"
};

#if defined(__WIIU__) && defined(USE_KEYBOARD)
#include "../libs/wiiu_kbd/keybdwrapper.h"
#endif

#define TOTAL_AXIS_COUNT 10

// for held simulated buttons we'll maintain a mapping of the stick axis values to regular CST button events
CST_Keycode axis_values[] = {
    SDL_LEFT_STICK_Y, SDL_LEFT_STICK_Y, SDL_LEFT_STICK_X, SDL_LEFT_STICK_X,
    SDL_RIGHT_STICK_Y, SDL_RIGHT_STICK_Y, SDL_RIGHT_STICK_X, SDL_RIGHT_STICK_X,
    SDL_TRIGGER_L, SDL_TRIGGER_R
};

CST_Keycode axis_buttons[] = {
    SDL_UP, SDL_DOWN, SDL_LEFT, SDL_RIGHT,
    SDL_UP, SDL_DOWN, SDL_LEFT, SDL_RIGHT,
    SDL_ZL, SDL_ZR
};