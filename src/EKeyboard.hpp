#include "Element.hpp"
#include "TextElement.hpp"
#pragma once

#define KEYCODE_COUNT 47

// hack to fire these events in SDL1
#ifdef SDL1
#define SDL_CONTROLLER_BUTTON_A 1
#define SDL_CONTROLLER_BUTTON_B 2
#define SDL_CONTROLLER_BUTTON_X 3
#define SDL_CONTROLLER_BUTTON_Y 3
#endif

class EKeyboard : public Element
{
public:
	EKeyboard();
	EKeyboard(std::function<void(char)> typeAction);
	~EKeyboard();
	void render(Element* parent);
	bool process(InputEvents* event);

	// setup field variables
	void updateSize();
	void just_type(const char input);
	bool listenForPhysicalKeys(InputEvents* e);

	// get text input on the keyboard so far
	// (only stored when storeOwnText is true (default true for empty constructor, false for callback constructor))
	const std::string& getTextInput();
	std::string textInput;

	// a function to be invoked when we receive a key press, which takes in the char
	// that's been pressed
	std::function<void(char)> typeAction = NULL;

	// information representing a default qwerty EKeyboard, lower and upper
	// alongside default EKeyboards will also be: tab, caps lock, return, delete, and shifts
	const char* lower_keys = "`1234567890-=qwertyuiop[]\\asdfghjkl;'zxcvbnm,./";
	const char* upper_keys = "~!@#$\%^&*()_+QWERTYUIOP{}|ASDFGHJKL:\"ZXCVBNM<>?";
	int breaks[4] = { 13, 13, 11, 10 };

	// the rest of the keys will be dynamically drawn by going through hex iterations
	// these EKeyboards will fill the extra space with more characters

	std::vector<std::string*> rows;

	const CST_Keycode usbKeys[KEYCODE_COUNT] = {
		SDLK_BACKQUOTE, SDLK_1, SDLK_2, SDLK_3, SDLK_4, SDLK_5, SDLK_6, SDLK_7, SDLK_8, SDLK_9, SDLK_0, SDLK_MINUS, SDLK_EQUALS,
		 SDLK_q, SDLK_w, SDLK_e, SDLK_r, SDLK_t, SDLK_y, SDLK_u, SDLK_i, SDLK_o, SDLK_p, SDLK_LEFTBRACKET, SDLK_RIGHTBRACKET, SDLK_BACKSLASH,
		  SDLK_a, SDLK_s, SDLK_d, SDLK_f, SDLK_g, SDLK_h, SDLK_j, SDLK_k, SDLK_l, SDLK_SEMICOLON, SDLK_QUOTE,
		   SDLK_z, SDLK_x, SDLK_c, SDLK_v, SDLK_b, SDLK_n, SDLK_m, SDLK_COMMA, SDLK_PERIOD, SDLK_SLASH
	};

	inline int rowCount() {
		return (int)rows.size();
	}

	inline int rowLength(int row) {
		return (int)rows[row]->size() / 2;
	}

	bool shiftOn = false;
	bool capsOn = false;
	int mode = 0; // the mode of which EKeyboard type we're on (first is special and has shift)

	// the currently selected row and index
	int curRow = -1;
	int index = -1;

	// the below variables are stored to be used in processing touch events
	// and rendering the drawings to screen

	// attributes of each key
	int keyWidth = 0;
	int padding = 0;
	int textSize = 0;

	// attributes of delete and backspace keys
	int dPos = 0;
	int dHeight = 0;
	int sPos = 0;
	int enterPos = 0;
	int enterHeight = 0;
	int dWidth = 0;
	int sWidth = 0;
	int enterWidth = 0;

	// positions of key location offset information
	int kXPad = 0;
	int kXOff = 0;
	int yYOff = 0;
	int kYPad = 0;
	int ySpacing = 0;

	// whether or not to use the rounded rectangle key style
	bool hasRoundedKeys = false;
	CST_Font* roundKeyFont = NULL;

	bool touchMode = false;
	bool immersiveMode = false; // no rendering, but still allow inputs
								// if using a USB keyboard, they can hide the on-screen one

	bool preventEnterAndTab = false; // hide and don't allow enter/tab inputs
	bool storeOwnText = false; // whether or not this keyboard will store the text input on its own

	void type(int y, int x);
	void generateEKeyboard();
	void backspace();
};
