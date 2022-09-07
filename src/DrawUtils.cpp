// This file should contain all external drawing SDL2/SDL1 calls
// programs outside of chesto should not be
// responsible for directly interacting with SDL!
#include "DrawUtils.hpp"
#include "RootDisplay.hpp"

#ifdef SDL1
static uint32_t CUR_DRAW_COLOR = 0xFFFFFFFF;
static uint32_t LAST_SDL1_FLIP = 0;
#endif

bool CST_DrawInit(RootDisplay* root)
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO) < 0)
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

#ifndef SDL1
	SDLFlags |= SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
#else
	SDLFlags |= SDL_DOUBLEBUF | SDL_HWSURFACE;
#ifdef _3DS
	SDLFlags |= SDL_HWSURFACE | SDL_DUALSCR;
#endif
#endif

#ifndef SDL1
	root->window = SDL_CreateWindow(NULL, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
	root->renderer = SDL_CreateRenderer(root->window, -1, SDLFlags);
	//Detach the texture
	SDL_SetRenderTarget(root->renderer, NULL);
#else
	SDL_WM_SetCaption("chesto", NULL);
	#ifdef _3DS
	root->renderer = SDL_SetVideoMode(400, 480, 8, SDLFlags);
	#else
	root->renderer = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 8, SDLFlags);
	#endif
	root->window = root->renderer;
#endif

	if (root->renderer == NULL || root->window == NULL)
	{
#ifdef _3DS
		char* err = SDL_GetError();
		FILE *file = fopen("error.txt", "w");
		fprintf(file, "%s", err);
		fclose(file);
#endif
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
	if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 4096) != 0) {
		printf("Failed to initialize SDL2 mixer: %s\n", Mix_GetError());
		return;
	}
	// root->music will be null if file does not exist or some issue
	root->music = Mix_LoadMUS("./background.mp3");
#endif
}

void CST_FadeInMusic(RootDisplay* root)
{
#if defined(MUSIC)
	if (root->music)
	{
		Mix_VolumeMusic(0.6 *  MIX_MAX_VOLUME);
		Mix_PlayMusic(root->music, -1);
		Mix_VolumeMusic(0.6 *  MIX_MAX_VOLUME);
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

#ifndef SDL1
	SDL_RenderPresent(renderer);
#else
	SDL_Flip(renderer); //TODO: replace this hack with SDL_gfx framerate limiter
#endif
}

void CST_FreeSurface(CST_Surface* surface)
{
	SDL_FreeSurface(surface);
}

void CST_RenderCopy(CST_Renderer* dest, CST_Texture* src, CST_Rect* src_rect, CST_Rect* dest_rect)
{
#ifndef SDL1
	SDL_RenderCopy(dest, src, src_rect, dest_rect);
#else
	if (((dest_rect ? dest_rect->w : dest->w) != (src_rect ? src_rect->w : src->w)) || ((dest_rect ? dest_rect->h : dest->h) != (src_rect ? src_rect->h : src->h))) //Avoid slow software zoom if at all possible
	{
		double xFactor=(dest_rect ? dest_rect->w : dest->w)/(double)(src_rect ? src_rect->w : src->w);
		double yFactor=(dest_rect ? dest_rect->h : dest->h)/(double)(src_rect ? src_rect->h : src->h);
		SDL_Surface* zoomed = zoomSurface(src, xFactor, yFactor, SMOOTHING_ON);
		SDL_BlitSurface(zoomed, src_rect, dest, dest_rect);
		SDL_FreeSurface(zoomed);
	}
	else SDL_BlitSurface(src, src_rect, dest, dest_rect);
#endif
}

void CST_RenderCopyRotate(CST_Renderer* dest, CST_Texture* src, CST_Rect* src_rect, CST_Rect* dest_rect, int angle)
{
#ifndef SDL1
	SDL_RenderCopyEx(dest, src, src_rect, dest_rect, angle, NULL, SDL_FLIP_NONE);
#else
	if(angle==0) CST_RenderCopy(dest, src, src_rect, dest_rect); //Avoid slow software rotozoom if at all possible
	else
	{
		int xCenter = (dest_rect ? dest_rect->x : 0)+((dest_rect ? dest_rect->w : dest->w)/2);
		int yCenter = (dest_rect ? dest_rect->y : 0)+((dest_rect ? dest_rect->h : dest->h)/2);
		double xFactor=(dest_rect ? dest_rect->w : dest->w)/(double)(src_rect ? src_rect->w : src->w);
		double yFactor=(dest_rect ? dest_rect->h : dest->h)/(double)(src_rect ? src_rect->h : src->h);
		SDL_Surface* rotozoomed = rotozoomSurfaceXY(src, (double) angle, xFactor, yFactor, SMOOTHING_ON);
		SDL_Rect recentered;
		recentered.x=xCenter-(rotozoomed->w)/2;
		recentered.y=yCenter-(rotozoomed->h)/2;
		SDL_BlitSurface(rotozoomed, src_rect, dest, &recentered);
		SDL_FreeSurface(rotozoomed);
	}
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

void CST_SetDrawColorRGBA(CST_Renderer* renderer, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
#ifndef SDL1
	SDL_SetRenderDrawColor(renderer, r, g, b, a);
#else
	CUR_DRAW_COLOR = SDL_MapRGBA(RootDisplay::renderer->format, r, g, b, a);
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

void CST_DrawRect(CST_Renderer* renderer, CST_Rect* dimens)
{
#ifndef SDL1
	SDL_RenderDrawRect(renderer, dimens);
#else
	// SDL_DrawRect(renderer, dimens, CUR_DRAW_COLOR);
#endif
}

void CST_DrawLine(CST_Renderer* renderer, int x, int y, int w, int h)
{
	#ifndef SDL1
	SDL_RenderDrawLine(renderer, x, y, w, h);
	#else
	// TODO: draw line for SDL1
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
	return SDL_ConvertSurface(surface, surface->format, NULL); //Creates duplicate of surface...psst it's not actually a texture
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

void CST_Delay(int time)
{
	SDL_Delay(time);
}

int CST_GetTicks()
{
	return SDL_GetTicks();
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

#ifdef SDL1
CST_Font* CST_CreateFont() { return NULL; }
void CST_LoadFont(CST_Font* font,  CST_Renderer* renderer, const char* filename_ttf, Uint32 pointSize, CST_Color color, int style) { }
CST_Color CST_MakeColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) { return { 0xFF, 0xFF, 0xFF, 0xFF }; }
Uint16 CST_GetFontLineHeight(CST_Font* font) { return 0; }
Uint16 CST_GetFontWidth(CST_Font* font, const char* formatted_text, ...) { return 0; }
Uint16 CST_GetFontHeight(CST_Font* font, const char* formatted_text, ...) { return 0; }
CST_Rect CST_DrawFont(CST_Font* font, CST_Renderer* dest, float x, float y, const char* formatted_text, ...) { return { x: 0, y:0, w: 0, h:0 }; }
#endif