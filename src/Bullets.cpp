#include "Bullets.h"
#include <SDL.h>
#include "game.h"
#include "ControlGfx.h"

ControlBullets BulletController;

SDL_Rect Bullet::UpdateCollisionBox(SDL_Rect Box)
{
	CollisionBox = Box;
	return CollisionBox;
}

void Bullet::Update()
{
	LocAndSize.x += 0.0003f * gamestate.DeltaTime;

	PrevFrame = Frame++;
	
	if( Frame >= BULLET_MAX_FRAMES )
	{
		Frame = 0;
	}

	UpdateCollisionBox( LocAndSize );
}

void Bullet::Draw()
{
	#ifdef _DEBUG 
	//SDL_FillRect(Gfx.BackBuffer, &CollisionBox,0xffffff );
	#endif

	SDL_BlitSurface( 
		Gfx.GetSurface( SurfaceID ),
		&Clips[ PrevFrame ], 
		Gfx.BackBuffer, 
		&GetLocAndSize() 
	);
}

int Bullet::GetSurfaceID()
{
	return SurfaceID;
}

SDL_Rect Bullet::GetLocAndSize()
{
	return LocAndSize;
}

Bullet::Bullet()
{
	LocAndSize.h = 64;
	LocAndSize.w = 64;
	LocAndSize.x = 0;
	LocAndSize.y = 0;

	// Setup collision box boundries
	CollisionBox.x = 0;
	CollisionBox.y = 0;
	CollisionBox.w = 64;
	CollisionBox.h = 64;

	PrevFrame = 0;
	Frame = 0;

	for( int i = 0; i < 16; i++ )
	{
		Clips[ i ].x = i * LocAndSize.w;
		Clips[ i ].y = 0;
		Clips[ i ].h = LocAndSize.h;
		Clips[ i ].w = LocAndSize.w;
	}
}

void ControlBullets::Draw_Bullets()
{
	list< Bullet >::iterator BulletCounter;
	BulletCounter = Bullets.begin();
	
	while( BulletCounter != Bullets.end() )
	{
		BulletCounter->Update();
		BulletCounter->Draw();
		if( BulletCounter->LocAndSize.x >= Gfx.screen->w - BulletCounter->LocAndSize.w )
		{
			BulletCounter = Bullets.erase(BulletCounter);
		}
		else
		{
			++BulletCounter;
		}
	}
}

void ControlBullets::LoadBullet( int xPos, int yPos, int surface )
{
	Bullet tempBullet;

	tempBullet.LocAndSize.x = xPos;
	tempBullet.LocAndSize.y = yPos;
	tempBullet.SurfaceID = surface;

	BulletArrayRef.push_back( tempBullet );
 }

Bullet ControlBullets::CreateBulletByReference( int xPos, int yPos, int surface )
{
	Bullet temp;
	temp.SurfaceID = surface;
	temp.LocAndSize.x = xPos;
	temp.LocAndSize.y = yPos;

	return temp;
}

void ControlBullets::Create_Bullets()
{
	static int bullet_timer = 0;

	if( bullet_timer <= 0 )
	{ 
		LoadBullet(BCPlayer.GetPosition().x + BCPlayer.CollisionBox.w / 2, BCPlayer.GetPosition().y + BCPlayer.CollisionBox.h / 2, gamestate.m_srfLaser );
		Bullets.push_back( CreateBulletByReference(BCPlayer.GetPosition().x + BCPlayer.CollisionBox.w / 2, BCPlayer.GetPosition().y + BCPlayer.CollisionBox.h / 2, gamestate.m_srfLaser ) );
		bullet_timer = 10;
	}
	else
	{
		bullet_timer--;
	}
}

ControlBullets::ControlBullets()
{
	cout << "Creating the ControlBullets class..." << endl;
}

ControlBullets::~ControlBullets()
{
	cout << "Destroying the ControlBullets class..." << endl;
}