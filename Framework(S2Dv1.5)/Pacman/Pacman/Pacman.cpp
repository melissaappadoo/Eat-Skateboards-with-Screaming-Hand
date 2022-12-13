#include "Pacman.h"

#include <sstream>
#include <time.h>
#include <iostream>

Pacman::Pacman(int argc, char* argv[]) : Game(argc, argv), _cPlayerSpeed(0.1), _cPlayerFrameTime(250), _cColFrameTime(500), _cPointValue(10)
{
	_player = new Player();
	_player->dead = false;
	_player->currentFrameTime = 0;
	_player->frame = 0;

	int i;

	for (int i = 0; i < MUNCHIECOUNT; i++)
	{
		_munchies[i] = new Collectable();
		_munchies[i]->frameCount = rand() % 1;
		_munchies[i]->currentFrameTime = 0;
		_munchies[i]->frameTime = rand() % 500 + 50;
	}
	_pop = new SoundEffect();
	_score = 0;

	_menu = new Menu();
	_menu->_paused = false;
	_menu->_pKeyDown = false;
	_menu->_started = false;
	_menu->_gameOver = false;
	_menu->_spaceKeyDown = false;

	// initialise ghost character
	for (int i = 0; i < GHOSTCOUNT; i++)
	{
		_ghosts[i] = new MovingEnemy();
		_ghosts[i]->direction = 0;
		_ghosts[i]->speed = 0.2f;
	}
	_dead = new SoundEffect();

	//Initialise important Game aspects
	Graphics::Initialise(argc, argv, this, 1024, 768, false, 25, 25, "Pacman", 60);
	Input::Initialise();
	Audio::Initialise();
		
	// Start the Game Loop - This calls Update and Draw in game loop
	Graphics::StartGameLoop();
}

Pacman::~Pacman()
{
	delete _player->texture;
	delete _player->sourceRect;
	delete _player->position;
	delete _player;

	delete _munchies[0]->texture;

	int nCount = 0;
	for (nCount = 0; nCount < MUNCHIECOUNT; nCount++)
	{
		delete _munchies[nCount]->texture;
		delete _munchies[nCount]->position;
		delete _munchies[nCount]->rect;
		delete _munchies[nCount];
	}
	delete[] _munchies;
	delete _pop;
	delete _dead;

	for (int i = 0; i < GHOSTCOUNT; i++)
	{
		delete _ghosts[i]->texture;
		delete _ghosts[i]->position;
		delete _ghosts[i]->sourceRect;
		delete _ghosts[i];
	}

	delete _menu->_menuBackground;
	delete _menu->_menuRectangle;
}

void Pacman::LoadContent()
{
	// Load Pacman
	_player->texture = new Texture2D();
	_player->texture->Load("Textures/Player.png", false);
	_player->position = new Vector2(350.0f, 350.0f);
	_player->sourceRect = new Rect(0.0f, 0.0f, 32, 32);
	_player->direction = 0;
	_player->speedMultiplier = 1.0f;

	// Load Munchie
	Texture2D* munchieTex = new Texture2D();
	munchieTex->Load("Textures/skateboard.png", true);
	Texture2D* invertedTex = new Texture2D();
	invertedTex->Load("Textures/MunchieInverted.tga", true);

	for (int i = 0; i < MUNCHIECOUNT; i++)
	{
		_munchies[i]->texture = munchieTex;
		_munchies[i]->rect = new Rect(0.0f, 0.0f, 12, 12);
		_munchies[i]->position = new Vector2((rand() % Graphics::GetViewportWidth()), (rand() % Graphics::GetViewportHeight()));
	}
	_pop->Load("Sounds/pop.wav");

	// Load Ghost
	for (int i = 0; i < GHOSTCOUNT; i++)
	{
		_ghosts[i]->texture = new Texture2D();
		_ghosts[i]->texture->Load("Textures/GhostBlue.png", false);
		_ghosts[i]->position = new Vector2((rand() % Graphics::GetViewportWidth()), (rand() % Graphics::GetViewportHeight()));
		_ghosts[i]->sourceRect = new Rect(0.0f, 0.0f, 20, 20);
	}
	_dead->Load("Sounds/pacman-is-dead");

	// Set string position
	_stringPosition = new Vector2(10.0f, 25.0f);

	// Set Menu Parameters
	_menu->_menuBackground = new Texture2D();
	_menu->_menuBackground->Load("Textures/Transparency.png", false);
	_menu->_menuRectangle = new Rect(0.0f, 0.0f, Graphics::GetViewportWidth(), Graphics::GetViewportHeight());
	_menu->_menuStringPosition = new Vector2(Graphics::GetViewportWidth() / 2.0f, Graphics::GetViewportHeight() / 2.0f);
}

