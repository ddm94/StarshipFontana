#include "SFApp.h"

SFApp::SFApp(std::shared_ptr<SFWindow> window) :
		fire(0), is_running(true), sf_window(window) {

	SDL_GetRendererOutputSize(sf_window->getRenderer(), &canvas_w, &canvas_h);

	app_box = make_shared < SFBoundingBox
			> (Vector2(canvas_w, canvas_h), canvas_w, canvas_h);

	//create wall at the bottom of the screen
	for (int i = 0; i <= (canvas_w / 24); i++) {
		auto wall = make_shared < SFAsset > (SFASSET_WALL, sf_window);
		auto pos = Point2(24 * i, 0);
		wall->SetPosition(pos);
		permaWalls.push_back(wall);
	}
	//create wall at the left side of the screen
	for (int i = 0; i <= (canvas_h / 24); i++) {
		auto wall = make_shared < SFAsset > (SFASSET_WALL, sf_window);
		auto pos = Point2(0, 24 * i);
		wall->SetPosition(pos);
		permaWalls.push_back(wall);
	}
	//create wall at the right side of the screen
	for (int i = 0; i <= (canvas_h / 24); i++) {
		auto wall = make_shared < SFAsset > (SFASSET_WALL, sf_window);
		auto pos = Point2(canvas_w, 24 * i);
		wall->SetPosition(pos);
		permaWalls.push_back(wall);
	}
	//create wall at the top of the screen
	for (int i = 0; i <= (canvas_w / 24); i++) {
		auto wall = make_shared < SFAsset > (SFASSET_WALL, sf_window);
		auto pos = Point2(24 * i, canvas_h);
		wall->SetPosition(pos);
		permaWalls.push_back(wall);
	}

	//first level
	Reset(0);

}

SFApp::~SFApp() {
}

/**
 * Handle all events that come from SDL.
 * These are timer or keyboard events.
 */
void SFApp::OnEvent(SFEvent& event) {
	SFEVENT the_event = event.GetCode();
	switch (the_event) {
	case SFEVENT_QUIT:
		is_running = false;
		break;
	case SFEVENT_UPDATE:
		if (!pause) { //If the game is not pause, thus is running
			OnUpdateWorld();
			OnRender();
		} else {
			if (difftime(time(nullptr), startTime) >= 3) { //3 seconds delays before resetting the level
				pause = false;
				Reset(level);
			}
		}
		break;
	case SFEVENT_PLAYER_LEFT:
		player->GoWest();
		break;
	case SFEVENT_PLAYER_RIGHT:
		player->GoEast();
		break;
	case SFEVENT_PLAYER_UP:
		player->GoUp();
		break;
	case SFEVENT_PLAYER_DOWN:
		player->GoSouth();
		break;
	case SFEVENT_FIRE:
		fire++;
		FireProjectile();
		break;
	}
}

int SFApp::OnExecute() {
	// Execute the app
	SDL_Event event;
	while (SDL_WaitEvent(&event) && is_running) {
		// wrap an SDL_Event with our SFEvent
		SFEvent sfevent((const SDL_Event) event);
		// handle our SFEvent
		OnEvent(sfevent);
	}
}

