/**************************************************************************
*
* File:		GaGameComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	29/12/11	
* Description:
*		Example user component.
*		
*
*
* 
**************************************************************************/

#include "GaGameComponent.h"

#include "GaTopState.h"

//////////////////////////////////////////////////////////////////////////
// Units
static GaGameUnitDescriptor GGameProjectile_Soldier = 
{
	5,
	BcFixedVec2d( 0.5f, 0.5f ),		// Unit size.
	BcFixed( 32.0f ),				// Move speed.
	BcFixed( 2.0f ),				// Rate of attack.
	BcFixed( 1.0f ),				// Cool down mult for rate of attack.
	BcFixed( 0.1f ),				// Range.
	BcFixed( 0.1f ),				// Range.
	BcFixed( 7.0f ),				// Health.
	BcFalse,						// Armoured.
	NULL,

	// Sound
	NULL,
	NULL,
	NULL,
};

static GaGameUnitDescriptor GGameProjectile_Archer = 
{
	6,
	BcFixedVec2d( 0.5f, 0.5f ),		// Unit size.
	BcFixed( 18.0f ),				// Move speed.
	BcFixed( 0.5f ),				// Rate of attack.
	BcFixed( 1.0f ),				// Cool down mult for rate of attack.
	BcFixed( 0.25f ),				// Range.
	BcFixed( 0.25f ),				// Range.
	BcFixed( 10.0f ),				// Health.
	BcFalse,						// Armoured.
	NULL,

	// Sound
	"ArrowLaunch",
	"ArrowHit",
	NULL,
};

static GaGameUnitDescriptor GGameProjectile_Trebuchet = 
{
	7,
	BcFixedVec2d( 0.5f, 0.5f ),		// Unit size.
	BcFixed( 12.0f ),				// Move speed.
	BcFixed( 0.6f ),				// Rate of attack.
	BcFixed( 1.0f ),				// Cool down mult for rate of attack.
	BcFixed( 2.0f ),				// Range.
	BcFixed( 2.0f ),				// Range.
	BcFixed( 30.0f ),				// Health.
	BcFalse,						// Armoured.
	NULL,

	// Sound
	"RockLaunch",
	"RockHit",
	NULL,
};

static GaGameUnitDescriptor GGameUnit_Soldier = 
{
	2,
	BcFixedVec2d( 1.0f, 1.0f ),		// Unit size.
	BcFixed( 2.5f ),				// Move speed.
	BcFixed( 1.2f ),				// Rate of attack.
	BcFixed( 0.1f ),				// Cool down mult for rate of attack.
	BcFixed( 1.5f ),				// Range.
	BcFixed( 0.0f ),				// Range.
	BcFixed( 20.0f ),				// Health.
	BcTrue,							// Armoured.
	&GGameProjectile_Soldier,

	// Sound
	NULL,
	NULL,
	"Die",
};

static GaGameUnitDescriptor GGameUnit_Archer = 
{
	3,
	BcFixedVec2d( 1.0f, 1.0f ),		// Unit size.
	BcFixed( 2.5f ),				// Move speed.
	BcFixed( 0.5f ),				// Rate of attack.
	BcFixed( 0.25f ),				// Cool down mult for rate of attack.
	BcFixed( 10.0f ),				// Range.
	BcFixed( 2.0f ),				// Range.
	BcFixed( 40.0f ),				// Health.
	BcFalse,						// Armoured.
	&GGameProjectile_Archer,

	// Sound
	NULL,
	NULL,
	"Die",
};

static GaGameUnitDescriptor GGameUnit_Trebuchet = 
{
	4,
	BcFixedVec2d( 2.0f, 2.0f ),		// Unit size.
	BcFixed( 1.1f ),				// Move speed.
	BcFixed( 0.25f ),				// Rate of attack.
	BcFixed( 1.0f ),				// Cool down mult for rate of attack.
	BcFixed( 18.0f ),				// Range.
	BcFixed( 5.0f ),				// Range.
	BcFixed( 100.0f ),				// Health.
	BcFalse,						// Armoured.
	&GGameProjectile_Trebuchet,

	// Sound
	NULL,
	NULL,
	"Die",
};

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( GaGameComponent );

