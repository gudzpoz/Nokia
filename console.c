#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <string.h>
#include "console.h"

#define MONOSPACE_FONT_PATH "res/DejaVuSansMono.ttf"
#define BGM_PATH "res/bgm_maoudamashii_8bit29.mp3"
#define SHOOT_SE "res/se_maoudamashii_retro04.wav"
#define HIT_SE "res/se_maoudamashii_retro02.wav"
#define SCORE_SE "res/se_maoudamashii_retro16.wav"
#define DEFAULT_FONT_SIZE 25
#define CONSOLE_WIDTH 80
#define CONSOLE_HEIGHT 24

static TTF_Font* font = 0;
static int characterWidth, characterHeight;

static SDL_Window* window = 0;
static SDL_Renderer* renderer = 0;
static char buffer[CONSOLE_WIDTH][CONSOLE_HEIGHT];

static int cursorX = 0, cursorY = 0;
static const SDL_Color colors[] = {
	{   0,   0,   0, 255 },
	{ 255, 255, 255, 255 },
	{ 255,   0,   0, 255 },
	{   0, 255,   0, 255 },
	{   0,   0, 255, 255 },
	{ 255, 255,   0, 255 },
	{   0, 255, 255, 255 },
	{ 255,   0, 255, 255 }
};
static const SDL_Color* color = &colors[WHITE];
static const SDL_Color* background = &colors[BLACK];
static int updatable = 1;

static Mix_Music* bgm = 0;
static Mix_Chunk* shootSE = 0;
static Mix_Chunk* hitSE = 0;
static Mix_Chunk* scoreSE = 0;

void free_BGM() {
	Mix_FreeMusic(bgm);
	Mix_FreeChunk(shootSE);
	Mix_FreeChunk(hitSE);
	Mix_FreeChunk(scoreSE);
	bgm = 0;
	shootSE = 0;
	hitSE = 0;
	scoreSE = 0;
}

void init_SDL2() {
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Init(SDL_INIT_AUDIO);
	TTF_Init();
	Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG);
	Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 1, 1024);
	bgm = Mix_LoadMUS(BGM_PATH);
	shootSE = Mix_LoadWAV(SHOOT_SE);
	hitSE = Mix_LoadWAV(HIT_SE);
	scoreSE = Mix_LoadWAV(SCORE_SE);
	Mix_Volume(-1, MIX_MAX_VOLUME);
	Mix_VolumeMusic(MIX_MAX_VOLUME/2);
}

void start_BGM() {
	Mix_PlayMusic(bgm, -1);
}

void play_SE(enum SoundEffect se) {
	switch(se) {
	case SHOOT:
		Mix_PlayChannel(-1, shootSE, 0);
		break;
	case HIT:
		Mix_PlayChannel(-1, hitSE, 0);
		break;
	case SCORE:
		Mix_PlayChannel(-1, scoreSE, 0);
		break;
	}
}

void stop_BGM() {
	Mix_HaltMusic();
}

void quit_SDL2() {
	free_BGM();
	TTF_Quit();
	SDL_Quit();
}

void init_font() {
	font = TTF_OpenFont(MONOSPACE_FONT_PATH, DEFAULT_FONT_SIZE);
	TTF_SizeText(font, "M", &characterWidth, &characterHeight); // Get the character size of the monospace font
}

void init_console(char* title) {
	if(font == 0) {
		init_SDL2();
		init_font();
		int windowWidth = characterWidth * CONSOLE_WIDTH;
		int windowHeight = characterHeight * CONSOLE_HEIGHT;
		window = SDL_CreateWindow(title, 100, 100, windowWidth, windowHeight, SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	} else {
		color = &colors[WHITE];
		background = &colors[BLACK];
		cursorX = 0;
		cursorY = 0;
	}
	SDL_RenderClear(renderer);
}

void quit_console() {
	TTF_CloseFont(font);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	font = 0;
	renderer = 0;
	window = 0;
	quit_SDL2();
}

void set_cursor(int x, int y) {
	// Allowing irregular x, y on purpose
	cursorX = x;
	cursorY = y;
}

static void update_renderer_at(int x, int y) {
	SDL_SetRenderDrawColor(renderer,
			       background->r,
			       background->g,
			       background->b,
			       background->a);
	SDL_Rect r;
	r.x = x * characterWidth;
	r.y = y * characterHeight;
	r.w = characterWidth;
	r.h = characterHeight;
	SDL_RenderFillRect(renderer, &r);
  
	char string[2] = { buffer[x][y], '\0' };
	SDL_Surface* text = TTF_RenderText_Solid(font, string, *color);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, text);
	SDL_Rect destination = { x * characterWidth, y * characterHeight, characterWidth, characterHeight };
	SDL_RenderCopy(renderer, texture, 0, &destination);
	SDL_DestroyTexture(texture);
	SDL_FreeSurface(text);
}

static void render() {
	SDL_RenderPresent(renderer);
}

void fill_char(char c) {
	if(0 <= cursorX && cursorX < CONSOLE_WIDTH) {
		if(0 <= cursorY && cursorY < CONSOLE_HEIGHT) {
			buffer[cursorX][cursorY] = c;
			update_renderer_at(cursorX, cursorY);
		}
	}
}

void putch(char c) {
	fill_char(c);
	++cursorX;
}

void delay(unsigned int mills) {
	SDL_Delay(mills);
}

void put_string(const char* c) {
	while(*c != '\0') {
		putch(*c);
		++c;
	}
}

static int poll_event(SDL_Event* event) {
	int notEmpty = SDL_PollEvent(event);
	if(notEmpty) {
		SDL_Event trash;
		while(SDL_PollEvent(&trash)) {
			if(trash.type == SDL_QUIT) {
				quit_console();
				exit(0);
			}
		}
	}
	return notEmpty;
}

static int wait_event(SDL_Event* event) {
	int notEmpty = SDL_WaitEvent(event);
	if(notEmpty) {
		SDL_Event trash;
		while(SDL_PollEvent(&trash)) {
			if(trash.type == SDL_QUIT) {
				quit_console();
				exit(0);
			}
		}
	}
	return notEmpty;
}

int get_char_from_keycode(SDL_Keycode code) {
	switch(code) {
	case SDLK_SPACE:
		return ' ';
	case SDLK_RETURN:
		return '\n';
	default:
		return SDL_GetKeyName(code)[0];
	}
}

int wait_getch() {
	SDL_Event event;
	while(wait_event(&event)) {
		if(event.type == SDL_QUIT) {
			quit_console();
			exit(0);
		} else if(event.type == SDL_KEYDOWN) {
			return get_char_from_keycode(event.key.keysym.sym);
		}
	}
	return EOF;
}

int getch() {
	SDL_PumpEvents();
	SDL_Event event;
	if(poll_event(&event)) {
		if(event.type == SDL_QUIT) {
			quit_console();
			exit(0);
		} else if(event.type == SDL_KEYDOWN) {
			return get_char_from_keycode(event.key.keysym.sym);
		} else {
			return EOF;
		}
	} else {
		return EOF;
	}
}

void update() {
	if(updatable) {
		render();
	}
}

void pause_update() {
	updatable = 0;
}

void resume_update() {
	updatable = 1;
	update();
}

void clear_screen() {
	SDL_RenderClear(renderer);
}

int get_console_width() {
	return CONSOLE_WIDTH;
}

int get_console_height() {
	return CONSOLE_HEIGHT;
}

void set_color(enum Color newColor) {
	color = &colors[newColor];
}

void set_background_color(enum Color newBackground) {
	background = &colors[newBackground];
}
