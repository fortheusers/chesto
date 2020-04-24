#if defined(SWITCH)
#include <switch.h>
#define PLATFORM "Switch"
#elif defined(__WIIU__)
#define PLATFORM "Wii U"
#elif defined(_3DS)
#define PLATFORM "3DS"
#else
#define PLATFORM "Console"
#endif

#if defined(USE_RAMFS)
#include "../libs/resinfs/include/romfs-wiiu.h"
#endif

#include "RootDisplay.hpp"
#include "Button.hpp"

CST_Renderer* RootDisplay::mainRenderer = NULL;
Element* RootDisplay::subscreen = NULL;
Element* RootDisplay::nextsubscreen = NULL;
RootDisplay* RootDisplay::mainDisplay = NULL;

RootDisplay::RootDisplay()
{
	// initialize the romfs for switch/wiiu
#if defined(USE_RAMFS)
	ramfsInit();
#endif

	// initialize internal drawing library
	CST_DrawInit(this);

	this->x = 0;
	this->y = 0;
	this->width = SCREEN_WIDTH;
	this->height = SCREEN_HEIGHT;

	this->hasBackground = true;
#if defined(__WIIU__)
	this->backgroundColor = (rgb){ 0x54/255.0, 0x55/255.0, 0x6e/255.0 };
#elif defined(_3DS) || defined(_3DS_MOCK)
	// this->backgroundColor = (rgb){ 0xe4/255.0, 0x00/255.0, 0x0/255.0f };
	this->backgroundColor = (rgb){ 1, 1, 1 };
#else
	this->backgroundColor = (rgb){ 0x42/255.0, 0x45/255.0, 0x48/255.0 };
#endif
}

void RootDisplay::initAndStartMusic()
{
	//Initialize CST_mixer
	CST_MixerInit(this);
}

RootDisplay::~RootDisplay()
{
	CST_DrawExit();

#if defined(USE_RAMFS)
	ramfsExit();
#endif
}

bool RootDisplay::process(InputEvents* event)
{
	if (nextsubscreen != subscreen)
	{
		delete subscreen;
		subscreen = nextsubscreen;
		return true;
	}

	if (RootDisplay::subscreen)
		return RootDisplay::subscreen->process(event);

	// keep processing child elements
	return super::process(event);
}

void RootDisplay::render(Element* parent)
{
	if (RootDisplay::subscreen)
	{
		RootDisplay::subscreen->render(this);
		this->update();
		return;
	}

	// render the rest of the subelements
	super::render(parent);

	// commit everything to the screen
	this->update();
}

void RootDisplay::update()
{
	// never exceed 60fps because there's no point
	// commented out, as if render isn't called manually,
	// the CST_Delay in the input processing loop should handle this

	//  int now = CST_GetTicks();
	//  int diff = now - this->lastFrameTime;

	//  if (diff < 16)
	//      return;

	CST_RenderPresent(this->renderer);
	//  this->lastFrameTime = now;
}

void RootDisplay::switchSubscreen(Element* next)
{
	if (nextsubscreen != subscreen)
		delete nextsubscreen;
	nextsubscreen = next;
}