void SFApp::OnUpdateWorld() {

	// Update missiles positions
	for (auto p : projectiles) {
		p->GoNorth();
	}

	for (auto c : coins) {
		//c->GoNorth();
	}

	// Update enemy positions
	for (auto a : aliens) {
		a->Pattern();
	}

	// Detect collisions between aliens and walls
	for (auto pw : permaWalls) {
		for (auto a : aliens) {
			if (a->CollidesWith(pw)) {
				a->HandleCollisionObject();
			}
		}
	}
	for (auto w : walls) {
		for (auto a : aliens) {
			if (a->CollidesWith(w)) {
				a->HandleCollisionObject();
			}
		}
	}

	// Detect collisions between missiles and aliens
	for (auto p : projectiles) {
		for (auto a : aliens) {
			if (p->CollidesWith(a)) {
				a->HandleCollision();
				p->HandleCollision();
				AddToScore(alien);
			}
		}
	}

	// Detect collisions between missiles and walls
	for (auto p : projectiles) {
		for (auto w : walls) {
			if (p->CollidesWith(w)) {
				p->HandleCollision();
			}
		}
	}
	for (auto p : projectiles) {
		for (auto pw : permaWalls) {
			if (p->CollidesWith(pw)) {
				p->HandleCollision();
			}
		}
	}

	// Detect collisions between player and star
	if (player->CollidesWith(star)) {
		star->HandleCollision();
		std::cout << "Congratulations! YOU WIN!" << std::endl << "score: "
				<< score << std::endl;
		if (level == 1) {
			level = 0;
		} else {
			level = 1;
		}
		Pause();
	}

	// Detect collisions between player and coin
	for (auto c : coins) {
		if (player->CollidesWith(c)) {
			c->HandleCollision();
			AddToScore(coin);
		}
	}

	// Detect collisions between aliens and player
	for (auto a : aliens) {
		if (player->CollidesWith(a)) {
			player->HandleCollision();
			if (lives == 1) {
				level = 0;
				lives = 3;
				std::cout << "GAME OVER" << std::endl << "score: " << score
						<< std::endl;
				Pause();
			} else {
				lives--;
				std::cout << lives << " Life/s left" << std::endl;
				Pause();
			}
		}
	}

	// Detect collisions between walls and player
	for (auto w : walls) {
		if (player->CollidesWith(w)) {
			player->HandleCollisionObject();
		}
	}
	for (auto pw : permaWalls) {
		if (player->CollidesWith(pw)) {
			player->HandleCollisionObject();
		}
	}

	// remove dead aliens (the long way)
	list < shared_ptr < SFAsset >> tmp;
	for (auto a : aliens) {
		if (a->IsAlive()) {
			tmp.push_back(a);
		}
	}
	aliens.clear();
	aliens = list < shared_ptr < SFAsset >> (tmp);

	// remove coin
	list < shared_ptr < SFAsset >> tmp2;
	for (auto c : coins) {
		if (c->IsAlive()) {
			tmp2.push_back(c);
		}
	}
	coins.clear();
	coins = list < shared_ptr < SFAsset >> (tmp2);

	// remove missiles (the long way)
	list < shared_ptr < SFAsset >> tmp3;
	for (auto p : projectiles) {
		if (p->IsAlive()) {
			tmp3.push_back(p);
		}
	}
	projectiles.clear();
	projectiles = list < shared_ptr < SFAsset >> (tmp3);

}

void SFApp::OnRender() {
	SDL_RenderClear(sf_window->getRenderer());

	// draw the player
	if (player->IsAlive()) {
		player->OnRender();
	}

	// draw the star
	if (star->IsAlive()) {
		star->OnRender();
	}

	for (auto p : projectiles) {
		if (p->IsAlive()) {
			p->OnRender();
		}
	}

	for (auto a : aliens) {
		if (a->IsAlive()) {
			a->OnRender();
		}
	}

	for (auto c : coins) {
		c->OnRender();
	}

	for (auto w : walls) {
		w->OnRender();
	}

	for (auto pw : permaWalls) {
		pw->OnRender();
	}

	// Switch the off-screen buffer to be on-screen
	SDL_RenderPresent(sf_window->getRenderer());

}

void SFApp::FireProjectile() {
	auto pb = make_shared < SFAsset > (SFASSET_PROJECTILE, sf_window);
	auto v = player->GetPosition();
	pb->SetPosition(v);
	projectiles.push_back(pb);
}

//update score
void SFApp::AddToScore(int n) {
	score += n;
}

int SFApp::GetScore() {
	return score;
}

void SFApp::Pause() {
	startTime = time(nullptr);
	pause = true;
}

/*Implementation of 2 levels done by using an if.
 * This could be changed to a switch should more levels be added.
 */
