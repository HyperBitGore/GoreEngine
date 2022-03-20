#pragma once
#include <iostream>
#include <fstream>
#include <SDL.h>
#include "lodepng.h"


struct TexListMem {
	SDL_Texture* current;
	TexListMem* next;
	std::string name;
};
typedef TexListMem* texp;

struct Alphabet {
	SDL_Texture* texs[26];
	char letters[26];
};

void insertTex(TexListMem*& tex, SDL_Texture* current, std::string name) {
	texp t;
	t = new TexListMem;
	t->current = current;
	t->next = tex;
	t->name = name;
	tex = t;
}

SDL_Texture* findTex(texp head, std::string name) {
	texp temp = head;
	while (temp != NULL) {
		if (std::strcmp(temp->name.c_str(), name.c_str()) == 0) {
			return temp->current;
		}
		temp = temp->next;
	}
	return NULL;
}

void Gore_SetPixelSurface(SDL_Surface* surf, int *y, int *x, Uint32 *pixel) {
	SDL_LockSurface(surf);
	Uint32* pixels = (Uint32*)surf->pixels;
	pixels[(*y * surf->w) + *x] = *pixel;
	SDL_UnlockSurface(surf);
}
Uint32 Gore_GetPixelSurface(SDL_Surface* surf, int* y, int *x) {
	SDL_LockSurface(surf);
	Uint32* pixels = (Uint32*)surf->pixels;
	SDL_UnlockSurface(surf);
	return (Uint32)pixels[(*y * surf->w) + *x];
}
void Gore_SetPixelSurfaceColor(SDL_Surface* surf, int* y, int* x, SDL_Color* color) {
	SDL_LockSurface(surf);
	Uint32* pixels = (Uint32*)surf->pixels;
	Uint32* pixel = pixels + (*y * surf->w) + *x;
	*pixel = SDL_MapRGB(surf->format, color->r, color->g, color->b);
	SDL_UnlockSurface(surf);
}

Uint32 Gore_ConvertColorToUint32(SDL_Color color, SDL_PixelFormat* format) {
	return SDL_MapRGB(format, color.r, color.g, color.b);
}
void Gore_clearSurface(SDL_Surface* surf) {
	SDL_LockSurface(surf);
	memset(surf->pixels, 0, (surf->w * surf->h) * (sizeof(surf->pixels)));
	SDL_UnlockSurface(surf);
}

//Texture has to be made with SDL_TEXTUREACCESS_STREAMING flag
void Gore_SetPixelTexture(SDL_Texture* tex, int *y, int *x, Uint32* pixel, int *pitch) {
	Uint32* pixels;
	SDL_LockTexture(tex, NULL, (void**)&pixels, pitch);
	pixels[(*y * (*pitch)) + *x] = *pixel;
	SDL_UnlockTexture(tex);
}
Uint32 Gore_GetPixelTexture(SDL_Texture* tex, int* y, int* x, int* w, int* pitch) {
	Uint32* pixels;
	SDL_LockTexture(tex, NULL, (void**)&pixels, pitch);
	SDL_UnlockTexture(tex);
	return pixels[(*y * (*w)) + *x];
}
//Decodes everything to a 32bit pixel format, but can convert your format to any type
SDL_Surface* Gore_loadPNG(std::string name, SDL_PixelFormatEnum format, int w, int h) {
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
SDL_Surface* Gore_LoadBMP(const char* file, SDL_PixelFormatEnum format) {
	SDL_Surface* temp = SDL_LoadBMP(file);
	SDL_Surface* remove = SDL_ConvertSurfaceFormat(temp, format, 0);
	SDL_FreeSurface(temp);
	return remove;
}

//Load texture into memory and read the text file with list of points for characters
SDL_Texture* Gore_loadTextAtlas(std::string name) {

}
SDL_Texture* findAlphabet(char c, Alphabet* alph) {
	for (int i = 0; i < 26; i++) {
		if (alph->letters[i] == c) {
			return alph->texs[i];
		}
	}
	return NULL;
}


//Gonna start from last texture added to list, so add your letters in reverse alphabetical order
void MapTextTextures(texp& textures, Alphabet* alph) {
	texp it = textures;
	int i = 0;
	while (it != NULL) {
		alph->texs[i] = it->current;
		i++;
		it = it->next;
	}
}
void drawText(SDL_Renderer* rend, Alphabet* alph, std::string text) {

	//SDL_RenderCopy(rend, );
}
