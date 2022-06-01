#pragma once
#include <iostream>
#include <fstream>
#include <random>
#include <SDL.h>
#include <array>
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
	int rangehigh;
	int rangelow;
	double animtime = 0;
	double movetime = 0;
	bool erase;
	SDL_Rect rect;
	texp bhead;
	texp head;
	Uint8 alpha = 255;
public:
	Particle() { rangehigh = 0; rangelow = 0; x = 0; y = 0; trajx = 0; trajy = 0; rect = { 0, 0, 0, 0 }; head = NULL; bhead = head; erase = false; }
	Particle(float cx, float cy, int rangel, int rangeh, SDL_Rect crect, texp list) { rangehigh = rangeh; rangelow = rangel; x = cx; y = cy; trajx = 0; trajy = 0; rect = crect; head = list; bhead = head; erase = false; };
	virtual void draw(SDL_Renderer* rend) {
		SDL_SetTextureAlphaMod(head->current, alpha);
		rect.x = x;
		rect.y = y;
		SDL_RenderCopy(rend, head->current, NULL, &rect);
		SDL_SetTextureAlphaMod(head->current, 0);
	}
	virtual void update(double *delta) {
		animtime += *delta;
		movetime += *delta;
		if (movetime > 0.05) {
			x += trajx;
			y += trajy;
			movetime = 0;
		}
		if (animtime > 0.1) {
			head = head->next;
			alpha-=5;
			if (alpha <= 0) {
				erase = true;
				alpha = 0;
			}
			if (head == NULL) {
				head = bhead;
			}
			animtime = 0;
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


class Emitter {
private:
	std::vector<Particle> particles;
	Particle* p;
public:
	Emitter() { p = NULL; timetospawn = 0.1; }
	Emitter(Particle* par, double spawntime) { p = par; timetospawn = spawntime; }
	double ctime = 0;
	double timetospawn;
	virtual void spawnParticle() {
		p->trajx = cos(double(p->rangelow + (std::rand() % (p->rangehigh - p->rangelow + 1))) * M_PI / 180.0);
		p->trajy = sin(double(p->rangelow + (std::rand() % (p->rangehigh - p->rangelow + 1))) * M_PI / 180.0);
		particles.push_back(*p);
	}
	virtual void update(double* delta, SDL_Renderer* rend) {
		ctime += *delta;
		if (ctime > timetospawn) {
			spawnParticle();
			ctime = 0;
		}
		for (int i = 0; i < particles.size();) {
			particles[i].update(delta);
			particles[i].draw(rend);
			if (particles[i].erase) {
				particles.erase(particles.begin() + i);
			}
			else {
				i++;
			}
		}
	}
};
//procedural animation system

class Joint {
private:
	
public:
	float x;
	float y;
	Joint(float ix, float iy) { x = ix; y = iy; }
};

//https://www.alanzucconi.com/2017/04/17/procedural-animations/
class FollowBone {
private:
	std::vector<Joint> children;
	std::vector<std::pair<float, float>> jdist;
	//movement variables
	double maxtime;
	double timer = 0;
	int cj = 0;
	float jlx = 0, jly = 0;
	float maxinc = 0;
	bool activetrans = false;
public:
	float x;
	float y;
	//constructor
	FollowBone(std::vector<Joint> ijoints, double intime, float maxi) { children = ijoints; 
	for (int i = 0; i < children.size(); i++) {
		if (i + 1 < children.size()) {
			std::pair<float, float> p;
			p.first = children[i].x - children[i + 1].x;
			p.second = children[i].y - children[i + 1].y;
			jdist.push_back(p);
		}
		else {
			std::pair<float, float> p = {0, 0};
			jdist.push_back(p);
		}
	}
	x = children[children.size() - 1].x; y = children[children.size() - 1].y; maxtime = intime;
	cj = children.size() - 1; maxinc = maxi;
	}
	//destructor
	~FollowBone() { children.clear(); jdist.clear(); }
	//move joint point
	void transform(float ix, float iy) {
		float lx = ix, ly = iy;
		for (int i = children.size() - 1; i >= 0; i--) {
			float cx = children[i].x;
			float cy = children[i].y;
			children[i].x = lx + jdist[i].first;
			children[i].y = ly + jdist[i].second;
			lx = cx;
			ly = cy;
		}
		x = ix;
		y = iy;
	}
	//Set new point for bone to animate towards
	void transformOverTime(float ix, float iy) {
		jlx = ix;
		jly = iy;
		cj = children.size() - 1;
		activetrans = true;
	}
	//update towards a transform set by transformOverTime
	void updateTransform(float delta) {
		timer += delta;
		if (timer > maxtime && activetrans) {
			float tempx = children[cj].x;
			float tempy = children[cj].y;
			float difx = children[cj].x - jlx;
			float dify = children[cj].y - jly;
			if (difx < 0) {
				children[cj].x += maxinc;
			}
			else if (difx > 0){
				children[cj].x -= maxinc;
			}
			else {
				children[cj].x = jlx;
			}
			if (dify < 0) {
				children[cj].y += maxinc;
			}
			else if(dify > 0){
				children[cj].y -= maxinc;
			}
			else {
				children[cj].y = jly;
			}
			jlx = tempx;
			jly = tempy;
			cj--;
			if (cj < 0) {
				cj = children.size() - 1;
				activetrans = false;
			}
			timer = 0;
		}
	}
	//will draw out joints and connections
	void debugDraw(SDL_Renderer* rend) {
		SDL_SetRenderDrawColor(rend, 100, 255, 50, 0);
		for (auto& i : children) {
			SDL_Rect rect = {i.x, i.y, 5, 5};
			SDL_RenderDrawRect(rend, &rect);
		}
		float lx = children[0].x, ly = children[0].y;
		for (int i = 1; i < children.size(); i++) {
			SDL_RenderDrawLineF(rend, lx, ly, children[i].x, children[i].y);
			lx = children[i].x;
			ly = children[i].y;
		}
	}
};
//Forward Kinematics bone
class FKBone {
private:
	float length;
public:
	bool forw = true;
	float angle;
	FKBone* forward;
	FKBone* backward;
	float x;
	float y;
	FKBone(float ia, float il, float ix, float iy) { angle = ia; length = il; x = ix; y = iy; forward = NULL; backward = NULL; }
	//need to calculate angle for each based on sum of parents angles
	float getEndX() {
		float tangle = angle;
		FKBone* t = backward;
		while (t != NULL) {
			tangle += t->angle;
			t = t->backward;
		}
		return x + std::cosf(tangle) * length;
	}
	float getEndY() {
		float tangle = angle;
		FKBone* t = backward;
		while (t != NULL) {
			tangle += t->angle;
			t = t->backward;
		}
		return y + std::sinf(tangle) * length;
	}

};
//class to easily initilize list of bones
class FKLimb {
public:
	std::vector<FKBone> bones;
	//put bones in order of where you want them to align in limb structure
	FKLimb(std::vector<FKBone> bs) {
		bones = bs;
		for (int i = 0; i < bones.size(); i++) {
			if (i-1 >= 0) {
				bones[i].backward = &bones[i - 1];
			}
			else {
				bones[i].backward = NULL;
			}
			if (i + 1 < bones.size()) {
				bones[i].forward = &bones[i + 1];
			}else{
				bones[i].forward = NULL;
			}

		}
	}
	//
	void update() {
		for (int i = 0; i < bones.size(); i++) {
			if (bones[i].backward != NULL) {
				bones[i].x = bones[i - 1].getEndX();
				bones[i].y = bones[i - 1].getEndY();
			}
		}
	}
	//handle timing yourself
	int animate(int index, float minang, float maxang, float ainc) {
		if (index > bones.size() - 1) {
			return -1;
		}
		//edit targeted angles in a set angle range	
		if (bones[index].forw) {
			bones[index].angle += ainc;
		}
		else {
			bones[index].angle -= ainc;
		}
		if (bones[index].angle <= minang) {
			bones[index].forw = true;
		}
		else if (bones[index].angle >= maxang) {
			bones[index].forw = false;
		}
		return 1;
	}
	void debugDraw(SDL_Renderer* rend) {
		for (int i = 0; i < bones.size(); i++) {
			SDL_SetRenderDrawColor(rend, 255, 100, 150, 0);
			SDL_RenderDrawLineF(rend, bones[i].x, bones[i].y, bones[i].getEndX(), bones[i].getEndY());
			SDL_SetRenderDrawColor(rend, 255, 0, 0, 0);
			SDL_Rect rect = { bones[i].x, bones[i].y, 5, 5 };
			SDL_RenderFillRect(rend, &rect);
		}
	}
};

//https://www.youtube.com/watch?v=sEKNoWyKUA0
//Inverse Kinematics bone
class IKBone {
private:

public:

};



class Bounder {
private:
public:
	float x, y;
	int w, h;
	Bounder() { x = 0; y = 0; w = 50; h = 50; }
	Bounder(float ix, float iy, int iw, int ih) { x = ix; y = iy; w = iw; h = ih; }

	bool contains(float ix, float iy){
		return!(ix < x || iy < y || ix >= (x + w) || iy >= (y + h));
	}
	bool contains(Bounder b) {
		return (b.x >= x && b.x + b.w < x + w && b.y >= y && b.y + b.h < y + h);
	}
};

//convert to using template after done writing
class QuadTree {
private:
	Bounder area_bounds[4];
	QuadTree* children[4] = {NULL, NULL, NULL, NULL};
	//items on this node
	std::vector<Particle> items;
	int w, h;
	float x, y;
public:
	QuadTree(float ix, float iy, int iw, int ih) {
		w = iw; h = ih; x = ix; y = iy;
		area_bounds[0] = Bounder(); area_bounds[1] = Bounder(); area_bounds[2] = Bounder(); area_bounds[3] = Bounder();
	}
};















/*const size_t MAX_DEPTH = 8;



//26:00; https://www.youtube.com/watch?v=ASAowY6yJII
template <typename OBJECT>
class QuadTree {
private:
	//depth
	size_t depth = 0;
	bounder b;
	//child areas
	std::array<bounder, 4> area_child{};
	//potential children pointers
	std::array<std::shared_ptr<QuadTree<OBJECT>>, 4> pot_child{};
	//items that belong to quad
	std::vector<std::pair<bounder, OBJECT>> m_items;
public:
	void clear() {
		m_items.clear();
		for (int i = 0; i < 4; i++) {
			if (pot_child[i]) {
				pot_child[i]->clear();
			}
			pot_child[i].reset();
		}
	}
	size_t size() {
		size_t count = m_items.size();
		for (int i = 0; i < 4; i++) {
			if (pot_child[i]) { count += pot_child[i]->size(); }
		}
		return count;
	}

	void resize(bounder& bin) {
		clear();
		b = bin;
		int childw = b.w / 2;
		int childh = b.h / 2;
		area_child = { bounder(b.posx, b.posy, childw, childh), bounder(b.posx + childw, b.posy, childw, childh), bounder(b.posx, b.posy + childh, childw, childh), bounder(b.posx + childw, b.posy + childh, childw, childh) };
	}
	void insert(OBJECT& item, bounder& be) {
		for (int i = 0; i < 4; i++) {
			if (area_child[i].contains(be)) {
				if (depth + 1 < MAX_DEPTH) {
					
					if (!pot_child[i]) {
						pot_child[i] = std::make_shared<QuadTree<OBJECT>>(area_child[i], depth + 1);
					}

					pot_child[i]->insert(item, be);
					return;
				}
			}
		}
		m_items.push_back({ be, item });
	}

	QuadTree(bounder inb, size_t indepth) {  
		b = inb;
		depth = indepth;
		resize(inb);
	};
};*/
