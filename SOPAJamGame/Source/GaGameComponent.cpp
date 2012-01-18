/**************************************************************************
*
* File:		GaSwarmComponent.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Swarm.
*		
*
*
* 
**************************************************************************/

#include "GaGameComponent.h"

#include "GaMainGameState.h"
#include "GaTopState.h"

////////////////////////////////////////////////////////////////////////////////
// Define resource.
DEFINE_RESOURCE( GaGameComponent )

//////////////////////////////////////////////////////////////////////////
// StaticPropertyTable
void GaGameComponent::StaticPropertyTable( CsPropertyTable& PropertyTable )
{
	Super::StaticPropertyTable( PropertyTable );

	PropertyTable.beginCatagory( "GaGameComponent" )
	.endCatagory();
}

////////////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void GaGameComponent::initialise()
{
	CsCore::pImpl()->requestResource( "spritesheet", SpriteSheetMaterial_ );
	CsCore::pImpl()->createResource( BcName::INVALID, SpriteSheetMaterialComponent_, SpriteSheetMaterial_, BcErrorCode );
	CsCore::pImpl()->createResource( BcName::INVALID, CanvasComponent_, 8192, SpriteSheetMaterialComponent_ );

	GameState_ = STATE_BEGIN;
}

////////////////////////////////////////////////////////////////////////////////
// update
//virtual
void GaGameComponent::update( BcReal Tick )
{
	BcMat4d Ortho;
	Ortho.orthoProjection( -240.0f, 240.0f, 320.0f, -320.0f, -1.0f, 1.0f );
	
	CanvasComponent_->clear();
	CanvasComponent_->pushMatrix( Ortho );

	static BcReal Timer = 0.0f;

	switch( GameState_ )
	{
	case STATE_BEGIN:
		{
			GameState_ = STATE_NEW_PIECE;
		}
		break;

	case STATE_NEW_PIECE:
		{
			addMovable( 2, 0 );
			addMovable( 3, 0 );
			addMovable( 4, 0 );
			addMovable( 5, 0 );
			Timer = 0.4f;
			GameState_ = STATE_CONTROL_PIECE;
		}
		break;

	case STATE_CONTROL_PIECE:
		{
			Timer -= Tick;
			if( Timer < 0.0f )
			{
				// Nudge game area down.
				BcBool MovedBlocks = nudgeGameArea( 0, 1, BcTrue );
	
				if( !MovedBlocks )
				{
					GameState_ = STATE_REMOVE_LINES;
				}

				Timer = 0.4f;
			}
		}
		break;

	case STATE_REMOVE_LINES:
		{
			while( removeGameAreaFullLines() );
			while( nudgeGameArea( 0, 1, BcTrue ) );
			GameState_ = STATE_NEW_PIECE;
		}
		break;
	}

	renderGameArea();
}

////////////////////////////////////////////////////////////////////////////////
// render
//virtual
void GaGameComponent::render( ScnCanvasComponentRef Canvas )
{

}

////////////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void GaGameComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );	

	Parent->attach( SpriteSheetMaterialComponent_ );
	Parent->attach( CanvasComponent_ );

	OsEventInputKeyboard::Delegate OnKeyDown = OsEventInputKeyboard::Delegate::bind< GaGameComponent, &GaGameComponent::onKeyDown >( this );
	OsCore::pImpl()->subscribe( osEVT_INPUT_KEYDOWN, OnKeyDown );

}

////////////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void GaGameComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );

	Parent->detach( SpriteSheetMaterialComponent_ );
	Parent->detach( CanvasComponent_ );

	OsCore::pImpl()->unsubscribeAll( this );
}

////////////////////////////////////////////////////////////////////////////////
// onKeyDown
eEvtReturn GaGameComponent::onKeyDown( EvtID ID, const OsEventInputKeyboard& Event )
{
	if( GameState_ != STATE_CONTROL_PIECE )
	{
		return evtRET_PASS;
	}

	// 37 left. 38 up. 39 right.
	switch( Event.KeyCode_ )
	{
	case 'A':
	case 37:
		{
			nudgeGameArea( -1, 0, BcFalse );
		}
		break;

	case 'D':
	case 39:
		{
			nudgeGameArea( 1, 0, BcFalse );
		}
		break;

	case 'W':
	case 38:
		{
			// ROTATE!
		}
		break;

	case 'S':
	case 40:
		{
			nudgeGameArea( 0, 1, BcFalse );
		}
		break;
	}

	return evtRET_PASS;
}

