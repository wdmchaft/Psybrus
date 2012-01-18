/**************************************************************************
*
* File:		GaTopState.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Game top state.
*		
*
*
* 
**************************************************************************/

#include "GaTopState.h"

#include "GaMainGameState.h"

////////////////////////////////////////////////////////////////////////////////
// Ctor
GaTopState::GaTopState()
{
	name( "GaTopState" );
}

////////////////////////////////////////////////////////////////////////////////
// Dtor
GaTopState::~GaTopState()
{
}

////////////////////////////////////////////////////////////////////////////////
// enterOnce
void GaTopState::enterOnce()
{
	CsCore::pImpl()->requestResource( "spritesheet", SpriteSheetMaterial_ );
	CsCore::pImpl()->requestResource( "font", FontMaterial_ );
}

////////////////////////////////////////////////////////////////////////////////
// enter
eSysStateReturn GaTopState::enter()
{
	BcBool Ready = BcTrue;

	Ready &= SpriteSheetMaterial_.isReady();
	Ready &= FontMaterial_.isReady();
	Ready &= ScnFont::Default.isReady();

	return Ready ? sysSR_FINISHED : sysSR_CONTINUE;
}

////////////////////////////////////////////////////////////////////////////////
// preMain
void GaTopState::preMain()
{
	// Create game entity.
	if( CsCore::pImpl()->createResource( BcName::INVALID, GameEntity_ ) )
	{
		GaGameComponentRef GameComponent;
		ScnViewComponentRef ViewComponent;
		CsCore::pImpl()->createResource( BcName::INVALID, GameComponent );
		CsCore::pImpl()->createResource( BcName::INVALID, ViewComponent, 0.0f, 0.0f, 1.0f, 1.0f, 0.1f, 1024.0f, BcPIDIV4, 0.0f );
		GameEntity_->attach( GameComponent );
		GameEntity_->attach( ViewComponent );
		ScnCore::pImpl()->addEntity( GameEntity_ );
	}
}

////////////////////////////////////////////////////////////////////////////////
// main
eSysStateReturn GaTopState::main()
{
	return sysSR_CONTINUE;
}

////////////////////////////////////////////////////////////////////////////////
// preLeave
void GaTopState::preLeave()
{
	ScnCore::pImpl()->removeEntity( GameEntity_ );
}

////////////////////////////////////////////////////////////////////////////////
// leave
eSysStateReturn GaTopState::leave()
{
	return sysSR_FINISHED;
}

////////////////////////////////////////////////////////////////////////////////
// leaveOnce
void GaTopState::leaveOnce()
{
	
}
