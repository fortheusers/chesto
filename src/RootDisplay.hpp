#pragma once

#include "Element.hpp"
#include "colorspaces.hpp"
#include <unordered_map>

#if defined(MUSIC)
#include <SDL2/SDL_mixer.h>
#endif

#if defined(__WIIU__)
#define ICON_SIZE 90
#else
#define ICON_SIZE 150
#endif

#define SCREEN_WIDTH	1280
#define SCREEN_HEIGHT	720

class RootDisplay : public Element
{
public:
	RootDisplay();
	virtual ~RootDisplay();

	bool process(InputEvents* event);
	void render(Element* parent);
	void background(int r, int g, int b);
	void update();

	rgb backgroundColor;

	static SDL_Renderer* mainRenderer;
	static Element* subscreen;
	static RootDisplay* mainDisplay;

	int lastFrameTime = 99;
	SDL_Event needsRender;

#if defined(MUSIC)
	Mix_Music* music;
#endif
};
