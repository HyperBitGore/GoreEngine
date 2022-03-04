#pragma once
#include <iostream>
#include <fstream>
#include <SDL.h>



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
//Load texture into memory and read the text file with list of points for characters
void Gore_loadTextAtlas(SDL_Texture* tex, std::string name) {
	
}
//Have to know what your files format is, only pngs parseable
//https://github.com/randy408/libspng/blob/master/spng/spng.c
//https://stackoverflow.com/questions/31079947/how-can-i-manually-read-png-files-in-c
void Gore_loadIMG(SDL_Surface* surf, std::string name, SDL_PixelFormatEnum format, int w, int h, int depth) {
	//Write image loading here
	std::ifstream m_file;
	m_file.open(name);


	//surf = SDL_CreateRGBSurfaceWithFormatFrom(pixel_data, w, h, depth, pitch, format);
}