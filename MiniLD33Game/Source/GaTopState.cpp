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

#include "GaMatchmakingState.h"

#include "GaTopState.h"

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
	pPackage_ = CsCore::pImpl()->requestPackage( "game" );
}

////////////////////////////////////////////////////////////////////////////////
// enter
eSysStateReturn GaTopState::enter()
{
	BcBool Ready = pPackage_->isReady();
	
	// Wait for default material to be ready.
	if( Ready == BcTrue )
	{
		return sysSR_FINISHED;
	}

	return sysSR_CONTINUE;
}

////////////////////////////////////////////////////////////////////////////////
// preMain
void GaTopState::preMain()
{ 
	ScnEntityRef BaseEntity;
	ScnEntityRef Entity;

	if( CsCore::pImpl()->requestResource( "game", "ScreenEntity", BaseEntity ) )
	{
		if( CsCore::pImpl()->createResource( "ScreenEntity_0", Entity, BaseEntity ) )
		{
			GameEntity_ = Entity;
			EntityList_.push_back( Entity );
		}
	}
	
	// Add entities to scene.
	for( BcU32 Idx = 0; Idx < EntityList_.size(); ++Idx )
	{
		ScnCore::pImpl()->addEntity( EntityList_[ Idx ] );
	}

	SpawnTitle_ = BcTrue;
	SpawnGame_ = BcFalse;
	Networked_ = BcFalse;

	startMatchmaking();
}

////////////////////////////////////////////////////////////////////////////////
// main
eSysStateReturn GaTopState::main()
{
	if( SpawnTitle_ )
	{
		SpawnTitle_ = BcFalse;
		if( GameComponent_.isValid() )
		{
			GameEntity_->detach( GameComponent_ );
			GameComponent_ = NULL;
		}

		if( TitleComponent_.isValid() )
		{
			GameEntity_->detach( TitleComponent_ );
			TitleComponent_ = NULL;
		}

		CsCore::pImpl()->createResource( BcName::INVALID, TitleComponent_ );
		GameEntity_->attach( TitleComponent_ ); 
	}
	else if( SpawnGame_ )
	{
		SpawnGame_ = BcFalse;
		if( TitleComponent_.isValid() )
		{
			GameEntity_->detach( TitleComponent_ );
			TitleComponent_ = NULL;
		}

		CsCore::pImpl()->createResource( BcName::INVALID, GameComponent_, GaMatchmakingState::getClientID(), Networked_ );
		GameEntity_->attach( GameComponent_ ); 

	}
	return sysSR_CONTINUE;
}

////////////////////////////////////////////////////////////////////////////////
// preLeave
void GaTopState::preLeave()
{
	// Remove entities from the scene.
	for( BcU32 Idx = 0; Idx < EntityList_.size(); ++Idx )
	{
		ScnCore::pImpl()->removeEntity( EntityList_[ Idx ] );
	}

	EntityList_.clear();
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

////////////////////////////////////////////////////////////////////////////////
// startMatchmaking
void GaTopState::startMatchmaking()
{
	SpawnTitle_ = BcTrue;
	SpawnGame_ = BcFalse;
}

////////////////////////////////////////////////////////////////////////////////
// startGame
void GaTopState::startGame( BcBool Networked )
{
	SpawnGame_ = BcTrue;
	SpawnTitle_ = BcFalse;
	Networked_ = Networked;
}

////////////////////////////////////////////////////////////////////////////////
// playSound
void GaTopState::playSound( const BcChar* pSoundName, const BcFixedVec2d& Position )
{
	ScnSoundRef Sound;
	CsCore::pImpl()->requestResource( "game", pSoundName, Sound );

	if( Sound.isReady() )
	{
		if( SsCore::pImpl() )
		{
			SsChannel* pChannel = SsCore::pImpl()->play( Sound->getSample(), NULL );
			if( pChannel != NULL )
			{
				pChannel->position( BcVec3d( Position.x(), -Position.y(), 2.0f ) / 800.0f );
				pChannel->gain( 0.2f );
				pChannel->pitch( BcAbs( BcRandom::Global.randReal() * 0.1f ) + 0.95f );
			}
		}
	}
}
