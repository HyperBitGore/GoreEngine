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
struct SpriteListMem {
	SDL_Surface* current;
	SpriteListMem* next;
	std::string name;
};
typedef SpriteListMem* spxp;
struct Point {
	int x;
	int y;
};
//4 byte: x, 4 byte: y, 4 byte: color data; repeat through data;
struct PixelTransform {
	char* data;
	size_t size;
	PixelTransform* next;
};
typedef PixelTransform* TrList;


class Particle {
public:
	float x;
	float y;
	float trajx;
	float trajy;
	double animtime = 0;
	SDL_Rect rect;
	texp bhead;
	Uint8 alpha = 255;
public:
	Particle(float cx, float cy, float ctrajx, float ctrajy, SDL_Rect crect, texp list) { x = cx; y = cy; trajx = ctrajx; trajy = ctrajy; rect = crect; head = list; bhead = head; };
	texp head;
	virtual void draw(SDL_Renderer* rend) {
		SDL_SetTextureAlphaMod(head->current, alpha);
		rect.x = x;
		rect.y = y;
		SDL_RenderCopy(rend, head->current, NULL, &rect);
		SDL_SetTextureAlphaMod(head->current, 0);
	}
	virtual void update(double *delta) {
		x += trajx;
		y += trajy;
		animtime += *delta;
		if (animtime > 0.1) {
			head = head->next;
			alpha--;
			if (alpha < 0) {
				alpha = 255;
			}
			if (head == NULL) {
				head = bhead;
			}
			animtime = 0;
		}
	}
};

class Emitter {
private:
	std::vector<Particle> particles;
	Particle* p;
public:
	Emitter(Particle* par, double spawntime) { p = par; timetospawn = spawntime; }
	double ctime = 0;
	double timetospawn;
	virtual void spawnParticle() {
		particles.push_back(*p);
	}
	virtual void update(double *delta, SDL_Renderer* rend) {
		ctime += *delta;
		if (ctime > timetospawn) {
			spawnParticle();
			ctime = 0;
		}
		for (auto& i : particles) {
			i.update(delta);
			i.draw(rend);
		}
	}
};



class Gore {
private:
	Uint64 LAST = 0;
	Uint64 NOW = SDL_GetPerformanceCounter();
public:
	//Texture lists
	void insertTex(TexListMem*& tex, SDL_Texture* current, std::string name);
	SDL_Texture* findTex(texp head, std::string name);
	//Surface lists
	void insertSprite(SpriteListMem*& sp, SDL_Surface* surf, std::string name);
	SDL_Surface* findSprite(spxp sp, std::string name);
	//pixel manipulation
	void SetPixelSurface(SDL_Surface* surf, int* y, int* x, Uint32* pixel);
	void SetPixelSurface(SDL_Surface* surf, int y, int x, Uint32 pixel);

	Uint32 GetPixelSurface(SDL_Surface* surf, int* y, int* x);
	void SetPixelSurfaceColorRGB(SDL_Surface* surf, int* y, int* x, SDL_Color* color);
	void SetPixelSurfaceColorRGBA(SDL_Surface* surf, int* y, int* x, SDL_Color* color);
	Uint32 ConvertColorToUint32RGB(SDL_Color color, SDL_PixelFormat* format);
	Uint32 ConvertColorToUint32RGBA(SDL_Color color, SDL_PixelFormat* format);
	void clearSurface(SDL_Surface* surf);
	//texture manipulation
	void clearTexture(SDL_Texture* tex, int* pitch, int w, int h);
	void SetPixelTexture(SDL_Texture* tex, int* y, int* x, Uint32* pixel, int* pitch);
	Uint32 GetPixelTexture(SDL_Texture* tex, int* y, int* x, int* pitch);
	//image loading
	SDL_Surface* loadPNG(std::string name, SDL_PixelFormatEnum format, int w, int h);
	SDL_Surface* LoadBMP(const char* file, SDL_PixelFormatEnum format);
	texp& loadTextureList(std::vector<std::string> names, std::vector<unsigned int> widths, std::vector<unsigned int> heights, SDL_PixelFormatEnum format, SDL_Renderer* rend, std::string filepath = "NULL");
	spxp& loadSpriteList(std::vector<std::string> names, std::vector<unsigned int> widths, std::vector<unsigned int> heights, SDL_PixelFormatEnum format, std::string filepath = "NULL");
	//Text functions
	void mapTextTextures(int start, texp& out, texp& input);
	void drawText(SDL_Renderer* rend, texp& texthead, std::string text, int x, int y, int w, int h);
	//misc utility
	double getDelta();
	Point raycast2DPixel(SDL_Surface* surf, int sx, int sy, float angle, int step);
	SDL_Surface* createCircle(int w, int h, SDL_Color startcolor);
	SDL_Surface* fillCircle(int w, int h, SDL_Color startcolor);
	SDL_Surface* createBloom(int w, int h, SDL_Color startcolor, float magnitude);
	//memory related
	char* serilizeStruct(char* ptr, int size);
	void deserilizeStruct(char* dest, char* data, int size);
	//point system
	bool* createPoints(SDL_Surface* surf);
	TrList generatePixelTransforms(spxp& spritelist);
	void switchTranformFrames(SDL_Surface* surf, TrList& frames, TrList& begin);
	SDL_Surface* initTransformSurf(spxp& head);
	//Particle system

	//misc
	float trajX(float deg);
	//Takes in degrees return radians
	float trajY(float deg);
};