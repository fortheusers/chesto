// This file should contain all external drawing SDL2/SDL1 calls
// programs outside of chesto should not be
// responsible for directly interacting with SDL!
#include "DrawUtils.hpp"
#include "RootDisplay.hpp"

void CST_DrawInit(RootDisplay* root)
{
  //if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO) < 0)
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
  {
    printf("Failed to initialize SDL2 drawing library: %s\n", SDL_GetError());
    return;
  }

	CST_SetQualityHint("linear");
  if (TTF_Init() < 0)
  {
    printf("Failed to initialize TTF font library: %s\n", SDL_GetError());
    return;
  }

  int imgFlags = IMG_INIT_PNG;
  if (!(IMG_Init(imgFlags) & imgFlags))
  {
    printf("Failed to initialize SDL IMG library: %s\n", SDL_GetError());
    return;
  }

#ifndef SDL1
  root->window = SDL_CreateWindow(NULL, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
  root->renderer = SDL_CreateRenderer(root->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  //Detach the texture
  SDL_SetRenderTarget(root->renderer, NULL);
#else
  SDL_WM_SetCaption("chesto", NULL);
  root->renderer = SDL_SetVideoMode(1280, 720, 0, SDL_DOUBLEBUF);
  root->window = root->renderer;
#endif

  RootDisplay::mainRenderer = root->renderer;
  RootDisplay::mainDisplay = root;

  for (int i = 0; i < SDL_NumJoysticks(); i++)
  {
    if (SDL_JoystickOpen(i) == NULL)
    {
      printf("SDL_JoystickOpen: %s\n", SDL_GetError());
      SDL_Quit();
      return;
    }
  }

  // set up the SDL needsRender event
  root->needsRender.type = SDL_USEREVENT;
}

void CST_DrawExit()
{
  IMG_Quit();
	TTF_Quit();

	SDL_Delay(10);
#ifndef SDL1
	SDL_DestroyWindow(RootDisplay::mainDisplay->window);
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
#endif

	SDL_Quit();
}

void CST_MixerInit(RootDisplay* root)
{
#if defined(MUSIC)
  Mix_Init(MIX_INIT_MP3);
	Mix_OpenAudio(22050, AUDIO_S16SYS, 2, 640);
	root->music = Mix_LoadMUS(RAMFS "./res/music.mp3");
	if (root->music)
	{
		Mix_FadeInMusic(root->music, -1, 300);
	}
#endif
}

void CST_RenderPresent(CST_Renderer* renderer)
{
#ifndef SDL1
  SDL_RenderPresent(renderer);
#else
  SDL_Flip(renderer);
#endif
}

void CST_FreeSurface(CST_Surface* surface)
{
  SDL_FreeSurface(surface);
}

void CST_RenderCopy(CST_Renderer* dest, CST_Texture* src, CST_Rect* src_rect, CST_Rect* dest_rect)
{
//if(src_rect==NULL) {
	//printf("WARNING: CST_RenderCopy fed a null src_rect, ignoring\n");
	//return;}
#ifndef SDL1
  SDL_RenderCopy(dest, src, src_rect, dest_rect);
#else
  SDL_BlitSurface(src, src_rect, dest, dest_rect);
#endif
}

void CST_RenderCopyRotate(CST_Renderer* dest, CST_Texture* src, CST_Rect* src_rect, CST_Rect* dest_rect, int angle)
{
#ifndef SDL1
  SDL_RenderCopyEx(dest, src, src_rect, dest_rect, angle, NULL, SDL_FLIP_NONE);
#else
  // TODO: figure out rotation on SDL1
  CST_RenderCopy(dest, src, src_rect, dest_rect);
#endif
}

void CST_SetDrawColor(CST_Renderer* renderer, CST_Color c)
{
#ifndef SDL1
  CST_SetDrawColorRGBA(renderer, c.r, c.g, c.b, c.a);
#else
  CST_SetDrawColorRGBA(renderer, c.r, c.g, c.b, c.unused);
#endif
}

void CST_SetDrawColorRGBA(CST_Renderer* renderer, uint32_t r, uint32_t g, uint32_t b, uint32_t a)
{
#ifndef SDL1
  SDL_SetRenderDrawColor(renderer, r, g, b, a);
#else
  CUR_DRAW_COLOR = SDL_MapRGBA(RootDisplay::mainRenderer->format, r, g, b, a);
#endif
}

void CST_FillRect(CST_Renderer* renderer, CST_Rect* dimens)
{
#ifndef SDL1
  SDL_RenderFillRect(renderer, dimens);
#else
  SDL_FillRect(renderer, dimens, CUR_DRAW_COLOR);
#endif
}

void CST_SetDrawBlend(CST_Renderer* renderer, bool enabled)
{
#ifndef SDL1
  SDL_BlendMode mode = enabled ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_NONE;
  SDL_SetRenderDrawBlendMode(renderer, mode);
#endif
}

void CST_QueryTexture(CST_Texture* texture, int* w, int* h)
{
#ifndef SDL1
  SDL_QueryTexture(texture, nullptr, nullptr, w, h);
#else
  *w = texture->w;
  *h = texture->h;
#endif
}

CST_Texture* CST_CreateTextureFromSurface(CST_Renderer* renderer, CST_Surface* surface)
{
#ifndef SDL1
  return SDL_CreateTextureFromSurface(renderer, surface);
#else
  // it's a secret to everyone
  return SDL_ConvertSurface(surface, surface->format, NULL);
#endif
}

void CST_SetQualityHint(const char* quality)
{
#ifndef SDL1
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, quality);
#endif
}

void CST_filledCircleRGBA(CST_Renderer* renderer, uint32_t x, uint32_t y, uint32_t radius, uint32_t r, uint32_t g, uint32_t b, uint32_t a)
{
  filledCircleRGBA(renderer, x, y, radius, r, g, b, a);
}
