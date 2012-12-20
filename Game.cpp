#pragma once

#include <iostream>
#include <windows.h>
#include <sstream>
#include <stdlib.h>
#include <fstream>
#include <cmath>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include "Game.h"
#include "characters.h"
#include "ControlGfx.h"
#include "Audio.h"
#include "Objects.h"
#include "Enemies.h"
#include "TImers.h"
#include "Paralaxlayers.h"
#include "Animals.h"
#include "FirstScreen.h"
#include "Collision.h"
#include "OutroFinish.h"
#include "GetInput.h"

// @date 2012-08-07

Gamestate gamestate;

Gamestate::Gamestate()
{

	cout << "Initializing Gamestate" << endl;
	SCREEN_HEIGHT = 600;
	SCREEN_WIDTH = 800;
	SCREEN_BPP = 32;
	cout << "Resolution: " << SCREEN_HEIGHT << "x" << SCREEN_WIDTH << "x" << SCREEN_BPP << endl;
	dt = 0.0f;
	CheckingHighScore = false;
	BossStart = false;
	IntroDone = false;

	FirstLevel = true;
	cout << "Setting FirstLevel" << endl;

	GameCondition = GS_INTRO;
	cout << "GameCondition: GS_INTRO" << endl;

	boss = NULL;
	Intro = NULL;
	outro = NULL;
	
	PreviousAnimArray = 0;
	PreviousFrame = 0;
	CurrentAnimArray = 0;
	CurrentFrame = 0;

	font = NULL;
	srfText = NULL;

	m_parallax = 0;

	//Score = 0;
	LevelProgress = 0;

	// Loading files

}

void Game::Handle_events( SDL_Event input )
{	
	// checks input for keypresses and releases, sets the appropriate settings for
	// the player updating his animations and attacks

	timer.Timer_FireBall++;
	timer.Timer_TriangleAttackOK++;

	if( input.type == SDL_KEYUP )
	{
		switch( input.key.keysym.sym )
		{
		case SDLK_RIGHT:
			{
				demon.isMovingRight = false;
				break;
			}
		case SDLK_LEFT:
			{
				demon.isMovingLeft = false;
				break;
			}
		case SDLK_UP:
			{
				demon.isJumping = false;
				break;
			}
		case SDLK_DOWN:
			{
				demon.isCrouching = false;
				break;
			}
		case SDLK_SPACE:
			{
				demon.isKicking = false;
				break;
			}
		case SDLK_LALT:
			{
				if( demon.SmallHunter )
				{
					demon.isPunching = false;
				}
				else
				{
					demon.TriangleAttack = false;
				}
				break;
			}
		default:
			{
				break;
			}
		}
	}
	else if( input.type == SDL_KEYDOWN )
	{
		switch( input.key.keysym.sym )
		{
		case SDLK_RIGHT:
			{
				demon.SetState(Demon::State::MOVING_RIGHT);
				if( demon.isHit == false && demon.DieOneLife == false && 
					demon.isCrouching == false && demon.isKicking == false 
					&& demon.isPunching == false && demon.isJumping == false && demon.yPos == GROUND_Y )
				{	
					demon.isMovingRight = true;
				}
				break;
			}
		case SDLK_LEFT:
			{
				demon.SetState(Demon::State::MOVING_LEFT);
				if( demon.isHit == false && demon.DieOneLife == false && 
					demon.isCrouching == false && demon.isKicking == false 
					&& demon.isPunching == false && demon.isJumping == false && demon.yPos == GROUND_Y )
				{
					demon.isMovingLeft = true;
				}
				break;
			}
		case SDLK_UP:
			{
				if( !demon.isJumping )
				{
					demon.JumpingSpeed = 20;
					demon.SetState(Demon::State::JUMPING);
				}

				if( demon.isHit == false && demon.DieOneLife == false && 
					demon.isCrouching == false && demon.isPunching == false 
					&& demon.isKicking == false && demon.isJumping == false 
					&& demon.yPos == GROUND_Y )
				{
					demon.isJumping = true;
				}
				break;
			}
		case SDLK_DOWN:
			{
				demon.SetState(demon.CROUCHING);
				if( demon.isHit == false && demon.DieOneLife == false && 
					demon.isJumping == false && demon.isKicking == false && demon.isPunching == false 
					&& demon.isMovingLeft == false && demon.isMovingRight == false )
				{
					demon.isCrouching = true;
				}
				break;
			}
		case SDLK_SPACE:
			{
				demon.SetState(Demon::State::KICKING);
				if( demon.isHit == false && demon.DieOneLife == false && 
					demon.isPunching == false  
				 && demon.isKicking == false && demon.isJumping == false && demon.yPos == GROUND_Y )
				{
					if( demon.DemonHunter == true )
					{
						
						if( timer.Timer_FireBall > 15 )
						{
							timer.Timer_FireBall = 0;
							demon.isKicking = true;
							break;
						}

						break;
					}

					demon.isKicking = true;
					Audio.PlaySoundEffect( SOUND_HIT );
											
					break;
				}
				break;
			}
		case SDLK_LALT:
			{
				demon.SetState(Demon::State::PUNCHING);
				if( demon.isHit == false && demon.DieOneLife == false && 
					demon.isPunching == false 
				 && demon.isKicking == false && demon.isJumping == false && 
				 demon.Triangle == false && demon.TriangleAttack == false && demon.yPos == GROUND_Y )
				{
					if( demon.SmallHunter )
					{ 
						demon.isPunching = true; 
						Audio.PlaySoundEffect( SOUND_HIT );
					}
					else
					{ 
						if( timer.Timer_TriangleAttackOK > 15 )
						{
							timer.Timer_TriangleAttackOK = 0;
							demon.TriangleAttack = true; 
							Audio.PlaySoundEffect( SOUND_FIRE );
						}

					}
				}
				break;
			}
		default:
			{
				break;
			}
		}
	}
	else
	{
		demon.SetState(Demon::State::IDLE);
	}
	// if intro checks mouseposition and checks for presses
	if( gamestate.GameCondition == GS_INTRO )
	{
		// checks for mouseMotion
		if( input.type == SDL_MOUSEMOTION || input.type == SDL_MOUSEBUTTONDOWN )
		{
			int x, y;
			x = input.button.x;
			y = input.button.y;

			// check if mouse is over something
			if( gamestate.TitleScreen->ButtonOptions == true )
			{
				for( int i = 4; i < 8; i++ )
				{
					if( x > gamestate.TitleScreen->DestClips[ i ].x && 
					x < gamestate.TitleScreen->DestClips[ i ].x + gamestate.TitleScreen->DestClips[ i ].w &&
					y > gamestate.TitleScreen->DestClips[ i ].y &&
					y < gamestate.TitleScreen->DestClips[ i ].y + gamestate.TitleScreen->DestClips[ i ].h )
					
					if( input.type == SDL_MOUSEBUTTONDOWN )
					{
						switch( i )
						{
						case 4:
							gamestate.TitleScreen->ButtonHighScore = true;
							gamestate.CheckingHighScore = true;
							break;
						case 5:
							gamestate.TitleScreen->ButtonCredits = true;
							break;
						case 6:
							gamestate.TitleScreen->ButtonSound = true;
							break;
						case 7:
							gamestate.TitleScreen->ButtonBack = true;
							break;

						}
					}
				}

			}
			else
			{
				// checks if musebutton is pressed at newgame, options or quit
				for( int i = 0; i < 3; i++ )
				{
					if( x > gamestate.TitleScreen->DestClips[ i ].x && 
						x < gamestate.TitleScreen->DestClips[ i ].x + gamestate.TitleScreen->DestClips[ i ].w &&
						y > gamestate.TitleScreen->DestClips[ i ].y &&
						y < gamestate.TitleScreen->DestClips[ i ].y + gamestate.TitleScreen->DestClips[ i ].h )
					{
						if( input.type == SDL_MOUSEBUTTONDOWN )
						{
							switch( i )
							{
							case 0:
								gamestate.TitleScreen->ButtonNewgame = true;
								break;
							case 1:
								gamestate.TitleScreen->ButtonOptions = true;
								break;
							case 2:
								gamestate.TitleScreen->ButtonQuit = true;
								break;
							}
						}
					}					
				}
			}
		}


	}
	else
	{
		return;
	}
}

