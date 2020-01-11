// This file should contain all external drawing SDL2/SDL1 calls
// programs outside of chesto should not be
// responsible for directly interacting with SDL!
#include "DrawUtils.hpp"
#include "RootDisplay.hpp"

void CST_DrawInit(RootDisplay* root)
{
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO) < 0)
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
  SDL_WM_SetCaption(NULL, "chesto");
  root->renderer = SDL_SetVideoMode(640, 480, 0, SDL_FULLSCREEN | SDL_OPENGL);
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
	SDL_DestroyWindow(RootDisplay::mainDisplay->window);

	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	SDL_Quit();
}

void CST_MixerInit(RootDisplay* root)
{
#if defined(MUSIC)
  Mix_Init(MIX_INIT_MP3);
	Mix_OpenAudio(22050, AUDIO_S16SYS, 2, 640);
	root->music = Mix_LoadMUS(RAMFS "./res/music.mp3");
	if (music)
	{
		Mix_FadeInMusic(music, -1, 300);
	}
#endif
}

void CST_RenderPresent(CST_Renderer* render)
{
  SDL_RenderPresent(render);
}

void CST_FreeSurface(CST_Surface* surface)
{
  SDL_FreeSurface(surface);
}

void CST_SetDrawColor(CST_Renderer* renderer, CST_Color c)
{
  CST_SetDrawColorRGBA(renderer, c.r, c.g, c.b, c.a);
}

void CST_SetDrawColorRGBA(CST_Renderer* renderer, uint32_t r, uint32_t g, uint32_t b, uint32_t a)
{
  SDL_SetRenderDrawColor(renderer, r, g, b, a);
}

void CST_FillRect(CST_Renderer* renderer, CST_Rect* dimens)
{
  SDL_RenderFillRect(renderer, dimens);
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

CST_Texture* CST_CreateTextureFromSurface(CST_Renderer* renderer, CST_Surface* surface)
{
  return SDL_CreateTextureFromSurface(renderer, surface);
}

void CST_SetQualityHint(const char* quality)
{
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, quality);
}

void CST_filledCircleRGBA(CST_Renderer* renderer, uint32_t x, uint32_t y, uint32_t radius, uint32_t r, uint32_t g, uint32_t b, uint32_t a)
{
  filledCircleRGBA(renderer, x, y, radius, r, g, b, a);
}