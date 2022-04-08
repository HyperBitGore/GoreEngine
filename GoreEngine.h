#pragma once
#include <iostream>
#include <fstream>
#include <SDL.h>
#include "lodepng.h"

//Didn't want to include windows header to do resource exe inclusion so just follow along with this page if you want to do this. 
//https://stackoverflow.com/questions/22151099/including-data-file-into-c-project
//or this
//https://stackoverflow.com/questions/48326287/is-there-a-cross-platform-way-to-embed-resources-in-a-binary-application-written

struct TexListMem {
	SDL_Texture* current;
	TexListMem* next;
	std::string name;
};
typedef TexListMem* texp;

class Gore {
public:
	void insertTex(TexListMem*& tex, SDL_Texture* current, std::string name);
	SDL_Texture* findTex(texp head, std::string name);
	void SetPixelSurface(SDL_Surface* surf, int* y, int* x, Uint32* pixel);
	Uint32 GetPixelSurface(SDL_Surface* surf, int* y, int* x);
	void SetPixelSurfaceColor(SDL_Surface* surf, int* y, int* x, SDL_Color* color);
	Uint32 ConvertColorToUint32(SDL_Color color, SDL_PixelFormat* format);
	void clearSurface(SDL_Surface* surf);
	void SetPixelTexture(SDL_Texture* tex, int* y, int* x, Uint32* pixel, int* pitch);
	Uint32 GetPixelTexture(SDL_Texture* tex, int* y, int* x, int* w, int* pitch);
	SDL_Surface* loadPNG(std::string name, SDL_PixelFormatEnum format, int w, int h);
	SDL_Surface* LoadBMP(const char* file, SDL_PixelFormatEnum format);
	texp& loadTextureList(std::vector<std::string> names, std::vector<unsigned int> widths, std::vector<unsigned int> heights, SDL_PixelFormatEnum format, SDL_Renderer* rend, std::string filepath = "NULL");
	void mapTextTextures(int start, texp& out, texp& input);
	void drawText(SDL_Renderer* rend, texp& texthead, std::string text, int x, int y, int w, int h);
};