Game::Game()
{
	//initialize all SDL subystems
	if( SDL_Init( SDL_INIT_EVERYTHING ) == -1)
	{
		cout << "SDL INIT FAILED..." << endl;
		SDL_Quit();
	}
	else
	{
		cout << "SDL_INIT_EVERYTHING..." << endl;
	}

	if( TTF_Init() == -1 )
	{
		cout << "TTF_INIT FAILED..." << endl;
	}
	else
	{
		gamestate.font = TTF_OpenFont( "cour.ttf", 28 );
	}

	// Setup of the application icons
	SDL_WM_SetCaption("", "res/big.ico");
	SDL_WM_SetIcon(SDL_LoadBMP("res/small.bmp"), NULL);

	gamestate.GameOK = true;
	gamestate.GameOK = Init( gamestate.screen );
	
	// h 100 w 130
	//demon.InitiateDemon( demon.DemonSurface, GROUND_X, GROUND_Y, DEMONHEIGHT, DEMONWIDTH ); 

	gamestate.load_files();
	demon.Set_clips();
}

// loads all graphic files and all new files and the font
void Gamestate::load_files()
{	
	m_srfCity = Gfx.Load_imageAlpha( "Graphics\\srfCity.png", 0, 0, 0 );
	m_srfClouds = Gfx.Load_imageAlpha( "Graphics\\srfClouds.png", 0, 0, 0 );
	m_srfBlack = Gfx.Load_imageAlpha( "Graphics\\srfBlack.png", 0, 0, 0 );
	m_srfSky = Gfx.Load_imageAlpha( "Graphics\\srfSky.png", 0, 0, 0 );
	m_srfTrees = Gfx.Load_imageAlpha( "Graphics\\srfTrees.png", 2, 2, 2 );
	demon.DemonSurface = Gfx.Load_imageAlpha( "Graphics\\DemonSurface.png", 255, 255, 255 );
	m_srfEnemyZombie = Gfx.Load_imageAlpha( "Graphics\\srfEnemyZombie.png", 106, 76, 48 );
	m_srfSkeleton = Gfx.Load_imageAlpha( "Graphics\\srfSkeleton.png", 106, 76, 48  );
	m_srfCrow = Gfx.Load_imageAlpha( "Graphics\\srfCrow.png", 255, 255, 255 );
	m_srfCoffin  = Gfx.Load_imageAlpha( "Graphics\\srfCoffin.png", 97, 68, 43 );
	m_srfBoss = Gfx.Load_imageAlpha( "Graphics\\srfBoss.png", 255, 255, 255 );
	m_srfDemonLife = Gfx.Load_imageAlpha( "Graphics\\srfDemonLife.png", 255, 255, 255 );
	m_srfDemonHealthAndFire = Gfx.Load_imageAlpha( "Graphics\\srfDemonHealthAndFire.png", 0, 0, 0 );
	m_srfDragon = Gfx.Load_imageAlpha( "Graphics\\srfDragon.png", 0, 0, 0 );
	m_srfStart = Gfx.Load_imageAlpha( "Graphics\\srfStart.png", 237, 234, 214 );
	m_srfButtons = Gfx.Load_imageAlpha( "Graphics\\srfButtons.png", 255, 255, 255 );
	m_srfIntro = Gfx.Load_imageAlpha( "Graphics\\srfIntro.png", 255, 255, 255 );
	m_srfMorphing = Gfx.Load_imageAlpha( "Graphics\\srfMorphing.png", 255, 255, 241 );
	m_srfReaper = Gfx.Load_imageAlpha( "Graphics\\srfReaper.png", 255, 255, 255 );
	m_srfOutro = Gfx.Load_imageAlpha( "Graphics\\srfOutro.png", 255, 255, 255 );
	m_srfButton = Gfx.Load_imageAlpha( "Graphics\\srfButton.png", 0, 0, 0 );

	m_srfCity = Load_imageAlpha( "Graphics\\srfCity.png", 0, 0, 0 );
	m_srfClouds = Load_imageAlpha( "Graphics\\srfClouds.png", 0, 0, 0 );
	m_srfBlack = Load_imageAlpha( "Graphics\\srfBlack.png", 0, 0, 0 );
	m_srfSky = Load_imageAlpha( "Graphics\\srfSky.png", 0, 0, 0 );
	m_srfTrees = Load_imageAlpha( "Graphics\\srfTrees.png", 2, 2, 2 );
	demon.DemonSurface = Load_imageAlpha( "Graphics\\DemonSurface.png", 255, 255, 255 );
	m_srfEnemyZombie = Load_imageAlpha( "Graphics\\srfEnemyZombie.png", 106, 76, 48 );
	m_srfSkeleton = Load_imageAlpha( "Graphics\\srfSkeleton.png", 106, 76, 48  );
	m_srfCrow = Load_imageAlpha( "Graphics\\srfCrow.png", 255, 255, 255 );
	m_srfCoffin  = Load_imageAlpha( "Graphics\\srfCoffin.png", 97, 68, 43 );
	m_srfBoss = Load_imageAlpha( "Graphics\\srfBoss.png", 255, 255, 255 );
	m_srfDemonLife = Load_imageAlpha( "Graphics\\srfDemonLife.png", 255, 255, 255 );
	m_srfDemonHealthAndFire = Load_imageAlpha( "Graphics\\srfDemonHealthAndFire.png", 0, 0, 0 );
	m_srfDragon = Load_imageAlpha( "Graphics\\srfDragon.png", 0, 0, 0 );
	m_srfStart = Load_imageAlpha( "Graphics\\srfStart.png", 237, 234, 214 );
	m_srfButtons = Load_imageAlpha( "Graphics\\srfButtons.png", 255, 255, 255 );
	m_srfIntro = Load_imageAlpha( "Graphics\\srfIntro.png", 255, 255, 255 );
	m_srfMorphing = Load_imageAlpha( "Graphics\\srfMorphing.png", 255, 255, 241 );
	m_srfReaper = Load_imageAlpha( "Graphics\\srfReaper.png", 255, 255, 255 );
	m_srfOutro = Load_imageAlpha( "Graphics\\srfOutro.png", 255, 255, 255 );
	m_srfButton = Load_imageAlpha( "Graphics\\srfButton.png", 0, 0, 0 );

	gamestate.CreateNewThings();

	for( int i = 0; i < 4; i++ )
	{
		MorphingPics[ i ].x = i * SCREEN_WIDTH;
		MorphingPics[ i ].y = 0;
		MorphingPics[ i ].h = SCREEN_HEIGHT;
		MorphingPics[ i ].w = SCREEN_WIDTH;
	}

	setUpParallaxLayers();
}

void Gamestate::CreateNewThings()
{
	Dragon = new DancingDragon( m_srfDragon );
	TitleScreen = new FirstScreen( 290,  m_srfStart, m_srfButtons ); 
	Intro = new IntroTalk( gamestate.m_srfIntro );
	name = new StringInput();
	ListHighScore = new FillHighScore();
}


// ----------------------------------------------------------------------------
// findAvailableIndex() - find free slot in surface list and return its index
// ----------------------------------------------------------------------------
int Gamestate::findAvailableIndex()
{
	for( int i = 0; i < MAX_SURFACE; i++ )
	{
		if( !m_surfaceList[i] )
		{
			return i;
		}
	}

	// None available
	return -1;
}

// ----------------------------------------------------------------------------
// DrawObjects() - Draws all objects, coffins, health etc.
// ----------------------------------------------------------------------------
void Gamestate::DrawObjects()
{
	if( gamestate.GameCondition == GS_LEVEL1BOSS )
	{
		Control_OBJ.DrawObjects();
	}
	else
	{
		AnimalController.Draw_Animals();
		Control_ENEMY.Draw_Enemies();
		Control_OBJ.DrawObjects();
	}	
}
// ----------------------------------------------------------------------------
// CreateBoss() - Creates the boss gives collisionCircle and pos
// ----------------------------------------------------------------------------
Boss * Gamestate::CreateBoss( int xPos, int yPos, int surface )
{
	Boss * temp = new Boss;
	temp->surface = surface;
	temp->xPos = xPos;
	temp->yPos = yPos;

	temp->Radius = ( temp->BossWidth > temp->BossHeight ) ? temp->BossWidth / 2 : temp->BossHeight  / 2;

	return temp;
}

