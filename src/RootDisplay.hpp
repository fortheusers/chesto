#pragma once
#include "Element.hpp"

#include "colorspaces.hpp"
#include <unordered_map>
#include <memory>
#include <vector>

#if defined(_3DS) || defined(_3DS_MOCK)
#define ICON_SIZE 48
#elif defined(WII) || defined(WII_MOCK)
#define ICON_SIZE 120
#else
#define ICON_SIZE 150
#endif

namespace Chesto {

class Screen;

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

	// New methods for managing the subscreen stack
	static void pushScreen(std::unique_ptr<Screen> screen);
	static void popScreen();
	static Screen* topScreen();
	static void clearScreens();
	static bool hasScreens();
	
	// Process any pending screen operations (called after event processing)
	static void processPendingScreenOps();

	// Screen stack storage, iterateable to draw layers at a time
	static std::vector<std::unique_ptr<Screen>> screenStack;
	
	enum class ScreenOp { PUSH, POP, CLEAR };
	struct PendingScreenOp {
		ScreenOp op;
		std::unique_ptr<Screen> screen;
	};
	static std::vector<PendingScreenOp> pendingScreenOps;
	static bool isProcessingEvents;

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
	std::unique_ptr<InputEvents> events;

	std::function<void()> windowResizeCallback = NULL; // Called when the window is resized

	void requestQuit();

#if !defined(SIMPLE_SDL2)
	Mix_Music* music = NULL;
#endif

private:
	// these bools are managed by RootDisplay mainLoop, and should not be modified
	// to break out. Instead, call requestQuit() which will update it if needed
	bool hasRequestedQuit = false;
	bool isAppRunning = true;
};

} // namespace Chesto
