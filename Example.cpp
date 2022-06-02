#include "GoreEngine.h"
#undef main
//https://stackoverflow.com/questions/2139637/hide-console-of-windows-application
//Use this for no console window
//#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
struct Entity {
	float x;
	float y;
	Uint32 pixel;
};

struct BASE {
	int x;
	int y;
	int w;
	int h;
};

//can't use base particle class anymore because fire overrides it here, at least the draw function
class Fire : public Particle {
public:
	Fire(float cx, float cy, int rangel, int rangeh, SDL_Rect crect, texp list) { rangehigh = rangeh; rangelow = rangel; x = cx; y = cy; trajx = 0; trajy = 0; rect = crect; head = list; bhead = head; erase = false; };
	void draw(SDL_Renderer* rend) {
		SDL_SetTextureColorMod(head->current, 235, 149, 52);
		SDL_SetTextureAlphaMod(head->current, alpha);
		rect.x = x;
		rect.y = y;
		SDL_RenderCopy(rend, head->current, NULL, &rect);
		SDL_SetTextureAlphaMod(head->current, 0);
		SDL_SetTextureColorMod(head->current, 0, 0, 0);
	}
};
class FireEmitter : public Emitter {
private:
	//std::vector<Fire> particles;
	Fire* fep;
	Bounder sc = Bounder(0.0f, 0.0f, 800, 800);
	QuadTree* quad = new QuadTree(sc, 1);
	QuadContainer* qc = new QuadContainer(quad);
public:
	FireEmitter(Fire* par, double spawntime) { fep = par; timetospawn = spawntime;}
	void spawnParticle() {
		//fep->trajx = cos(double(fep->rangelow + (std::rand() % (fep->rangehigh - fep->rangelow + 1))) * M_PI / 180.0);
		//fep->trajy = sin(double(fep->rangelow + (std::rand() % (fep->rangehigh - fep->rangelow + 1))) * M_PI / 180.0);
		fep->trajx = 0;
		fep->trajy = 0;
		qc->insert(*fep, { fep->x, fep->y, 1, 1 });
		//particles.push_back(*fep);
	}
	void update(double* delta, SDL_Renderer* rend) {
		ctime += *delta;
		if (ctime > timetospawn) {
			spawnParticle();
			ctime = 0;
		}
		SDL_SetRenderDrawColor(rend, 255, 100, 155, 0);
		//wont use fireparticle draw until template being used
		std::list<ContainerItem>::iterator it;
		std::list<ContainerItem> contai = qc->searchContainer(sc);
		for (it = contai.begin(); it != contai.end();) {
			it->p.update(delta);
			it->p.draw(rend);
			if (it->p.erase) {
				std::list<ContainerItem>::iterator ct = it;
				std::list<ContainerItem>::iterator out;
				if (std::next(ct) != contai.end()) {
					out = it;
					std::next(out);
				}
				else {
					out = contai.end();
				}
				qc->remove(it);
				it = out;
			}
			else {
				std::next(it);
			}
		}
	}
};
class Water : public Particle {
public:
	Water(float cx, float cy, int rangel, int rangeh, SDL_Rect crect, texp list) { rangehigh = rangeh; rangelow = rangel; x = cx; y = cy; trajx = 0; trajy = 0; rect = crect; head = list; bhead = head; erase = false; };
	void draw(SDL_Renderer* rend) {
		SDL_SetTextureColorMod(head->current, 150, 85, 255);
		SDL_SetTextureAlphaMod(head->current, alpha);
		rect.x = x;
		rect.y = y;
		SDL_RenderCopy(rend, head->current, NULL, &rect);
		SDL_SetTextureAlphaMod(head->current, 0);
		SDL_SetTextureColorMod(head->current, 0, 0, 0);
	}

};
class WaterEmitter : public Emitter {
private:
	std::vector<Water> particles;
	Water* fep;
public:
	WaterEmitter(Water* par, double spawntime) { fep = par; timetospawn = spawntime; }
	void spawnParticle() {
		fep->trajx = cos(double(fep->rangelow + (std::rand() % (fep->rangehigh - fep->rangelow + 1))) * M_PI / 180.0);
		fep->trajy = sin(double(fep->rangelow + (std::rand() % (fep->rangehigh - fep->rangelow + 1))) * M_PI / 180.0);
		particles.push_back(*fep);
	}
	void update(double* delta, SDL_Renderer* rend) {
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
Gore gore;

//https://nintervik.github.io/2D-Particle-System/
//https://glusoft.com/tutorials/sdl2/creating-particle-system
//https://lazyfoo.net/SDL_tutorials/lesson28/index.php
int main() {
	srand(time(NULL));
	if (SDL_Init(SDL_INIT_EVERYTHING) > 0) {
		std::cout << "SDL Init failed: " << SDL_GetError << std::endl;
	}
	SDL_Window* window = SDL_CreateWindow("Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 800, SDL_WINDOW_SHOWN);
	SDL_Renderer* rend = SDL_CreateRenderer(window, -1, 0);
	SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, 800, 800, 32, SDL_PIXELFORMAT_RGBA8888);
	bool exitf = false;
	SDL_Event e;
	gore.clearSurface(surface);
	Entity player = { 200, 300, 16737380 };
	SDL_Color yellow = { 255, 100, 100 };
	texp elist = gore.loadTextureList({ "enemy1.png", "enemy2.png", "enemy3.png", "enemy4.png", "enemy5.png" }, { 50, 50, 50, 50, 50 }, { 100, 100, 100, 100, 100 }, SDL_PIXELFORMAT_RGBA8888, rend, "TexListExample/");
	texp textlist = gore.loadTextureList({ "CK_StarGlowing_Z.png", "CK_StarGlowing_Y.png","CK_StarGlowing_X.png","CK_StarGlowing_W.png", "CK_StarGlowing_V.png"
		, "CK_StarGlowing_U.png", "CK_StarGlowing_T.png", "CK_StarGlowing_S.png", "CK_StarGlowing_R.png", "CK_StarGlowing_Q.png", "CK_StarGlowing_P.png",
		"CK_StarGlowing_O.png", "CK_StarGlowing_N.png", "CK_StarGlowing_M.png", "CK_StarGlowing_L.png", "CK_StarGlowing_K.png", "CK_StarGlowing_J.png", "CK_StarGlowing_I.png",
		"CK_StarGlowing_H.png", "CK_StarGlowing_G.png", "CK_StarGlowing_F.png", "CK_StarGlowing_E.png","CK_StarGlowing_D.png", "CK_StarGlowing_C.png", "CK_StarGlowing_B.png",
		"CK_StarGlowing_A.png" },
		{ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, }
		, { 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, }, SDL_PIXELFORMAT_RGBA8888, rend,
		"TextExample/");
	texp alph;
	gore.mapTextTextures(97, alph, textlist);
	std::cout << gore.ConvertColorToUint32RGBA({ 255, 100, 100 }, surface->format) << std::endl;
	SDL_Surface* pngsurf = gore.loadPNG("bplayer1.png", SDL_PIXELFORMAT_RGB888, 50, 100);
	SDL_Surface* imgsurf = gore.LoadBMP("hatemalice.bmp", SDL_PIXELFORMAT_RGBA8888);
	SDL_Texture* etex5 = gore.findTex(elist, "enemy5.png");
	SDL_Surface* testblum = gore.createBloom(50, 50, { 255, 150, 100, 0 }, 1.6);
	SDL_Texture* texblum = SDL_CreateTextureFromSurface(rend, testblum);
	SDL_SetTextureBlendMode(texblum, SDL_BLENDMODE_BLEND);
	int x = 0;
	int y = 10;
	Uint32 col = 1694126335;
	for (int i = 0; i < 300; i++) {
		x++;
		if (i % 50 == 0) {
			y++;
			x = 0;
		}
		gore.SetPixelSurface(imgsurf, &y, &x, &col);
	}
	SDL_Texture* tex2 = SDL_CreateTextureFromSurface(rend, pngsurf);
	SDL_Texture* tex1 = SDL_CreateTextureFromSurface(rend, imgsurf);
	const Uint8* keys;
	keys = SDL_GetKeyboardState(NULL);
	float ang = 90;
	double delta;
	//memory related stuff
	BASE b = { 124, 0, 60, 120 };
	char* st = (char*)&b;
	gore.serilizeStruct(st, sizeof(BASE));
	BASE a = {0, 0, 0, 0};
	char* des = (char*)&a;
	gore.deserilizeStruct(des, st, sizeof(BASE));
	std::cout << a.x << std::endl;
	//animation/destructio system related
	bool* points = gore.createPoints(imgsurf);
	spxp animlist = gore.loadSpriteList({ "enem1_1.png", "enem1_2.png", "enem1_3.png", "enem1_4.png"}, {30, 30, 30, 30}, {50, 50, 50, 50}, SDL_PIXELFORMAT_RGBA8888, "AnimationTest/");
	SDL_Surface* animsurf = gore.initTransformSurf(animlist);
	TrList translist = gore.generatePixelTransforms(animlist);
	TrList transbegin = translist;
	double animtime = 0;
	//particle stuff
	texp particelist1 = gore.loadTextureList({ "particle1.png", "particle2.png" }, { 5, 5 }, {5, 5}, SDL_PIXELFORMAT_RGBA8888, rend, "ParticleTest/");
	texp ttp = particelist1;
	while (ttp != NULL) {
		SDL_BlendMode bp = SDL_BLENDMODE_BLEND;
		SDL_GetTextureBlendMode(ttp->current, &bp);
		ttp = ttp->next;
	}
	Particle fp(400, 600, 190, 360, {400, 600, 5, 5}, particelist1);
	Emitter emit(&fp, 0.2);
	Fire firep(200, 650, 25, 105, { 300, 600, 5, 5 }, particelist1);
	firep.erase = false;
	FireEmitter fireemit(&firep, 0.5);
	Water waterp(500, 650, 90, 180, { 500, 650, 5, 5 }, particelist1);
	WaterEmitter watemit(&waterp, 0.2);
	//just use default fullscreen SDL2 provides
	//SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
	FollowBone bone1({Joint(300, 300), Joint(310, 310), Joint(310, 325), Joint(310, 335)}, 0.05, 25.0f);
	FKLimb bone2({ FKBone(0.15, 100, 300, 600), FKBone(0.45, 100, 400, 550), FKBone(0.15, 50, 400, 600), FKBone(0.55, 50, 400, 600) });
	float thangle = 0.15;
	float secangle = 0.45;
	double bone3time = 0;
	double bonetime = 0;
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
		delta = gore.getDelta();
		animtime += delta;
		bonetime += delta;
		bone3time += delta;
		int fps = 1 / delta;
		std::string temp = "Example - " + std::to_string(fps);
		SDL_SetWindowTitle(window, temp.c_str());
		if (keys[SDL_SCANCODE_RIGHT]) {
			ang += 0.1;
			thangle += 0.01;
		}
		else if (keys[SDL_SCANCODE_LEFT]) {
			ang -= 0.1;
			thangle -= 0.01;
		}
		if (ang > 360) {
			ang = 0;
		}
		else if (ang < 0) {
			ang = 360;
		}
		if (keys[SDL_SCANCODE_W]) {
			player.y -= 100 * delta;
		}
		else if (keys[SDL_SCANCODE_S]) {
			player.y += 100 * delta;
		}
		if (keys[SDL_SCANCODE_A]) {
			player.x -= 100 * delta;
			secangle -= 0.01;
		}
		else if (keys[SDL_SCANCODE_D]) {
			player.x += 100 * delta;
			secangle += 0.01;
		}
		int mx, my;
		//not the most efficent way to do this
		if (SDL_GetMouseState(&mx, &my) & SDL_BUTTON(SDL_BUTTON_LEFT)) {
			if (mx >= 0 && mx < 578 && my >= 0 && my < 496) {
				if (!points[(my * (imgsurf->pitch / 8)) + mx]) {
					points[(my * (imgsurf->pitch / 8)) + mx] = true;
					Uint32 col = 0;
					gore.SetPixelSurface(imgsurf, &my, &mx, &col);
					SDL_DestroyTexture(tex1);
					tex1 = SDL_CreateTextureFromSurface(rend, imgsurf);
				}
			}
		}
		gore.clearSurface(surface);
		int yy = 500;
		for (int i = 0; i < 800; i++) {
			gore.SetPixelSurface(surface, &yy, &i, &col);
		}
		yy = 750;
		for (int i = 0; i < 500; i++) {
			gore.SetPixelSurface(surface, &i, &yy, &col);
		}
		Point p = gore.raycast2DPixel(surface, 200, 400, ang, 1);
		SDL_Texture* tex = SDL_CreateTextureFromSurface(rend, surface);
		SDL_Rect rect = { 0, 0, surface->w, surface->h };
		SDL_RenderCopy(rend, tex, NULL, &rect);
		SDL_SetRenderDrawColor(rend, 255, 50, 120, 0);
		SDL_RenderDrawLine(rend, player.x, player.y, p.x, p.y);
		//uncomment this code for testing of images and text drawing
		SDL_Rect erect = { 0, 0, 578, 496 };
		SDL_RenderCopy(rend, tex1, NULL, &erect);
		SDL_Rect rrect = { player.x, player.y, 50, 50 };
		SDL_RenderCopy(rend, texblum, NULL, &rrect);
		if (animtime > 0.4) {
			gore.switchTranformFrames(animsurf, translist, transbegin);
			animtime = 0;
		}
		SDL_Rect anrect = { 400, 600, 30, 50 };
		SDL_Texture* antex = SDL_CreateTextureFromSurface(rend, animsurf);
		SDL_RenderCopy(rend, antex, NULL, &anrect);
		SDL_DestroyTexture(antex);
		//cant be used once you make a class that overrides base virtual function
		//emit.update(&delta, rend);
		fireemit.update(&delta, rend);
		watemit.update(&delta, rend);
		//SDL_Rect prect = { 100, 100, 50, 100 };
		//SDL_RenderCopy(rend, tex2, NULL, &prect);
		//SDL_Rect enemy1rect = { 300, 300, 50, 100 };
		//SDL_RenderCopy(rend, gore.findTex(elist, "enemy1.png"), NULL, &enemy1rect);
		//SDL_Rect enemy5rect = { 250, 400, 50, 100 };
		//SDL_RenderCopy(rend, etex5, NULL, &enemy5rect);
		//gore.drawText(rend, alph, "hello world a", 0, 550, 25, 30);
		SDL_DestroyTexture(tex);
		if (SDL_GetMouseState(&mx, &my) & SDL_BUTTON(SDL_BUTTON_LEFT)) {
			if (bonetime > 0.5) {
				bone1.transformOverTime(mx, my);
				bonetime = 0;
			}
		}
		bone1.debugDraw(rend);
		bone1.updateTransform(delta);
		bone2.bones[0].angle = thangle;
		bone2.bones[1].angle = secangle;
		if (bone3time > 0.01) {
			bone2.animate(2, 0.05, 0.75, 0.01);
			bone3time = 0;
		}
		bone2.update();
		bone2.debugDraw(rend);
		SDL_RenderPresent(rend);
	}
	bone1.~FollowBone();
	return 0;
}