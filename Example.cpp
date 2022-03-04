#include "GoreEngine.h"
#undef main

struct Entity {
	int x;
	int y;
	Uint32 pixel;
};


int main() {
	srand(time(NULL));
	if (SDL_Init(SDL_INIT_EVERYTHING) > 0) {
		std::cout << "SDL Init failed: " << SDL_GetError << std::endl;
	}
	SDL_Window* window = SDL_CreateWindow("Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 800, SDL_WINDOW_SHOWN);
	SDL_Renderer* rend = SDL_CreateRenderer(window, -1, 0);
	SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, 800, 800, 32, SDL_PIXELFORMAT_RGB888);
	bool exitf = false;
	SDL_Event e;
	Gore_clearSurface(surface);
	Entity player = { 200, 300, 16737380};
	SDL_Color yellow = { 255, 100, 100 };
	std::cout << Gore_ConvertColorToUint32({ 255, 100, 100 }, surface->format) << std::endl;
	while (!exitf) {
		while (SDL_PollEvent(&e)) {
			switch (e.type) {
			case SDL_QUIT:
				exitf = true;
				break;
			}
		}
		SDL_SetRenderDrawColor(rend, 0, 0, 0, 0);
		SDL_RenderClear(rend);
		int prevx, prevy;
		prevx = player.x;
		prevy = player.y;
		for (int i = 0; i < 100; i++) {
			player.x++;
			player.y++;
			Gore_SetPixelSurface(surface, &player.x, &player.y, &player.pixel);
		}
		player.x = prevx;
		player.y = prevy;
		SDL_Texture* tex = SDL_CreateTextureFromSurface(rend, surface);
		SDL_Rect rect = { 0, 0, surface->w, surface->h };
		SDL_RenderCopy(rend, tex, NULL, &rect);
		SDL_DestroyTexture(tex);
		SDL_RenderPresent(rend);
	}
}