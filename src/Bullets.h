#pragma once
#include <list>
#include <stack>
#include <SDL.h>

/// <summary>A basic ammo object</summary>
class AmmoObject
{
public:
	float xPos,yPos;
	int Height,Width;
	int SurfaceID;
	SDL_Rect CollisionBox;

private:
};

/// <summary>Keeps the state of any bullet</summary>
class BulletState
{
public:
	int Frame, State;
	int Radius;
	int Surface;
	SDL_Rect Destination;
};

/// <summary>Used to create a Bullet entity</summary>
class Bullet : public BulletState,public AmmoObject
{

public:
	Bullet();
	
	SDL_Rect UpdateCollisionBox(SDL_Rect Box);
	void Setframe();
	void Update();
	void Draw();
	int GetSurfaceID();
	
	SDL_Rect GetDestination();

private:
	SDL_Rect Clips[ 16 ];
	int PrevFrame;
};

/// <summary>A class the controls the animals</summary> 
class ControlBullets
{
public:
	ControlBullets();
	~ControlBullets();
	void Draw_Bullets();
	void Create_Bullets();
	std::list< Bullet* > GetBullets(){ return My_Bullets;};

	int Turf;

	Bullet * CreateBullet( int xPos, int yPos, int surface );

	
private:
	std::list< Bullet* > My_Bullets;
};

extern ControlBullets BulletController;