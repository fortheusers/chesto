#pragma once

#ifndef SDL1
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL2_gfxPrimitives.h>
  #if defined(MUSIC)
  #include <SDL2/SDL_mixer.h>
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

#define TTF_RenderText_Blended_Wrapped(a, b, c, d) TTF_RenderUTF8_Blended(a, b, c)
#endif

typedef SDL_Surface CST_Surface;

typedef SDL_Color CST_Color;
typedef SDL_Rect CST_Rect;

class RootDisplay;

static uint32_t CUR_DRAW_COLOR = 0xFFFFFFFF;
#ifdef SDL1
static uint32_t LAST_SDL1_FLIP = 0;
#endif

bool CST_DrawInit(RootDisplay* root);
void CST_MixerInit(RootDisplay* root);
void CST_DrawExit();
void CST_RenderPresent(CST_Renderer* render);
void CST_FreeSurface(CST_Surface* surface);

void CST_RenderCopy(CST_Renderer* dest, CST_Texture* src, CST_Rect* src_rect, CST_Rect* dest_rect);
void CST_RenderCopyRotate(CST_Renderer* dest, CST_Texture* src, CST_Rect* src_rect, CST_Rect* dest_rect, int angle);

void CST_SetDrawColor(CST_Renderer* renderer, CST_Color c);
void CST_SetDrawColorRGBA(CST_Renderer* renderer, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void CST_FillRect(CST_Renderer* renderer, CST_Rect* dimens);
void CST_DrawRect(CST_Renderer* renderer, CST_Rect* dimens);
void CST_SetDrawBlend(CST_Renderer* renderer, bool enabled);

void CST_QueryTexture(CST_Texture* texture, int* w, int* h);
CST_Texture* CST_CreateTextureFromSurface(CST_Renderer* renderer, CST_Surface* surface);
void CST_SetQualityHint(const char* quality);

void CST_filledCircleRGBA(CST_Renderer* renderer, uint32_t x, uint32_t y, uint32_t radius, uint32_t r, uint32_t g, uint32_t b, uint32_t a);
void CST_Delay(int time);

int CST_GetTicks();
bool CST_isRectOffscreen(CST_Rect* rect);
