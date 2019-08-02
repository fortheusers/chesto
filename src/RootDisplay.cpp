#if defined(SWITCH)
#include <switch.h>
#define PLATFORM "Switch"
#elif defined(__WIIU__)
#include <romfs-wiiu.h>
#define PLATFORM "Wii U"
#else
#define PLATFORM "Console"
#endif

#include "RootDisplay.hpp"
#include "Button.hpp"

SDL_Renderer* RootDisplay::mainRenderer = NULL;
Element* RootDisplay::subscreen = NULL;
RootDisplay* RootDisplay::mainDisplay = NULL;

RootDisplay::RootDisplay()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO) < 0)
	{
		return;
	}

	// use linear filtering when available
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

//Initialize SDL_mixer
#if defined(MUSIC)
	Mix_Init(MIX_INIT_MP3);
	Mix_OpenAudio(22050, AUDIO_S16SYS, 2, 640);
	this->music = Mix_LoadMUS(ROMFS "./res/music.mp3");
	if (music)
	{
		Mix_FadeInMusic(music, -1, 300);
	}
#endif

	if (TTF_Init() < 0)
	{
		//        printf("SDL ttf init failed: %s\n", SDL_GetError());
		return;
	}

	int imgFlags = IMG_INIT_PNG;
	if (!(IMG_Init(imgFlags) & imgFlags))
	{
		//        printf("SDL image init failed: %s\n", SDL_GetError());
		return;
	}

	//    printf("initialized SDL\n");

	int height = 720;
	int width = 1280;

	this->window = SDL_CreateWindow(NULL, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, 0);
	this->renderer = SDL_CreateRenderer(this->window, -1, SDL_RENDERER_ACCELERATED);

	//Detach the texture
	SDL_SetRenderTarget(this->renderer, NULL);

	RootDisplay::mainRenderer = this->renderer;
	RootDisplay::mainDisplay = this;

	for (int i = 0; i < SDL_NumJoysticks(); i++)
	{
		if (SDL_JoystickOpen(i) == NULL)
		{
			//                printf("SDL_JoystickOpen: %s\n", SDL_GetError());
			SDL_Quit();
			return;
		}
	}

	// set up the SDL needsRender event
	this->needsRender.type = SDL_USEREVENT;
}

RootDisplay::~RootDisplay()
{
	IMG_Quit();
	TTF_Quit();

	SDL_Delay(10);
	SDL_DestroyWindow(RootDisplay::mainDisplay->window);

	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	SDL_Quit();
}

bool RootDisplay::process(InputEvents* event)
{
	if (RootDisplay::subscreen)
		return RootDisplay::subscreen->process(event);

	// keep processing child elements
	return super::process(event);
}

void RootDisplay::render(Element* parent)
{
	// set the background color
	RootDisplay::background(0x42, 0x45, 0x48);
//    RootDisplay::background(0x60, 0x7d, 0x8b);
#if defined(__WIIU__)
	RootDisplay::background(0x54, 0x55, 0x6e);
#endif

	if (RootDisplay::subscreen)
	{
		RootDisplay::subscreen->render(this);
		this->update();
		return;
	}

	// render the rest of the subelements
	super::render(this);

	// commit everything to the screen
	this->update();
}

void RootDisplay::background(int r, int g, int b)
{
	SDL_SetRenderDrawColor(this->renderer, r, g, b, 0xFF);
	SDL_RenderFillRect(this->renderer, NULL);
}

void RootDisplay::update()
{
	// never exceed 60fps because there's no point

	//    int now = SDL_GetTicks();
	//    int diff = now - this->lastFrameTime;
	//
	//    if (diff < 16)
	//        return;

	SDL_RenderPresent(this->renderer);
	//    this->lastFrameTime = now;
}
