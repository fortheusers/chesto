#pragma once
#include "Element.hpp"

#include "colorspaces.hpp"
#include <unordered_map>

#if defined(__WIIU__)
#define ICON_SIZE 90
#elif defined(_3DS) || defined(_3DS_MOCK)
#define ICON_SIZE 48
#else
#define ICON_SIZE 150
#endif

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

	static CST_Renderer* renderer;
	static CST_Window* window;
	static RootDisplay* mainDisplay;

	static void switchSubscreen(Element* next);
	static Element* subscreen;
	static Element* nextsubscreen;

	static bool isDebug;
	bool isRunning = true;
	bool exitRequested = false;
	bool canUseSelectToExit = false;

	int lastFrameTime = 99;
	SDL_Event needsRender;

	// our main input events
	InputEvents* events;

	std::vector<Element*> trash;
	void recycle();

#if defined(MUSIC)
	Mix_Music* music;
#endif
};
