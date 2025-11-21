#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// This file should contain all external drawing SDL2 calls
// programs outside of chesto should not be
// responsible for directly interacting with SDL!
#include "DrawUtils.hpp"
#include "RootDisplay.hpp"

namespace Chesto {

char* musicData = NULL;

bool CST_DrawInit(RootDisplay* root)
{
	int sdl2Flags = SDL_INIT_GAMECONTROLLER;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO | sdl2Flags) < 0)
	{
		printf("Failed to initialize SDL2 drawing library: %s\n", SDL_GetError());
		return false;
	}

	CST_SetQualityHint("linear");
	if (TTF_Init() < 0)
	{
		printf("Failed to initialize TTF font library: %s\n", SDL_GetError());
		return false;
	}

	int SDLFlags = 0;
	int windowFlags = 0;

	SDLFlags |= SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
	windowFlags |= SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;

	root->window = SDL_CreateWindow(
		NULL, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		SCREEN_WIDTH, SCREEN_HEIGHT, windowFlags);
	root->renderer = SDL_CreateRenderer(root->window, -1, SDLFlags);
	//Detach the texture
	SDL_SetRenderTarget(root->renderer, NULL);

	if (root->renderer == NULL || root->window == NULL)
	{
		SDL_Quit();
		return false;
	}

	RootDisplay::mainDisplay = root;

	for (int i = 0; i < SDL_NumJoysticks(); i++)
	{
		if (SDL_JoystickOpen(i) == NULL)
		{
			printf("SDL_JoystickOpen: %s\n", SDL_GetError());
			SDL_Quit();
			return false;
		}
	}

	// set up the SDL needsRender event
	root->needsRender.type = SDL_USEREVENT;
	return true;
}

void CST_DrawExit()
{
	//IMG_Quit();
	TTF_Quit();

	SDL_Delay(10);
	SDL_DestroyWindow(RootDisplay::mainDisplay->window);
	SDL_QuitSubSystem(SDL_INIT_VIDEO);

#ifndef SIMPLE_SDL2
	auto root = RootDisplay::mainDisplay;
	if (root->music != NULL)
	{
		Mix_FreeMusic(root->music);
		root->music = NULL;
	}
	if (musicData != NULL)
	{
		free(musicData);
		musicData = NULL;
	}
#endif

	SDL_Quit();
}

void CST_MixerInit(RootDisplay* root)
{
#if !defined(SIMPLE_SDL2)
	Mix_Init(MIX_INIT_MP3);
	if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 4096) != 0) {
		printf("Failed to initialize SDL2 mixer: %s\n", Mix_GetError());
		return;
	}
	// root->music will be null if file does not exist or some issue
	// if it does exist, we'll read it all into memory to avoid streaming from disk
	const char* filename = "background.mp3";
	FILE* f = fopen(filename, "rb");
	if (f == NULL) {
		return;
	}
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	if (fsize <= 0) {
		fclose(f);
		return;
	}

	fseek(f, 0, SEEK_SET);

	musicData = (char*)(malloc(fsize + 1));
	fread(musicData, fsize, 1, f);
	fclose(f);

	musicData[fsize] = 0;

	root->music = Mix_LoadMUS_RW(SDL_RWFromMem(musicData, fsize), 1);
#endif
}


void CST_GetRGBA(Uint32 pixel, SDL_PixelFormat* format, CST_Color* cstColor)
{
	SDL_GetRGBA(pixel, format, &cstColor->r, &cstColor->g, &cstColor->b, &cstColor->a);
}

// https://stackoverflow.com/a/51238719/4953343
bool CST_SavePNG(CST_Texture* texture, const char* file_name)
{
	auto renderer = RootDisplay::mainDisplay->renderer;
    SDL_Texture* target = SDL_GetRenderTarget(renderer);
    SDL_SetRenderTarget(renderer, texture);
	printf("Error: %s\n", SDL_GetError());
    int width, height;
    SDL_QueryTexture(texture, NULL, NULL, &width, &height);
    SDL_Surface* surface = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
    SDL_RenderReadPixels(renderer, NULL, surface->format->format, surface->pixels, surface->pitch);
    IMG_SavePNG(surface, file_name);
    SDL_FreeSurface(surface);
    SDL_SetRenderTarget(renderer, target);
	return true;
}

void CST_FadeInMusic(RootDisplay* root)
{
#if !defined(SIMPLE_SDL2)
	if (root->music)
	{
		Mix_VolumeMusic(0.85 *  MIX_MAX_VOLUME);
		Mix_PlayMusic(root->music, -1);
		Mix_VolumeMusic(0.85 *  MIX_MAX_VOLUME);
	}
#endif
}