// ----------------------------------------------------------------------------
// MorphMyDude - does the morphing sequence
// ----------------------------------------------------------------------------
void Gamestate::MorphMyDude()
{
	Audio.PlaySoundEffect( SOUND_MORPH );

	int State = 3;
	SDL_Rect destRect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
	while( State != -1 )
	{
		
		DrawBackgroundBlack();
	
		switch( State )
		{
		case 0:
			{
				if( timer.Timer_MorphPics > 10 )
				{
					SDL_BlitSurface(	m_surfaceList[ m_srfMorphing ], &MorphingPics[ State ], 
										gamestate.BackBuffer, &destRect );
					State--;
					timer.Timer_MorphPics = 0;
				}
				else
				{
					SDL_BlitSurface(	m_surfaceList[ m_srfMorphing ], &MorphingPics[ State ], 
					gamestate.BackBuffer, &destRect );
					timer.Timer_MorphPics++;
				}

				break;
			}
		case 1:
			{
				if( timer.Timer_MorphPics > 10 )
				{
					SDL_BlitSurface(	m_surfaceList[ m_srfMorphing ], &MorphingPics[ State ], 
										gamestate.BackBuffer, &destRect );
					State--;
					timer.Timer_MorphPics = 0;
				}
				else
				{
					SDL_BlitSurface(	m_surfaceList[ m_srfMorphing ], &MorphingPics[ State ], 
					gamestate.BackBuffer, &destRect );
					timer.Timer_MorphPics++;
				}

				break;
			}
		case 2:
			{
				if( timer.Timer_MorphPics > 10 )
				{
					SDL_BlitSurface(	m_surfaceList[ m_srfMorphing ], &MorphingPics[ State ], 
										gamestate.BackBuffer, &destRect );
					State--;
					timer.Timer_MorphPics = 0;
				}
				else
				{
					SDL_BlitSurface(	m_surfaceList[ m_srfMorphing ], &MorphingPics[ State ], 
					gamestate.BackBuffer, &destRect );
					timer.Timer_MorphPics++;
				}
				break;
			}
		case 3:
			{
				if( timer.Timer_MorphPics > 10 )
				{
					SDL_BlitSurface(	m_surfaceList[ m_srfMorphing ], &MorphingPics[ State ], 
										gamestate.BackBuffer, &destRect );
					State--;
					timer.Timer_MorphPics = 0;
				}
				else
				{
					SDL_BlitSurface(	m_surfaceList[ m_srfMorphing ], &MorphingPics[ State ], 
					gamestate.BackBuffer, &destRect );
					timer.Timer_MorphPics++;
				}
				break;
			}
		}
		gamestate.FLIP();
	}

	gamestate.GameCondition = GS_LEVEL1;

	// safety Check
	demon.SmallHunter = false;
	demon.DemonHunter = true;
	demon.isJumping = false;
	demon.isPunching = false;
	demon.isCrouching = false;
	demon.isKicking = false;
	//demon.Crouch = false;
	//demon.Kick = false;
	//demon.Punch = false;
	//demon.Jump = false;
	demon.isMovingLeft = false;
	demon.isMovingRight = false;
	demon.FireBall = false;
	demon.CrouchFireBall = false;
}

// ----------------------------------------------------------------------------
// DrawBackgroundBlack - draws a background black with the size of screen
// ----------------------------------------------------------------------------
void Gamestate::DrawBackgroundBlack()
{
	if( gamestate.GameCondition == GS_OUTRO )
	{
		SDL_FillRect(gamestate.BackBuffer, NULL, SDL_MapRGB(gamestate.BackBuffer->format, 0,0,0));
		/*
		ParallaxLayer  * MyParaBackGround;
		MyParaBackGround = gamestate.Paralax->getLayer( gamestate.m_srfBlack );

		SDL_Rect scRect = { 0, 0,	100, 70 };
									
		SDL_Rect dtRect = {	600, 530, 100, 50 };

		SDL_BlitSurface( gamestate.GetSurface( gamestate.m_srfBlack ), &scRect, gamestate.BackBuffer, &dtRect ); 
		*/
	}
	else
	{
		SDL_FillRect(gamestate.BackBuffer, NULL, SDL_MapRGB(gamestate.BackBuffer->format, 0,0,0));
		/*
		ParallaxLayer  * MyParaBackGround;
		MyParaBackGround = gamestate.Paralax->getLayer( gamestate.m_srfBlack );

		SDL_Rect scRect = { 0, 0,	MyParaBackGround->m_width, 
									600 };

		SDL_Rect dtRect = {	0, 0, MyParaBackGround->m_width, 600 };

		SDL_BlitSurface( gamestate.GetSurface( gamestate.m_srfBlack ), &scRect, gamestate.BackBuffer, &dtRect );
		*/
	}
}

void Gamestate::ResetBoss()
{
	if( gamestate.boss != NULL )
	{
		delete gamestate.boss;
	}
}

void Gamestate::ResetEnemies()
{
	if( Control_ENEMY.My_Enemies.size() != NULL )
	{
		Control_ENEMY.My_Enemies.clear();
	}
}

// ----------------------------------------------------------------------------
// ResetObjects() - resets all object to its starting values
// ----------------------------------------------------------------------------
void Gamestate::ResetObjects()
{
	if( Control_OBJ.List_Coffins.size() != NULL )
	{
		Control_OBJ.List_Coffins.clear();
	}

	if( Control_OBJ.List_FireBalls.size() != NULL )
	{
		Control_OBJ.List_FireBalls.clear();
	}

	if( Control_OBJ.List_PowerUps.size() != NULL )
	{
		Control_OBJ.List_PowerUps.clear();
	}

	Control_OBJ.FrameHealth = 0;
	Control_OBJ.PowerUpMan = false;
	if( Control_OBJ.WereWolf != NULL )
	{
		delete Control_OBJ.WereWolf;
		Control_OBJ.WereWolf = NULL;
	}

	Control_OBJ.WhichLifeToShow = 0;
	
	return;
}

// ----------------------------------------------------------------------------
// ResetPlayer() - resets the player to its starting values
// ----------------------------------------------------------------------------
void Gamestate::ResetPlayer()
{
	demon.isMovingRight = false;
	demon.isMovingLeft = false;
	demon.isJumping = false;
	demon.isCrouching = false;
	demon.isKicking = false;
	demon.isPunching = false;
	demon.CrouchFireBall = false;
	demon.Right = false;
	demon.Left = false;
	demon.isImmortal = false;
	

	demon.isHit = false; 
	demon.DieOneLife = false;

	demon.SmallHunter = true;
	demon.MediumHunter = false;
	demon.LargeHunter = false;
	demon.DemonHunter = false;

	demon.LifeFull_Small = true;
	demon.LifeMedium_Small = false;
	demon.LifeLittle_Small = false;

	demon.Demon_Dead = false;

	//demon.Crouch = false;
	//demon.Kick = false; 
	//demon.Jump = false;
	//demon.Punch = false;
	demon.FireBall = false;
	demon.CrouchFire = false;
	demon.TriangleAttack = false;
	demon.Triangle = false;
	demon.isGettingUp = false;

	demon.KickRight = 5;
	demon.KickLeft = 28;
	demon.WhereWalkLeft = 24;
	demon.WhereWalkRight = 0;
	demon.WhereJumpLeft = 35;
	demon.WhereJumpRight = 12;
	demon.CrouchRight = 19;
	demon.CrouchLeft = 41;
	demon.PunchRight = 8; 
	demon.PunchLeft = 31;

	demon.WalkLeft_Demon = 22;
	demon.WalkRight_Demon = 1;
	demon.FireBallRight_Demon = 9;
	demon.FireBallLeft_Demon = 30;
	demon.JumpRight_Demon = 5;
	demon.JumpLeft_Demon = 26;
	demon.CrouchRightFire = 14;
	demon.CrouchLeftFire = 35;
	demon.FireBallRight = 39;
	demon.FireBallLeft = 42;

	demon.AlphaImmortal = SDL_ALPHA_OPAQUE;

	demon.Demon_Life = 2;

	demon.TriangleFireLeft = 34;
	demon.TriangleFireRight = 13;

	demon.DieRightDemon = 44;
	demon.DieLeftDemon = 40;

	demon.DieState = 0;

	demon.Radius = 0;
	demon.RadiusFist = 0;
	demon.RadiusFeet = 0;
	demon.Demon_Health = 100;

	demon.Feet_W = 10;
	demon.Feet_H = 10;
	demon.Fist_W = 15;
	demon.Fist_H = 15;

	demon.WhereIsEnd = 0;
	demon.TriangleState = 0;
}

