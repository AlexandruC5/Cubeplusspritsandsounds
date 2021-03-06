#include "stdio.h"
#include "../SDL/include/SDL.h"
#include "../SDL/include/SDL_image.h"
#include "../SDL/include/SDL_mixer.h"
#pragma comment( lib, "../SDL/libx86/SDL2.lib" )
#pragma comment( lib, "../SDL/libx86/SDL2main.lib" )
#pragma comment( lib, "../SDL/libx86/SDL2_image.lib" )
#pragma comment( lib, "../SDL/libx86/SDL2_mixer.lib" )

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define SCROLL_SPEED 5
#define SHIP_SPEED 3
#define NUM_SHOTS 32
#define SHOT_SPEED 5

struct projectile
{
	int x, y;
	bool alive;
};

struct globals
{
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	SDL_Texture* background = nullptr;
	SDL_Texture* ship = nullptr;
	SDL_Texture* shot = nullptr;
	int background_width = 0;
	int ship_x = 0;
	int ship_y = 0;
	int last_shot = 0;
	bool fire, up, down, left, right;
	Mix_Music* music = nullptr;
	Mix_Chunk* fx_shoot = nullptr;
	int scroll = 0;
	projectile shots[NUM_SHOTS];
} g; // automatically create an insteance called "g"

	 // ----------------------------------------------------------------
void Start()
{
	SDL_Init(SDL_INIT_EVERYTHING);

	// Create window & renderer
	g.window = SDL_CreateWindow("QSS - Quick Side Scroller - 0.5", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
	g.renderer = SDL_CreateRenderer(g.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	// Load image lib --
	IMG_Init(IMG_INIT_PNG);
	g.background = SDL_CreateTextureFromSurface(g.renderer, IMG_Load("game/background.png"));
	g.ship = SDL_CreateTextureFromSurface(g.renderer, IMG_Load("Game/ship.png"));
	g.shot = SDL_CreateTextureFromSurface(g.renderer, IMG_Load("Game/shot.png"));
	SDL_QueryTexture(g.background, nullptr, nullptr, &g.background_width, nullptr);

	// Create mixer --
	Mix_Init(MIX_INIT_OGG);
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
	g.music = Mix_LoadMUS("Game/soundtrack.wav");
	Mix_PlayMusic(g.music, -1);
	g.fx_shoot = Mix_LoadWAV("Game/laser.wav");

	// Init other vars --
	g.ship_x = 100;
	g.ship_y = SCREEN_HEIGHT / 2;
	g.fire = g.up = g.down = g.left = g.right = false;
}

// ----------------------------------------------------------------
void Finish()
{
	Mix_FreeMusic(g.music);
	Mix_FreeChunk(g.fx_shoot);
	Mix_CloseAudio();
	Mix_Quit();
	SDL_DestroyTexture(g.shot);
	SDL_DestroyTexture(g.ship);
	SDL_DestroyTexture(g.background);
	IMG_Quit();
	SDL_DestroyRenderer(g.renderer);
	SDL_DestroyWindow(g.window);
	SDL_Quit();
}

// ----------------------------------------------------------------
bool CheckInput()
{
	bool ret = true;
	SDL_Event event;

	while (SDL_PollEvent(&event) != 0)
	{
		if (event.type == SDL_KEYUP)
		{
			switch (event.key.keysym.sym)
			{
			case SDLK_UP: g.up = false;	break;
			case SDLK_DOWN:	g.down = false;	break;
			case SDLK_LEFT:	g.left = false;	break;
			case SDLK_RIGHT: g.right = false; break;
			}
		}
		else if (event.type == SDL_KEYDOWN)
		{
			switch (event.key.keysym.sym)
			{
			case SDLK_UP: g.up = true; break;
			case SDLK_DOWN: g.down = true; break;
			case SDLK_LEFT: g.left = true; break;
			case SDLK_RIGHT: g.right = true; break;
			case SDLK_ESCAPE: ret = false; break;
			case SDLK_SPACE: g.fire = (event.key.repeat == 0); break;
			}
		}
		else if (event.type == SDL_QUIT)
			ret = false;
	}

	return ret;
}

// ----------------------------------------------------------------
void MoveStuff()
{
	// Calc new ship position
	if (g.up) g.ship_y -= SHIP_SPEED;
	if (g.down) g.ship_y += SHIP_SPEED;
	if (g.left) g.ship_x -= SHIP_SPEED;
	if (g.right)	g.ship_x += SHIP_SPEED;

	if (g.fire)
	{
		Mix_PlayChannel(-1, g.fx_shoot, 0);
		g.fire = false;

		if (g.last_shot == NUM_SHOTS)
			g.last_shot = 0;

		g.shots[g.last_shot].alive = true;
		g.shots[g.last_shot].x = g.ship_x + 32;
		g.shots[g.last_shot].y = g.ship_y;
		++g.last_shot;
	}

	for (int i = 0; i < NUM_SHOTS; ++i)
	{
		if (g.shots[i].alive)
		{
			if (g.shots[i].x < SCREEN_WIDTH)
				g.shots[i].x += SHOT_SPEED;
			else
				g.shots[i].alive = false;
		}
	}
}

// ----------------------------------------------------------------
void Draw()
{
	SDL_Rect target;

	// Scroll and draw background
	g.scroll += SCROLL_SPEED;
	if (g.scroll >= g.background_width)
		g.scroll = 0;

	target = { -g.scroll, 0, g.background_width, SCREEN_HEIGHT };

	SDL_RenderCopy(g.renderer, g.background, nullptr, &target);
	target.x += g.background_width;
	SDL_RenderCopy(g.renderer, g.background, nullptr, &target);

	// Draw player's ship --
	target = { g.ship_x, g.ship_y, 64, 64 };
	SDL_RenderCopy(g.renderer, g.ship, nullptr, &target);

	// Draw lasers --
	for (int i = 0; i < NUM_SHOTS; ++i)
	{
		if (g.shots[i].alive)
		{
			target = { g.shots[i].x, g.shots[i].y, 64, 64 };
			SDL_RenderCopy(g.renderer, g.shot, nullptr, &target);
		}
	}

	// Finally swap buffers
	SDL_RenderPresent(g.renderer);
}

// ----------------------------------------------------------------
int main(int argc, char* args[])
{
	Start();

	while (CheckInput())
	{
		MoveStuff();
		Draw();
	}

	Finish();

	return(0); // EXIT_SUCCESS
}