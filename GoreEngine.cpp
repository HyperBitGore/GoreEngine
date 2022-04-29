#include "GoreEngine.h"

void Gore::insertTex(TexListMem*& tex, SDL_Texture* current, std::string name) {
	texp t;
	t = new TexListMem;
	t->current = current;
	t->next = tex;
	t->name = name;
	tex = t;
}

SDL_Texture* Gore::findTex(texp head, std::string name) {
	texp temp = head;
	while (temp != NULL) {
		if (std::strcmp(temp->name.c_str(), name.c_str()) == 0) {
			return temp->current;
		}
		temp = temp->next;
	}
	return NULL;
}

void Gore::insertSprite(spxp& sp, SDL_Surface* surf, std::string name) {
	spxp t;
	t = new SpriteListMem;
	t->current = surf;
	t->next = sp;
	t->name = name;
	sp = t;
}
SDL_Surface* Gore::findSprite(spxp sp, std::string name) {
	spxp temp = sp;
	while (temp != NULL) {
		if (temp->name.compare(name) == 0) {
			return temp->current;
		}
		temp = temp->next;
	}
	return NULL;
}


//Pixel manipulation
void Gore::SetPixelSurface(SDL_Surface* surf, int* y, int* x, Uint32* pixel) {
	SDL_LockSurface(surf);
	Uint32* pixels = (Uint32*)surf->pixels;
	pixels[(*y * surf->w) + *x] = *pixel;
	SDL_UnlockSurface(surf);
}
Uint32 Gore::GetPixelSurface(SDL_Surface* surf, int* y, int* x) {
	//SDL_LockSurface(surf);
	Uint32* pixels = (Uint32*)surf->pixels;
	//SDL_UnlockSurface(surf);
	return pixels[*y * surf->w + *x];
}
void Gore::SetPixelSurfaceColor(SDL_Surface* surf, int* y, int* x, SDL_Color* color) {
	SDL_LockSurface(surf);
	Uint32* pixels = (Uint32*)surf->pixels;
	Uint32* pixel = pixels + (*y * surf->w) + *x;
	*pixel = SDL_MapRGB(surf->format, color->r, color->g, color->b);
	SDL_UnlockSurface(surf);
}

