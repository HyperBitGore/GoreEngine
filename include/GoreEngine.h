#pragma once
#include <iostream>
#include <vector>
#include <fstream>
#include <random>
#include <SDL.h>
#include <map>
#include <ft2build.h>
#include "lodepng.h"
#include "g_primitive_funcs.h"
#include FT_FREETYPE_H
#include FT_COLOR_H

//Didn't want to include windows header to do resource exe inclusion so just follow along with this page if you want to do this. 
//https://stackoverflow.com/questions/22151099/including-data-file-into-c-project
//or this
//https://stackoverflow.com/questions/48326287/is-there-a-cross-platform-way-to-embed-resources-in-a-binary-application-written





namespace Gore {

	template<typename TP>
	struct FreeElement {
		TP el;
		int next;
	};
	template<class T>
	class FreeList {
	private:
		std::vector<FreeElement<T>> items;
		int first_free;
	public:
		FreeList() { first_free = -1; };
		//returns index of insertion point
		int insert(T element) {
			if (first_free != -1) {
				const int index = first_free;
				first_free = items[first_free].next;
				items[index].el = element;
				return index;
			}
			FreeElement<T> f = { element, -1 };
			items.push_back(f);
			return int(items.size() - 1);
		}

		void erase(int n) {
			items[n].next = first_free;
			first_free = n;
		}
		void clear() {
			items.clear();
			first_free = -1;
		}
		T& operator[](int n) {
			return items[n].el;
		}

		int size() {
			return static_cast<int>(items.size());
		}
	};
	struct Point {
		int x;
		int y;
	};
	struct FPoint {
		float x;
		float y;
	};
	struct Insert {
		int key;
		Insert* next;
	};
	struct COLLIDER {
		float x;
		float y;
		int w;
		int h;
		int key;
		//for if inserted outside one cell
		Insert* inserts = nullptr;
		Gore::FPoint inspoint;
	};
	struct castReturn {
		bool hit;
		Gore::FPoint point;
		float length;
	};
	//4 byte: x, 4 byte: y, 4 byte: color data; repeat through data;
	struct PixelTransform {
		char* data;
		size_t size;
		PixelTransform* next;
	};
	typedef PixelTransform* TrList;

