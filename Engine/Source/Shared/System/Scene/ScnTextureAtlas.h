/**************************************************************************
*
* File:		ScnTextureAtlas.h
* Author:	Neil Richardson 
* Ver/Date:	24/04/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __ScnTextureAtlas_H__
#define __ScnTextureAtlas_H__

#include "Base/BcTypes.h"

#include "System/Scene/ScnTexture.h"
#include "System/Scene/ScnTypes.h"

//////////////////////////////////////////////////////////////////////////
// ScnTextureRef
typedef CsResourceRef< class ScnTextureAtlas > ScnTextureAtlasRef;

//////////////////////////////////////////////////////////////////////////
// ScnTextureAtlas
class ScnTextureAtlas:
	public ScnTexture
{
public:
	DECLARE_RESOURCE( ScnTexture, ScnTextureAtlas );
	
#if PSY_SERVER
	virtual BcBool						import( class CsPackageImporter& Importer, const Json::Value& Object );
#endif	

	virtual const ScnRect&				getRect( BcU32 Idx );
	virtual BcU32						noofRects();

protected:
	virtual void						fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData );

protected:
	struct TAtlasHeader
	{
		BcU32 NoofTextures_;
	};
	
	TAtlasHeader* pAtlasHeader_;	
	ScnRect* pAtlasRects_;
};



#endif