////////////////////////////////////////////////////////////////////////////////
// nudgeGameArea
BcBool GaGameComponent::nudgeGameArea( BcS32 DeltaX, BcS32 DeltaY, BcBool NotMovableOnCollision )
{
	//
	BcBool ReverseX = DeltaX > 0;
	BcBool ReverseY = DeltaY > 0;

	// Iterate over blocks from the bottom and bail is a movable block can not move (except into another movable).
	BcBool CanMoveBlocks = BcTrue;
	BcU32 MoveableBlock = 0;
	for( BcS32 Y = ReverseY ? GAME_AREA_H - 1 : 0; ReverseY ? Y >= 0 : Y < GAME_AREA_H; ReverseY ? --Y : ++Y )
	{
		for( BcS32 X = ReverseX ? GAME_AREA_W - 1 : 0; ReverseX ? X >= 0 : X < GAME_AREA_W; ReverseX ? --X : ++X )
		{
			TBlock& Block( getBlock( X, Y ) );
			TBlock& MoveToBlock( getBlock( X + DeltaX, Y + DeltaY ) );

			if( Block.IsFilled_ &&
				Block.IsMoveable_ &&
				!MoveToBlock.IsMoveable_ &&
				MoveToBlock.IsFilled_ )
			{
				CanMoveBlocks = BcFalse;
				break;
			}

			if( Block.IsMoveable_ )
			{
				++MoveableBlock;
			}
		}
	}

	// Move all movables down.
	if( CanMoveBlocks )
	{
		for( BcS32 Y = ReverseY ? GAME_AREA_H - 1 : 0; ReverseY ? Y >= 0 : Y < GAME_AREA_H; ReverseY ? --Y : ++Y )
		{
			for( BcS32 X = ReverseX ? GAME_AREA_W - 1 : 0; ReverseX ? X >= 0 : X < GAME_AREA_W; ReverseX ? --X : ++X )
			{
				TBlock& Block( getBlock( X, Y ) );
				TBlock& MoveToBlock( getBlock( X + DeltaX, Y + DeltaY ) );

				if( Block.IsFilled_ &&
					Block.IsMoveable_ &&
					!MoveToBlock.IsFilled_ )
				{
					MoveToBlock = Block;
					Block.IsFilled_ = BcFalse;
					Block.IsMoveable_ = BcFalse;
				}
			}
		}
	}
	else if( NotMovableOnCollision )
	{
		for( BcS32 Y = 0; Y < GAME_AREA_H; ++Y )
		{
			for( BcS32 X = 0; X < GAME_AREA_W; ++X )
			{
				TBlock& Block( getBlock( X, Y ) );
				Block.IsMoveable_ = BcFalse;
			}
		}
	}

	return CanMoveBlocks && MoveableBlock > 0;
}

////////////////////////////////////////////////////////////////////////////////
// removeGameAreaFullLines
BcBool GaGameComponent::removeGameAreaFullLines()
{
	BcBool MakeAllMoveable = BcFalse;
	for( BcS32 Y = 0; Y < GAME_AREA_H; ++Y ) 
	{
		BcU32 NoofInLine = 0;
		for( BcS32 X = 0; X < GAME_AREA_W; ++X )
		{
			TBlock& Block( getBlock( X, Y ) );
			if( Block.IsFilled_ )
			{
				++NoofInLine;
			}
		}

		if( NoofInLine == GAME_AREA_W )
		{
			for( BcS32 X = 0; X < GAME_AREA_W; ++X )
			{
				TBlock& Block( getBlock( X, Y ) );
				Block.IsFilled_ = BcFalse;
				MakeAllMoveable = BcTrue;
			}
		}
	}

	if( MakeAllMoveable )
	{
		for( BcS32 Y = 0; Y < GAME_AREA_H; ++Y )
		{
			for( BcS32 X = 0; X < GAME_AREA_W; ++X )
			{
				TBlock& Block( getBlock( X, Y ) );
				Block.IsMoveable_ = BcTrue;
			}
		}

		return BcTrue;
	}

	return BcFalse;
}

////////////////////////////////////////////////////////////////////////////////
// renderGameArea
void GaGameComponent::renderGameArea()
{
	BcVec2d Size( 32.0f, 32.0f );
	BcVec2d Offset( BcVec2d( -Size.x() * ( GAME_AREA_W / 2 ), -Size.y() * ( GAME_AREA_H / 2 ) ) + ( Size * 0.5f ) );

	for( BcS32 Y = -1; Y < GAME_AREA_H + 1; ++Y )
	{
		for( BcS32 X = -1; X < GAME_AREA_W + 1; ++X )
		{
			const TBlock& Block( getBlock( X, Y ) );
			BcU32 Sprite( Block.IsFilled_ ? 0 : 2 );
			RsColour Colour( Block.IsMoveable_ ? RsColour::GREEN : RsColour::WHITE );
			BcVec2d Position( ( Size * BcVec2d( static_cast< BcReal >( X ), static_cast< BcReal >( Y ) ) + Offset ) );
			CanvasComponent_->drawSpriteCentered( Position, BcVec2d( 34.0f, 34.0f ), Sprite, Colour, 0 );
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// getBlock
GaGameComponent::TBlock& GaGameComponent::getBlock( BcS32 X, BcS32 Y )
{
	static TBlock ImmovableBlock( BcTrue, BcFalse );

	if( X >= 0 && X < GAME_AREA_W &&
	    Y >= 0 && Y < GAME_AREA_H )
	{
		return GameArea_[ X + ( Y * GAME_AREA_W ) ];
	}

	return ImmovableBlock;
}

////////////////////////////////////////////////////////////////////////////////
// addMovable
void GaGameComponent::addMovable( BcS32 X, BcS32 Y )
{
	TBlock& Block( getBlock( X, Y ) );
	Block.IsFilled_ = BcTrue;
	Block.IsMoveable_ = BcTrue;
}