#if defined(SWITCH)
#include <switch.h>
#define PLATFORM "Switch"
#elif defined(__WIIU__)
#define PLATFORM "Wii U"
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

#if defined(__WIIU__)
  backgroundColor = {0x54, 0x55, 0x6e};
#else
  backgroundColor = {0x42, 0x45, 0x48};
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
	// set the background color
	RootDisplay::background((int)(backgroundColor.r), (int)(backgroundColor.g), (int)(backgroundColor.b));

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
	CST_SetDrawColorRGBA(this->renderer, r, g, b, 0xFF );
  CST_FillRect(this->renderer, NULL);
}

void RootDisplay::update()
{
	// never exceed 60fps because there's no point

	//    int now = CST_GetTicks();
	//    int diff = now - this->lastFrameTime;
	//
	//    if (diff < 16)
	//        return;

	CST_RenderPresent(this->renderer);
	//    this->lastFrameTime = now;
}

void RootDisplay::switchSubscreen(Element* next)
{
	if (nextsubscreen != subscreen)
		delete nextsubscreen;
	nextsubscreen = next;
}