// ----------------------------------------------------------------------------
// Update() - Updates the whole game depending on which state it is in
// ----------------------------------------------------------------------------
void Game::upDate( SDL_Event input )
{
	gamestate.LevelProgress++;
	cout << gamestate.LevelProgress << endl;
	demon.UpdateEndPosition();

	if( demon.isImmortal )
	{
		//start countdown to vulnerability
		timer.Timer_Immortal++;
		if( timer.Timer_Immortal > 120 )
		{
			timer.Timer_Immortal = 0;
			demon.isImmortal = false;
		}
	}
	// WhereIsEnd is @ image width + screenwidth 800+5100
	//if( demon.WhereIsEnd >= 5700 ) 
	if( gamestate.LevelProgress >= 6100 )
	{
		gamestate.GameCondition = GS_LEVEL1BOSS;
		demon.WhereIsEnd = 0;
		Audio.PlaySoundEffect( SOUND_BOSS );
	}

		// Check game state
	switch( gamestate.GameCondition )
	{

		// Intro sequence
		case GS_INTRO:
			{
				Audio.PlayIntroSong();
				Handle_events( input );
				gamestate.MainScreen();
				break;
			}
		case GS_ENTERNAME:
			{
				Audio.PlayIntroSong();
				gamestate.EnterName();

				break;
			}	
		case GS_INTROSTORY:
			{
				Audio.PlayIntroSong();
				gamestate.DoIntroTalk();
				gamestate.FLIP();
				break;
			}
		case GS_LOADING:
			{
				Audio.PauseMusic();
				gamestate.Loading();
				break;
			}
		case GS_MORPH:
			{
				Audio.PauseMusic();
				gamestate.MorphMyDude();
				break;
			}

		// Level1 Loop
		case GS_LEVEL1:
			{
				if( Audio.MusicPaused == true )
				{
					Audio.UnpauseMusic();
				}

				if( Audio.LevelSong == false )
				{
					Audio.PlayMusic( 0 );
				}

				// handles events what the user does with the character
				Handle_events( input );
				
				//draws layers
				gamestate.drawParallaxLayers();
				gamestate.DrawObjects();
				gamestate.DrawSprite();
				
				gamestate.DrawAllText();
				
				gamestate.FLIP();
			
				break;
			}
		case GS_LEVEL1BOSS:
			{
				if( Audio.MusicPaused == true )
				{
					Audio.UnpauseMusic();
				}

				// handles events what the user does with the character
				Handle_events( input );

				//draws layers
				gamestate.drawParallaxLayers();
				gamestate.DrawObjects();
				gamestate.DrawBoss();
				gamestate.DrawAllText();
				gamestate.DrawSprite();
				gamestate.FLIP();

				if( gamestate.boss->BossDead == true )
				{
					gamestate.GameCondition = GS_OUTRO;
				}
				break;
			}

		case GS_OUTRO:
			{
				Audio.PauseMusic();
				Audio.PlayMusic( 2 );
				gamestate.PlayOutro();
				break;
			}
		case GS_DEAD:
			{
				Audio.PauseMusic();
				Audio.PlaySoundEffect( SOUND_DIE );
				gamestate.PlayerDied();
				break;
			}
	}
	
}

// ----------------------------------------------------------------------------
// PlayOutro() - Plays the whole outro sequence
// ----------------------------------------------------------------------------
void Gamestate::PlayOutro()
{	
	demon.isKicking = false;
	demon.isJumping = false;
	demon.isCrouching = false;
	demon.isPunching = false;
	demon.CrouchFireBall = false;
	demon.TriangleAttack = false;
	demon.isMovingRight = true;
	//demon.Crouch = false;
	demon.CrouchFire = false;
	demon.CrouchFireBall = false;

	demon.UpdatePlayer();

	int IntroState = 0;
	int Letter = 0, LetterWidth = 0, Line = 0;
	string FinishLine = " There is Still Work To Be Done, This Autumn The Battle Continues In Hell, Do You Have What It Takes To Open A Can Of Whoopassss On The Devil ";
	string FinishSlow[ 7 ];
	int Counter = 0;
	demon.xPos = 530;
	SDL_Surface * FinishSurface;

	bool Walk = true;

	SDL_Color textColor = { 255, 255, 255 };

	float speedJumpDemon = 0.0;

	bool JumpDown = false, JumpUp = true;

	Timer speed;
	speed.start();

	SDL_Rect srcRect = { 0, 0, 800, 600 };
	SDL_Rect destRect = { 0, 0, 800, 600 };

	bool Finish = false;
	while( Finish != true )
	{
		while( speed.get_ticks() < 1000 / 40 )
        {
            //wait    
			
        }
		
		switch( IntroState )
		{
		speedJumpDemon += 100.0f * ( gamestate.dt / 1000 );

		case 0:
			{
				if( demon.xPos > 580 )
				{
					Walk = false;
					if( JumpUp ) 
					{
						//demon.yPos -= abs( 20 * cos( speedJumpDemon ) );
						//demon.xPos += abs( 5 * cos( speedJumpDemon ) );
						//demon.xVel += abs( 5 * cos( speedJumpDemon ) );
						//demon.yVel -= abs( 20 * cos( speedJumpDemon ) );
						/*
						if( demon.yPos < GROUND_Y - 100 )
						{
							JumpUp = false;
							JumpDown = true;
						}*/
					}
					else if( JumpDown )
					{
						//demon.yPos += abs( 10 * cos( speedJumpDemon ) );
						//demon.yVel += abs( 10 * cos( speedJumpDemon ) );
						if( demon.yPos > GROUND_Y + 80 )
						{
							IntroState = 1;
						}
					}
					
				}
				else if( demon.xPos < 580 && Walk == true )
				{
    					demon.isMovingRight = true;
				}

				gamestate.drawParallaxLayers();
				gamestate.DrawSprite();
				gamestate.DrawBackgroundBlack();
				break;
			}
		case 1:
			{
				SDL_BlitSurface(	gamestate.m_surfaceList[ gamestate.m_srfOutro ],
									&srcRect, gamestate.BackBuffer, &destRect );
				if( Counter > 2 )
				{
					Counter = 0;
					
					LetterWidth++;

					if( Letter < FinishLine.length() )
					{
						Letter++;
					}
					else
					{
						IntroState = 2;
					}

					FinishSlow[ Line ] += FinishLine[ Letter ];
					for( int i = 0; i < 7; i++ )
					{
						FinishSurface = TTF_RenderText_Solid( gamestate.font, FinishSlow[ i ].c_str(), textColor );
						gamestate.apply_surface( 300, i * 40, FinishSurface, gamestate.BackBuffer );
					}
				}

				for( int i = 0; i < 7; i++ )
				{
					FinishSurface = TTF_RenderText_Solid( gamestate.font, FinishSlow[ i ].c_str(), textColor );
					gamestate.apply_surface( 300, i * 40, FinishSurface, gamestate.BackBuffer );
				}
				
				if( LetterWidth > 20 )
				{
					LetterWidth = 0;
					Line++;
				}

				Counter++;
				break;
			}
		case 2:
			{
				Finish = true;
				break;
			}
		}
		gamestate.FLIP();

	}
	gamestate.GameCondition = GS_INTRO;
}

// ----------------------------------------------------------------------------
// DoIntroTalk() - does the intro where all the talking takes place.
// ----------------------------------------------------------------------------
void Gamestate::DoIntroTalk()
{
	Intro->DoTalk();
}

// ----------------------------------------------------------------------------
// PlayerDied() - Plays the whole death sequence
// ----------------------------------------------------------------------------
void Gamestate::PlayerDied()
{
	ListHighScore->sort( gamestate.name->str.c_str(), demon.Score );
	ListHighScore->Save();

	int PlayerDieState = 0;
	outro = new Outro;
	int Letter = 0;
	int LetterWidth = 0;
	int Line = 0;
	int Counter = 0;
	string DeathTalks = " Your sole is mine fallen one, At this rate all the souls of the earth will be mine soon!!! Ha Ha Ha ";
	string DeathTalkSlow[ 8 ];

	SDL_Color textColor = { 255, 255, 255 };
	SDL_Event input;

	SDL_Color Stone = { 105, 105, 136 };
	SDL_Color StoneFront = { 0, 0, 0, 255 };

	bool PlayDeadAnimation = true;
	while( PlayDeadAnimation == true )
	{
		SDL_PollEvent( &input );
		if( input.type == SDL_KEYDOWN )
		{
			switch( input.key.keysym.sym )
			{
			case SDLK_SPACE:
				PlayerDieState = 2;
				break;
			}
		}

		DrawBackgroundBlack();
		gamestate.DrawAllText();
		switch( PlayerDieState )
		{
		case 0:
			{
				if( timer.Timer_ShowDead > 10 )
				{
					PlayerDieState = 1;
					timer.Timer_ShowDead = 0.0f;
				}

				timer.Timer_ShowDead++;
				SDL_BlitSurface(	m_surfaceList[ outro->surface ], &outro->ClipsOutro[ 0 ],
									gamestate.BackBuffer, &outro->ClipsOutro[ 1 ] );
				break;
			}
		case 1:
			{
				if( Counter > 5 )
				{
					Counter = 0;

					if( Letter < DeathTalks.length() )
					{
						Letter++;
					}
					else
					{
						PlayerDieState = 2;
					}

					DeathTalkSlow[ Line ] += DeathTalks[ Letter ];
					for( int i = 0; i < 8; i++ )
					{
						DeathSurface[ 0 ] = TTF_RenderText_Solid( gamestate.font, DeathTalkSlow[ i ].c_str(), textColor );
						gamestate.apply_surface( 300, i * 40, DeathSurface[ 0 ], gamestate.BackBuffer );
					}

					LetterWidth++;
				}
						
				SDL_BlitSurface(	m_surfaceList[ outro->surface ], &outro->ClipsOutro[ 0 ],
									gamestate.BackBuffer, &outro->ClipsOutro[ 1 ] );

				for( int i = 0; i < 8; i++ )
				{
					DeathSurface[ 0 ] = TTF_RenderText_Solid( gamestate.font, DeathTalkSlow[ i ].c_str(), textColor );
					gamestate.apply_surface( 300, i * 40, DeathSurface[ 0 ], gamestate.BackBuffer );
				}
				
					
				if( LetterWidth > 20 )
				{
					LetterWidth = 0;
					Line++;
				}

				Counter++;
				break;
			}
		case 2:
			{
				PlayDeadAnimation = false;
				break;
			}	
		}
		
		DeathSurface[ 1 ] = TTF_RenderText_Blended( gamestate.font, gamestate.name->str.c_str(), StoneFront );
		gamestate.apply_surface( 160, 450, DeathSurface[ 1 ], gamestate.BackBuffer );
		gamestate.FLIP();
	}

	gamestate.GameCondition = GS_INTRO;

}

