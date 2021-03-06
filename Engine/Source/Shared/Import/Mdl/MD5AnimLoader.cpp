/**************************************************************************
*
* File:		MD5AnimLoader.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "MD5AnimLoader.h"

#include "Base/BcDebug.h"
#include "Base/BcFile.h"
#include "Base/BcString.h"
#include "Base/BcQuat.h"

#include "MdlAnim.h"

//////////////////////////////////////////////////////////////////////////
// Constructor
MD5AnimLoader::MD5AnimLoader()
{

}

//////////////////////////////////////////////////////////////////////////
// Destructor
MD5AnimLoader::~MD5AnimLoader()
{

}

//////////////////////////////////////////////////////////////////////////
// load
MdlAnim* MD5AnimLoader::load( const BcChar* FileName, const BcChar* NodeName )
{
	BcFile File;
	BcBool Ret;

	ParseMode_ = PM_MAIN;

	BcU32 iJoint = 0;
	BcU32 iBound = 0;
	BcU32 iFrame = 0;
	BcU32 iAnimComp = 0;

	File.open( FileName );

	BcChar Buffer[1024];
	BcChar Command[1024];
	BcChar* pBuffer;

	// Begin the parsage.
	while( !File.eof() )
	{
		// Parse a line	without	the	spaces at the start	or tabs.
		{
			pBuffer	= &Buffer[0];

			BcChar TheChar = 0;
			BcBool bAtSentence = BcFalse;
			while( TheChar != 10 &&	!File.eof()	)
			{
				File.read( &TheChar, 1 );

				if ( TheChar != '\t' )
				{
					bAtSentence	= BcTrue;
				}

				if ( TheChar != 10 &&
				     TheChar != 13 &&
				     bAtSentence )
				{
					*pBuffer = TheChar;
					++pBuffer;
					Ret	= ( pBuffer < ( Buffer + 1024 ) );
					BcAssert( Ret );
					if( !Ret )
					{
						return NULL;
					}
				}
			}

			// Terminate it.
			*pBuffer = 0;
			++pBuffer;
		}

		sscanf( Buffer, "%s", Command );

		if ( BcStrLength( Buffer ) == 0 )
		{
			Command[ 0 ] = 0;
		}

		switch( ParseMode_ )
		{
		case PM_MAIN:
			{
				if( BcStrCompare( "numJoints", Command ) )
				{
					sscanf( Buffer, "numJoints %u", &nJoints_ );
					
					pJoints_ = new MD5_Joint[ nJoints_ ]; 

					for( BcU32 i = 0; i < nFrames_; ++i )
					{
						pFrames_[ i ].nKeys_ = nJoints_;
						pFrames_[ i ].pKeys_ = new MD5_Joint[ nJoints_ ];
					}
				}
				else if( BcStrCompare( "numFrames", Command ) )
				{
					sscanf( Buffer, "numFrames %u", &nFrames_ );
					nBounds_ = nFrames_;
					
					pFrames_ = new MD5_Frame[ nFrames_ ]; 
					pBounds_ = new MD5_Bound[ nBounds_ ]; 
				}
				else if( BcStrCompare( "frameRate", Command ) )
				{
					sscanf( Buffer, "frameRate %f", &FrameRate_ );
				}
				else if( BcStrCompare( "numAnimatedComponents", Command ) )
				{
					sscanf( Buffer, "numAnimatedComponents %u", &nAnimComponents_ );
					pAnimComponents_ = new BcF32[ nAnimComponents_ ];
				}
				else if( BcStrCompare( "hierarchy", Command ) )
				{
					ParseMode_ = PM_HIERARCHY;
					iJoint = 0;
				}
				else if( BcStrCompare( "bounds", Command ) )
				{
					ParseMode_ = PM_BOUNDS;
					iBound = 0;
				}
				else if( BcStrCompare( "baseframe", Command ) )
				{
					ParseMode_ = PM_BASEFRAME;
					iJoint = 0;
				}
				else if ( BcStrCompare( "frame", Command ) )
				{
					ParseMode_ = PM_FRAME;
					iJoint = 0;
					iAnimComp = 0;
				}
			}
			break;
		
		case PM_HIERARCHY:
			{
				if( BcStrCompare( "}", Command ) )
				{
					ParseMode_ = PM_MAIN;
				}
				else
				{
					BcAssert( iJoint < nJoints_ );
					sscanf( Buffer, "%s %u %u %u", &pJoints_[ iJoint ].Name_[0],
					                               &pJoints_[ iJoint ].ParentID_,
					                               &pJoints_[ iJoint ].AnimMask_,
					                               &pJoints_[ iJoint ].AnimOffset_ );
					++iJoint;
				}
			}
			break;

		case PM_BOUNDS:
			{
				if( BcStrCompare( "}", Command ) )
				{
					ParseMode_ = PM_MAIN;
				}
				else
				{
					BcAssert( iBound < nBounds_ );
					sscanf( Buffer, "( %f %f %f ) ( %f %f %f )", &pBounds_[ iBound ].MinX_,
					                                             &pBounds_[ iBound ].MinY_,
					                                             &pBounds_[ iBound ].MinZ_,
					                                             &pBounds_[ iBound ].MaxX_,
					                                             &pBounds_[ iBound ].MaxY_,
					                                             &pBounds_[ iBound ].MaxZ_ );
					iBound++;
				}
			}
			break;

		case PM_BASEFRAME:
			{
				if( BcStrCompare( "}", Command ) )
				{
					ParseMode_ = PM_MAIN;
				}
				else
				{
					BcAssert( iJoint < nJoints_ );
					sscanf( Buffer, "( %f %f %f ) ( %f %f %f )", &pJoints_[ iJoint ].TX_,
					                                             &pJoints_[ iJoint ].TY_,
					                                             &pJoints_[ iJoint ].TZ_,
					                                             &pJoints_[ iJoint ].QX_,
					                                             &pJoints_[ iJoint ].QY_,
					                                             &pJoints_[ iJoint ].QZ_ );
					iJoint++;
				}
			}
			break;

		case PM_FRAME:
			{
				if( BcStrCompare( "}", Command ) )
				{
					// Setup all joints.
					BcAssert( iFrame < nFrames_ );
					MD5_Frame* pFrame = &pFrames_[ iFrame++ ];

					for( BcU32 i = 0; i < nJoints_; ++i )
					{
						// Copy from base.
						pFrame->pKeys_[ i ] = pJoints_[ i ];

						// Parse from anim data.
						BcF32* pData = &pAnimComponents_[ pFrame->pKeys_[ i ].AnimOffset_ ];
						BcU32 iVal = 0;
						BcU32 AnimMask = pFrame->pKeys_[ i ].AnimMask_;

						if( AnimMask & 1 )
						{
							pFrame->pKeys_[ i ].TX_ = pData[ iVal++ ];
						}

						if( AnimMask & 2 )
						{
							pFrame->pKeys_[ i ].TY_ = pData[ iVal++ ];
						}

						if( AnimMask & 4 )
						{
							pFrame->pKeys_[ i ].TZ_ = pData[ iVal++ ];
						}

						if( AnimMask & 8 )
						{
							pFrame->pKeys_[ i ].QX_ = pData[ iVal++ ];
						}

						if( AnimMask & 16 )
						{
							pFrame->pKeys_[ i ].QY_ = pData[ iVal++ ];
						}

						if( AnimMask & 32 )
						{
							pFrame->pKeys_[ i ].QZ_ = pData[ iVal++ ];
						}
					}

					ParseMode_ = PM_MAIN;
				}
				else
				{
					BcF32 Data[]=
					{
						1e6f,
						1e6f,
						1e6f,
						1e6f,
						1e6f,
						1e6f,
					};

					// NOTE: This may not actually work..
					sscanf( Buffer, "%f %f %f %f %f %f", &Data[0],
					                                     &Data[1],
														 &Data[2],
														 &Data[3],
														 &Data[4],
														 &Data[5] );

					// Parse into array.
					for( BcU32 i = 0; i < 6; ++i )
					{
						if( Data[i] < 1e6f )
						{
							BcAssert( iAnimComp < nAnimComponents_ );
							pAnimComponents_[ iAnimComp ] = Data[i];
							iAnimComp++;
						}
						else
						{
							break;
						}
					}
				}
			}
			break;
		}
	}

	// Create animation.
	MdlAnim* pAnimation = new MdlAnim();

	// Build up all nodes.
	for( BcU32 i = 0; i < nJoints_; ++i )
	{
		MdlAnimNode Node;

		BcStrCopy( Node.Name_, pJoints_[ i ].Name_ );
		if( pJoints_[ i ].ParentID_ != -1 )
		{
			BcStrCopy( Node.Parent_, pJoints_[ pJoints_[ i ].ParentID_ ].Name_ );
		}
		else
		{
			Node.Parent_[ 0 ] = '\0';
		}

		for( BcU32 j = 0; j < nFrames_; ++j )
		{
			MD5_Joint* pJoint = &pFrames_[ j ].pKeys_[ i ];

			MdlAnimKey Key;

			Key.R_.set( pJoint->QX_, pJoint->QY_, pJoint->QZ_, 0.0f );
			Key.S_.set( 1.0f, 1.0f, 1.0f );
			Key.T_.set( pJoint->TX_, pJoint->TY_, pJoint->TZ_ );

			Key.R_.calcFromXYZ();

			Node.KeyList_.push_back( Key );
		}

		pAnimation->addNode( Node );
	}

	// Cleanup
	nJoints_ = 0;
	delete [] pJoints_;
	pJoints_ = NULL;

	nBounds_ = 0;
	delete [] pBounds_;
	pBounds_ = NULL;

	nAnimComponents_ = 0;
	delete [] pAnimComponents_;
	pAnimComponents_ = NULL;

	for( BcU32 i = 0; i < nFrames_; ++i )
	{
		delete [] pFrames_[ i ].pKeys_;
	}

	nFrames_ = 0;
	delete [] pFrames_;
	pFrames_ = NULL;

	//
	/*
	BcMat4d RootTransform;
	RootTransform.identity();
	pRootNode->makeRelativeTransform( RootTransform );
	*/

	return pAnimation;//pRootNode;
}

