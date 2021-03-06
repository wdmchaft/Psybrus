/**************************************************************************
*
* File:		GaLayeredSpriteComponent.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Renderer for bunny ^_^
*		
*
*
* 
**************************************************************************/

#include "GaLayeredSpriteComponent.h"

#include "GaTopState.h"

#include "GaMainGameState.h"

////////////////////////////////////////////////////////////////////////////////
// Define resource.
DEFINE_RESOURCE( GaLayeredSpriteComponent );

//////////////////////////////////////////////////////////////////////////
// StaticPropertyTable
void GaLayeredSpriteComponent::StaticPropertyTable( CsPropertyTable& PropertyTable )
{
	Super::StaticPropertyTable( PropertyTable );

	PropertyTable.beginCatagory( "GaLayeredSpriteComponent" )
	.endCatagory();
}

////////////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void GaLayeredSpriteComponent::initialise( ScnMaterialRef Material, const BcVec3d& Scale )
{
	// Feet offsets.
	Layers_[ LAYER_FOOT_REAR_0 ].TimeTicker_ = 0.5f;
	Layers_[ LAYER_FOOT_FRONT_0 ].TimeTicker_ = 0.0f;
	Layers_[ LAYER_FOOT_FRONT_1 ].TimeTicker_ = 0.1f;

	Layers_[ LAYER_FOOT_REAR_0 ].StaticMultiplier_ = BcVec2d( 0.0f, 0.0f );
	Layers_[ LAYER_FOOT_FRONT_0 ].StaticMultiplier_ = BcVec2d( 0.0f, 0.0f );
	Layers_[ LAYER_FOOT_FRONT_1 ].StaticMultiplier_ = BcVec2d( 0.0f, 0.0f );

	// No body anim.
	Layers_[ LAYER_BODY ].Multiplier_ = BcVec2d( 0.0f, 0.0f );
	Layers_[ LAYER_BODY ].StaticMultiplier_ = BcVec2d( 0.0f, 0.0f );

	// Ear bounce.
	Layers_[ LAYER_EARS ].Multiplier_ = BcVec2d( 3.0f, 4.0f );
	Layers_[ LAYER_EARS ].StaticMultiplier_ = BcVec2d( 0.0f, 2.0f );

	// Head.
	Layers_[ LAYER_HEAD ].Multiplier_ = BcVec2d( 2.0f, 2.0f );
	Layers_[ LAYER_HEAD ].StaticMultiplier_ = BcVec2d( 0.0f, 1.0f );
	
	// Nose.
	Layers_[ LAYER_NOSE ].Multiplier_ = BcVec2d( 1.0f, 1.5f );
	Layers_[ LAYER_NOSE ].StaticMultiplier_ = BcVec2d( 2.0f, 2.0f );

	// Eyes.
	Layers_[ LAYER_EYES ].Multiplier_ = BcVec2d( 2.0f, 2.0f );
	Layers_[ LAYER_EYES ].StaticMultiplier_ = BcVec2d( 0.0f, 1.0f );

	// Tail.
	Layers_[ LAYER_TAIL ].Multiplier_ = BcVec2d( 0.5f, 0.5f );
	Layers_[ LAYER_TAIL ].StaticMultiplier_ = BcVec2d( 0.5f, 0.5f );
	Layers_[ LAYER_TAIL ].TimeSpeed_ = 1.3f;

	// Direction.
	FaceDirection_ = BcVec2d( 1.0f, 0.0f );
	SmoothFaceDirection_ = FaceDirection_;

	// Material.
	CsCore::pImpl()->createResource( BcName::INVALID, MaterialComponent_, Material, scnSPF_DEFAULT );
	CsCore::pImpl()->createResource( BcName::INVALID, ShadowMaterialComponent_, ScnMaterial::Default, scnSPF_DEFAULT );
	Scale_ = Scale;
}

////////////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void GaLayeredSpriteComponent::destroy()
{
}

////////////////////////////////////////////////////////////////////////////////
// isReady
BcBool GaLayeredSpriteComponent::isReady()
{
	return Super::isReady() && MaterialComponent_.isReady();
}