// Call functions in the Update function
void Pacman::Update(int elapsedTime)
{
	// Gets the current state of the keyboard
	Input::KeyboardState* keyboardState = Input::Keyboard::GetState();
	//Gets the current state of the mouse
	Input::MouseState* mouseState = Input::Mouse::GetState();

	if (!Audio::IsInitialised())
	{
		std::cout << "Audio is not initialised" << std::endl;
	}
	if (!_pop->IsLoaded())
	{
		std::cout << "_pop member sound effect has not loaded" << std::endl;
	}
	if (!_dead->IsLoaded())
	{
		std::cout << "_dead member sound effect has not loaded" << std::endl;
	}

	if (!_menu->_started)
	{
		//check for start
		if (keyboardState->IsKeyDown(Input::Keys::SPACE))
		{
			_menu->_started = true;
		}
	}
	else
	{
		CheckPaused(keyboardState, Input::Keys::P);

		if (!_menu->_paused && !_menu->_gameOver)
		{
			Input(elapsedTime, keyboardState, mouseState);
			UpdatePlayer(elapsedTime);
			for (int i = 0; i < GHOSTCOUNT; i++)
			{
				UpdateGhost(_ghosts[i], elapsedTime);
			}
			CheckGhostCollisions();
			CheckViewportCollision();

			for (int i = 0; i < MUNCHIECOUNT; i++)
			{
				if (CollisionCheck(_player->position->X, _player->position->Y, _player->sourceRect->Width, _player->sourceRect->Height, _munchies[i]->position->X, _munchies[i]->position->Y, _munchies[i]->rect->Width, _munchies[i]->rect->Height))
				{
					_munchies[i]->position->X = 10000.0f;
					_munchies[i]->position->Y = 10000.0f;
					Audio::Play(_pop);
				}
			}

			for (int i = 0; i < MUNCHIECOUNT; i++)
			{
				UpdateMunchies(_munchies[i], elapsedTime);
			}
		}
		else
		{
			CheckReset(keyboardState, Input::Keys::SPACE);
		}
	}
}

void Pacman::Input(int elapsedTime, Input::KeyboardState* keyboardState, Input::MouseState* mouseState)
{
	float playerSpeed = _cPlayerSpeed * elapsedTime * _player->speedMultiplier;
	// Checks if D key is pressed
	if (keyboardState->IsKeyDown(Input::Keys::D))
	{
		_player->position->X += playerSpeed; //Moves Player across X axis
		_player->direction = 0;
	}

	// Checks if A key is pressed
	else if (keyboardState->IsKeyDown(Input::Keys::A))
	{
		_player->position->X -= playerSpeed;
		_player->direction = 2;
	}

	// Checks if W key is pressed
	else if (keyboardState->IsKeyDown(Input::Keys::W))
	{
		_player->position->Y -= playerSpeed;
		_player->direction = 3;
	}

	// Checks if S key is pressed
	else if (keyboardState->IsKeyDown(Input::Keys::S))
	{
		_player->position->Y += playerSpeed;
		_player->direction = 1;
	}

	if (keyboardState->IsKeyDown(Input::Keys::LEFTSHIFT))
	{
		_player->speedMultiplier = 2.0f;
	}
	else
	{
		_player->speedMultiplier = 1.0f;
	}
}

void Pacman::CheckPaused(Input::KeyboardState* state, Input::Keys pauseKey)
{
	if (state->IsKeyDown(pauseKey) && !_menu->_pKeyDown)
	{
		_menu->_pKeyDown = true;
		_menu->_paused = !_menu->_paused;
	}

	if (state->IsKeyUp(pauseKey))
		_menu->_pKeyDown = false;
}

void Pacman::CheckReset(Input::KeyboardState* state, Input::Keys resetKey)
{
	if (state->IsKeyDown(resetKey) && !_menu->_spaceKeyDown)
	{
		_menu->_spaceKeyDown = true;
		Reset();
	}

	if (state->IsKeyUp(resetKey))
		_menu->_spaceKeyDown = false;
}

void Pacman::CheckViewportCollision()
{
	//Checks if player is trying to disappear

	if (_player->position->X > Graphics::GetViewportWidth())
	{
		// Player hit left wall - reset his position
		_player->position->X = 30 - _player->sourceRect->Width;
	}

	else if (_player->position->X + _player->sourceRect->Width < 4)
	{
		// Player hit right wall - reset his position
		_player->position->X = 1024 - _player->sourceRect->Width;
	}

	else if (_player->position->Y + _player->sourceRect->Height < 4)
	{
		// Player hit bottom wall - reset his position
		_player->position->Y = 770 - _player->sourceRect->Height;
	}

	else if (_player->position->Y > Graphics::GetViewportHeight())
	{
		// Player hit top wall - reset his position
		_player->position->Y = 30 - _player->sourceRect->Height;
	}
}

void Pacman::UpdatePlayer(int elapsedTime)
{
	_player->currentFrameTime += elapsedTime;

	if (_player->currentFrameTime > _cPlayerFrameTime)
	{
		_player->frame++;

		if (_player->frame >= 2)
			_player->frame = 0;

		_player->currentFrameTime = 0;
	}

	_player->sourceRect->Y = _player->sourceRect->Height * _player->direction;
	_player->sourceRect->X = _player->sourceRect->Width * _player->frame;
}

