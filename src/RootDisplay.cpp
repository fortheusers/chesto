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

#if defined(__WIIU__)
  backgroundColor = { 0x54/255.0, 0x55/255.0, 0x6e/255.0 };
#elif defined(_3DS)
	backgroundColor = { 0xe4/255.0, 0x00/255.0, 0x0/255.0f };
#else
  backgroundColor = { 0x42/255.0, 0x45/255.0, 0x48/255.0 };
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
	RootDisplay::background((uint8_t)(backgroundColor.r*255), (uint8_t)(backgroundColor.g*255), (uint8_t)(backgroundColor.b*255));

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

void RootDisplay::background(uint8_t r, uint8_t g, uint8_t b)
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