/*
//////////////////////////////////////////////////////////////////////////
// buildNormals
void MD5AnimLoader::buildNormals()
{
	for ( BcU32 i = 0; i < ( nIndices_ / 3 ); ++i )
	{
		BcU32 TA = pIndices_[ ( i * 3 ) + 0 ];
		BcU32 TB = pIndices_[ ( i * 3 ) + 1 ];
		BcU32 TC = pIndices_[ ( i * 3 ) + 2 ];

		RsVertexSkinGL& VertA = pVerts_[ TA ];
		RsVertexSkinGL& VertB = pVerts_[ TB ];
		RsVertexSkinGL& VertC = pVerts_[ TC ];

		BcVec3d VertPosA( VertA.X_, VertA.Y_, VertA.Z_ );
		BcVec3d VertPosB( VertB.X_, VertB.Y_, VertB.Z_ );
		BcVec3d VertPosC( VertC.X_, VertC.Y_, VertC.Z_ );

		BcVec3d Normal = ( VertPosA - VertPosB ).cross( ( VertPosB - VertPosC ) );

		VertA.NX_ += Normal.x();
		VertB.NX_ += Normal.x();
		VertC.NX_ += Normal.x();

		VertA.NY_ += Normal.y();
		VertB.NY_ += Normal.y();
		VertC.NY_ += Normal.y();

		VertA.NZ_ += Normal.z();
		VertB.NZ_ += Normal.z();
		VertC.NZ_ += Normal.z();
	}

	for ( BcU32 i = 0; i < nVerts_; ++i )
	{
		RsVertexSkinGL& Vert = pVerts_[ i ];
		BcVec3d Normal( Vert.NX_, Vert.NY_, Vert.NZ_ );
		Normal.normalise();

		Vert.NX_ = Normal.x();
		Vert.NY_ = Normal.y();
		Vert.NZ_ = Normal.z();

		BcReal Mag = Normal.magnitude(); 
		BcAssert( BcAbs( Mag - 1.0f ) < 0.00001f );
	}
}

//////////////////////////////////////////////////////////////////////////
// buildTangents
void MD5AnimLoader::buildTangents()
{
	BcVec3d *pTan1 = new BcVec3d[ nVerts_ * 2 ];
	BcVec3d *pTan2 = pTan1 + nVerts_;

	memset( pTan1, 0, sizeof( BcVec3d ) * nVerts_ * 2 );

	for ( BcU32 i = 0; i < ( nIndices_ / 3 ); ++i )
	{
		BcU32 TA = pIndices_[ ( i * 3 ) + 0 ];
		BcU32 TB = pIndices_[ ( i * 3 ) + 1 ];
		BcU32 TC = pIndices_[ ( i * 3 ) + 2 ];

		RsVertexSkinGL& VertA = pVerts_[ TA ];
		RsVertexSkinGL& VertB = pVerts_[ TB ];
		RsVertexSkinGL& VertC = pVerts_[ TC ];

		BcVec3d VertPosA( VertA.X_, VertA.Y_, VertA.Z_ );
		BcVec3d VertPosB( VertB.X_, VertB.Y_, VertB.Z_ );
		BcVec3d VertPosC( VertC.X_, VertC.Y_, VertC.Z_ );

		BcVec2d VertUVA( VertA.U_, VertA.V_ );
		BcVec2d VertUVB( VertB.U_, VertB.V_ );
		BcVec2d VertUVC( VertC.U_, VertC.V_ );

		BcReal X1 = VertPosB.x() - VertPosA.x();
		BcReal X2 = VertPosC.x() - VertPosA.x();
		BcReal Y1 = VertPosB.y() - VertPosA.y();
		BcReal Y2 = VertPosC.y() - VertPosA.y();
		BcReal Z1 = VertPosB.z() - VertPosA.z();
		BcReal Z2 = VertPosC.z() - VertPosA.z();

		BcReal S1 = VertUVB.x() - VertUVA.x();
		BcReal S2 = VertUVC.x() - VertUVA.x();
		BcReal T1 = VertUVB.y() - VertUVA.y();
		BcReal T2 = VertUVC.y() - VertUVA.y();

		BcReal InvR = ( S1 * T2 - S2 * T1 );
		BcReal R = 1.0f / InvR;

		// Validation so it doesn't break everything, just set to a dummy value.
		if( InvR == 0.0f )
		{
			R = 0.0f;
		}

		BcVec3d SDir( ( T2 * X1 - T1 * X2 ) * R, ( T2 * Y1 - T1 * Y2 ) * R, ( T2 * Z1 - T1 * Z2 ) * R );
		BcVec3d TDir( ( S1 * X2 - S2 * X1 ) * R, ( S1 * Y2 - S2 * Y1 ) * R, ( S1 * Z2 - S2 * Z1 ) * R );

		pTan1[ TA ] += SDir;
		pTan1[ TB ] += SDir;
		pTan1[ TC ] += SDir;

		pTan2[ TA ] += TDir;
		pTan2[ TB ] += TDir;
		pTan2[ TC ] += TDir;
	}

	for ( BcU32 i = 0; i < nVerts_ ; ++i )
	{
		RsVertexSkinGL& Vert = pVerts_[ i ];
		BcVec3d Tangent;

		const BcVec3d N( Vert.NX_, Vert.NY_, Vert.NZ_ );
		const BcVec3d& T = pTan1[ i ];

		Tangent = ( T - N * N.dot( T ) );
		Tangent.normalise();

		// Calculate handedness
		BcReal W = ( N.cross( T ).dot( pTan2[ i ] ) < 0.0f ) ? -1.0f : 1.0f;

		if ( W < 0.0f )
		{
			Tangent = -Tangent;
		}

		Vert.TX_ = Tangent.x();
		Vert.TY_ = Tangent.y();
		Vert.TZ_ = Tangent.z();

		BcReal Mag = Tangent.magnitude(); 
		BcAssert( BcAbs( Mag - 1.0f ) < 0.00001f );
	}

	delete[] pTan1;
}*/
