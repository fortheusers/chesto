#include "Element.hpp"
#include "ImageElement.hpp"
#include "TextElement.hpp"
#include <unordered_map>

#if defined(MUSIC)
#include <SDL2/SDL_mixer.h>
#endif

#if defined(__WIIU__)
#define ICON_SIZE 90
#else
#define ICON_SIZE 150
#endif

class RootDisplay : public Element
{
public:
	RootDisplay();
	bool process(InputEvents* event);
	void render(Element* parent);
	void background(int r, int g, int b);
	void update();
	void exit();

	TextElement* notice = NULL;

	static SDL_Renderer* mainRenderer;
	static Element* subscreen;
	static RootDisplay* mainDisplay;

	int lastFrameTime = 99;
	SDL_Event needsRender;

#if defined(MUSIC)
	Mix_Music* music;
#endif

	int count = 0;
};

void quit();