// ----------------------------------------------------------------------------
// DrawAllText() - draws all text thats currently shown on the screen.
// ----------------------------------------------------------------------------
void Gamestate::DrawAllText()
{
	if( GameCondition == GS_INTROSTORY || GameCondition == GS_DEAD )
	{
		if( timer.Timer_Color > 2 )
		{
			timer.Timer_R += 20 + rand() % 100;
			timer.Timer_G += ( rand() % 6 )* ( rand() % 40 ); 
			timer.Timer_B += ( rand() % 10 ) * ( rand() * 20 );
			timer.Timer_Color = 0;
		}
			timer.Timer_Color++;

		//The color of the font
		SDL_Color textColor = { timer.Timer_R, timer.Timer_G, timer.Timer_B };
		SDL_Color textColor2 = { 0, 0, 0 };
		
		if( GameCondition == GS_DEAD )
		{
			sprintf_s( gamestate.Text, 256, " Press Space For Menu " );
			gamestate.textIntro = TTF_RenderText_Shaded( gamestate.font, gamestate.Text, textColor, textColor2 );
			gamestate.apply_surface( 200, 500, gamestate.textIntro, gamestate.BackBuffer ); 
		}
		else
		{
			sprintf_s( gamestate.Text, 256, " Press Space To Start " );		
			gamestate.textIntro = TTF_RenderText_Shaded( gamestate.font, gamestate.Text, textColor, textColor2 );
			gamestate.apply_surface( 200, 500, gamestate.textIntro, gamestate.BackBuffer ); 
		}
	}
	else
	{
		//The color of the font
		SDL_Color textColor = { 251, 245, 32 };

		// print out the score
		sprintf_s(gamestate.Text, 256, "Score: %i ", demon.Score );		
		gamestate.srfText = TTF_RenderText_Solid( gamestate.font, Text, textColor );
		gamestate.apply_surface( 600, 20, gamestate.srfText, gamestate.BackBuffer ); 
	}
}

// ----------------------------------------------------------------------------
// Loading() - draws the loading screen on the screen. Dragon dancing
// ----------------------------------------------------------------------------
void Gamestate::Loading()
{
	//currentAnimFrame += deltaTime * animFramesPerSecond;
	SDL_SetAlpha( m_surfaceList[  Dragon->surface ], SDL_SRCALPHA, 255 );

	float Speed = 1000.0f * ( dt / 1000.0f );
	SDL_Rect dstRect = { Dragon->xPos, Dragon->yPos, Dragon->Width, Dragon->Height };
	if ( gamestate.IntroDone == false )
	{
					SDL_BlitSurface(	m_surfaceList[ Dragon->surface ], &Dragon->Clips[ Dragon->Frame ],
								gamestate.BackBuffer, &dstRect );
			//Dragon->PrevFrame = Dragon->Frame;
			Dragon->SetFrame();		
			/*
		if( timer.Timer_Dancing >= (1000.f /dt) / 60 )
		{
			timer.Timer_Dancing = 0;
			
			SDL_BlitSurface(	m_surfaceList[ Dragon->surface ], &Dragon->Clips[ Dragon->Frame ],
								gamestate.BackBuffer, &dstRect );
			Dragon->PrevFrame = Dragon->Frame;
			Dragon->SetFrame();		
		}
		else
		{
			SDL_BlitSurface(	m_surfaceList[ Dragon->surface ], &Dragon->Clips[ Dragon->PrevFrame ],
								gamestate.BackBuffer, &dstRect );
			timer.Timer_Dancing++;
		}*/
		
	}
}

// ----------------------------------------------------------------------------
// DrawBoss() - hmmm.
// ----------------------------------------------------------------------------
void Gamestate::DrawBoss()
{
	gamestate.boss->UpdateBoss();
}

// ----------------------------------------------------------------------------
// GetSurface() - hmmm.
// ----------------------------------------------------------------------------
SDL_Surface * Gamestate::GetSurface( int WhichSurface )
{
	return m_surfaceList[ WhichSurface ];	
}

// ----------------------------------------------------------------------------
// GetSurface() - gives backbuffer to destination buffer streches and all
// ----------------------------------------------------------------------------
void Gamestate::FLIP()
{
	SDL_Rect srcRect = { 0, 0, gamestate.BackBuffer->w, gamestate.BackBuffer->h };
	SDL_Rect destRect = {	0, 0, gamestate.SCREEN_WIDTH, gamestate.SCREEN_HEIGHT };
					
	gamestate.PasteScreenToAnother( srcRect, destRect );

	//flips screen
	if( SDL_Flip( gamestate.screen ) == -1)
	{
		gamestate.GameOK = false;
	}
}

