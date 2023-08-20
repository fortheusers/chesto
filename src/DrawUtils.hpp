#include <vector>

#ifndef SDL1
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL2_gfxPrimitives.h>

#include "../libs/SDL_FontCache/SDL_FontCache.h"

#if defined(MUSIC)
	#include <SDL2/SDL_mixer.h>
	#include <string>
	#include <mpg123.h>
	typedef Mix_Music CST_Music;
#endif

typedef SDL_Window CST_Window;
typedef SDL_Renderer CST_Renderer;
typedef SDL_Texture CST_Texture;
#else
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_gfxPrimitives.h>
#include <SDL/SDL_rotozoom.h>

typedef SDL_Surface CST_Window;
typedef SDL_Surface CST_Renderer;
typedef SDL_Surface CST_Texture;

#pragma once

#define TTF_RenderText_Blended_Wrapped(a, b, c, d) TTF_RenderUTF8_Blended(a, b, c)
#endif

typedef SDL_Surface CST_Surface;

typedef SDL_Color CST_Color;
typedef SDL_Rect CST_Rect;

class RootDisplay;

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
float CST_GetDpiScale();
void CST_SetWindowSize(CST_Window* renderer, int w, int h);
void CST_Delay(int time);

int CST_GetTicks();
bool CST_isRectOffscreen(CST_Rect* rect);

bool CST_SavePNG(CST_Texture* texture, const char* filename);
void CST_SetWindowTitle(const char* title);

#ifdef MUSIC
std::vector<std::string> CST_GetMusicInfo(CST_Music* music);
#endif

// font cache analogues
#ifndef SDL1
// SDL2, will be backed by SDL_FontCache
#define CST_Font FC_Font
#define CST_CreateFont FC_CreateFont
#define CST_LoadFont FC_LoadFont
#define CST_MakeColor FC_MakeColor
#define CST_GetFontLineHeight FC_GetLineHeight
#define CST_GetFontWidth FC_GetWidth
#define CST_GetFontHeight FC_GetHeight
#define CST_DrawFont FC_Draw
#else
// SDL1 font ops are backed by our own cache (could also be used with other backends)
// TODO: all these are stubs for now, for SDL1
typedef TTF_Font CST_Font; // for now, these are the same, we may need our own representation later
CST_Font* CST_CreateFont();
void CST_LoadFont(CST_Font* font,  CST_Renderer* renderer, const char* filename_ttf,  Uint32 pointSize, CST_Color color, int style);
CST_Color CST_MakeColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a);
Uint16 CST_GetFontLineHeight(CST_Font* font);
Uint16 CST_GetFontWidth(CST_Font* font, const char* formatted_text, ...);
Uint16 CST_GetFontHeight(CST_Font* font, const char* formatted_text, ...);
CST_Rect CST_DrawFont(CST_Font* font, CST_Renderer* dest, float x, float y, const char* formatted_text, ...);
#endif

void chdirForPlatform();