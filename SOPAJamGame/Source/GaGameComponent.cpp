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

static BcS32 GPiecePositions_[] =
{
	0, 0, 1, 0, 2, 0, 3, 0,
	0, 0, 1, 0, 1, 1, 2, 1,
	0, 0, 1, 0, 2, 0, 1, 1,
	0, 0, 1, 0, 2, 0, 2, 1,
	0, 1, 1, 1, 2, 1, 2, 0,
	0, 0, 1, 0, 0, 1, 1, 1,
	0, 1, 1, 1, 1, 0, 2, 0,
};

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
	CsCore::pImpl()->requestResource( "font", FontMaterial_ );
	CsCore::pImpl()->createResource( BcName::INVALID, SpriteSheetMaterialComponent_, SpriteSheetMaterial_, BcErrorCode );
	CsCore::pImpl()->createResource( BcName::INVALID, CanvasComponent_, 8192, SpriteSheetMaterialComponent_ );
	CsCore::pImpl()->createResource( BcName::INVALID, FontComponent_, ScnFont::Default, FontMaterial_ );

	GameState_ = STATE_BEGIN;
}

////////////////////////////////////////////////////////////////////////////////
// update
//virtual
void GaGameComponent::update( BcReal Tick )
{
	OsClient* pClient = OsCore::pImpl()->getClient( 0 );
	BcReal HW = static_cast< BcReal >( pClient->getWidth() ) * 0.5f;
	BcReal HH = static_cast< BcReal >( pClient->getHeight() ) * 0.5f;

	BcMat4d Ortho;
	Ortho.orthoProjection( -HW, HW, HH, -HH, -1.0f, 1.0f );
	
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
			static BcRandom Random;
			ShapeIdx_ = NextShapeIdx_;
			NextShapeIdx_ = Random.rand();
			if( addPieceToArea( ShapeIdx_, BcFalse, 0, 1, BcTrue ) )
			{
				Timer = 0.4f;
				IsSwapped_ = BcFalse;
				GameState_ = STATE_CONTROL_PIECE;
			}
			else
			{
				GameState_ = STATE_GAME_OVER;
			}
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
			removeGameAreaFullLines();
			if( !nudgeGameArea( 0, 1, BcTrue ) )
			{
				GameState_ = STATE_NEW_PIECE;
			}
		}
		break;
	}

	if( GameState_ != STATE_GAME_OVER )
	{
		renderGameArea();
		renderNextPiece();
	}
	else
	{
		const BcChar* pText = "YOU GOT SOPAWNED!";
		BcVec2d Size = FontComponent_->draw( CanvasComponent_, pText, BcTrue );
		BcMat4d Translation;
		Translation.translation( BcVec3d( -Size.x() * 0.5f, -Size.y() * 0.5f, 0.0f ) );
		CanvasComponent_->pushMatrix( Translation );
		FontComponent_->draw( CanvasComponent_, pText, BcFalse );
		CanvasComponent_->popMatrix();
		
	}
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
	Parent->attach( FontComponent_ );
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
	Parent->detach( FontComponent_ );
	Parent->detach( CanvasComponent_ );

	OsCore::pImpl()->unsubscribeAll( this );
}

////////////////////////////////////////////////////////////////////////////////
// onKeyDown
eEvtReturn GaGameComponent::onKeyDown( EvtID ID, const OsEventInputKeyboard& Event )
{
	if( GameState_ == STATE_GAME_OVER )
	{
		BcMemZero( &GameArea_[ 0 ], sizeof( GameArea_ ) );
		GameState_ = STATE_BEGIN;
		return evtRET_PASS;
	}

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
			rotateMoveables();
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
			// Move all above current Y down.
			for( BcS32 MY = Y; MY >= 1; --MY )
			{
				for( BcS32 MX = 0; MX < GAME_AREA_W; ++MX )
				{
					TBlock& Block( getBlock( MX, MY ) );
					TBlock& MoveBlock( getBlock( MX, MY - 1 ) );

					Block = MoveBlock;
				}
			}
		}
	}
	
	return BcFalse;
}