void CST_RenderPresent(CST_Renderer* renderer)
{
#ifdef _3DS_MOCK
	// draw some borders around parts of the 3ds screen
	CST_SetDrawColorRGBA(renderer, 0, 0, 0, 255);
	CST_Rect rect = { 0, 240, 40, 240 };
	CST_FillRect(renderer, &rect);
	CST_Rect rect2 = { 360, 240, 40, 240 };
	CST_FillRect(renderer, &rect2);
	CST_Rect rect3 = { 0, 240, 400, 240 };
	CST_DrawRect(renderer, &rect3);
#endif

	SDL_RenderPresent(renderer);
}

void CST_FreeSurface(CST_Surface* surface)
{
	SDL_FreeSurface(surface);
}

void CST_RenderCopy(CST_Renderer* dest, CST_Texture* src, CST_Rect* src_rect, CST_Rect* dest_rect)
{
	SDL_RenderCopy(dest, src, src_rect, dest_rect);
}

void CST_RenderCopyRotate(CST_Renderer* dest, CST_Texture* src, CST_Rect* src_rect, CST_Rect* dest_rect, int angle)
{
	SDL_RenderCopyEx(dest, src, src_rect, dest_rect, angle, NULL, SDL_FLIP_NONE);
}

void CST_SetDrawColor(CST_Renderer* renderer, CST_Color c)
{
	CST_SetDrawColorRGBA(renderer, c.r, c.g, c.b, c.a);
}

void CST_SetDrawColorRGBA(CST_Renderer* renderer, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	SDL_SetRenderDrawColor(renderer, r, g, b, a);
}

void CST_FillRect(CST_Renderer* renderer, CST_Rect* dimens)
{
	SDL_RenderFillRect(renderer, dimens);
}

void CST_DrawRect(CST_Renderer* renderer, CST_Rect* dimens)
{
	SDL_RenderDrawRect(renderer, dimens);
}

void CST_DrawLine(CST_Renderer* renderer, int x, int y, int w, int h)
{
	SDL_RenderDrawLine(renderer, x, y, w, h);
}

void CST_SetDrawBlend(CST_Renderer* renderer, bool enabled)
{
	SDL_BlendMode mode = enabled ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_NONE;
	SDL_SetRenderDrawBlendMode(renderer, mode);
}

void CST_QueryTexture(CST_Texture* texture, int* w, int* h)
{
	SDL_QueryTexture(texture, nullptr, nullptr, w, h);
}

CST_Texture* CST_CreateTextureFromSurface(CST_Renderer* renderer, CST_Surface* surface, bool isAccessible )
{
	return SDL_CreateTextureFromSurface(renderer, surface);	
}

void CST_SetQualityHint(const char* quality)
{
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, quality);
}

void CST_filledCircleRGBA(CST_Renderer* renderer, uint32_t x, uint32_t y, uint32_t radius, uint32_t r, uint32_t g, uint32_t b, uint32_t a)
{
	#if !defined(SIMPLE_SDL2)
	// TODO: filledCircleRGBA needs to take a surface in SIMPLE_SDL2
	filledCircleRGBA(renderer, x, y, radius, r, g, b, a);
	#endif
}

void CST_SetWindowSize(CST_Window* window, int w, int h)
{
	// actually resize the window, and adjust it for high DPI
	SDL_SetWindowSize(window, w, h);
}


void CST_Delay(int time)
{
	SDL_Delay(time);
}

int CST_GetTicks()
{
	return SDL_GetTicks();
}

void CST_LowRumble(InputEvents* event, int ms)
{
	auto joystick = SDL_JoystickFromInstanceID(event->event.jdevice.which);
	if (joystick && SDL_JoystickGetAttached(joystick)) {
		SDL_JoystickRumble(joystick, 0x400, 0x400, 200);
	}
}

void CST_SetCursor(int cursor)
{
	if (cursor == CST_CURSOR_NONE) {
		SDL_ShowCursor(SDL_DISABLE);
		return;
	}
	
	SDL_ShowCursor(SDL_ENABLE);
	auto sdlCursor = SDL_SYSTEM_CURSOR_ARROW;
	if (cursor == CST_CURSOR_HAND) {
		sdlCursor = SDL_SYSTEM_CURSOR_HAND;
	} else if (cursor == CST_CURSOR_TEXT) {
		sdlCursor = SDL_SYSTEM_CURSOR_IBEAM;
	} else if (cursor == CST_CURSOR_SPINNER) {
		sdlCursor = SDL_SYSTEM_CURSOR_WAIT;
	}

	SDL_SetCursor(SDL_CreateSystemCursor(sdlCursor));
}

