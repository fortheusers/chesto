#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#elif defined(SWITCH)
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

CST_Renderer* RootDisplay::renderer = NULL;
CST_Window* RootDisplay::window = NULL;
Element* RootDisplay::subscreen = NULL;
Element* RootDisplay::nextsubscreen = NULL;
RootDisplay* RootDisplay::mainDisplay = NULL;
bool RootDisplay::isDebug = false;

int RootDisplay::screenWidth = 1280;
int RootDisplay::screenHeight = 720;
float RootDisplay::dpiScale = 1.0f;

bool RootDisplay::idleCursorPulsing = false;

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

#ifdef __APPLE__
	chdirForPlatform();
#endif

	// set the display scale on high resolution displays
	RootDisplay::dpiScale = CST_GetDpiScale();

	// default background color is dark-gray, can be overridden by the implementing library
	this->backgroundColor = fromRGB(30, 30, 30);

	// set starting resolution based on SDL version
#if defined(WII) || defined(WII_MOCK)
	setScreenResolution(640, 480);
#elif defined(_3DS) || defined(_3DS_MOCK)
	setScreenResolution(400, 480); // 3ds has a special resolution!
#else
	// setScreenResolution(640, 480);
	setScreenResolution(1280, 720);
#endif
	// the main input handler
	this->events = new InputEvents();

	// TODO: initialize this in a way that doesn't block the main thread
	// always load english first, to initialize defaults
	TextElement::loadI18nCache("en-us");

	// TODO: detect language and system, and store preference
	// TextElement::loadI18nCache("zh-cn");
}

void RootDisplay::initMusic()
{
#ifdef SWITCH
	// no music if we're in applet mode
	// they use up too much memory, and a lot of people only use applet mode
	AppletType at = appletGetAppletType();
	if (at != AppletType_Application && at != AppletType_SystemApplication) {
		return;
	}
#endif

	// Initialize CST_mixer
	CST_MixerInit(this);
}

void RootDisplay::startMusic()
{
	CST_FadeInMusic(this);
}

void RootDisplay::setScreenResolution(int width, int height)
{
	// set the screen resolution
	SCREEN_WIDTH = width;
	SCREEN_HEIGHT = height;

	// update the root element
	this->width = SCREEN_WIDTH;
	this->height = SCREEN_HEIGHT;

	// update the renderer, but respect the DPI scaling
	CST_SetWindowSize(window, SCREEN_WIDTH / RootDisplay::dpiScale, SCREEN_HEIGHT / RootDisplay::dpiScale);
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

	// process either the subscreen or the children elements, always return true if "dragging"
	// (may be a mouse cursor or wiimote pointing and moving on the screen)

	if (RootDisplay::subscreen)
		return RootDisplay::subscreen->process(event) || event->isTouchDrag();

	// keep processing child elements
	return super::process(event) || event->isTouchDrag();
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

void RootDisplay::requestQuit()
{
	// if we've already requested quit, don't proceed
	if (hasRequestedQuit) {
		return;
	}
	hasRequestedQuit = true;

	// depending on the platform, either break our loop or (wiiu) switch to the home menu
#ifdef __WIIU__
	SYSLaunchMenu();
#else
	this->isAppRunning = false;
#endif
}

int RootDisplay::mainLoop()
{
	DownloadQueue::init();

#ifdef __WIIU__
	// setup procui callback for resuming application to force a chesto render
	// https://stackoverflow.com/a/56145528 and http://bannalia.blogspot.com/2016/07/passing-capturing-c-lambda-functions-as.html
	auto updateDisplay = +[](void* display) -> unsigned int {
		((RootDisplay*)display)->futureRedrawCounter = 10;
		return 0;
	};
	ProcUIRegisterCallback(PROCUI_CALLBACK_ACQUIRE, updateDisplay, this, 100);

	// also, register a callback for when we need to quit, to break out the main loop
	// (other platforms will do this directly, but wiiu needs procui to do stuff first)
	auto actuallyQuit = +[](void* display) -> unsigned int {
		((RootDisplay*)display)->isAppRunning = false;
		return 0;
	};
	ProcUIRegisterCallback(PROCUI_CALLBACK_EXIT, actuallyQuit, this, 100);
#endif

	while (isAppRunning)
	{
		bool atLeastOneNewEvent = false;
		bool viewChanged = false;

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
			if (this->canUseSelectToExit && events->pressed(SELECT_BUTTON)) {
				requestQuit();
			}
		}

		// one more event update if nothing changed or there were no previous events seen
		// needed to non-input related processing that might update the screen to take place
		if ((!atLeastOneNewEvent && !viewChanged))
		{
			events->update();
			viewChanged |= this->process(events);
		}

		// draw the display if we processed an event or the view
		if (viewChanged)
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

	delete events;

	if (!isProtected) delete this;
	DownloadQueue::quit();

	return 0;
}

void RootDisplay::recycle()
{
	for (auto e : trash)
		e->wipeAll(true);
	trash.clear();
}