// ----------------------------------------------------------------------------
// MainScreen() - Draws the mainscreen, checks conditions. MenuScreen
// ----------------------------------------------------------------------------
void Gamestate::MainScreen()
{
	SDL_Surface * Surface_Credits = NULL;
	SDL_Surface * Surface_HighScore = NULL;

	ParallaxLayer  * MyParaBackGround;
	MyParaBackGround = Parallax->getLayer( TitleScreen->surface );

	SDL_Rect scRect = { 0, 0, 800, 600 };

	SDL_Rect dtRect = {	0, 0, 800, 600 };

	SDL_BlitSurface( gamestate.GetSurface( TitleScreen->surface ), &scRect, gamestate.BackBuffer, &dtRect ); 
	if( CheckingHighScore == true )
	{
		ListHighScore->sort( gamestate.name->str.c_str(), demon.Score );
		ListHighScore->Save();

		delete ListHighScore;
		ListHighScore = new FillHighScore;
		CheckingHighScore = false;

		for( int i = 0; i < 5; i++ )
		{
			char temp[ 256 ];
			sprintf_s(	temp, 256, "Name: %s Score: %i", ListHighScore->list[ i ].name.c_str(), ListHighScore->list[ i ].Score );	
			gamestate.HighScoreList[ i ] = temp;
		}
	}
	
	if( TitleScreen->ButtonOptions == false )
	{
		for( int i = 0; i < 4; i++ )
		{
			//SDL_FillRect(gamestate.GetSurface( TitleScreen->SurfaceButt),&TitleScreen->DestClips[ i ],SDL_MapRGB(gamestate.GetSurface( TitleScreen->SurfaceButt)->format,255,0,255) );
			
			SDL_BlitSurface(	gamestate.GetSurface( TitleScreen->SurfaceButt ), 
								&TitleScreen->ButtonClips[ i ],
								gamestate.BackBuffer, &TitleScreen->DestClips[ i ] ); 
		}
	}
	else if( TitleScreen->ButtonHighScore == true )
	{
		SDL_BlitSurface(	gamestate.GetSurface( TitleScreen->SurfaceButt ),
							&TitleScreen->ButtonClips[ 4 ],
							gamestate.BackBuffer, &TitleScreen->DestClips[ 8 ] ); 	

		SDL_BlitSurface(	gamestate.GetSurface( TitleScreen->SurfaceButt ),
							&TitleScreen->ButtonClips[ 7 ],
							gamestate.BackBuffer, &TitleScreen->DestClips[ 7 ] ); 

		SDL_Color textColor = { 255,255,255 };
		
		for( int i = 0; i < 5; i++ )
		{
			Surface_HighScore =		TTF_RenderText_Solid( gamestate.font, 
									gamestate.HighScoreList[ i ].c_str(), textColor );
			apply_surface( 50, 70 + i * 40, Surface_HighScore, gamestate.BackBuffer );
		}

	}
	else if( TitleScreen->ButtonCredits == true )
	{
		SDL_BlitSurface(	gamestate.GetSurface( TitleScreen->SurfaceButt ),
							&TitleScreen->ButtonClips[ 8 ],
							gamestate.BackBuffer, &TitleScreen->DestClips[ 9 ] ); 

		SDL_BlitSurface(	gamestate.GetSurface( TitleScreen->SurfaceButt ),
							&TitleScreen->ButtonClips[ 7 ],
							gamestate.BackBuffer, &TitleScreen->DestClips[ 7 ] ); 

		
		SDL_Color textColor = { 255,255,255 };
		
	
		Surface_Credits =		TTF_RenderText_Solid( gamestate.font, 
								" A Risk Production ", textColor );
		apply_surface( 50, 100, Surface_Credits, gamestate.BackBuffer );
	}
	else
	{
		for( int i = 0; i < 8; i++ )
		{
			if( i == 2 || i == 0 || i == 3 )
			{
											
			}
			else
			{
				SDL_BlitSurface(	gamestate.GetSurface( TitleScreen->SurfaceButt ), &TitleScreen->ButtonClips[ i ],
									gamestate.BackBuffer, &TitleScreen->DestClips[ i ] ); 
			}
		}
	}
	
	if( TitleScreen->ButtonNewgame == true )
	{
		gamestate.GameCondition = GS_ENTERNAME;
		TitleScreen->ButtonNewgame = false;

		if( Surface_Credits != NULL )
		{
			SDL_FreeSurface( Surface_Credits );
		}
		if( Surface_HighScore != NULL )
		{
			SDL_FreeSurface( Surface_HighScore );
		}

		gamestate.RestartGame();
	}
	if( TitleScreen->ButtonQuit == true )
	{
		gamestate.GameOK = false;
		TitleScreen->ButtonQuit == false;
	}

	if( TitleScreen->ButtonBack == true )
	{
		TitleScreen->ButtonOptions = false;
		TitleScreen->ButtonBack = false;
		TitleScreen->ButtonHighScore = false;
		TitleScreen->ButtonCredits = false;
		TitleScreen->ButtonBack = false;
	}
	
	// Render frame
	gamestate.FLIP();
}

// ----------------------------------------------------------------------------
// EnterName() - checks for input player name
// ----------------------------------------------------------------------------
void Gamestate::EnterName()
{
	bool Name =  false;
	SDL_Color textColor = { 255,255,255 };
	SDL_Event input;
	gamestate.IntroDone = false;
	SDL_Surface * srfEnter;
	
	while( Name != true )
	{
		if( timer.Timer_Name > 2 )
		{
			timer.Timer_Name = 0;
			SDL_PollEvent( &input );	
			if( ( input.type == SDL_KEYDOWN ) && ( input.key.keysym.sym == SDLK_RETURN ) )
			{
				Name = true;
			}
			gamestate.DrawBackgroundBlack();
			gamestate.Loading();
			gamestate.name->handle_input( input );
			Sleep(100);
		}
		timer.Timer_Name++;

		
		TypeName = TTF_RenderText_Solid( font, " Enter your name ", textColor ); 
		gamestate.name->show_centered();
		gamestate.apply_surface( 250, 200, TypeName, gamestate.BackBuffer );

		srfEnter = TTF_RenderText_Solid( font, " Press Enter To Finish ", textColor );
		gamestate.apply_surface( 200, 500, srfEnter, gamestate.BackBuffer );

		gamestate.FLIP();
	}

	gamestate.GameCondition = GS_INTROSTORY;
}

void Gamestate::RestartGame()
{
	ResetPlayer();
	ResetBoss();
	ResetEnemies();
	ResetObjects();
	ResetRest();

	timer.RestartAllTimers();

	demon.InitiateDemon( demon.DemonSurface, GROUND_X, GROUND_Y, DEMONHEIGHT, DEMONWIDTH );
	//gamestate.Score = 0;
	gamestate.m_parallax = 0;

}

void Gamestate::ResetRest()
{
	if( Dragon != NULL )
	{
		delete Dragon;
	}
	if( TitleScreen != NULL )
	{
		delete TitleScreen;
	}
	if( Intro != NULL )
	{
		delete Intro;
	}
	if( ListHighScore != NULL )
	{
		delete ListHighScore;
	}

	gamestate.CreateNewThings();
}

// Frees surfaces and deletes thing thats not NULL
void Gamestate::EndAll()
{
	for( int i = 0; i < Parallax->getLayerCount(); i++ )
	{
		SDL_FreeSurface( m_surfaceList[ i ] );
	}

	if( gamestate.Intro != NULL )
	{
		delete Intro;
	}
	if( gamestate.boss != NULL )
	{
		delete boss;
	}
	if( gamestate.outro != NULL )
	{ 
		delete outro;
	}
	if( gamestate.Dragon != NULL )
	{
		delete Dragon;
	}


	ListHighScore->sort( gamestate.name->str, demon.Score );
	ListHighScore->Save();

	if( gamestate.name != NULL )
	{
		delete name;
	}

}

void Game::cleanUp()
{
	TTF_Quit();
	SDL_Quit();
}

// writes to file 
void Gamestate::RecordAllData()
{
	ParallaxLayer * LayerToFile;
	ofstream OutToFile("LevelOptions.txt");
	for( int i = 0; i < Parallax->getLayerCount(); i++ )
	{
		LayerToFile = Parallax->getLayer( i );
		OutToFile << "WhichLayer: " << i << endl;
		OutToFile << "Layer_Surface: " << LayerToFile->m_surface << endl;
		OutToFile << "Layer_Width: " << LayerToFile->m_width << endl;
		OutToFile << "Layer_Height: " << LayerToFile->m_height << endl;
		OutToFile << "Layer_WhereOn_Y: " << LayerToFile->m_surfaceYOffset << endl;
		OutToFile << "Layer_Parallax: " << LayerToFile->m_parallax << endl;
		OutToFile << "Destination_X: " << LayerToFile->DX << endl;
		OutToFile << "Destination_Y: " << LayerToFile->DY << endl;
		OutToFile << "Destination_Width: " << LayerToFile->DW << endl;
		OutToFile << "Destination_Height: " << LayerToFile->DH << endl;
		OutToFile << endl;
	}
}

// loads image with chosen value to not show
int Gamestate::Load_imageAlpha( std::string filename, int r, int g, int b )
{
	//temp storage for the image loaded
	SDL_Surface * loadedimage = NULL;

	//optimized image for storage and flipping
	SDL_Surface * optimizedImage = NULL;

	//load image 
	loadedimage = IMG_Load( filename.c_str() );

	//if something went wrong
	if( loadedimage != NULL )
	{
		//create an optimized image 
		optimizedImage = SDL_DisplayFormat( loadedimage );

		//free old image
		SDL_FreeSurface( loadedimage );
	}

	if(optimizedImage != NULL)
	{
		SDL_SetColorKey(optimizedImage, SDL_RLEACCEL | SDL_SRCCOLORKEY, SDL_MapRGB(optimizedImage->format, r, g, b ) );
	}
	
	int index = findAvailableIndex();
	if( index == -1 )
	{
		return -1;
	}

	m_surfaceList[ index ] = optimizedImage;
	return index;
}

// inits sdl, font and videomode
bool Game::Init(SDL_Surface * &screen)
{
	gamestate.screen = 0;


	//initialize all SDL subystems
/*	if( SDL_Init( SDL_INIT_EVERYTHING ) == -1)
	{
		return false;
	} */
	//SDL_WM_SetCaption("", "res/big.ico");
	//SDL_WM_SetIcon(SDL_LoadBMP("res/small.bmp"), NULL);

	//set up the screen
	gamestate.screen = SDL_SetVideoMode(gamestate.SCREEN_WIDTH, gamestate.SCREEN_HEIGHT, gamestate.SCREEN_BPP, SDL_HWSURFACE | SDL_DOUBLEBUF);

	if( screen == NULL )
	{
		return false;
	}

/*	if( TTF_Init() == -1 )
	{
		return false;
	}
	else
	{
		gamestate.font = TTF_OpenFont( "cour.ttf", 28 );
	} */
	
	//set window caption
	SDL_WM_SetCaption( " Demon hunter ", NULL);
	
	/* Create a 32-bit surface with the bytes of each pixel in R,G,B,A order,
       as expected by OpenGL for textures */
    Uint32 rmask, gmask, bmask, amask;

	/* SDL interprets each pixel as a 32-bit number, so our masks must depend
	   on the endianness (byte order) of the machine */

	rmask = 0x00000000;
	gmask = 0x00000000;
	bmask = 0x00000000;
	amask = 0x00000000;

	gamestate.BackBuffer = SDL_CreateRGBSurface( SDL_HWSURFACE, gamestate.SCREEN_WIDTH, gamestate.SCREEN_HEIGHT, gamestate.SCREEN_BPP,
								   rmask, gmask, bmask, amask);
	
	if( gamestate.BackBuffer == NULL )
	{
		return false;
	}
	return true;
}