void Pacman::UpdateMunchies(Collectable*, int elapsedTime)
{
	for (int i = 0; i < MUNCHIECOUNT; i++)
	{
		_munchies[i]->frameCount++;

		_munchies[i]->currentFrameTime += elapsedTime;

		if (_munchies[i]->currentFrameTime > _cColFrameTime)
		{
			_munchies[i]->frameCount++;

			if (_munchies[i]->frameCount >= 2)
				_munchies[i]->frameCount = 0;

			_munchies[i]->currentFrameTime = 0;
		}
		_munchies[i]->rect->X = _munchies[i]->rect->Width * _munchies[i]->frameCount;
	}
	
}

void Pacman::UpdateGhost(MovingEnemy* ghost, int elapsedTime)
{
	if (ghost->direction == 0) //Moves Right
	{
		ghost->position->X += ghost->speed * elapsedTime;
	}
	else if (ghost->direction == 1) // Moves Left
	{
		ghost->position->X -= ghost->speed * elapsedTime;
	}
	if (ghost->position->X + ghost->sourceRect->Width >= Graphics::GetViewportWidth()) // Hits Right edge
	{
		ghost->direction = 1; // Change direction
	}
	else if (ghost->position->X <= 0) //Hits left edge
	{
		ghost->direction = 0; // Change direction
	}

	ghost->sourceRect->Y = ghost->sourceRect->Height * ghost->direction;
}

void Pacman::CheckGhostCollisions()
{
	// Local Variables
	int i = 0;
	int bottom1 = _player->position->Y + _player->sourceRect->Height;
	int bottom2 = 0;
	int left1 = _player->position->X;
	int left2 = 0;
	int right1 = _player->position->X + _player->sourceRect->Width;
	int right2 = 0;
	int top1 = _player->position->Y;
	int top2 = 0;

	for (int i = 0; i < GHOSTCOUNT; i++)
	{
		// Populate variables with Ghost data
		bottom2 = _ghosts[i]->position->Y + _ghosts[i]->sourceRect->Height;
		left2 = _ghosts[i]->position->X;
		right2 = _ghosts[i]->position->X + _ghosts[i]->sourceRect->Width;
		top2 = _ghosts[i]->position->Y;

		if ((bottom1 > top2) && (top1 < bottom2) && (right1 > left2) && (left1 < right2))
		{
			_player->dead = true;
			_menu->_gameOver = true;
			i = GHOSTCOUNT;
			Audio::Play(_dead);
		}
	}
}

bool Pacman::CollisionCheck(int x1, int y1, int width1, int height1, int x2, int y2, int width2, int height2)
{
	int left1 = x1;
	int left2 = x2;
	int right1 = x1 + width1;
	int right2 = x2 + width2;
	int top1 = y1;
	int top2 = y2;
	int bottom1 = y1 + height1;
	int bottom2 = y2 + height2;

	if (bottom1 < top2)
		return false;
	if (top1 > bottom2)
		return false;
	if (right1 < left2)
		return false;
	if (left1 > right2)
		return false;

	_score += _cPointValue;
	return true;

}

void Pacman::Reset()
{
	_player->position = new Vector2(350.0f, 350.0f);
	_player->dead = false;
	_menu->_gameOver = false;
}

void Pacman::Draw(int elapsedTime)
{
	// Allows us to easily create a string
	std::stringstream stream;
	stream << "SCORE: " << _score;

	SpriteBatch::BeginDraw(); // Starts Drawing
	
	//Draws Player
	if (!_player->dead) 
	{
		SpriteBatch::Draw(_player->texture, _player->position, _player->sourceRect);
	}

	for (int i = 0; i < MUNCHIECOUNT; i++)
	{
		SpriteBatch::Draw(_munchies[i]->texture, _munchies[i]->position, _munchies[i]->rect);
	}

	//draw ghosts
	for (int i = 0; i < GHOSTCOUNT; i++)
	{
		SpriteBatch::Draw(_ghosts[i]->texture, _ghosts[i]->position, _ghosts[i]->sourceRect);
	}

	if (_menu->_paused)
	{
		std::stringstream menuStream;
		menuStream << "PAUSED!";

		SpriteBatch::Draw(_menu->_menuBackground, _menu->_menuRectangle, nullptr);
		SpriteBatch::DrawString(menuStream.str().c_str(), _menu->_menuStringPosition, Color::Red);
	}

	if (!_menu->_started)
	{
		std::stringstream menuStream;
		menuStream << "START";

		SpriteBatch::Draw(_menu->_menuBackground, _menu->_menuRectangle, nullptr);
		SpriteBatch::DrawString(menuStream.str().c_str(), _menu->_menuStringPosition, Color::Green);
	}

	if (_menu->_gameOver)
	{
		std::stringstream menuStream;
		menuStream << "GAME OVER";

		SpriteBatch::Draw(_menu->_menuBackground, _menu->_menuRectangle, nullptr);
		SpriteBatch::DrawString(menuStream.str().c_str(), _menu->_menuStringPosition, Color::Red);
	}

	// Draws String
	SpriteBatch::DrawString(stream.str().c_str(), _stringPosition, Color::White);
	SpriteBatch::EndDraw(); // Ends Drawing
	
}