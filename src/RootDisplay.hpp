#pragma once

#include "Element.hpp"
#include "colorspaces.hpp"
#include <unordered_map>

#if defined(__WIIU__)
#define ICON_SIZE 90
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
	void background(uint8_t r, uint8_t g, uint8_t b);
  void initAndStartMusic();
	void update();

	rgb backgroundColor;

	static CST_Renderer* mainRenderer;
	static RootDisplay* mainDisplay;

	static void switchSubscreen(Element* next);
	static Element* subscreen;
	static Element* nextsubscreen;

	int lastFrameTime = 99;
	SDL_Event needsRender;

#if defined(MUSIC)
	Mix_Music* music;
#endif
};
