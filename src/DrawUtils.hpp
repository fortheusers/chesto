#include <vector>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#if !defined(SIMPLE_SDL2)
#include <SDL2/SDL2_gfxPrimitives.h> // 3ds has no gfx library
#endif

#include "../libs/SDL_FontCache/SDL_FontCache.h"

#include <string>

#if !defined(SIMPLE_SDL2)
	#include <SDL2/SDL_mixer.h>
	#include <mpg123.h>
	typedef Mix_Music CST_Music;
#endif

#include "colorspaces.hpp"

typedef SDL_Window CST_Window;
typedef SDL_Renderer CST_Renderer;
typedef SDL_Texture CST_Texture;

typedef SDL_Surface CST_Surface;
typedef SDL_Color CST_Color;
typedef SDL_Rect CST_Rect;

class RootDisplay;
class InputEvents;

#define CST_CURSOR_NONE -1
#define CST_CURSOR_ARROW 0
#define CST_CURSOR_HAND 1
#define CST_CURSOR_TEXT 2
#define CST_CURSOR_SPINNER 3

// init / rendering analogues
bool CST_DrawInit(RootDisplay* root);
void CST_MixerInit(RootDisplay* root);
void CST_FadeInMusic(RootDisplay* root);
void CST_DrawExit();
void CST_RenderPresent(CST_Renderer* render);
void CST_FreeSurface(CST_Surface* surface);

void CST_RenderCopy(CST_Renderer* dest, CST_Texture* src, CST_Rect* src_rect, CST_Rect* dest_rect);
void CST_RenderCopyRotate(CST_Renderer* dest, CST_Texture* src, CST_Rect* src_rect, CST_Rect* dest_rect, int angle);

// color analogues
void CST_SetDrawColor(CST_Renderer* renderer, CST_Color c);
void CST_SetDrawColorRGBA(CST_Renderer* renderer, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void CST_FillRect(CST_Renderer* renderer, CST_Rect* dimens);
void CST_DrawRect(CST_Renderer* renderer, CST_Rect* dimens);
void CST_SetDrawBlend(CST_Renderer* renderer, bool enabled);
void CST_DrawLine(CST_Renderer* renderer, int x, int y, int w, int h);

void CST_QueryTexture(CST_Texture* texture, int* w, int* h);
CST_Texture* CST_CreateTextureFromSurface(CST_Renderer* renderer, CST_Surface* surface, bool isAccessible);
void CST_SetQualityHint(const char* quality);

void CST_filledCircleRGBA(CST_Renderer* renderer, uint32_t x, uint32_t y, uint32_t radius, uint32_t r, uint32_t g, uint32_t b, uint32_t a);
void CST_roundedBoxRGBA (
	CST_Renderer *renderer,
	Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2,
	Sint16 rad, Uint8 r, Uint8 g, Uint8 b, Uint8 a
);
void CST_roundedRectangleRGBA (
	CST_Renderer *renderer,
	Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2,
	Sint16 rad, Uint8 r, Uint8 g, Uint8 b, Uint8 a
);
void CST_rectangleRGBA (
	CST_Renderer *renderer,
	Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2,
	Uint8 r, Uint8 g, Uint8 b, Uint8 a
);
float CST_GetDpiScale();
void CST_SetWindowSize(CST_Window* renderer, int w, int h);
void CST_Delay(int time);

int CST_GetTicks();
bool CST_isRectOffscreen(CST_Rect* rect);

void CST_GetRGBA(Uint32 pixel, SDL_PixelFormat* format, CST_Color* cstColor);
bool CST_SavePNG(CST_Texture* texture, const char* filename);
void CST_SetWindowTitle(const char* title);

void CST_SetCursor(int cursor);
void CST_LowRumble(InputEvents* event, int ms);

CST_Color toCST(rgb color);
rgb fromCST(CST_Color color);

#ifndef SIMPLE_SDL2
std::vector<std::string> CST_GetMusicInfo(CST_Music* music);
#endif

// font cache analogues
// SDL2, will be backed by SDL_FontCache
#define CST_Font FC_Font
#define CST_CreateFont FC_CreateFont
#define CST_LoadFont FC_LoadFont
#define CST_MakeColor FC_MakeColor
#define CST_GetFontLineHeight FC_GetLineHeight
#define CST_GetFontWidth FC_GetWidth
#define CST_GetFontHeight FC_GetHeight
#define CST_DrawFont FC_Draw

void chdirForPlatform();
std::string replaceAll(std::string str, const std::string& from, const std::string& to);