//////////////////////////////////////////////////////////////////////////
// initialise
void GaGameComponent::initialise( BcU32 TeamID,  BcBool Networked )
{
	Super::initialise();

	if( TeamID == BcErrorCode )
	{
		TeamID = 0;
	}

	MouseDown_ = BcFalse;
	BoxSelection_ = BcFalse;
	CtrlDown_ = BcFalse; 
	AttackMove_ = BcFalse;
	TeamID_ = TeamID;
	Networked_ = Networked;
	GameState_ = GS_PLAYING;

	// Randomly decide for some variation.
	AITickTime_ = 0.0f;
	AITickMaxTime_ = BcAbs( BcRandom::Global.randReal() * 0.4f ) + 0.1f;

	// Setup control groups.
	for( BcU32 Idx = 0; Idx < 10; ++Idx )
	{
		ControlGroups_.push_back( GaGameUnitIDList() );
	}

	pSimulator_ = new GaGameSimulator( 1.0f / 15.0f, 1.0f, TeamID, Networked );

	pSimulator_->addUnit( GGameUnit_Trebuchet, 0, BcFixedVec2d( -19.0f,  0.0f ) );

	pSimulator_->addUnit( GGameUnit_Archer, 0, BcFixedVec2d( -17.0f, -2.0f  ) );
	pSimulator_->addUnit( GGameUnit_Archer, 0, BcFixedVec2d( -17.0f, -1.0f  ) );
	pSimulator_->addUnit( GGameUnit_Archer, 0, BcFixedVec2d( -17.0f,  0.0f  ) );
	pSimulator_->addUnit( GGameUnit_Archer, 0, BcFixedVec2d( -17.0f,  1.0f  ) );
	pSimulator_->addUnit( GGameUnit_Archer, 0, BcFixedVec2d( -17.0f,  2.0f  ) );

	pSimulator_->addUnit( GGameUnit_Soldier, 0, BcFixedVec2d( -15.0f, -2.0f  ) );
	pSimulator_->addUnit( GGameUnit_Soldier, 0, BcFixedVec2d( -15.0f, -1.0f  ) );
	pSimulator_->addUnit( GGameUnit_Soldier, 0, BcFixedVec2d( -15.0f,  0.0f  ) );
	pSimulator_->addUnit( GGameUnit_Soldier, 0, BcFixedVec2d( -15.0f,  1.0f  ) );
	pSimulator_->addUnit( GGameUnit_Soldier, 0, BcFixedVec2d( -15.0f,  2.0f  ) );

	pSimulator_->addUnit( GGameUnit_Soldier, 0, BcFixedVec2d( -14.0f, -2.0f  ) );
	pSimulator_->addUnit( GGameUnit_Soldier, 0, BcFixedVec2d( -14.0f, -1.0f  ) );
	pSimulator_->addUnit( GGameUnit_Soldier, 0, BcFixedVec2d( -14.0f,  0.0f  ) );
	pSimulator_->addUnit( GGameUnit_Soldier, 0, BcFixedVec2d( -14.0f,  1.0f  ) );
	pSimulator_->addUnit( GGameUnit_Soldier, 0, BcFixedVec2d( -14.0f,  2.0f  ) );

	pSimulator_->addUnit( GGameUnit_Trebuchet, 1, BcFixedVec2d(  19.0f,  0.0f ) );
	
	pSimulator_->addUnit( GGameUnit_Archer, 1, BcFixedVec2d(  17.0f, -2.0f  ) );
	pSimulator_->addUnit( GGameUnit_Archer, 1, BcFixedVec2d(  17.0f, -1.0f  ) );
	pSimulator_->addUnit( GGameUnit_Archer, 1, BcFixedVec2d(  17.0f,  0.0f  ) );
	pSimulator_->addUnit( GGameUnit_Archer, 1, BcFixedVec2d(  17.0f,  1.0f  ) );
	pSimulator_->addUnit( GGameUnit_Archer, 1, BcFixedVec2d(  17.0f,  2.0f  ) );

	pSimulator_->addUnit( GGameUnit_Soldier, 1, BcFixedVec2d(  15.0f, -2.0f  ) );
	pSimulator_->addUnit( GGameUnit_Soldier, 1, BcFixedVec2d(  15.0f, -1.0f  ) );
	pSimulator_->addUnit( GGameUnit_Soldier, 1, BcFixedVec2d(  15.0f,  0.0f  ) );
	pSimulator_->addUnit( GGameUnit_Soldier, 1, BcFixedVec2d(  15.0f,  1.0f  ) );
	pSimulator_->addUnit( GGameUnit_Soldier, 1, BcFixedVec2d(  15.0f,  2.0f  ) );

	pSimulator_->addUnit( GGameUnit_Soldier, 1, BcFixedVec2d(  14.0f, -2.0f  ) );
	pSimulator_->addUnit( GGameUnit_Soldier, 1, BcFixedVec2d(  14.0f, -1.0f  ) );
	pSimulator_->addUnit( GGameUnit_Soldier, 1, BcFixedVec2d(  14.0f,  0.0f  ) );
	pSimulator_->addUnit( GGameUnit_Soldier, 1, BcFixedVec2d(  14.0f,  1.0f  ) );
	pSimulator_->addUnit( GGameUnit_Soldier, 1, BcFixedVec2d(  14.0f,  2.0f  ) );
}