void SFApp::Reset(int level) {
	//Create level 0 (start of game)
	if (level == 0) {

		aliens.clear();
		coins.clear();
		projectiles.clear();
		walls.clear();

		player = make_shared < SFAsset > (SFASSET_PLAYER, sf_window);
		auto player_pos = Point2(canvas_w / 2, 50);
		player->SetPosition(player_pos);

		const int number_of_aliens = 11;
		for (int i = 1; i < number_of_aliens; i++) {
			// place an alien at width/number_of_aliens * i
			auto alien = make_shared < SFAsset > (SFASSET_ALIEN, sf_window);
			auto pos = Point2((canvas_w / number_of_aliens) * i, 250.0f);
			alien->SetPosition(pos);
			aliens.push_back(alien);
		}

		auto coin = make_shared < SFAsset > (SFASSET_COIN, sf_window);
		auto pos = Point2((canvas_w / 2), 150);
		coin->SetPosition(pos);
		coins.push_back(coin);

		auto wall = make_shared < SFAsset > (SFASSET_WALL, sf_window);
		pos = Point2(512, 126);
		wall->SetPosition(pos);
		walls.push_back(wall);

		//revive star
		star = make_shared < SFAsset > (SFASSET_STAR, sf_window);
		auto star_pos = Point2(46, 432);
		star->SetPosition(star_pos);

		score = 0;

	}
	//Create next level
	if (level == 1) {

		//clears the vectors.
		aliens.clear();
		coins.clear();
		projectiles.clear();
		walls.clear();

		// place an player
		player = make_shared < SFAsset > (SFASSET_PLAYER, sf_window);
		auto player_pos = Point2(62, 42);
		player->SetPosition(player_pos);

		// place an alien
		auto alien = make_shared < SFAsset > (SFASSET_ALIEN, sf_window);
		auto pos = Point2(320,346);
		alien->SetPosition(pos);
		aliens.push_back(alien);
		alien = make_shared < SFAsset > (SFASSET_ALIEN, sf_window);
		pos = Point2(192, 120);
		alien->SetPosition(pos);
		aliens.push_back(alien);
		alien = make_shared < SFAsset > (SFASSET_ALIEN, sf_window);
		pos = Point2(384, 120);
		alien->SetPosition(pos);
		aliens.push_back(alien);
		alien = make_shared < SFAsset > (SFASSET_ALIEN, sf_window);
		pos = Point2(192, 264);
		alien->SetPosition(pos);
		aliens.push_back(alien);
		alien = make_shared < SFAsset > (SFASSET_ALIEN, sf_window);
		pos = Point2(506, 264);
		alien->SetPosition(pos);
		aliens.push_back(alien);
		alien = make_shared < SFAsset > (SFASSET_ALIEN, sf_window);
		pos = Point2(476, 384);
		alien->SetPosition(pos);
		aliens.push_back(alien);

		// place an coins
		auto coin = make_shared < SFAsset > (SFASSET_COIN, sf_window);
		pos = Point2(64, 432);
		coin->SetPosition(pos);
		coins.push_back(coin);
		coin = make_shared < SFAsset > (SFASSET_COIN, sf_window);
		pos = Point2(64, 214);
		coin->SetPosition(pos);
		coins.push_back(coin);
		coin = make_shared < SFAsset > (SFASSET_COIN, sf_window);
		pos = Point2(376, 192);
		coin->SetPosition(pos);
		coins.push_back(coin);
		coin = make_shared < SFAsset > (SFASSET_COIN, sf_window);
		pos = Point2(586, 302);
		coin->SetPosition(pos);
		coins.push_back(coin);

		/*sort of maze looking thing
		 * I am fully aware the following is a tedious process.
		 * This should have been done differently by creating other classes
		 * and implement a mapping method.
		 * PS: The comments regarding the position of the walls are  were used
		 * as a remainder to get the aliens in position.
		 */
		//4 walls - top left
		auto wall = make_shared < SFAsset > (SFASSET_WALL, sf_window);
		pos = Point2(128, 384);
		wall->SetPosition(pos);
		walls.push_back(wall);
		wall = make_shared < SFAsset > (SFASSET_WALL, sf_window);
		pos = Point2(128, 408);
		wall->SetPosition(pos);
		walls.push_back(wall);
		wall = make_shared < SFAsset > (SFASSET_WALL, sf_window);
		pos = Point2(128, 432);
		wall->SetPosition(pos);
		walls.push_back(wall);
		wall = make_shared < SFAsset > (SFASSET_WALL, sf_window);
		pos = Point2(128, 456);
		wall->SetPosition(pos);
		walls.push_back(wall);

		//4 walls - mid left
		wall = make_shared < SFAsset > (SFASSET_WALL, sf_window);
		pos = Point2(32, 264);
		wall->SetPosition(pos);
		walls.push_back(wall);
		wall = make_shared < SFAsset > (SFASSET_WALL, sf_window);
		pos = Point2(64, 264);
		wall->SetPosition(pos);
		walls.push_back(wall);
		wall = make_shared < SFAsset > (SFASSET_WALL, sf_window);
		pos = Point2(96, 264);
		wall->SetPosition(pos);
		walls.push_back(wall);
		wall = make_shared < SFAsset > (SFASSET_WALL, sf_window);
		pos = Point2(128, 264);
		wall->SetPosition(pos);
		walls.push_back(wall);

		// 4 walls - centre ish
		wall = make_shared < SFAsset > (SFASSET_WALL, sf_window);
		pos = Point2(318, 264);
		wall->SetPosition(pos);
		walls.push_back(wall);
		wall = make_shared < SFAsset > (SFASSET_WALL, sf_window);
		pos = Point2(340, 264);
		wall->SetPosition(pos);
		walls.push_back(wall);
		wall = make_shared < SFAsset > (SFASSET_WALL, sf_window);
		pos = Point2(372, 264);
		wall->SetPosition(pos);
		walls.push_back(wall);
		wall = make_shared < SFAsset > (SFASSET_WALL, sf_window);
		pos = Point2(404, 264);
		wall->SetPosition(pos);
		walls.push_back(wall);

		//4 vertical walls - centre ish
		wall = make_shared < SFAsset > (SFASSET_WALL, sf_window);
		pos = Point2(318, 240);
		wall->SetPosition(pos);
		walls.push_back(wall);
		wall = make_shared < SFAsset > (SFASSET_WALL, sf_window);
		pos = Point2(318, 216);
		wall->SetPosition(pos);
		walls.push_back(wall);
		wall = make_shared < SFAsset > (SFASSET_WALL, sf_window);
		pos = Point2(318, 192);
		wall->SetPosition(pos);
		walls.push_back(wall);
		wall = make_shared < SFAsset > (SFASSET_WALL, sf_window);
		pos = Point2(318, 168);
		wall->SetPosition(pos);
		walls.push_back(wall);
		wall = make_shared < SFAsset > (SFASSET_WALL, sf_window);
		pos = Point2(318, 144);
		wall->SetPosition(pos);
		walls.push_back(wall);

		//1 wall middle far right
		wall = make_shared < SFAsset > (SFASSET_WALL, sf_window);
		pos = Point2(608, 264);
		wall->SetPosition(pos);
		walls.push_back(wall);

		//1 wall mid (3 walls unit above the mid walls)
		wall = make_shared < SFAsset > (SFASSET_WALL, sf_window);
		pos = Point2(318, 384);
		wall->SetPosition(pos);
		walls.push_back(wall);

		//2 walls bottom centre
		wall = make_shared < SFAsset > (SFASSET_WALL, sf_window);
		pos = Point2(318, 120);
		wall->SetPosition(pos);
		walls.push_back(wall);
		wall = make_shared < SFAsset > (SFASSET_WALL, sf_window);
		pos = Point2(350, 120);
		wall->SetPosition(pos);
		walls.push_back(wall);

		//two walls top left
		wall = make_shared < SFAsset > (SFASSET_WALL, sf_window);
		pos = Point2(576, 384);
		wall->SetPosition(pos);
		walls.push_back(wall);
		wall = make_shared < SFAsset > (SFASSET_WALL, sf_window);
		pos = Point2(608, 384);
		wall->SetPosition(pos);
		walls.push_back(wall);

		//revive star
		star = make_shared < SFAsset > (SFASSET_STAR, sf_window);
		auto star_pos = Point2(576, 432);
		star->SetPosition(star_pos);

	}
}