////////////////////////////////////////////////////////////////////////////////
// update
//virtual
void GaLayeredSpriteComponent::update( BcReal Tick )
{
	Super::update( Tick );

	for( BcU32 Idx = 0; Idx < LAYER_MAX; ++Idx )
	{
		TLayer& Layer = Layers_[ Idx ];
		Layer.TimeTicker_ += Tick * Layer.TimeSpeed_;
		
		if( Layer.TimeTicker_ > 1.0f )
		{
			Layer.TimeTicker_ -= 1.0f;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// render
void GaLayeredSpriteComponent::render( GaMainGameState* pParent, ScnCanvasComponentRef Canvas, const BcVec3d& Position, const BcVec2d& Velocity )
{
	BcBool ShouldAnimate = BcFalse;

	if( Velocity.magnitude() > 8.0f )
	{
		FaceDirection_ = Velocity.normal();
		ShouldAnimate = BcTrue;
	}

	SmoothFaceDirection_ += FaceDirection_;
	SmoothFaceDirection_.normalise();

	BcReal Angle = BcPIMUL2 - BcAtan2( SmoothFaceDirection_.y(), SmoothFaceDirection_.x() );

	BcMat4d PositionMatrix;
	BcMat4d RotationMatrix;

	BcReal RockAngle = ( ShouldAnimate ? BcSin( BcPIMUL2 * Layers_[ LAYER_BODY ].TimeTicker_ ) * 0.4f : 0.0f );

	RotationMatrix.rotation( BcVec3d( 0.0f, 0.0f, Angle ) );

	PositionMatrix.scale( Scale_ );
	PositionMatrix.translation( BcVec3d( Position.x(), Position.y(), 0.0f ) );
	
	Canvas->setMaterialComponent( ShadowMaterialComponent_ );
	pParent->setMaterialComponentParams( ShadowMaterialComponent_, RotationMatrix * PositionMatrix );
	Canvas->drawSpriteCentered3D( BcVec3d( 0.0f, 0.0f, 0.2f ), BcVec2d( 128.0f, 128.0f ), 0, RsColour::WHITE, 6 );

	Canvas->setMaterialComponent( MaterialComponent_ );
	pParent->setMaterialComponentParams( MaterialComponent_, RotationMatrix * PositionMatrix );

	BcVec2d Size = BcVec2d( 192.0f, -172.0f ) * 0.5f;

	BcVec3d LayerDepthOffset( 0.0f, 0.0f, 0.0f );
	for( BcU32 Idx = 0; Idx < LAYER_MAX; ++Idx )
	{
		TLayer& Layer = Layers_[ Idx ];

		BcVec2d LayerOffset = ( ShouldAnimate ? Layer.Multiplier_ : Layer.StaticMultiplier_ );
		BcVec3d LayerPosition = BcVec3d( -BcCos( BcPIMUL2 * Layer.TimeTicker_ ), 0.0f, BcSin( BcPIMUL2 * Layer.TimeTicker_ ) ) * BcVec3d( LayerOffset.x(), 0.0f, LayerOffset.y() );
		
		Canvas->drawSpriteCenteredUp3D( BcVec3d( LayerPosition.x(), LayerPosition.y(), ( LayerPosition.z() - Size.y() * 0.5f ) + Position.z() ) + LayerDepthOffset, Size, Idx, RsColour::WHITE, 8 );

		LayerDepthOffset += BcVec3d( 0.0f, 0.2f, 0.0f ) * RotationMatrix;
	}
	
	//Canvas->popMatrix();
	//Canvas->popMatrix();
}

////////////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void GaLayeredSpriteComponent::onAttach( ScnEntityWeakRef Parent )
{
	Parent->attach( MaterialComponent_ );
	Parent->attach( ShadowMaterialComponent_ );
	
	Super::onAttach( Parent );
}

////////////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void GaLayeredSpriteComponent::onDetach( ScnEntityWeakRef Parent )
{
	Parent->detach( MaterialComponent_ );
	Parent->detach( ShadowMaterialComponent_ );

	Super::onDetach( Parent );
}
