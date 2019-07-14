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

}


bool RootDisplay::process(InputEvents* event)
{
	// if we're on the splash/loading screen, we need to fetch icons+screenshots from the remote repo
	// and load them into our surface cache with the pkg_name+version as the key
	if (this->showingSplash && event->noop)
	{
		// should be a progress bar
		if (this->get->packages.size() != 1)
			((ProgressBar*)this->elements[0])->percent = (this->count / ((float)this->get->packages.size() - 1));

		// no packages, prevent crash TODO: display offline in bottom bar
		if (this->get->packages.size() == 0)
		{
			((ProgressBar*)this->elements[0])->percent = -1;
			this->showingSplash = false;
			return true;
		}

		if (notice && ((ProgressBar*)this->elements[0])->percent > 0.5)
			notice->hidden = false;

		// update the counter (TODO: replace with fetching app icons/screen previews)
		this->count++;

		// get the package whose icon+screen to process
		Package* current = this->get->packages[this->count - 1];

		// the path to the cache location of the icon and screen for this pkg_name and version number
		std::string key_path = imageCache->cache_path + current->pkg_name;

		// check if this package exists in our cache, but the version doesn't match
		// (if (it's not in the cache) OR (it's in the cache but the version doesn't match)
		if (this->imageCache->version_cache.count(current->pkg_name) == 0 || (this->imageCache->version_cache.count(current->pkg_name) && this->imageCache->version_cache[current->pkg_name] != current->version))
		{
			// the version in our cache doesn't match the one that will be on the server
			// so we need to download the images now
			mkdir(key_path.c_str(), 0700);

			bool success = downloadFileToDisk(*(current->repoUrl) + "/packages/" + current->pkg_name + "/icon.png", key_path + "/icon.png");
			if (!success) // manually add default icon to cache if downloading failed
				cp(ROMFS "res/default.png", (key_path + "/icon.png").c_str());
			// TODO: generate a custom icon for this version with a color and name

			// no more default banners, just try to download the file (don't do this on Wii U)
			#if !defined(__WIIU__)
			downloadFileToDisk(*(current->repoUrl) + "/packages/" + current->pkg_name + "/screen.png", key_path + "/screen.png");
			#endif

			// add these versions to the version map
			this->imageCache->version_cache[current->pkg_name] = current->version;
		}

		// whether we just downloaded it or it was already there from the cache, load this image element into our memory cache
		// (making an AppCard and calling update() will do this, even if we don't intend to do anything with it yet)
		AppCard a(current);
		a.update();

		// write the version we just got to the cache as well so that we can know whether or not we need to up date it next time

		// are we done processing all packages
		if (this->count == this->get->packages.size())
		{
			// write whatever we have in the icon version cache to a file
			this->imageCache->writeVersionCache();

			// remove the splash screen elements
			this->wipeElements();

			// add in the sidebar, footer, and main app listing
			Sidebar* sidebar = new Sidebar();
			this->elements.push_back(sidebar);

			AppList* applist = new AppList(this->get, sidebar);
			this->elements.push_back(applist);
			sidebar->appList = applist;

			this->showingSplash = false;
			this->needsRedraw = true;
		}

		return true;
	}
	else
	{
		if (RootDisplay::subscreen)
			return RootDisplay::subscreen->process(event);
		// keep processing child elements
		return super::process(event);
	}

	return false;
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

void RootDisplay::exit()
{
	quit();
}

void quit()
{
	IMG_Quit();
	TTF_Quit();

	SDL_Delay(10);
	SDL_DestroyWindow(RootDisplay::mainDisplay->window);

	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	SDL_Quit();

#if defined(__WIIU__)
	romfsExit();
#endif

#if defined(SWITCH)
	socketExit();
#endif
	exit(0);
}

void RootDisplay::background(int r, int g, int b)
{
	SDL_SetRenderDrawColor(this->renderer, r, g, b, 0xFF);
	SDL_RenderFillRect(this->renderer, NULL);
}