// ----------------------------------------------------------------------------
// drawParallaxLayers() - renders parallax-layers to backbuffer and gives it to Main Surface
// ----------------------------------------------------------------------------
void Gamestate::drawParallaxLayers()
{
	//demon.xPosHotSpot++;


	if( gamestate.GameCondition != GS_LEVEL1BOSS && gamestate.GameCondition != GS_OUTRO )
	{
		/*
		if( demon.xVel >= STARTSCROLLING - 50 )
		{
			// Updating the background to scroll when character is moving
			if( demon.isMovingRight )
			{
				gamestate.m_paralax += 20;
			}
			else if( demon.isMovingLeft )
			{
				gamestate.m_paralax -= 20;
			}
		}
		*/
		gamestate.CreateAll();
	}
	else
	{
		if( !BossStart )
		{
			demon.DemonHunter = true;
			demon.SmallHunter = false;
			gamestate.boss = gamestate.CreateBoss( SCREEN_WIDTH - 180, GROUND_Y - 210, m_srfBoss );
			BossStart = true;
		}
	}

		//// Draw parallax layers
		ParallaxLayer  * MyParaBackGround;
		MyParaBackGround = Parallax->getLayer( 0 );

		SDL_Rect scRect = { 0, 0,	MyParaBackGround->m_width, 
									MyParaBackGround->m_height };

		SDL_Rect dtRect = {	0, 0, MyParaBackGround->DW, MyParaBackGround->DH };

		SDL_BlitSurface( Gfx.GetSurface(MyParaBackGround->m_surface), &scRect, gamestate.BackBuffer, &dtRect );
		//SDL_BlitSurface( m_surfaceList[ MyParaBackGround->m_surface ], &scRect, gamestate.BackBuffer, &dtRect ); 

		//gamestate.stretchPicToBackBuffer( MyParaBackGround, scRect, dtRect );

		int x = 0;
		for( int i = 1; i < Parallax->getLayerCount(); ++i )
		{		
			// Calc rects
			MyParaBackGround = Parallax->getLayer( i );
			if( MyParaBackGround->m_surface == m_srfClouds )
			{
				MyParaBackGround->AnimClouds++;

				//////// Calc parallax position
				x = (int)( MyParaBackGround->m_parallax * (float)( +MyParaBackGround->AnimClouds ) );  
				if( x < 0 )	// neg -> pos
				{
					x *= -1;	// invert sign, because % only works with positive integers
					x = MyParaBackGround->m_width - (x % MyParaBackGround->m_width);
				}
				else
				{
					x %= MyParaBackGround->m_width;
				}

				x -= MyParaBackGround->m_width;	// move one back
			}
			else
			{

				//////// Calc parallax position
				x = (int)( MyParaBackGround->m_parallax * (float)( +gamestate.m_parallax ) );  
				if( x < 0 )	// neg -> pos
				{
					x *= -1;	// invert sign, because % only works with positive integers
					x = MyParaBackGround->m_width - (x % MyParaBackGround->m_width);
				}
				else
				{
					x %= MyParaBackGround->m_width;
				}

				x -= MyParaBackGround->m_width;	// move one back
			}

			for( int i = 0; i < 2; i++ )
			{
				SDL_Rect sourceRect = { 0 + x, MyParaBackGround->m_surfaceYOffset,
										MyParaBackGround->m_width, MyParaBackGround->m_height };

				SDL_Rect destinationRect = {	MyParaBackGround->DX, MyParaBackGround->DY, 
												MyParaBackGround->DW, MyParaBackGround->DH };

				SDL_BlitSurface( Gfx.GetSurface( MyParaBackGround->m_surface ), &sourceRect, gamestate.BackBuffer, &destinationRect ); 			
				//SDL_BlitSurface( m_surfaceList[ MyParaBackGround->m_surface ], &sourceRect, gamestate.BackBuffer, &destinationRect ); 
				
				
				x += MyParaBackGround->m_width;
			}


			MyParaBackGround->HowFarGone = MyParaBackGround->Xpos - MyParaBackGround->m_width;

		}
		gamestate.m_parallax++;
}

void Gamestate::CreateAll()
{
	AnimalController.Create_Animals();
	Control_ENEMY.Create_Enemies();
	Control_OBJ.CreateObjects();
}

// draws MyFellow on the screen and changes animations
void Gamestate::DrawSprite()
{
		SDL_Rect demonDest = { demon.xPos, demon.yPos, demon.Demon_Width, demon.Demon_Height };
		demonDest = demon.GetPosition();
		if( demon.isImmortal )
		{
			if( demon.AlphaImmortal < SDL_ALPHA_OPAQUE - 100 )
			{
				demon.AlphaImmortal += 50;
			}
			else
			{
				demon.AlphaImmortal -= 50;
			}
			SDL_SetAlpha( Gfx.GetSurface( demon.DemonSurface ), SDL_SRCALPHA | SDL_RLEACCEL, demon.AlphaImmortal );
		}
		else
		{
			SDL_SetAlpha( m_surfaceList[ demon.DemonSurface ], SDL_SRCALPHA | SDL_RLEACCEL, SDL_ALPHA_OPAQUE );
		}

		
		////////////////////////////////////////////////////////////
		if( gamestate.OK_Pace() )
		{			
			//Current_AnimArray = gamestate.WhichMorphType();
			//Current_Frame = demon.UpdatePlayer();
			demon.UpdatePlayer();
		
			// Rendering Demon Character
			/*
			SDL_BlitSurface(	m_surfaceList[ demon.DemonSurface ], 
								&demon.AnimationArrays[ Current_AnimArray ][ Current_Frame ],
								gamestate.BackBuffer, &demonDest );*/
			if( CurrentFrame == 3 )
			{
				CurrentFrame = 0;
			}

			SDL_BlitSurface(	Gfx.GetSurface( demon.DemonSurface ), 
				&demon.AnimationArrays[ demon.GetState() ][ ++CurrentFrame ],
				gamestate.BackBuffer, &demon.GetPosition() );
			/*
			SDL_BlitSurface(	m_surfaceList[ emon.DemonSurface ], 
				&demon.AnimationArrays[ demon.GetState() ][ ++CurrentFrame ],
								gamestate.BackBuffer, &demonDest );*/


			gamestate.resetAnimationPace();
			PreviousAnimArray = CurrentAnimArray;
			PreviousFrame = CurrentFrame;
			if( gamestate.GameCondition == GS_OUTRO )
			{
				gamestate.resetAnimationPace();
			}
		}
		else
		{
			if( CurrentFrame == 3 )
			{
				CurrentFrame = 0;
			}

			SDL_BlitSurface( Gfx.GetSurface( demon.DemonSurface ), 
				&demon.AnimationArrays[ demon.GetState() ][ ++CurrentFrame ],
								gamestate.BackBuffer, &demonDest );
			/*
			SDL_BlitSurface(	m_surfaceList[ demon.DemonSurface ], 
				&demon.AnimationArrays[ demon.GetState() ][ ++CurrentFrame ],
								gamestate.BackBuffer, &demonDest );*/

			/*SDL_BlitSurface(	m_surfaceList[ demon.DemonSurface ], 
								&demon.AnimationArrays[ Previous_AnimArray ][ Previous_Frame ],
								gamestate.BackBuffer, &demonDest );*/
			//demon.UpdateXPos();
			demon.UpdatePosition(0,0);
			gamestate.AddTick();
		}

}

// checks which state the demon is in which animation to use
int Gamestate::WhichMorphType()
{
	return demon.GetMorphState();
/*
	if( demon.SmallHunter )
	{
		return 0;
	}

	else if( demon.MediumHunter )
	{
		return 1;
	}

	else if( demon.LargeHunter )
	{
		return 2;
	}

	else if( demon.DemonHunter )
	{
		return 3;
	}

	return 0;*/
}