	class DeltaTimer {
	private:
		Uint64 LAST = 0;
		Uint64 NOW = SDL_GetPerformanceCounter();
	public:
		double getDelta();
	};
	class Text {
	private:
		struct Glyph {
			SDL_Texture* tex;
			Gore::Point size;
			Gore::Point bearing;
			unsigned int advance;
		};
		struct FontObj {
			FT_Face font;
			std::map<char, Glyph> fontmap;
			int b_size;
			//Gore::ForwardList<SDL_Texture*> texlist;
		};
		SDL_Renderer* rend;
		FT_Library ft = nullptr;
		Gore::ForwardList<FontObj> fonts;
	public:
		Text(SDL_Renderer* re) {
			if (FT_Init_FreeType(&ft)) {
				std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
			}
			rend = re;
		}
		//loads a font into the text object will override any previously loaded font
		void loadFont(std::string file, int size, SDL_Color color) {
			FT_Face font = nullptr;
			if (FT_New_Face(ft, file.c_str(), 0, &font)) {
				std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
				return;
			}
			std::map<char, Glyph> list;
			FT_Set_Pixel_Sizes(font, 0, size);
			//font->charmap->face;
			FT_Select_Charmap(font, FT_ENCODING_UNICODE);
			for (unsigned char i = 0; i < 128; i++) {
				if (i != 32) {
					unsigned long c = FT_Get_Char_Index(font, FT_ULong(i));
					FT_Error er = FT_Load_Glyph(font, c, FT_LOAD_DEFAULT);
					if (!er) {
						er = FT_Render_Glyph(font->glyph, FT_RENDER_MODE_NORMAL);
						if (!er) {
							//https://kevinboone.me/fbtextdemo.html?i=1
							//https://stackoverflow.com/questions/50145739/rendering-text-with-freetype-and-sdl2-produces-white-squares-instead-of-characte
							SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormatFrom(font->glyph->bitmap.buffer, font->glyph->bitmap.width, font->glyph->bitmap.rows, 8, font->glyph->bitmap.pitch, SDL_PIXELFORMAT_INDEX8);
							//have to create a color surface from surf, and then push that into tex
							SDL_Color colors[256];
							for (int j = 0; j < 256; j++)
							{
								colors[j].a = colors[j].r = colors[j].g = colors[j].b = j;
							}
							SDL_SetPaletteColors(surf->format->palette, colors, 0, 256);

							//try switching back to using linked list for pair storage
							SDL_Surface* surf2 = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGBA8888, 0);
							SDL_Texture* tex = SDL_CreateTextureFromSurface(rend, surf2);
							SDL_SetTextureColorMod(tex, color.r, color.g, color.b);
							SDL_FreeSurface(surf);
							SDL_FreeSurface(surf2);
							std::string temp = std::to_string(i);
							Glyph c_glp = { tex, {font->glyph->bitmap.width, font->glyph->bitmap.rows}, {font->glyph->bitmap_left, font->glyph->bitmap_top}, static_cast<unsigned int>(font->glyph->advance.x) };
							list.insert(std::pair<char, Glyph>(char(i), c_glp));
						}
					}
				}
			}
			fonts.insert({ font, list, size}, file);
		}
		//loads a font into the text object will override any previously loaded font
		void loadFont(std::string file, int size, SDL_Color color, std::string name) {
			FT_Face font = nullptr;
			if (FT_New_Face(ft, file.c_str(), 0, &font)) {
				std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
				return;
			}
			std::map<char, Glyph> list;
			FT_Set_Pixel_Sizes(font, 0, size);
			//coloring
			FT_Color col = { color.b, color.g, color.r, color.a };
			//font->charmap->face;
			FT_Select_Charmap(font, FT_ENCODING_UNICODE);
			for (unsigned char i = 0; i < 128; i++) {
				if (i != 32) {
					unsigned long c = FT_Get_Char_Index(font, FT_ULong(i));
					FT_Error er = FT_Load_Glyph(font, c, FT_LOAD_DEFAULT);
					if (!er) {
						er = FT_Render_Glyph(font->glyph, FT_RENDER_MODE_NORMAL);
						if (!er) {
							//https://kevinboone.me/fbtextdemo.html?i=1
							//https://stackoverflow.com/questions/50145739/rendering-text-with-freetype-and-sdl2-produces-white-squares-instead-of-characte
							SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormatFrom(font->glyph->bitmap.buffer, font->glyph->bitmap.width, font->glyph->bitmap.rows, 8, font->glyph->bitmap.pitch, SDL_PIXELFORMAT_INDEX8);
							//have to create a color surface from surf, and then push that into tex
							SDL_Color colors[256];
							for (int j = 0; j < 256; j++)
							{
								colors[j].a = colors[j].r = colors[j].g = colors[j].b = j;
							}
							SDL_SetPaletteColors(surf->format->palette, colors, 0, 256);
							//try switching back to using linked list for pair storage
							SDL_Surface* surf2 = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGBA8888, 0);
							SDL_Texture* tex = SDL_CreateTextureFromSurface(rend, surf2);
							SDL_SetTextureColorMod(tex, color.r, color.g, color.b);
							SDL_FreeSurface(surf);
							SDL_FreeSurface(surf2);
							std::string temp = std::to_string(i);
							Glyph c_glp = { tex, {font->glyph->bitmap.width, font->glyph->bitmap.rows}, {font->glyph->bitmap_left, font->glyph->bitmap_top}, static_cast<unsigned int>(font->glyph->advance.x) };
							list.insert(std::pair<char, Glyph>(char(i), c_glp));
						}
					}
				}
			}
			fonts.insert({ font, list, size }, name);
		}
		void drawText(std::string text, std::string fontfile, int x, int y, int fontsize) {
			FontObj* font = fonts.search(fontfile);
			if (font == nullptr) {
				std::cerr << "Failed to find font" << std::endl;
				return;
			}
			//normalize fontsize
			float n_f = (float(fontsize) / float(font->b_size));
			for (auto& i : text) {
				Glyph* gl = &font->fontmap[i];
				int xpos = x + gl->bearing.x;
				int ypos = y - gl->bearing.y;
				float w = gl->size.x * n_f;
				float h = gl->size.y * n_f;
				//float h = fontsize;
				SDL_Texture* tex = gl->tex;
				SDL_FRect rect = { xpos, ypos, w, h };
				SDL_RenderCopyF(rend, tex, NULL, &rect);
				(i > 32) ? x += (gl->advance >> 6) * n_f : x += fontsize;
			}
		}

	};

	class Engine {
	public:
		//pixel manipulation
		static void SetPixelSurface(SDL_Surface* surf, int* y, int* x, Uint32* pixel);
		static void SetPixelSurface(SDL_Surface* surf, int y, int x, Uint32 pixel);
		static void massSetPixelSurface(SDL_Surface* surf, std::vector<Point> points, Uint32 Pixel);
		static void massSetPixelSurface(SDL_Surface* surf, Point tp, Point bt, Uint32 Pixel);
		static bool massGetPixelSurface(SDL_Surface* surf, Point tp, Point bt, Uint32 pixel);
		static bool massGetPixelSurface(SDL_Surface* surf, std::vector<Point> points, Uint32 pixel);

		static Uint32 GetPixelSurface(SDL_Surface* surf, int* y, int* x);
		static void SetPixelSurfaceColorRGB(SDL_Surface* surf, int* y, int* x, SDL_Color* color);
		static void SetPixelSurfaceColorRGBA(SDL_Surface* surf, int* y, int* x, SDL_Color* color);
		static Uint32 ConvertColorToUint32RGB(SDL_Color color, SDL_PixelFormat* format);
		static Uint32 ConvertColorToUint32RGBA(SDL_Color color, SDL_PixelFormat* format);
		static void clearSurface(SDL_Surface* surf);
		//texture manipulation
		static void clearTexture(SDL_Texture* tex, int* pitch, int w, int h);
		static void SetPixelTexture(SDL_Texture* tex, int* y, int* x, Uint32* pixel, int* pitch);
		static Uint32 GetPixelTexture(SDL_Texture* tex, int* y, int* x, int* pitch);
		static void MassTextureSet(SDL_Texture* tex, int sy, int sx, int endx, int endy, Uint32* pixel, int* pitch);
		static void MassTextureSet(SDL_Texture* tex, std::vector<Point> points, std::vector<Uint32>colors, int* pitch);
		static bool MassTextureCheck(SDL_Texture* tex, int sy, int sx, int endx, int endy, Uint32* pixel, int* pitch);
		static bool MassTextureCheck(SDL_Texture* tex, std::vector<Point> points, Uint32* pixel, int* pitch);

		//image loading
		static SDL_Surface* loadPNG(std::string name, SDL_PixelFormatEnum format, int w, int h);
		static SDL_Surface* LoadBMP(const char* file, SDL_PixelFormatEnum format);
		static Gore::ForwardList<SDL_Texture*>& loadTextureList(std::vector<std::string> names, std::vector<unsigned int> widths, std::vector<unsigned int> heights, SDL_PixelFormatEnum format, SDL_Renderer* rend, std::string filepath = "NULL");
		static  Gore::ForwardList<SDL_Surface*>& loadSpriteList(std::vector<std::string> names, std::vector<unsigned int> widths, std::vector<unsigned int> heights, SDL_PixelFormatEnum format, std::string filepath = "NULL");
		//Text functions
		static void mapTextTextures(int start, Gore::ForwardList<SDL_Texture*>& out, Gore::ForwardList<SDL_Texture*>& input);
		static void drawText(SDL_Renderer* rend, Gore::ForwardList<SDL_Texture*>& texthead, std::string text, int x, int y, int w, int h);
		//misc utility
		static Point raycast2DPixel(SDL_Surface* surf, int sx, int sy, float angle, int step);
		static SDL_Surface* createCircle(int w, int h, SDL_Color startcolor);
		static SDL_Surface* fillCircle(int w, int h, SDL_Color startcolor);
		static SDL_Surface* createBloom(int w, int h, SDL_Color startcolor, float magnitude);
		//memory related
		static char* serilizeStruct(char* ptr, int size);
		static void deserilizeStruct(char* dest, char* data, int size);
		//point system
		static bool* createPoints(SDL_Surface* surf);
		static TrList generatePixelTransforms(Gore::ForwardList<SDL_Surface*>& spritelist);
		static void switchTranformFrames(SDL_Surface* surf, TrList& frames, TrList& begin);
		static SDL_Surface* initTransformSurf(Gore::ForwardList<SDL_Surface*>& head);
		//misc
		static float trajX(float deg);
		//Takes in degrees return radians
		static float trajY(float deg);

	};

	//procedural animation system
	namespace Animate {
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
					if (i - 1 >= 0) {
						bones[i].backward = &bones[i - 1];
					}
					else {
						bones[i].backward = NULL;
					}
					if (i + 1 < bones.size()) {
						bones[i].forward = &bones[i + 1];
					}
					else {
						bones[i].forward = NULL;
					}

				}
			}
			~FKLimb() {
				bones.clear();
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
		//Inverse Kinematics bone
		class IKBone {
		private:
			float angle;
		public:
			float length;
			IKBone* backward = NULL;
			IKBone* forward = NULL;
			float x, y;
			IKBone(float ix, float iy, float il, float ia) { x = ix; y = iy; length = il; angle = ia; }
			float getEndX() {
				return x + std::cosf(angle) * length;
			}
			float getEndY() {
				return y + std::sinf(angle) * length;
			}
			void pointAt(float ix, float iy) {
				float dx = ix - x;
				float dy = iy - y;
				angle = std::atan2f(dy, dx);
			}
			void drag(float ix, float iy) {
				pointAt(ix, iy);
				x = ix - std::cosf(angle) * length;
				y = iy - std::sinf(angle) * length;
				if (backward != NULL) {
					backward->drag(x, y);
				}
			}
		};

		class IKLimb {
		private:
			float x;
			float y;
		public:
			std::vector<IKBone> bones;
			IKLimb(IKBone b, int num) {
				bones.push_back(b);
				x = b.x;
				y = b.y;
				for (int i = 0; i < num; i++) {
					b.x += b.length;
					bones.push_back(b);
				}
				for (int i = 0; i < bones.size(); i++) {
					if (i - 1 >= 0) {
						bones[i].backward = &bones[i - 1];
					}
					else {
						bones[i].backward = NULL;
					}
					if (i + 1 < bones.size()) {
						bones[i].forward = &bones[i + 1];
					}
					else {
						bones[i].forward = NULL;
					}

				}

			}
			IKLimb(std::vector<IKBone> bs) {
				if (bs.size() > 0) {
					x = bs[0].x;
					y = bs[0].y;
					bones = bs;
					for (int i = 0; i < bones.size(); i++) {
						if (i - 1 >= 0) {
							bones[i].backward = &bones[i - 1];
						}
						else {
							bones[i].backward = NULL;
						}
						if (i + 1 < bones.size()) {
							bones[i].forward = &bones[i + 1];
						}
						else {
							bones[i].forward = NULL;
						}

					}
				}
				else {
					x = 0;
					y = 0;
				}

			}
			~IKLimb() {
				bones.clear();
			}
			void drag(float ix, float iy) {
				if (bones.size() > 0) {
					bones[bones.size() - 1].drag(ix, iy);
				}
			}
			void update() {
				for (int i = 0; i < bones.size(); i++) {
					if (bones[i].backward != NULL) {
						bones[i].x = bones[i - 1].getEndX();
						bones[i].y = bones[i - 1].getEndY();
					}
					else {
						bones[i].x = x;
						bones[i].y = y;
					}
				}
			}
			void reach(float ix, float iy) {
				drag(ix, iy);
				update();
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
	}
	namespace Particles {
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
			Gore::ForwardList<SDL_Texture*> head;
			Gore::FObj<SDL_Texture*>* ptr;
			Uint8 alpha = 255;
		public:
			Particle() { rangehigh = 0; rangelow = 0; x = 0; y = 0; trajx = 0; trajy = 0; rect = { 0, 0, 0, 0 }; erase = false; ptr = head.getHead(); }
			Particle(float cx, float cy, int rangel, int rangeh, SDL_Rect crect, Gore::ForwardList<SDL_Texture*> list) { rangehigh = rangeh; rangelow = rangel; x = cx; y = cy; trajx = 0; trajy = 0; rect = crect; head = list; ptr = head.getHead(); erase = false; };
			virtual void draw(SDL_Renderer* rend) {
				SDL_SetTextureAlphaMod(*ptr->current, alpha);
				rect.x = x;
				rect.y = y;
				SDL_RenderCopy(rend, *ptr->current, NULL, &rect);
				SDL_SetTextureAlphaMod(*ptr->current, 0);
			}
			virtual void update(double* delta) {
				animtime += *delta;
				movetime += *delta;
				if (movetime > 0.05) {
					x += trajx;
					y += trajy;
					movetime = 0;
				}
				if (animtime > 0.1) {
					ptr = ptr->next;
					alpha -= 5;
					if (alpha <= 0) {
						erase = true;
						alpha = 0;
					}
					if (ptr == nullptr) {
						ptr = head.getHead();
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
	}
	class TextureDraw {
	private:
		SDL_Texture* tex;
		SDL_Surface* surf;
		SDL_Renderer* rend;
		bool changed = false;
		char* pixels;
	public:
		TextureDraw() {
			tex = nullptr;
			surf = nullptr;
			pixels = nullptr;
			rend = nullptr;
		}
		TextureDraw(int win, int hin, int depth, Uint32 form, SDL_Renderer* ren) {
			surf = SDL_CreateRGBSurfaceWithFormat(0, win, hin, depth, form);
			rend = ren;
			tex = SDL_CreateTexture(rend, form, SDL_TEXTUREACCESS_STREAMING, win, hin);
			SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
			SDL_LockTexture(tex, NULL, (void**)&pixels, &surf->pitch);
			SDL_UnlockTexture(tex);
		}
		~TextureDraw() {
			//SDL_FreeSurface(surf);
			SDL_DestroyTexture(tex);
		}
		//has to be same format as this objects surf
		void appendSurf(SDL_Surface* in, int x, int y) {
			if (in->format->format != surf->format->format) {
				return;
			}
			if (!changed) { SDL_LockTexture(tex, NULL, (void**)&pixels, &surf->pitch); }
			char* dst = pixels;
			char* src = (char*)in->pixels;
			size_t pos = 0;
			dst += (y * surf->pitch);
			pos += y;
			dst += (x * surf->format->BytesPerPixel);
			for (int i = 0; i < in->h; i++) {
				//memcpy data into row
				std::memcpy(dst, src, in->pitch);
				src += in->pitch;
				//added ? here to check if dst will go out of bounds, and if it does just return
				(pos + 1 > surf->h) ? dst = pixels : dst += surf->pitch;
				pos++;
				if (dst == pixels) { changed = true;  return; }
			}
			changed = true;
		}
		void drawTex(SDL_Rect rect) {
			if (changed) {
				SDL_UnlockTexture(tex);
				changed = false;
			}
			SDL_RenderCopy(rend, tex, NULL, &rect);
		}
		void clear() {
			Engine::clearTexture(tex, &surf->pitch, surf->w, surf->h);
		}
	};
	class Bounder {
	private:
	public:
		int x, y;
		int w, h;
		Bounder() { x = 0; y = 0; w = 50; h = 50; }
		Bounder(int ix, int iy, int iw, int ih) { x = ix; y = iy; w = iw; h = ih; }

		bool contains(float ix, float iy) {
			return!(ix < x || iy < y || ix >= (x + w) || iy >= (y + h));
		}
		bool contains(Bounder b) {
			//this may not run faster depending on system, but is faster on my AMD ryzen 3600
			bool ip = false;
			(b.x >= x) ? ip = true : ip = false;
			(b.x + b.w < x + w) ? ip = true : ip = false;
			(b.y >= y) ? ip = true : ip = false;
			(b.y + b.h < y + h) ? ip = true : ip = false;
			return ip;
		}
		bool overlaps(Bounder b) {
			bool ip = false;
			(x < b.x + b.w) ? ip = true : ip = false;
			(x + w >= b.x) ? ip = true : ip = false;
			(y < b.y + b.h) ? ip = true : ip = false;
			(y + h >= b.y) ? ip = true : ip = false;
			return ip;
		}
	};
	namespace SpatialAcceleration {

		template<typename W>
		struct HashObj {
			W* obj;
			int id;
			HashObj* next;
		};
		template<class T>
		class SpatialHashMap {
		private:
			std::vector<HashObj<T>*> buckets;
			int cellsize;
			int grid_width;
		public:
			SpatialHashMap() {
				cellsize = 25;
				grid_width = 10;
				for (int i = 0; i < 10; i++) {
					for (int j = 0; j < 10; j++) {
						buckets.push_back(nullptr);
					}
				}
			}
			SpatialHashMap(float max, float min, int cell_size) {
				//create the buckets
				cellsize = cell_size;
				grid_width = (max - min)/cellsize;
				for (int i = 0; i < grid_width; i++) {
					for (int j = 0; j < grid_width; j++) {
						buckets.push_back(nullptr);
					}
				}
			}
			int hash(FPoint pt) {
				return int((std::floor(pt.x / float(cellsize))) + (std::floor(pt.y / float(cellsize))) * grid_width);
			}
			//returns bucket id, will not insert if outside of range
			int insert(T* obj, FPoint pt) {
				int p = hash(pt);
				if (p >= buckets.size()) {
					return -1;
				}
				HashObj<T>* tp = new HashObj<T>;
				tp->id = p;
				tp->next = buckets[p];
				tp->obj = obj;
				buckets[p] = tp;
				return p;
			}
			//returns true on success, returns false on out of bounds or failure to locate
			bool remove(FPoint pt, T* obj) {
				int p = hash(pt);
				if (p >= buckets.size()) {
					return false;
				}
				HashObj<T>* ptr1 = buckets[p];
				HashObj<T>* prev = nullptr;
				while (ptr1 != nullptr) {
					if (ptr1->obj == obj) {
						//delete current ptr and set it equal to last object
						if (prev != nullptr) {
							prev->next = ptr1->next;
						}
						else {
							buckets[p] = ptr1->next;
						}
						delete ptr1;
						return true;
					}
					prev = ptr1;
					ptr1 = ptr1->next;
				}
				return false;
			}
			//removes from certain bucket
			bool remove(int key, T* obj) {
				int p = key;
				if (p >= buckets.size()) {
					return false;
				}
				HashObj<T>* ptr1 = buckets[p];
				HashObj<T>* prev = nullptr;
				while (ptr1 != nullptr) {
					if (ptr1->obj == obj) {
						//delete current ptr and set it equal to last object
						if (prev != nullptr) {
							prev->next = ptr1->next;
						}
						else {
							buckets[p] = ptr1->next;
						}
						delete ptr1;
						return true;
					}
					prev = ptr1;
					ptr1 = ptr1->next;
				}
				return false;
			}

			//finds bucket point is located in
			HashObj<T>* find(FPoint pt) {
				int p = hash(pt);
				if (p < buckets.size()) {
					return buckets[p];
				}
				else { return nullptr; }
			}
			~SpatialHashMap() {
				//destroy the chain data and then clear buckets
				for (auto& i : buckets) {
					HashObj<T>* ptr = i;
					HashObj<T>* prev = nullptr;
					while (ptr != nullptr) {
						prev = ptr;
						ptr = ptr->next;
						delete prev;
					}
				}
				buckets.clear();
			}
		};

		//quad tree rewrite 2
		template<typename T>
		struct QuadElt {
			T obj;
			Gore::Bounder b;
		};

		struct QuadEltNode {
			int next;
			int prev;
			int index;
			bool move;
		};

		struct QuadNode {
			//creates four children at once, so this is index of first child
			int32_t child;

			//count of elements in node
			int32_t count;

			//elt index
			int eltn_index;

			//point where node is located
			Gore::Point pt;
		};

		template<class T>
		class QuadTree {
		private:
			FreeList<QuadElt<T>> elements;
			FreeList<QuadEltNode> elt_nodes;
			std::vector<QuadNode> nodes;
			Gore::Bounder bounds;
			int m_depth;

		public:
			QuadTree(Gore::Bounder b, int inm) {
				bounds = b;
				//create first node, which represents root node
				QuadNode nd = { -1, 0, -1, {b.x, b.y} };
				nodes.push_back(nd);
				m_depth = inm;
			}
			//finds whatever node bounder will fit into
			int findNode(Bounder b, int depth, int c_node) {
				Bounder b1(nodes[c_node].pt.x, nodes[c_node].pt.y, bounds.w >> depth, bounds.h >> depth);
				if (!b1.contains(b) || depth + 1 >= m_depth) {
					return -1;
				}
				int out = -1;
				//search through four potential child nodes
				Bounder bt1(nodes[c_node].pt.x, nodes[c_node].pt.y, b1.w >> 1, b1.h >> 1);
				depth++;
				for (int i = 0; i < 4; i++) {
					switch (i) {
					case 1:
						bt1 = Gore::Bounder(nodes[c_node].pt.x + (b1.w >> 1), nodes[c_node].pt.y, b1.w >> 1, b1.h >> 1);
						break;
					case 2:
						bt1 = Gore::Bounder(nodes[c_node].pt.x, nodes[c_node].pt.y + (b1.h >> 1), b1.w >> 1, b1.h >> 1);
						break;
					case 3:
						bt1 = Gore::Bounder(nodes[c_node].pt.x + (b1.w >> 1), nodes[c_node].pt.y + (b1.h >> 1), b1.w >> 1, b1.h >> 1);
						break;
					}
					if (bt1.contains(b)) {
						if (nodes[c_node].child != -1) {
							out = nodes[c_node].child + i;
							int tem = findNode(b, depth, nodes[c_node].child + i);
							if (tem != -1) {
								out = tem;
							}
						}
						else {
							//generate nodes
							for (int j = 0; j < 4; j++) {
								QuadNode np;
								np.pt = { bt1.x, bt1.y };
								np.child = -1;
								np.count = 0;
								np.eltn_index = -1;
								switch (j) {
								case 0:
									np.pt.x = bt1.x;
									np.pt.y = bt1.y;
									break;
								case 1:
									np.pt.x = (bt1.w > 0) ? bt1.x + (bt1.w >> 1) : 0;
									np.pt.y = bt1.y;
									break;
								case 2:
									np.pt.x = bt1.x;
									np.pt.y = (bt1.y > 0) ? bt1.y + (bt1.h >> 1) : 0;
									break;
								case 3:
									np.pt.x = (bt1.w > 0) ? bt1.x + (bt1.w >> 1) : 0;
									np.pt.y = (bt1.y > 0) ? bt1.y + (bt1.h >> 1) : 0;
									break;
								}
								nodes.push_back(np);
							}
							nodes[c_node].child = nodes.size() - 4;
							out = nodes[c_node].child + i;
							int tem = findNode(b, depth, nodes[c_node].child + i);
							if (tem != -1) {
								out = tem;
							}
						}
						i = 4;
					}
				}
				return out;

			}
			//inserts element into tree, returns elt node index
			int insert(T in, Bounder bin) {
				QuadElt<T> obj = { in, bin };
				//now we find the node this will go into
				int node = findNode(bin, 0, 0);
				//insert into node
				if (node != -1) {
					//add into returned node
					QuadEltNode np;
					np.index = elements.insert(obj);
					np.next = nodes[node].eltn_index;
					np.prev = -1;
					np.move = false;
					int ttem = elt_nodes.insert(np);
					(nodes[node].eltn_index != -1) ? elt_nodes[nodes[node].eltn_index].prev = ttem : np.prev = -1;
					nodes[node].eltn_index = ttem;
					nodes[node].count++;
					return nodes[node].eltn_index;
				}
				//add into root node
				QuadEltNode np;
				np.index = elements.insert(obj);
				np.next = nodes[0].eltn_index;
				np.prev = -1;
				np.move = false;
				int ttem = elt_nodes.insert(np);
				(nodes[0].eltn_index != -1) ? elt_nodes[nodes[0].eltn_index].prev = ttem : np.prev = -1;
				nodes[0].eltn_index = ttem;
				nodes[0].count++;
				return nodes[node].eltn_index;
			}
			//erases element as well as node
			void erase(int eltn_index, int node) {
				QuadEltNode* cur_eltn = &elt_nodes[eltn_index];
				(cur_eltn->prev != -1) ? elt_nodes[cur_eltn->prev].next = cur_eltn->next : nodes[node].eltn_index = cur_eltn->next;
				(cur_eltn->next != -1) ? elt_nodes[cur_eltn->next].prev = cur_eltn->prev : cur_eltn->index;
				elements.erase(cur_eltn->index);
				elt_nodes.erase(eltn_index);
				nodes[node].count--;
			}
			//completely removes elt node from tree
			void remove(int eltn_index, int node) {
				QuadEltNode* cur_eltn = &elt_nodes[eltn_index];
				(cur_eltn->prev != -1) ? elt_nodes[cur_eltn->prev].next = cur_eltn->next : nodes[node].eltn_index = cur_eltn->next;
				(cur_eltn->next != -1) ? elt_nodes[cur_eltn->next].prev = cur_eltn->prev : cur_eltn->index;
				elt_nodes.erase(eltn_index);
				nodes[node].count--;
			}
			//moves an element if it doesn't fit into current node
			int move(int eltn_index, int nod, Bounder new_b) {
				QuadEltNode cur_eltn = elt_nodes[eltn_index];
				int elt_in = cur_eltn.index;
				remove(eltn_index, nod);
				int node = findNode(new_b, 0, 0);
				if (node != -1) {
					QuadEltNode np;
					np.index = elt_in;
					np.next = nodes[node].eltn_index;
					np.prev = -1;
					np.move = false;
					int ttem = elt_nodes.insert(np);
					(nodes[node].eltn_index != -1) ? elt_nodes[nodes[node].eltn_index].prev = ttem : np.prev = -1;
					nodes[node].eltn_index = ttem;
					nodes[node].count++;
					return ttem;
				}
				QuadEltNode np;
				np.index = elt_in;
				np.next = nodes[0].eltn_index;
				np.prev = -1;
				np.move = false;
				int ttem = elt_nodes.insert(np);
				(nodes[0].eltn_index != -1) ? elt_nodes[nodes[0].eltn_index].prev = ttem : np.prev = -1;
				nodes[0].eltn_index = ttem;
				nodes[0].count++;
				return ttem;
			}
			//moves all element nodes that need to be moved
			void move_call() {

			}
			size_t getSize() {
				return elements.size();
			}
			std::vector<QuadNode>& getNodes() {
				return nodes;
			}
			FreeList<QuadEltNode>& getEltNodes() {
				return elt_nodes;
			}
			FreeList<QuadElt<T>>& getElements() {
				return elements;
			}
		};

	}
	class CollisionMap {
	private:
		SpatialAcceleration::SpatialHashMap<COLLIDER> map;
	public:
		CollisionMap(float width, int cell_width) {
			map.~SpatialHashMap();
			map = Gore::SpatialAcceleration::SpatialHashMap<COLLIDER>(width, 0.0f, cell_width);
		}
		bool isColliding(COLLIDER e1, COLLIDER e2) {
			return (e1.x < e2.x + e2.w && e1.x + e1.w > e2.x && e1.y < e2.y + e2.h && e1.y + e1.h > e2.y);
		}

		Gore::SpatialAcceleration::HashObj<COLLIDER>* find(Gore::FPoint point) {
			return map.find({ point.x, point.y });
		}
		void insert(COLLIDER* e) {
			if (map.hash({ e->x, e->y }) == map.hash({ e->x + e->w, e->y + e->h })) {
				e->key = map.insert(e, { e->x, e->y });
				e->inspoint = { e->x, e->y };
			}
			else {
				multiInsert(e);
			}
		}
		void multiInsert(COLLIDER* e) {
			//check with both
			for (int y = e->y + e->h; y > e->y; y--) {
				for (int x = e->x + e->w; x > e->x; x--) {
					int hash = map.hash({ float(x), float(y) });
					Insert* ptr = e->inserts;
					bool skip = false;
					while (ptr != nullptr) {
						if (ptr->key == hash) {
							skip = true;
							ptr = nullptr;
						}
						else {
							ptr = ptr->next;
						}
					}
					if (!skip) {
						Insert* in = new Insert;
						in->key = hash;
						in->next = e->inserts;
						e->inserts = in;
						map.insert(e, { float(x), float(y) });
					}
				}
			}
		}
		void multiRemove(COLLIDER* e) {
			Insert* ptr = e->inserts;
			while (ptr != nullptr) {
				Insert* next = ptr->next;
				map.remove(ptr->key, e);
				//now delete current ptr
				delete ptr;
				ptr = next;
			}
			e->inserts = nullptr;
		}
		void remove(COLLIDER* e) {
			if (e->inserts != nullptr) {
				multiRemove(e);
			}
			else {
				map.remove(e->inspoint, e);
			}
		}
		void move(COLLIDER* e) {
			map.remove(e->inspoint, e);
			e->key = map.insert(e, { e->x, e->y });
			e->inspoint = { e->x, e->y };
		}
		SpatialAcceleration::HashObj<COLLIDER>* getBucket(Gore::FPoint p) {
			return map.find(p);
		}
		COLLIDER* checkCollision(COLLIDER e) {
			Gore::SpatialAcceleration::HashObj<COLLIDER>* ptr = map.find({ e.x, e.y });
			while (ptr != nullptr) {
				if (isColliding(e, *ptr->obj)) {
					return ptr->obj;
				}
				ptr = ptr->next;
			}
			return nullptr;
		}
		//for making sure it doesn't collide with itself
		COLLIDER* checkCollisionSelf(COLLIDER* e) {
			Gore::SpatialAcceleration::HashObj<COLLIDER>* ptr = map.find({ e->x, e->y });
			while (ptr != nullptr) {
				if (isColliding(*e, *ptr->obj) && ptr->obj != e) {
					return ptr->obj;
				}
				ptr = ptr->next;
			}
			return nullptr;
		}
		//will raycast till it hits end point, collides with something, or hits max length
		castReturn raycast(Gore::FPoint start, Gore::FPoint target, float maxlength) {
			bool hit = false;
			Gore::FPoint cur = start;
			float len = 0;
			float angle = atan2f(target.y - start.y, target.x - start.x);
			float stepx = cosf(angle);
			float stepy = sinf(angle);
			do {
				Gore::SpatialAcceleration::HashObj<COLLIDER>* ptr = map.find(cur);
				while (ptr != nullptr) {
					if (isColliding({ cur.x, cur.y, 1, 1 }, *ptr->obj)) {
						return { false, cur, len};
					}
					ptr = ptr->next;
				}
				if (isColliding({ cur.x, cur.y, 1, 1 }, { target.x, target.y, 5, 5 })) {
					hit = true;
				}
				else {
					cur.x += stepx;
					cur.y += stepy;
					len += angle;
				}
			} while (!hit);
			return { true, cur, len };
		}
		//pass an angle in radians
		castReturn raycast(Gore::FPoint start, float angle, float maxlength) {
			bool hit = false;
			Gore::FPoint cur = start;
			float len = 0;
			float stepx = cosf(angle);
			float stepy = sinf(angle);
			do {
				Gore::SpatialAcceleration::HashObj<COLLIDER>* ptr = map.find(cur);
				while (ptr != nullptr) {
					if (isColliding({ cur.x, cur.y, 1, 1 }, *ptr->obj)) {
						return { false, cur, len };
					}
					ptr = ptr->next;
				}
				if (len >= maxlength) {
					hit = true;
				}
				else {
					cur.x += stepx;
					cur.y += stepy;
					len += angle;
				}
			} while (!hit);
			return { true, cur, len };
		}

	};


}
