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
	texp elist = Gore_loadTextureList({ "enemy1.png", "enemy2.png", "enemy3.png", "enemy4.png", "enemy5.png" }, { 50, 50, 50, 50, 50 }, { 100, 100, 100, 100, 100 }, SDL_PIXELFORMAT_RGBA8888, rend, "TexListExample/");
	texp textlist = Gore_loadTextureList({"CK_StarGlowing_A.png", "CK_StarGlowing_B.png","CK_StarGlowing_C.png","CK_StarGlowing_D.png", "CK_StarGlowing_E.png"
		, "CK_StarGlowing_F.png", "CK_StarGlowing_G.png", "CK_StarGlowing_H.png", "CK_StarGlowing_I.png", "CK_StarGlowing_J.png", "CK_StarGlowing_K.png", 
		"CK_StarGlowing_L.png", "CK_StarGlowing_M.png", "CK_StarGlowing_N.png", "CK_StarGlowing_O.png", "CK_StarGlowing_P.png", "CK_StarGlowing_Q.png", "CK_StarGlowing_R.png", 
		"CK_StarGlowing_S.png", "CK_StarGlowing_T.png", "CK_StarGlowing_U.png", "CK_StarGlowing_V.png","CK_StarGlowing_W.png", "CK_StarGlowing_X.png", "CK_StarGlowing_Y.png", 
		"CK_StarGlowing_Z.png" },
		{128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, }
	, { 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, }, SDL_PIXELFORMAT_RGBA8888, rend,
		"TextExample/");
	Alphabet alph;
	Gore_MapTextTextures(textlist, &alph);
	std::cout << Gore_ConvertColorToUint32({ 255, 100, 100 }, surface->format) << std::endl;
	SDL_Surface* pngsurf = Gore_loadPNG("bplayer1.png", SDL_PIXELFORMAT_RGB888, 50, 100);
	SDL_Surface* imgsurf = Gore_LoadBMP("hatemalice.bmp", SDL_PIXELFORMAT_RGBA8888);
	SDL_Texture* etex5 = Gore_findTex(elist, "enemy5.png");
	int x = 0;
	int y = 10;
	Uint32 col = 16737380;
	for (int i = 0; i < 300; i++) {
		x++;
		if (i % 50 == 0) {
			y++;
			x = 0;
		}
		Gore_SetPixelSurface(imgsurf, &y, &x, &col);
	}
	SDL_Texture* tex2 = SDL_CreateTextureFromSurface(rend, pngsurf);
	SDL_Texture* tex1 = SDL_CreateTextureFromSurface(rend, imgsurf);
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
		SDL_Rect erect = { 0, 0, 578, 496 };
		SDL_RenderCopy(rend, tex1, NULL, &erect);
		SDL_Rect prect = { 100, 100, 50, 100 };
		SDL_RenderCopy(rend, tex2, NULL, &prect);
		SDL_Rect enemy1rect = { 300, 300, 50, 100 };
		SDL_RenderCopy(rend, Gore_findTex(elist, "enemy1.png"), NULL, &enemy1rect);
		SDL_Rect enemy5rect = { 250, 400, 50, 100 };
		SDL_RenderCopy(rend, etex5, NULL, &enemy5rect);
		Gore_drawText(rend, &alph, "hello world", 0, 550, 25, 30);
		SDL_DestroyTexture(tex);
		SDL_RenderPresent(rend);
	}
}