////////////////////////////////////////////////////////////////////////////////
// rotateMoveables
void GaGameComponent::rotateMoveables()
{
	// Find bounds of the moveables.
	BcS32 LX = GAME_AREA_W;
	BcS32 HX = 0;
	BcS32 LY = GAME_AREA_H;
	BcS32 HY = 0;
	for( BcS32 Y = 0; Y < GAME_AREA_H; ++Y )
	{
		for( BcS32 X = 0; X < GAME_AREA_W; ++X )
		{
			const TBlock& Block( getBlock( X, Y ) );

			if( Block.IsFilled_ && Block.IsMoveable_ )
			{
				LX = BcMin( X, LX );
				HX = BcMax( X, HX );
				LY = BcMin( Y, LY );
				HY = BcMax( Y, HY );
			}
		}
	}

	// Check if X/Y can be swapped (LX,HY is offset).
	BcBool CanSwap = BcTrue;
	BcS32 NX = 0;
	BcS32 NY = 0;
	for( BcU32 BlockIdx = 0; BlockIdx < 4; ++BlockIdx )
	{
		getPiecePosition( ShapeIdx_, BlockIdx, NY, NX, IsSwapped_ );
		const TBlock& Block( getBlock( NX + LX, NY + HY ) );
		
		if( Block.IsFilled_ && !Block.IsMoveable_ )
		{
			CanSwap = BcFalse;
			break;
		}
	}

	// If we can swap, remove block, and add new one!
	if( CanSwap )
	{
		for( BcS32 Y = LY; Y <= HY; ++Y )
		{
			for( BcS32 X = LX; X <= HX; ++X )
			{
				TBlock& Block( getBlock( X, Y ) );
				if( Block.IsMoveable_ )
				{
					Block.IsFilled_ = BcFalse;
					Block.IsMoveable_ = BcFalse;
				}
			}
		}
		
		IsSwapped_ = !IsSwapped_;
		addPieceToArea( ShapeIdx_, IsSwapped_, LX, HY, BcFalse );
	}
}

