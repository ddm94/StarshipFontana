#ifndef SFAPP_H
#define SFAPP_H

#include <memory>   // Pull in std::shared_ptr
#include <iostream> // Pull in std::cerr, std::endl
#include <list>     // Pull in list
#include <sstream>
#include <ctime>    // Pull in std::time_t object to store the time

using namespace std;

#include "SFCommon.h"
#include "SFEvent.h"
#include "SFAsset.h"

/**
 * Represents the StarshipFontana application.  It has responsibilities for
 * * Creating and destroying the app window
 * * Processing game events
 */
class SFApp {
public:
	SFApp (std::shared_ptr<SFWindow>);
	virtual ~SFApp();
	void OnEvent(SFEvent &);
	int OnExecute();
	void OnUpdateWorld();
	void OnRender();

	void FireProjectile();

	void AddToScore(int n);
	int GetScore();

	void Reset(int level);
	void Pause();

private:
	bool is_running;
	time_t startTime;
	bool pause = false;

	shared_ptr<SFWindow> sf_window;

	shared_ptr<SFAsset> player;
	shared_ptr<SFBoundingBox> app_box;
	list<shared_ptr<SFAsset> > projectiles;
	list<shared_ptr<SFAsset> > aliens;
	list<shared_ptr<SFAsset> > coins;
	//Variable created for the 4 walls covering the margins of the screen
	list<shared_ptr<SFAsset> > permaWalls;
	list<shared_ptr<SFAsset> > walls;
	shared_ptr<SFAsset> star;

	int canvas_w, canvas_h;

	int fire;

	int score = 0;
	//points
	int coin = 10;
	int alien = 5;
	//current level
	int level = 0;

	int lives = 3;

};
#endif
