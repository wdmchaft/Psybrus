/**************************************************************************
*
* File:		GaSwarmComponent.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Swarm.
*		
*
*
* 
**************************************************************************/

#ifndef __GaGameComponent_H__
#define __GaGameComponent_H__

#include "Psybrus.h"

////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
class GaMainGameState;
typedef CsResourceRef< class GaGameComponent > GaGameComponentRef;

////////////////////////////////////////////////////////////////////////////////
// GaGameComponent
class GaGameComponent:
	public ScnComponent
{
public:
	enum
	{
		GAME_AREA_W = 8,
		GAME_AREA_H = 16,
		GAME_AREA_SIZE = GAME_AREA_W * GAME_AREA_H,
		
		LAYER_GAME_AREA = 0,
		LAYER_UI,
	};

	enum GameState
	{
		 STATE_BEGIN = 0,
		 STATE_NEW_PIECE,
		 STATE_CONTROL_PIECE,
		 STATE_REMOVE_LINES,
		 STATE_MAX
	};

public:
	DECLARE_RESOURCE( ScnComponent, GaGameComponent );

	virtual void initialise();
	virtual void update( BcReal Tick );
	virtual void render( ScnCanvasComponentRef Canvas );
	virtual void onAttach( ScnEntityWeakRef Parent );
	virtual void onDetach( ScnEntityWeakRef Parent );

	eEvtReturn onKeyDown( EvtID ID, const OsEventInputKeyboard& Event );


private:
	BcBool nudgeGameArea( BcS32 DeltaX, BcS32 DeltaY, BcBool NotMovableOnCollision );
	BcBool removeGameAreaFullLines();
	void renderGameArea();
	
private:
	// Resources.
	ScnMaterialRef				SpriteSheetMaterial_;
	ScnMaterialComponentRef		SpriteSheetMaterialComponent_;
	ScnCanvasComponentRef		CanvasComponent_;

	// Game.
	struct TBlock
	{
		TBlock():
			IsFilled_( BcFalse ),
			IsMoveable_( BcFalse )
		{
		}

		TBlock( BcBool IsFilled, BcBool IsMoveable ):
			IsFilled_( IsFilled ),
			IsMoveable_( IsMoveable )
		{
		}

		BcBool IsFilled_;
		BcBool IsMoveable_;
	};
	
	TBlock						GameArea_[ GAME_AREA_SIZE ];
	
	GameState					GameState_;
	BcU32						ShapeIdx_;
	BcU32						NextShapeIdx_;
	
private:
	TBlock&						getBlock( BcS32 X, BcS32 Y );
	void						addMovable( BcS32 X, BcS32 Y );

};

#endif
