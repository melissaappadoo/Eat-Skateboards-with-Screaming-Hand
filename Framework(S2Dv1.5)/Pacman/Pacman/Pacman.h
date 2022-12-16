#pragma once

// If Windows and not in Debug, this will run without a console window
// You can use this to output information when debugging using cout or cerr
#ifdef WIN32 
	#ifndef _DEBUG
		#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
	#endif
#endif

// Includes libraries and definitions of data
#define MUNCHIECOUNT 50
#define GHOSTCOUNT 4
#include "S2D/S2D.h"

// Reduces the amount of typing by including all classes in S2D namespace
using namespace S2D;

// Declares the Pacman class which inherits from the Game class.
// This allows us to overload the Game class methods to help us
// load content, draw and update our game.
class Pacman : public Game
{
private:

	struct Player
	{
		// Data to represent Pacman
		bool dead;
		Vector2* position;
		Rect* sourceRect;
		Texture2D* texture;
		int direction;
		int frame;
		int currentFrameTime;
		float speedMultiplier;
	};

	Player* _player;
	SoundEffect* _pop;
	SoundEffect* _dead;
	int _score;

	struct Collectable
	{
		// Data to represent Munchie
		int frameCount;
		Rect* rect;
		Texture2D* texture;
		Texture2D* invertedTexture;
		int frame;
		int currentFrameTime;
		int frameTime;
		Vector2* position;
	};

	struct MovingEnemy
	{
		Vector2* position;
		Texture2D* texture;
		Rect* sourceRect;
		int direction;
		float speed;
		int frame;
		int currentFrameTime;
	};

	Collectable* _munchies[MUNCHIECOUNT];
	MovingEnemy* _ghosts[GHOSTCOUNT];

	const float _cPlayerSpeed;
	const int _cPlayerFrameTime;
	const int _cColFrameTime;
	const int _cPointValue;
	const int _cEnemyFrameTime;

	// Position for String
	Vector2* _stringPosition;

	struct Menu
	{
		// Data for Menu
		Texture2D* _menuBackground;
		Rect* _menuRectangle;
		Vector2* _menuStringPosition;
		bool _paused;
		bool _pKeyDown;
		bool _started;
		bool _gameOver;
		bool _spaceKeyDown;
	};

	Menu* _menu;

	//Input methods
	void Input(int elapsedTime, Input::KeyboardState* state, Input::MouseState* mouseState);

	//Check methods
	void CheckPaused(Input::KeyboardState* state, Input::Keys pauseKey);
	void CheckReset(Input::KeyboardState* state, Input::Keys resetKey);
	void CheckViewportCollision();
	void CheckGhostCollisions();

	//Update methods
	void UpdatePlayer(int elapsedTime);
	void UpdateMunchies(Collectable*, int elapsedTime);
	void UpdateGhost(MovingEnemy*, int elapsedTime);

	//Collision methods
	bool CollisionCheck(int x1, int y1, int width1, int height1, int x2, int y2, int width2, int height2);

	void Reset();


public:

	/// <summary> Constructs the Pacman class. </summary>
	Pacman(int argc, char* argv[]);

	/// <summary> Destroys any data associated with Pacman class. </summary>
	virtual ~Pacman();

	/// <summary> All content should be loaded in this method. </summary>
	void virtual LoadContent();

	/// <summary> Called every frame - update game logic here. </summary>
	void virtual Update(int elapsedTime);

	/// <summary> Called every frame - draw game here. </summary>
	void virtual Draw(int elapsedTime);
};