// checks if OK to move
bool Gamestate::OK_Pace()
{
	if( GameCondition != GS_OUTRO )
	{
		if( UpdateAnimationSpeed > ANIMPACE )
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		if( UpdateAnimationSpeed > ANIMPACESLOWER )
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}

// if enemy OK to move
bool Gamestate::OK_PaceEnemy()
{
	if( UpdateAnimationSpeed > ANIMPACEENEMY )
	{
		return true;
	}
	return false;
}

void Gamestate::AddTick()
{
	 float Speed = 1000.0f * ( gamestate.dt / 1000.0f );
	 UpdateAnimationSpeed += Speed;
}

void Gamestate::resetAnimationPace()
{
	UpdateAnimationSpeed = 0;
}

void Gamestate::apply_surface( int x, int y, SDL_Surface* source, SDL_Surface* destination, SDL_Rect* clip )
{
    //Holds offsets
    SDL_Rect offset;
    
    //Get offsets
    offset.x = x;
    offset.y = y;
    
    //Blit
    SDL_BlitSurface( source, clip, destination, &offset );
}

void Gamestate::stretchPicToBackBuffer( ParallaxLayer * layer, SDL_Rect srcRect, SDL_Rect destRect )	
{			

	int srcWidth = srcRect.w - srcRect.x;
	int srcHeight = srcRect.h - srcRect.y,
		DestWidth = destRect.w - destRect.x,
		DestHeight = destRect.h - destRect.y;

	SDL_LockSurface( gamestate.BackBuffer );
	SDL_LockSurface( m_surfaceList[ layer->m_surface ] );

	int dstPitch = gamestate.BackBuffer->pitch;
	int pitch = m_surfaceList[ layer->m_surface ]->pitch;

	DWORD * dst = ( DWORD * )gamestate.BackBuffer->pixels;

	DWORD * src = ( DWORD * )m_surfaceList[ layer->m_surface ]->pixels;

	float scaleWidth = srcWidth / ( float )DestWidth;
	float scaleHeight = srcHeight / ( float )DestHeight; 


	float fSrcX = srcRect.x,
		  fSrcY = srcRect.y;

	for(int y = destRect.y;  y < destRect.y + destRect.h; y++ )
	{
		fSrcX = 0;

		for(int x = destRect.x ;  x < destRect.x + destRect.w ; x++)
		{
			dst[ ( y * dstPitch / 4) + ( x ) ] = src[ int( ( fSrcY ) * ( pitch / 4 ) + int( fSrcX ) )];

			fSrcX += scaleWidth;
		}

		fSrcY += scaleHeight;	
	}

	SDL_UnlockSurface( gamestate.BackBuffer );
	SDL_UnlockSurface( m_surfaceList[ layer->m_surface ] );

}

void Gamestate::stretchBlit( ParallaxLayer * layer, SDL_Rect srcRect, SDL_Rect destRect )	
{			

	SDL_LockSurface( gamestate.BackBuffer );
	SDL_LockSurface( m_surfaceList[ layer->m_surface ] );

	int dstPitch = gamestate.BackBuffer->pitch;
	int pitch = m_surfaceList[ layer->m_surface ]->pitch;

	DWORD * dst = ( DWORD * )gamestate.BackBuffer->pixels;

	DWORD * src = ( DWORD * )m_surfaceList[ layer->m_surface ]->pixels;

	float scaleWidth = srcRect.w / ( float )destRect.w;
	float scaleHeight = srcRect.h / ( float )destRect.h; 


	float fSrcX = srcRect.x,
		  fSrcY = srcRect.y;

	for(int y = destRect.y;  y < destRect.y + destRect.h; y++ )
	{
		fSrcX = srcRect.x;

		for(int x = destRect.x ;  x < destRect.x + destRect.w ; x++)
		{
			dst[ (y * dstPitch / 4) + (x) ] = src[ int((fSrcY) * (pitch / 4) + int(fSrcX) )];

			fSrcX += scaleWidth;
		}
		fSrcY += scaleHeight;	
	}

	SDL_UnlockSurface( gamestate.BackBuffer );
	SDL_UnlockSurface( m_surfaceList[ layer->m_surface ] );

}

void Gamestate::PasteScreenToAnother( SDL_Rect srcRect, SDL_Rect destRect )	
{			

	SDL_LockSurface( gamestate.screen );
	SDL_LockSurface( gamestate.BackBuffer );

	int dstPitch = gamestate.screen->pitch;
	int pitch = gamestate.BackBuffer->pitch;

	DWORD * dst = ( DWORD * )gamestate.screen->pixels;
	DWORD * src = ( DWORD * )gamestate.BackBuffer->pixels;

	float scaleWidth = gamestate.BackBuffer->w / ( float )destRect.w;
	float scaleHeight = gamestate.BackBuffer->h / ( float )destRect.h; 


	float fSrcX = 0.0f,
		  fSrcY = 0.0f;

	for(int y = destRect.y;  y < destRect.y + destRect.h; y++ )
	{
		fSrcX = 0.0f;

		for(int x = destRect.x ;  x < destRect.x + destRect.w ; x++)
		{
			dst[ (y * dstPitch / 4) + (x) ] = src[ int(fSrcY) * (pitch / 4) + int(fSrcX) ];

			fSrcX += scaleWidth;
		}

		fSrcY += scaleHeight;	
	}

	SDL_UnlockSurface( gamestate.screen );
	SDL_UnlockSurface( gamestate.BackBuffer );

}

// --------------------------------------------------------------------------------------
// blit() - blits offscreen surface to (xpos,ypos) on backbuffer
// --------------------------------------------------------------------------------------
void Gamestate::blit( int index, int xpos, int ypos, bool transparent )
{
	// Calc rects
	SDL_Rect srcRect = { 0, 0, m_surfaceList[ index ]->w, m_surfaceList[ index ]->h };
	SDL_Rect destRect = { xpos, ypos, xpos+m_surfaceList[ index ]->w, ypos+m_surfaceList[ index ]->h };


	// Blit!
	if( transparent )
		SDL_BlitSurface(m_surfaceList[index], &destRect, gamestate.screen, &srcRect );
		/*m_lpddsBackbuffer->Blt( &destRect, m_surfaceList[index], &srcRect, DDBLT_WAIT|DDBLT_KEYSRC, 0 );*/
	else
		SDL_BlitSurface(m_surfaceList[index], &destRect, gamestate.screen, &srcRect );

}

// ----------------------------------------------------------------------------
// setUpParallaxLayers() - initializes parallax-struct
// ----------------------------------------------------------------------------
void Gamestate::setUpParallaxLayers()
{
	// Create background
	int i = 0;
	Parallax = new ParallaxBackground();
	Parallax->createLayers( 10 );

	//Firstlayer
	Parallax->setLayer( 0, 0, m_srfBlack, 0, gamestate.BackBuffer->w, gamestate.BackBuffer->h, 0, 0, gamestate.BackBuffer->w, gamestate.BackBuffer->h );

	////sky
	Parallax->setLayer( 1, 0.0f, m_srfSky, 0, 800, 400, 0, 0, gamestate.BackBuffer->w, gamestate.BackBuffer->h );


	//clouds
	Parallax->setLayer(	3, 0.5f, m_srfClouds, 
						0, 800, 38, 0, 0, gamestate.BackBuffer->w, 
						gamestate.BackBuffer->h );

	Parallax->setLayer(	4, 0.4f, m_srfClouds, 
						38, 800, 87, 0, 38, gamestate.BackBuffer->w, 
						gamestate.BackBuffer->h );

	Parallax->setLayer(	5, 0.3f, m_srfClouds, 
						126, 800, 46, 0, 126, gamestate.BackBuffer->w, 
						gamestate.BackBuffer->h );

	Parallax->setLayer(	6, 0.2f, m_srfClouds, 
						172, 800, 21, 0, 172, gamestate.BackBuffer->w, 
						gamestate.BackBuffer->h );

	Parallax->setLayer(	7, 0.1f, m_srfClouds, 
						193, 800, 12, 0, 193, gamestate.BackBuffer->w, 
						gamestate.BackBuffer->h );

	// City
	Parallax->setLayer( 8, 0.7f, m_srfCity, 0, 5100, 535, 0, 0, gamestate.BackBuffer->w, gamestate.BackBuffer->h );

	// trees
	Parallax->setLayer( 2, 0.7f, m_srfTrees, 0, 1172, 170, 0, 370, gamestate.BackBuffer->w, 170 ); 

	// WalkPath
	Parallax->setLayer(	9, 1.0f, m_srfCity, 
						540, 5100, 60, 0, 535, gamestate.BackBuffer->w, 
						gamestate.BackBuffer->h );

	//gamestate.RecordAllData();
}