Uint32 Gore::ConvertColorToUint32(SDL_Color color, SDL_PixelFormat* format) {
	return SDL_MapRGB(format, color.r, color.g, color.b);
}
void Gore::clearSurface(SDL_Surface* surf) {
	SDL_LockSurface(surf);
	memset(surf->pixels, 0, (surf->w * surf->h) * surf->format->BytesPerPixel);
	SDL_UnlockSurface(surf);
}
void Gore::clearTexture(SDL_Texture* tex, int* pitch, int w, int h) {
	Uint32* pixels;
	SDL_LockTexture(tex, NULL, (void**)&pixels, pitch);
	memset(pixels, 0, (w * h) * (sizeof(pixels)));
	SDL_UnlockTexture(tex);
}
//Texture has to be made with SDL_TEXTUREACCESS_STREAMING flag
//Have to divide the pitch by sizeof(unsigned int) to get proper x and y 
void Gore::SetPixelTexture(SDL_Texture* tex, int* y, int* x, Uint32* pixel, int* pitch) {
	Uint32* pixels;
	SDL_LockTexture(tex, NULL, (void**)&pixels, pitch);
	pixels[*y * ((*pitch) / sizeof(unsigned int)) + *x] = *pixel;
	SDL_UnlockTexture(tex);
}
//Faster if you leave this locked, 
Uint32 Gore::GetPixelTexture(SDL_Texture* tex, int* y, int* x, int* pitch) {
	Uint32* pixels;
	SDL_LockTexture(tex, NULL, (void**)&pixels, pitch);
	return pixels[*y * ((*pitch) / sizeof(unsigned int)) + *x];
	SDL_UnlockTexture(tex);
}
//Decodes everything to a 32bit pixel format, but can convert your format to any type
SDL_Surface* Gore::loadPNG(std::string name, SDL_PixelFormatEnum format, int w, int h) {
	unsigned int wr = w;
	unsigned int hr = h;
	unsigned char* data;
	unsigned error = lodepng_decode32_file(&data, &wr, &hr, name.c_str());
	if (error) {
		std::cout << lodepng_error_text(error) << std::endl;
	}
	//Uses x86 little endians for masks
	SDL_Surface* surf = SDL_CreateRGBSurfaceFrom(data, w, h, 32, 4 * w, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
	SDL_Surface* ret = SDL_ConvertSurfaceFormat(surf, format, 0);
	if (surf == NULL) {
		std::cout << SDL_GetError() << std::endl;
	}
	SDL_FreeSurface(surf);
	free(data);
	return ret;
}
//Setpixel won't work without formats without 32bits of color data to use, you can just edit the setpixel yourself if you want to workout outside of RGBA8888
SDL_Surface* Gore::LoadBMP(const char* file, SDL_PixelFormatEnum format) {
	SDL_Surface* temp = SDL_LoadBMP(file);
	SDL_Surface* remove = SDL_ConvertSurfaceFormat(temp, format, 0);
	SDL_FreeSurface(temp);
	return remove;
}

//Loads textures into memory as a linked list. Keep width/heights even with names or you'll get an error
texp& Gore::loadTextureList(std::vector<std::string> names, std::vector<unsigned int> widths, std::vector<unsigned int> heights, SDL_PixelFormatEnum format, SDL_Renderer* rend, std::string filepath) {
	texp head = NULL;
	int j = 0;
	for (auto& i : names) {
		std::string t = i;
		if (filepath != "NULL") {
			t = filepath + i;
		}
		SDL_Surface* surf = loadPNG(t, format, widths[j], heights[j]);
		SDL_Texture* tex = SDL_CreateTextureFromSurface(rend, surf);
		insertTex(head, tex, i);
		SDL_FreeSurface(surf);
		j++;
	}
	return head;
}
spxp& Gore::loadSpriteList(std::vector<std::string> names, std::vector<unsigned int> widths, std::vector<unsigned int> heights, SDL_PixelFormatEnum format, SDL_Renderer* rend, std::string filepath) {
	spxp head = NULL;
	int j = 0;
	for (auto& i : names) {
		std::string t = i;
		if (filepath != "NULL") {
			t = filepath + i;
		}
		SDL_Surface* surf = loadPNG(t, format, widths[j], heights[j]);
		insertSprite(head, surf, i);
		j++;
	}
	return head;
}


//Text functions
//Input starting integer number for character then will loop through till it hits end of input whole time adding to out
void Gore::mapTextTextures(int start, texp& out, texp& input) {
	texp t = input;
	while (t != NULL) {
		std::string temp;
		temp.push_back(start);
		insertTex(out, t->current, temp);
		start++;
		t = t->next;
	}
}

//Width and height for individual letters
void Gore::drawText(SDL_Renderer* rend, texp& texthead, std::string text, int x, int y, int w, int h) {
	int sx = x;
	int sy = y;
	for (auto& i : text) {
		if (i == ' ') {
			sx += w + 1;
		}
		else {
			SDL_Rect rect = { sx, sy, w, h };
			std::string t;
			t.push_back(i);
			SDL_Texture* temp = findTex(texthead, t);
			if (temp != NULL) {
				SDL_RenderCopy(rend, temp, NULL, &rect);
				sx += w + 1;
			}
		}
	}
}


//Misc
double Gore::getDelta() {
	double delta = 0;
	LAST = NOW;
	NOW = SDL_GetPerformanceCounter();
	delta = (double)((NOW - LAST) * 1000 / (double)SDL_GetPerformanceFrequency());
	delta = delta * 0.001;
	return delta;
}
Point Gore::raycast2DPixel(SDL_Surface* surf, int sx, int sy, float angle, int step) {
	//get the trajectory to step for each axis
	float stepx = cosf(angle * M_PI / 180);
	float stepy = sinf(angle * M_PI / 180);
	//init needed variables
	Uint32 col = GetPixelSurface(surf, &sy, &sx);
	SDL_Color set = { 100, 250, 80, 0 };
	float cx = sx;
	float cy = sy;
	int lbx = sx;
	int lby = sy;
	//instead of a bunch of getpixel calls we just get pixel data once
	Uint32* pixels = (Uint32*)surf->pixels;
	while (col == 0 && cx > 0 && cx < surf->w && cy > 0 && cy < surf->h) {
		cx += stepx;
		cy += stepy;
		int bx = (int)cx;
		int by = (int)cy;
		//have to check if current int position has already been checked so we don't check it twice
		if (lbx != bx || lby != by) {
			col = pixels[by * surf->w + bx];
		}
		lbx = bx;
		lby = by;
	}
	return { lbx , lby };
}