bool CST_isRectOffscreen(CST_Rect* rect)
{
	// if this element will be offscreen, don't try to render it
	if (rect->x > SCREEN_WIDTH + 10 || rect->y > SCREEN_HEIGHT + 10)
		return true;

	// same but for up direction (weirder, cause width and height need to be correct)
	// (which may not be true for container elements (sounds like a css float problem...))
	if (rect->x + rect->w < -10 || rect->y + rect->h < -10)
		return true;

	return false;
}

// returns the high-dpi scaling factor, by measuring the window size and the drawable size (ratio)
float CST_GetDpiScale()
{
	int w, h;
	SDL_GetWindowSize(RootDisplay::window, &w, &h);
	int dw, dh;
	SDL_GL_GetDrawableSize(RootDisplay::window, &dw, &dh);
	return (dw / (float) w);
}

void CST_SetWindowTitle(const char* title)
{
	SDL_SetWindowTitle(RootDisplay::mainDisplay->window, title);
}

void CST_roundedBoxRGBA (
	CST_Renderer *renderer,
	Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2,
	Sint16 rad, Uint8 r, Uint8 g, Uint8 b, Uint8 a
) {
	#if !defined(SIMPLE_SDL2)
	// TODO: roundedBoxRGBA needs to take a surface in SIMPLE_SDL2
	roundedBoxRGBA(renderer, x1, y1, x2, y2, rad, r, g, b, a);
	#endif
}

void CST_roundedRectangleRGBA (
	CST_Renderer *renderer,
	Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2,
	Sint16 rad, Uint8 r, Uint8 g, Uint8 b, Uint8 a
) {
	#if !defined(SIMPLE_SDL2)
	// TODO: roundedRectangleRGBA needs to take a surface in SIMPLE_SDL2
	roundedRectangleRGBA(renderer, x1, y1, x2, y2, rad, r, g, b, a);
	#endif
}

void CST_rectangleRGBA (
	CST_Renderer *renderer,
	Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2,
	Uint8 r, Uint8 g, Uint8 b, Uint8 a
) {
	#if !defined(SIMPLE_SDL2)
	// TODO: rectangleRGBA needs to take a surface in SIMPLE_SDL2
	rectangleRGBA(renderer, x1, y1, x2, y2, r, g, b, a);
	#endif
}

#ifndef SIMPLE_SDL2

// returns a size-3 vector of: (title, artist, album)
std::vector<std::string> CST_GetMusicInfo(CST_Music* music) {
	std::vector<std::string> info;
	// these methods can use recent (late 2019) SDL_Mixer versions to read info about the song
	// (Currently commented out)
	// info.push_back(Mix_GetMusicTitle(music));
	// info.push_back(Mix_GetMusicArtistTag(music));
	// info.push_back(Mix_GetMusicAlbumTag(music));

	// have to use the mpg123 library manually to get this info
	// adapted from https://gist.github.com/deepakjois/988032/640b7a41b0e62a394515697c142777ad3a1b8905
	auto m = mpg123_new(NULL, NULL);
	mpg123_open(m, "./background.mp3");
	mpg123_seek(m, 0, SEEK_SET);
	auto meta = mpg123_meta_check(m);

	mpg123_id3v1* v1;
  	mpg123_id3v2* v2;

	if (meta == MPG123_ID3 && mpg123_id3(m, &v1, &v2) == MPG123_OK) {
		if (v2 != NULL) {
			// fmt.Println("ID3V2 tag found")
			info.push_back(v2->title && v2->title->p  ? v2->title->p  : "");
			info.push_back(v2->artist && v2->artist->p ? v2->artist->p : "");
			info.push_back(v2->album && v2->album->p  ? v2->album->p  : "");
			mpg123_close(m);
			return info;
		}
		if (v1 != NULL) {
			// fmt.Println("ID3V1 tag found")
			info.push_back(v1->title[0]  ? v1->title  : "");
			info.push_back(v1->artist[0] ? v1->artist : "");
			info.push_back(v1->album[0]  ? v1->album  : "");
			mpg123_close(m);
			return info;			
		}
	}

	// we couldn't find the tags, so just return empty strings
	info.push_back("");
	info.push_back("");
	info.push_back("");
	mpg123_close(m);
	return info;

}
#endif

// change into the directory of the executable for the current platform
void chdirForPlatform()
{
	auto basePath = SDL_GetBasePath();
	if (basePath != NULL) {
		chdir(basePath);
		SDL_free(basePath);
	}
}

std::string replaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}

CST_Color toCST(rgb color)
{
	CST_Color cstColor;
	cstColor.r = (uint8_t)(color.r * 255);
	cstColor.g = (uint8_t)(color.g * 255);
	cstColor.b = (uint8_t)(color.b * 255);
	cstColor.a = 255;
	return cstColor;
}

rgb fromCST(CST_Color color)
{
	return (rgb){ color.r / 255.0, color.g / 255.0, color.b / 255.0 };
}

} // namespace Chesto