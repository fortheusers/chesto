#pragma once
#include "Element.hpp"

#include "colorspaces.hpp"
#include <unordered_map>

#if defined(_3DS) || defined(_3DS_MOCK)
#define ICON_SIZE 48
#else
#define ICON_SIZE 150
#endif

#define SCREEN_WIDTH RootDisplay::screenWidth
#define SCREEN_HEIGHT RootDisplay::screenHeight

class RootDisplay : public Element
{
public:
	RootDisplay();
	virtual ~RootDisplay();

	bool process(InputEvents* event);
	void render(Element* parent);

	void update();
	int mainLoop();

	void initMusic();
	void startMusic();

	void setScreenResolution(int width, int height);

	static CST_Renderer* renderer;
	static CST_Window* window;
	static RootDisplay* mainDisplay;

	static void switchSubscreen(Element* next);
	static Element* subscreen;
	static Element* nextsubscreen;

	// dimensions of the screen, which can be modified
	static int screenWidth;
	static int screenHeight;
	static float dpiScale;

	// if enabled, the cursor will pulse when idle
	// this uses more energy and forces more redraws
	// TODO: enable or disable based on battery level or user preference
	static bool idleCursorPulsing;

	static bool isDebug;
	bool canUseSelectToExit = false;

	int lastFrameTime = 99;
	SDL_Event needsRender;

	// our main input events
	InputEvents* events = NULL;

	std::function<void()> windowResizeCallback = NULL; // Called when the window is resized

	std::vector<Element*> trash;
	void recycle();

	void requestQuit();

#if defined(MUSIC)
	Mix_Music* music = NULL;
#endif

private:
	// these bools are managed by RootDisplay mainLoop, and should not be modified
	// to break out. Instead, call requestQuit() which will update it if needed
	bool hasRequestedQuit = false;
	bool isAppRunning = true;
};