////////////////////////////////////////////////////////////////////////////////
// renderGameArea
void GaGameComponent::renderGameArea()
{
	BcVec2d Size( 32.0f, 32.0f );
	BcVec2d Offset( BcVec2d( -Size.x() * ( GAME_AREA_W / 2 ), -Size.y() * ( GAME_AREA_H / 2 ) ) + ( Size * 0.5f ) );

	for( BcS32 Y = -2; Y < GAME_AREA_H + 2; ++Y )
	{
		for( BcS32 X = -9; X < GAME_AREA_W + 9; ++X )
		{
			const TBlock& Block( getBlock( X, Y ) );
			BcU32 Sprite( Block.IsFilled_ ? 0 : 1 );
			RsColour Colour( Block.IsMoveable_ ? RsColour::GREEN : RsColour::WHITE );
			if( X < -1 || X >= GAME_AREA_W + 1 || Y < -1 || Y >= GAME_AREA_H + 1 )
			{
				Colour = Colour * RsColour( 0.5f, 0.5f, 0.5f, 1.0f );
			}
			BcVec2d Position( ( Size * BcVec2d( static_cast< BcReal >( X ), static_cast< BcReal >( Y ) ) + Offset ) );
			CanvasComponent_->drawSpriteCentered( Position, BcVec2d( 34.0f, 34.0f ), Sprite, Colour, LAYER_GAME_AREA );
		}
	}

	// Find bounds of the moveables.
	BcS32 LX = GAME_AREA_W;
	BcS32 HX = 0;
	BcS32 LY = GAME_AREA_H;
	BcS32 HY = 0;
	for( BcS32 Y = 0; Y < GAME_AREA_H; ++Y )
	{
		for( BcS32 X = 0; X < GAME_AREA_W; ++X )
		{
			const TBlock& Block( getBlock( X, Y ) );

			if( Block.IsFilled_ && Block.IsMoveable_ )
			{
				LX = BcMin( X, LX );
				HX = BcMax( X, HX );
				LY = BcMin( Y, LY );
				HY = BcMax( Y, HY );
			}
		}
	}

	// Render censor.
	for( BcS32 Y = LY; Y <= HY; ++Y )
	{
		for( BcS32 X = LX; X <= HX; ++X )
		{
			BcVec2d Position( ( Size * BcVec2d( static_cast< BcReal >( X ), static_cast< BcReal >( Y ) ) + Offset ) );
			CanvasComponent_->drawSpriteCentered( Position, BcVec2d( 64.0f, 64.0f ), 2, RsColour::BLACK, LAYER_CENSOR );
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// renderNextPiece
void GaGameComponent::renderNextPiece()
{
	BcVec2d Size( 32.0f, 32.0f );
	BcVec2d Offset( BcVec2d( -Size.x() * ( GAME_AREA_W / 2 ), -Size.y() * ( GAME_AREA_H / 2 ) ) + ( Size * 0.5f ) );
	BcS32 X = 0;
	BcS32 Y = 0;
	for( BcS32 X = 0; X < 6; ++X )
	{
		for( BcS32 Y = 0; Y < 4; ++Y )
		{
			BcVec2d Position( ( Size * BcVec2d( static_cast< BcReal >( X - 8 ), static_cast< BcReal >( Y ) ) + Offset ) );
			CanvasComponent_->drawSpriteCentered( Position, BcVec2d( 34.0f, 34.0f ), 2, RsColour::WHITE, LAYER_UI );
		}
	}

	for( BcU32 BlockIdx = 0; BlockIdx < 4; ++BlockIdx )
	{
		getPiecePosition( NextShapeIdx_, BlockIdx, X, Y, BcFalse );

		X -= 7;
		Y += 1;
		BcVec2d Position( ( Size * BcVec2d( static_cast< BcReal >( X ), static_cast< BcReal >( Y ) ) + Offset ) );
		CanvasComponent_->drawSpriteCentered( Position, BcVec2d( 34.0f, 34.0f ), 0, RsColour::WHITE, LAYER_UI );
	}
}

/////////////////////////////////////////////////////// /////////////////////////
// getPiecePosition
void GaGameComponent::getPiecePosition( BcU32 Idx, BcU32 BlockIdx, BcS32& X, BcS32& Y, BcBool IsSwapped )
{
	Idx = Idx % ( sizeof( GPiecePositions_ ) / ( 8 * sizeof( BcU32 ) ) );
	BcU32 ArrayIdx = ( Idx * 8 ) + ( BlockIdx * 2 );
	BcS32* pBlockPosition = &GPiecePositions_[ ArrayIdx ];
	if( !IsSwapped )
	{
		X = pBlockPosition[0];
		Y = pBlockPosition[1];
	}
	else
	{
		X = pBlockPosition[1];
		Y = pBlockPosition[0];
	}
}

////////////////////////////////////////////////////////////////////////////////
// addPieceToArea
BcBool GaGameComponent::addPieceToArea( BcU32 Idx, BcBool IsSwapped, BcU32 OffX, BcU32 OffY, BcBool Centered )
{
	Idx = Idx % ( sizeof( GPiecePositions_ ) / ( 8 * sizeof( BcU32 ) ) );
	BcS32 Width = 0;
	BcS32 X = 0;
	BcS32 Y = 0;
	for( BcU32 BlockIdx = 0; BlockIdx < 4; ++BlockIdx )
	{
		getPiecePosition( Idx, BlockIdx, X, Y, IsSwapped );
		Width = BcMax( Width, X );
	}
	for( BcU32 BlockIdx = 0; BlockIdx < 4; ++BlockIdx )
	{
		getPiecePosition( Idx, BlockIdx, X, Y, IsSwapped );
		BcS32 PosX = OffX + X + ( Centered ? ( GAME_AREA_W - ( Width + 1 ) ) / 2 : 0 );
		BcS32 PosY = OffY + Y;
		const TBlock& Block( getBlock( PosX, PosY ) );
		if( Block.IsFilled_ )
		{
			return BcFalse;
		}
		addMovable( PosX, PosY );	
	}
	return BcTrue;
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