//////////////////////////////////////////////////////////////////////////
// destroy
void GaGameComponent::destroy()
{
	delete pSimulator_;
	pSimulator_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// GaGameComponent
//virtual
void GaGameComponent::update( BcReal Tick )
{
	BcU32 Param = FontMaterial_->findParameter( "aAlphaTestStep" );
	FontMaterial_->setParameter( Param, BcVec2d( 0.4f, 0.5f ) );

	if( GameState_ == GS_PLAYING )
	{
		pSimulator_->tick( Tick );

		if( pSimulator_->getTimeOut() > 5.0f )
		{
			GameState_ = GS_TIMEOUT;
		}

		if( pSimulator_->getUnitCount( TeamID_ ) == 0 ||
			pSimulator_->getUnitCount( 1 - TeamID_ ) == 0 )
		{
			GameState_ = GS_OVER;
		}


		if( !Networked_ )
		{
			AITickTime_ -= Tick;

			if( AITickTime_ < 0.0f )
			{
				pSimulator_->runAI( 1 - TeamID_ );
				AITickTime_ = AITickMaxTime_;
			}
		}
	}
	
	//if( TeamID_ == 0 )
	{
		if( CanvasComponent_.isValid() )
		{
			OsClient* pClient = OsCore::pImpl()->getClient( 0 );
			BcReal HW = static_cast< BcReal >( pClient->getWidth() ) / 2.0f;
			BcReal HH = static_cast< BcReal >( pClient->getHeight() ) / 2.0f;
			BcReal AspectRatio = HW / HH;

			BcMat4d Ortho;
			Ortho.orthoProjection( -HW, HW, HH, -HH, -1.0f, 1.0f );

			// Clear canvas and push projection matrix.
			CanvasComponent_->clear();   

			CanvasComponent_->pushMatrix( Ortho );

			CanvasComponent_->setMaterialComponent( BackgroundMaterial_ );
			CanvasComponent_->drawSpriteCentered( BcVec2d( 0.0f, 0.0f ), BcVec2d( 1280.0f, 720.0f ), 0, RsColour( 1.0f, 1.0f, 1.0f, 1.0f ), 0 );

			CanvasComponent_->setMaterialComponent( SpriteSheetMaterials_[ 0 ] );
			pSimulator_->render( CanvasComponent_, 0 );
			CanvasComponent_->setMaterialComponent( SpriteSheetMaterials_[ 1 ] );
			pSimulator_->render( CanvasComponent_, 1 );

			// Find unit over mouse.
			GaGameUnitIDList SelectionList;
			if( MouseDown_ )
			{
				if( BoxSelection_ )
				{
					pSimulator_->findUnits( SelectionList, StartGameCursorPosition_, EndGameCursorPosition_, BcErrorCode, BcErrorCode );
				}
			}
			else
			{
				pSimulator_->findUnits( SelectionList, GameCursorPosition_, 0.8f, BcErrorCode, BcErrorCode );
				while( SelectionList.size() > 1 )
				{
					SelectionList.pop_back();
				}
			}

			for( BcU32 Idx = 0; Idx < UnitSelection_.size(); ++Idx )
			{
				SelectionList.push_back( UnitSelection_[ Idx ] );
			}

			CanvasComponent_->setMaterialComponent( HUDMaterial_ );
			pSimulator_->renderHUD( CanvasComponent_, SelectionList, TeamID_ );
		}

		switch( GameState_ )
		{
		case GS_PLAYING:
			{
				// Draw cursor.
				CanvasComponent_->setMaterialComponent( HUDMaterial_ );
				CanvasComponent_->drawSpriteCentered( BcVec2d( CursorPosition_.x(), CursorPosition_.y() ), BcVec2d( 64.0f, 64.0f ), 1, AttackMove_ ? RsColour::RED : RsColour::WHITE, 10 );

				// Draw selection box.
				if( MouseDown_ && BoxSelection_ ) 
				{
					BcVec2d Min = BcVec2d( StartGameCursorPosition_.x(), StartGameCursorPosition_.y() ) * 32.0f;
					BcVec2d Max = BcVec2d( GameCursorPosition_.x(), GameCursorPosition_.y() ) * 32.0f;
					CanvasComponent_->drawSprite( Min, Max - Min, 0, RsColour::GREEN * RsColour( 1.0f, 1.0f, 1.0f, 0.1f ), 11 );
				}

				std::string Text0 = TeamID_ == 0 ? "YOU ARE RED!" : "YOU ARE BLUE!";
				BcVec2d TextSize0 = Font_->draw( CanvasComponent_, BcVec2d( 0.0f, 0.0f ), Text0, RsColour::WHITE, BcTrue );
				Font_->draw( CanvasComponent_, ( -TextSize0 / 2.0f ) + BcVec2d( 0.0f, -350.0f ), Text0, TeamID_ == 0 ? RsColour::RED : RsColour::BLUE, BcFalse );

			}
			break;

		case GS_TIMEOUT:
			{
				std::string Text0 = "Connection timed out :(";
				std::string Text1 = "Press ESC to Quit.";
				BcVec2d TextSize0 = Font_->draw( CanvasComponent_, BcVec2d( 0.0f, 0.0f ), Text0, RsColour::WHITE, BcTrue );
				Font_->draw( CanvasComponent_, ( -TextSize0 / 2.0f ) + BcVec2d( 0.0f, -64.0f ), Text0, RsColour::WHITE, BcFalse );
				BcVec2d TextSize1 = Font_->draw( CanvasComponent_, BcVec2d( 0.0f, 0.0f ), Text1, RsColour::WHITE, BcTrue );
				Font_->draw( CanvasComponent_, ( -TextSize1 / 2.0f ) + BcVec2d( 0.0f, 64.0f ), Text1, RsColour::WHITE, BcFalse );
			}
			break;

		case GS_OVER:
			{
				if( pSimulator_->getUnitCount( TeamID_ ) > 0 )
				{
					std::string Text0 = "You Won! :D";
					std::string Text1 = "Press ESC to Quit.";
					BcVec2d TextSize0 = Font_->draw( CanvasComponent_, BcVec2d( 0.0f, 0.0f ), Text0, RsColour::WHITE, BcTrue );
					Font_->draw( CanvasComponent_, ( -TextSize0 / 2.0f ) + BcVec2d( 0.0f, -64.0f ), Text0, RsColour::WHITE, BcFalse );
					BcVec2d TextSize1 = Font_->draw( CanvasComponent_, BcVec2d( 0.0f, 0.0f ), Text1, RsColour::WHITE, BcTrue );
					Font_->draw( CanvasComponent_, ( -TextSize1 / 2.0f ) + BcVec2d( 0.0f, 64.0f ), Text1, RsColour::WHITE, BcFalse );
				}
				else
				{
					std::string Text0 = "You Lost! :<";
					std::string Text1 = "Press ESC to Quit.";
					BcVec2d TextSize0 = Font_->draw( CanvasComponent_, BcVec2d( 0.0f, 0.0f ), Text0, RsColour::WHITE, BcTrue );
					Font_->draw( CanvasComponent_, ( -TextSize0 / 2.0f ) + BcVec2d( 0.0f, -64.0f ), Text0, RsColour::WHITE, BcFalse );
					BcVec2d TextSize1 = Font_->draw( CanvasComponent_, BcVec2d( 0.0f, 0.0f ), Text1, RsColour::WHITE, BcTrue );
					Font_->draw( CanvasComponent_, ( -TextSize1 / 2.0f ) + BcVec2d( 0.0f, 64.0f ), Text1, RsColour::WHITE, BcFalse );
				}
			}
			break;
		}
	}


}

//////////////////////////////////////////////////////////////////////////
// GaGameComponent
//virtual
void GaGameComponent::onAttach( ScnEntityWeakRef Parent )
{
	// Find canvas component on parent. TODO: Make a utility function for this.
	for( BcU32 Idx = 0; Idx < Parent->getNoofComponents(); ++Idx )
	{
		CanvasComponent_ = Parent->getComponent( Idx );

		if( CanvasComponent_.isValid() )
		{
			break;
		}
	}

	// Materials.
	ScnMaterialRef Material;
	if( CsCore::pImpl()->requestResource( "game", "default", Material ) )
	{
		if( CsCore::pImpl()->createResource( BcName::INVALID, DefaultMaterial_, Material, BcErrorCode ) )
		{
			Parent->attach( DefaultMaterial_ );
		}
	}
	if( CsCore::pImpl()->requestResource( "game", "background", Material ) )
	{
		if( CsCore::pImpl()->createResource( BcName::INVALID, BackgroundMaterial_, Material, BcErrorCode ) )
		{
			Parent->attach( BackgroundMaterial_ );
		}
	}
	if( CsCore::pImpl()->requestResource( "game", "spritesheet0", Material ) )
	{
		if( CsCore::pImpl()->createResource( BcName::INVALID, SpriteSheetMaterials_[ 0 ], Material, BcErrorCode ) )
		{
			Parent->attach( SpriteSheetMaterials_[ 0 ] );
		}
	}
	if( CsCore::pImpl()->requestResource( "game", "spritesheet1", Material ) )
	{
		if( CsCore::pImpl()->createResource( BcName::INVALID, SpriteSheetMaterials_[ 1 ], Material, BcErrorCode ) )
		{
			Parent->attach( SpriteSheetMaterials_[ 1 ] );
		}
	}
	if( CsCore::pImpl()->requestResource( "game", "hud", Material ) )
	{
		if( CsCore::pImpl()->createResource( BcName::INVALID, HUDMaterial_, Material, BcErrorCode ) )
		{
			Parent->attach( HUDMaterial_ );
		}
	}

	// Font
	ScnFontRef Font;
	if( CsCore::pImpl()->requestResource( "game", "default", Font ) && CsCore::pImpl()->requestResource( "game", "font", Material ) )
	{
		if( CsCore::pImpl()->createResource( BcName::INVALID, Font_, Font, Material ) )
		{
			FontMaterial_ = Font_->getMaterialComponent();
			Parent->attach( Font_ );
		}
	}
	
	// Bind input events.
	//if( TeamID_ == 0 )
	{
		OsEventInputMouse::Delegate OnMouseEvent = OsEventInputMouse::Delegate::bind< GaGameComponent, &GaGameComponent::onMouseEvent >( this );
		OsEventInputKeyboard::Delegate OnKeyEvent = OsEventInputKeyboard::Delegate::bind< GaGameComponent, &GaGameComponent::onKeyEvent >( this );
		OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEUP, OnMouseEvent );
		OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEMOVE, OnMouseEvent );
		OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEDOWN, OnMouseEvent );
		OsCore::pImpl()->subscribe( osEVT_INPUT_KEYDOWN, OnKeyEvent );
		OsCore::pImpl()->subscribe( osEVT_INPUT_KEYUP, OnKeyEvent );
	}

	// Don't forget to attach!
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// GaGameComponent
//virtual
void GaGameComponent::onDetach( ScnEntityWeakRef Parent )
{
	// Null canvas reference.
	CanvasComponent_ = NULL;

	// Detach materials.
	Parent->detach( DefaultMaterial_ );
	Parent->detach( BackgroundMaterial_ );
	Parent->detach( SpriteSheetMaterials_[ 0 ] );
	Parent->detach( SpriteSheetMaterials_[ 1 ] );
	Parent->detach( HUDMaterial_ );
	Parent->detach( Font_ );

	// Unsubscribe.
	OsCore::pImpl()->unsubscribeAll( this );

	// Don't forget to detach!
	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onMouseEvent
eEvtReturn GaGameComponent::onMouseEvent( EvtID ID, const OsEventInputMouse& Event )
{
	// Convert to rendering space.
	OsClient* pClient = OsCore::pImpl()->getClient( 0 );
	BcReal HW = static_cast< BcReal >( pClient->getWidth() ) / 2.0f;
	BcReal HH = static_cast< BcReal >( pClient->getHeight() ) / 2.0f;
	CursorPosition_.set( Event.MouseX_ - HW, Event.MouseY_ - HH );
	GameCursorPosition_ = CursorPosition_ / 32.0f;
	EndGameCursorPosition_ = GameCursorPosition_;

	if( MouseDown_ && ( StartGameCursorPosition_ - EndGameCursorPosition_ ).magnitudeSquared() > BcFixed( 8.0f ) )
	{
		BoxSelection_ = BcTrue;
	}

	if( ID == osEVT_INPUT_MOUSEDOWN )
	{
		StartGameCursorPosition_ = GameCursorPosition_;
		MouseDown_ = BcTrue;
		BoxSelection_ = BcFalse;
	}
	else if( ID == osEVT_INPUT_MOUSEUP )
	{
		EndGameCursorPosition_ = GameCursorPosition_;
		MouseDown_ = BcFalse;
		GaGameUnitIDList FoundUnits;

		if( BoxSelection_ )
		{
			pSimulator_->findUnits( FoundUnits, StartGameCursorPosition_, EndGameCursorPosition_, BcErrorCode, 1 << TeamID_ );
		}
		else
		{
			pSimulator_->findUnits( FoundUnits, GameCursorPosition_, 0.8f, BcErrorCode, 1 << TeamID_ );
			while( FoundUnits.size() > 1 )
			{
				FoundUnits.pop_back();
			}
		}

		// If we found units, then set selection.
		if( FoundUnits.size() > 0 )
		{
			UnitSelection_ = FoundUnits;
		}
		else
		{
			// If we aren't box selection do action.
			if( BoxSelection_ == BcFalse )
			{
				BcU32 TargetUnitID = BcErrorCode;

				// Determine if it's an attack move or not.
				pSimulator_->findUnits( FoundUnits, GameCursorPosition_, 0.8f, BcErrorCode, 1 << ( 1 - TeamID_ ) );
				while( FoundUnits.size() > 1 )
				{
					FoundUnits.pop_back();
				}

				if( FoundUnits.size() == 1 )
				{
					TargetUnitID = FoundUnits[ 0 ];
				}

				// Otherwise, tell found units to move.
				BcFixedVec2d CentralPosition;
				BcFixed Divisor;
				for( BcU32 Idx = 0; Idx < UnitSelection_.size(); ++Idx )
				{
					GaGameUnit* pGameUnit( pSimulator_->getUnit( UnitSelection_[ Idx ] ) );
					if( pGameUnit != NULL )
					{
						CentralPosition += pGameUnit->getPosition();
						Divisor += 1.0f;
					}
				}

				if( UnitSelection_.size() > 0 && Divisor > 0.0f )
				{
					CentralPosition /= Divisor;

					GameCursorPosition_ = BcFixedVec2d( ( GameCursorPosition_.x() ), ( GameCursorPosition_.y() ) );

					BcFixed PlayfieldHW = 1280.0f * 0.5f / 32.0f;
					BcFixed PlayfieldHH = 720.0f * 0.5f / 32.0f;

					for( BcU32 Idx = 0; Idx < UnitSelection_.size(); ++Idx )
					{
						GaGameUnit* pGameUnit( pSimulator_->getUnit( UnitSelection_[ Idx ] ) );
						if( pGameUnit != NULL )
						{
							if( TargetUnitID == BcErrorCode )
							{
								GaGameUnitMoveEvent Event;
								Event.UnitID_ = pGameUnit->getID();
								Event.Position_ = ( ( pGameUnit->getPosition() - CentralPosition ) * 0.0f ) + GameCursorPosition_;

								Event.Position_.x( BcClamp( Event.Position_.x(), -PlayfieldHW, PlayfieldHW ) );
								Event.Position_.y( BcClamp( Event.Position_.y(), -PlayfieldHH, PlayfieldHH ) );

								Event.IsAttackMove_ = AttackMove_;
								
								pSimulator_->publish( gaEVT_UNIT_MOVE, Event );
							}
							else
							{
								GaGameUnitAttackEvent Event;
								Event.UnitID_ = pGameUnit->getID();
								Event.TargetUnitID_ = TargetUnitID;
								pSimulator_->publish( gaEVT_UNIT_ATTACK, Event );
							}
						}
					}
				}

				// Toggle off attack move.
				AttackMove_ = BcFalse;
			}
			else
			{
				// If we were box selecting clear selection.
				UnitSelection_.clear();
			}
		}

		BoxSelection_ = BcFalse;
	}
	
	return evtRET_PASS;
}


//////////////////////////////////////////////////////////////////////////
// onKeyEvent
eEvtReturn GaGameComponent::onKeyEvent( EvtID ID, const OsEventInputKeyboard& Event )
{
	if( Event.KeyCode_ == OsEventInputKeyboard::KEYCODE_CONTROL )
	{
		if( ID == osEVT_INPUT_KEYDOWN )
		{
			CtrlDown_ = BcTrue;
		}
		else if( ID == osEVT_INPUT_KEYUP )
		{
			CtrlDown_ = BcFalse;
		}
	}
	else if( Event.KeyCode_ >= '0' && Event.KeyCode_ <= '9' )
	{
		if( ID == osEVT_INPUT_KEYDOWN )
		{
			BcU32 Idx = Event.KeyCode_ - '0';
			if( CtrlDown_ )
			{
				ControlGroups_[ Idx ] = UnitSelection_;
			}
			else
			{
				UnitSelection_ = ControlGroups_[ Idx ];
			}
		}
	}
	else if( Event.KeyCode_ == 'A' )
	{
		if( ID == osEVT_INPUT_KEYDOWN )
		{
			AttackMove_ = !AttackMove_;
		}
	}
	else if( Event.KeyCode_ == 'S' )
	{
		if( ID == osEVT_INPUT_KEYDOWN )
		{
			for( BcU32 Idx = 0; Idx < UnitSelection_.size(); ++Idx )
			{
				GaGameUnit* pGameUnit( pSimulator_->getUnit( UnitSelection_[ Idx ] ) );
				if( pGameUnit != NULL )
				{
					GaGameUnitIdleEvent Event;
					Event.UnitID_ = pGameUnit->getID();
					pSimulator_->publish( gaEVT_UNIT_IDLE, Event );
				}
			}
		}
	}
	else if( Event.KeyCode_ == OsEventInputKeyboard::KEYCODE_ESCAPE )
	{
		GaTopState::pImpl()->startMatchmaking();
	}


	return evtRET_PASS;
}
