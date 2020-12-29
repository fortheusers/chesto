#if defined(SWITCH)
#include <switch.h>
#define PLATFORM "Switch"
#elif defined(__WIIU__)
#define PLATFORM "Wii U"
#include <coreinit/core.h>
#include <coreinit/foreground.h>
#include <proc_ui/procui.h>
#include <sysapp/launch.h>
#elif defined(_3DS)
#define PLATFORM "3DS"
#else
#define PLATFORM "Console"
#endif

#if defined(USE_RAMFS)
#include "../libs/resinfs/include/romfs-wiiu.h"
#endif

#include "RootDisplay.hpp"
#include "DownloadQueue.hpp"
#include "Button.hpp"

CST_Renderer* RootDisplay::mainRenderer = NULL;
Element* RootDisplay::subscreen = NULL;
Element* RootDisplay::nextsubscreen = NULL;
RootDisplay* RootDisplay::mainDisplay = NULL;
bool RootDisplay::isDebug = false;

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
	this->backgroundColor = fromRGB(0x54, 0x55, 0x6e);
#elif defined(_3DS) || defined(_3DS_MOCK)
	this->backgroundColor = fromRGB(0xe4, 0x00, 0x00);
#else
	this->backgroundColor = fromRGB(0x42, 0x45, 0x48);
#endif

	// the main input handler
	this->events = new InputEvents();
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

#ifdef __WIIU__
// proc ui will block if it keeps control
void processWiiUHomeOverlay() {
		auto status = ProcUIProcessMessages(true);
    if (status == PROCUI_STATUS_EXITING)
			exit(0);
		else if (status == PROCUI_STATUS_RELEASE_FOREGROUND)
			ProcUIDrawDoneRelease();
}
#endif

void RootDisplay::mainLoop()
{
	// consoleDebugInit(debugDevice_SVC);
	// stdout = stderr; // for yuzu

#if defined(__WIIU__)
	// WHBLogUdpInit();
	// WHBLogCafeInit();
#endif

	// start music (only if MUSIC defined)
	this->initAndStartMusic();

	DownloadQueue::init();
	
#ifdef __WIIU__
	// setup procui callback for resuming application to force a chesto render
	// https://stackoverflow.com/a/56145528 and http://bannalia.blogspot.com/2016/07/passing-capturing-c-lambda-functions-as.html
	auto updateDisplay = +[](void* display) -> unsigned int {
		((RootDisplay*)display)->futureRedrawCounter = 10;
		return 0;
	};
	ProcUIRegisterCallback(PROCUI_CALLBACK_ACQUIRE, updateDisplay, this, 100);
#endif

	while (isRunning)
	{
		bool atLeastOneNewEvent = false;
		bool viewChanged = false;

	#ifdef __WIIU__
		processWiiUHomeOverlay();
	#endif

		int frameStart = CST_GetTicks();

		// update download queue
		DownloadQueue::downloadQueue->process();

		// get any new input events
		while (events->update())
		{
			// process the inputs of the supplied event
			viewChanged |= this->process(events);
			atLeastOneNewEvent = true;

			// if we see a minus, exit immediately!
			// TODO: allow this to be specified
			if (events->pressed(SELECT_BUTTON) && this->canUseSelectToExit) {
				if (events->quitaction) events->quitaction();
				isRunning = false;
			}

			// one more event update if nothing changed or there were no previous events seen
			// needed to non-input related processing that might update the screen to take place
			if ((!atLeastOneNewEvent && !viewChanged) || forceProcessEvents)
			{
				events->update();
				viewChanged |= this->process(events);
			}

			// draw the display if we processed an event or the view
			if (viewChanged || forceProcessEvents)
				this->render(NULL);
			else
			{
				// delay for the remainder of the frame to keep up to 60fps
				// (we only do this if we didn't draw to not waste energy
				// if we did draw, then proceed immediately without waiting for smoother progress bars / scrolling)
				int delayTime = (CST_GetTicks() - frameStart);
				if (delayTime < 0)
					delayTime = 0;
				if (delayTime < 16)
					CST_Delay(16 - delayTime);
			}

			// free up any elements that are in the trash
			this->recycle();
		}
	}

	DownloadQueue::quit();

	delete events;
	delete this;
}

void RootDisplay::recycle()
{
	for (auto e : trash)
		e->wipeAll(true);
	trash.clear();
}