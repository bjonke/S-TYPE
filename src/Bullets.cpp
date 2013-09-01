#include "Bullets.h"
#include <SDL.h>
#include "game.h"
#include "ControlGfx.h"

ControlBullets BulletController;

void Bullet::Setframe()
{	
	if( Frame >= 15 )
	{
		Frame = 0;
	}
}

int Bullet::Collision(SDL_Rect Boundry)
{
	return 0;
}

void Bullet::Update()
{
	this->xPos += 0.0003f * gamestate.DeltaTime;
	this->Destination.h = this->Height;
	this->Destination.w = this->Width;
	this->Destination.x = this->xPos;
	this->Destination.y = this->yPos; 

	this->PrevFrame = this->Frame++;
	this->Setframe();
}

void Bullet::Draw()
{
	SDL_BlitSurface( 
		Gfx.GetSurface( this->Surface ),
		&this->Clips[ this->PrevFrame ], 
		Gfx.BackBuffer, 
		&this->GetDestination() 
	);
}

int Bullet::GetSurfaceID()
{
	return this->Surface;
}

SDL_Rect Bullet::GetDestination()
{
	return this->Destination;
}

Bullet::Bullet()
{
	PrevFrame = 0;
	Frame = 0;
	Height = 64;
	Width =	64;

	for( int i = 0; i < 16; i++ )
	{
		Clips[ i ].x = i * Width;
		Clips[ i ].y = 0;
		Clips[ i ].h = Height;
		Clips[ i ].w = Width;
	}
}

void ControlBullets::Draw_Bullets()
{
 	list< Bullet* >::iterator i;

	i = My_Bullets.begin();
	while(i != My_Bullets.end() )
	{
		(*i)->Update();
		(*i)->Draw();
		
		if( (*i)->xPos >= Gfx.screen->w - (*i)->Width )
		{
			i = My_Bullets.erase(i);
		}
		else
		{
			++i;
		}
	}
}

Bullet * ControlBullets::CreateBullet( int xPos, int yPos, int surface )
{
	Bullet * temp = new Bullet;
	temp->Surface = surface;
	temp->xPos = xPos;
	temp->yPos = yPos;

	temp->Radius = ( temp->Width > temp->Height ) ? temp->Width / 2 : temp->Height / 2;

	return temp;
}
  
void ControlBullets::Create_Bullets()
{
	static int bullet_timer = 0;

	if( bullet_timer <= 0 )
	{
		My_Bullets.push_back( CreateBullet(BCPlayer.GetPosition().x, BCPlayer.GetPosition().y , gamestate.m_srfCrow ) ); // 75 + ( rand() % Turf )
		bullet_timer = 20;
	}
	else
	{
		bullet_timer--;
	}
}

ControlBullets::ControlBullets()
{
	Turf = 200;
}

ControlBullets::~ControlBullets()
{
	cout << "Destroying the Bullet Controller..." << endl;
}