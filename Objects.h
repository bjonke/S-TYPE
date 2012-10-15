#pragma once
#include <SDL.h>
#include <list>

// @date 2012-08-07

class Object
{
public:

	int Initialize(float _xPos, float _yPos, int _Width, int _Height,int Frame, int _Radius);
	int Object::SetClips(int _xStepping, int _yStepping, int _Width, int _Height);
	float xPos, yPos;
	int Width, Height;
	int surface;
	int Frame;
	int radius;

	SDL_Rect Clips[ 10 ];
	SDL_Rect SingleClip; // Tree
};

class ThingsToDemon : public Object
{
public:
	ThingsToDemon();
	void SetClips();
	SDL_Rect HealthClips[ 6 ];
	int SurfaceHealth;
};

class PowerUp : public Object
{
public:
	PowerUp( int xpos, int ypos, int Surface );
	//void SetClips();
	void SetFrame();
	bool Left, Right;
};

class Tree : public Object
{
public:
	Tree();
};

class FireBall : public Object
{
public:
	FireBall();
	//void SetClips();
	int FrameRight, FrameLeft;
	bool FireRight, FireLeft;

	int FireWidth, FireHeight;
};

class Coffin : public Object
{
public:
	Coffin();
	//void SetClips();
	void SetFrames();
};

class Control_Objects
{
public:
	Control_Objects();
	void DrawObjects();
	void CreateObjects();

	
	int WhichLifeToShow;
	SDL_Rect destHealth;
	int FrameHealth;

	int Zombie, Skeleton, Skull;

	bool PowerUpMan;
	
	FireBall * CreateFireBall( int xPos, int yPos, int Surface, bool Right, bool Left );
	Coffin * CreateCoffin( int xPos, int yPos, int Surface );

	PowerUp * WereWolf;
	
	std::list< PowerUp * > List_PowerUps;
	std::list< Tree * > List_Trees;
	std::list< FireBall * > List_FireBalls;
	std::list< Coffin * > List_Coffins;

	ThingsToDemon * DemonLife;

private:
};

extern Control_Objects Control_OBJ;