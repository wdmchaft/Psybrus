/**************************************************************************
*
* File:		RsCoreImplGL.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __RSCOREIMPLGL_H__
#define __RSCOREIMPLGL_H__

#include "System/Renderer/RsCore.h"
#include "System/Renderer/GL/RsGL.h"

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class RsFrameGL;
class RsContextGL;
class RsStateBlockGL;

//////////////////////////////////////////////////////////////////////////
// RsCoreImplGL
class RsCoreImplGL:
	public RsCore
{
public:
	RsCoreImplGL();
	virtual ~RsCoreImplGL();
	
public:
	virtual void			open();
	void					open_threaded();
	virtual void			update();
	void					update_threaded();
	virtual void			close();
	void					close_threaded();
	
public:
	virtual RsContext*		getContext( OsClient* pClient );
	virtual void			destroyContext( OsClient* pClient );

	virtual RsTexture*		createTexture( BcU32 Width, BcU32 Height, BcU32 Levels, eRsTextureFormat Format, void* pData = NULL );
	virtual RsRenderTarget*	createRenderTarget( BcU32 Width, BcU32 Height, eRsColourFormat ColourFormat, eRsDepthStencilFormat DepthStencilFormat );
	virtual RsVertexBuffer*	createVertexBuffer( BcU32 Descriptor, BcU32 NoofVertices, void* pVertexData = NULL );
	virtual RsIndexBuffer*	createIndexBuffer( BcU32 NoofIndices, void* pIndexData = NULL );
	virtual RsShader*		createShader( eRsShaderType ShaderType, eRsShaderDataType ShaderDataType, void* pShaderData, BcU32 ShaderDataSize );
	virtual RsProgram*		createProgram( RsShader* pVertexShader, RsShader* pFragmentShader );
	virtual RsPrimitive*	createPrimitive( RsVertexBuffer* pVertexBuffer, RsIndexBuffer* pIndexBuffer );
	virtual void			destroyResource( RsResource* pResource );
	void					updateResource( RsResource* pResource );

private:
	void					createResource( RsResource* pResource );

public:
	RsFrame*				allocateFrame( RsContext* pContext );
	void					queueFrame( RsFrame* pFrame );
	void					queueFrame_threaded( RsFrameGL* pFrame );
	RsStateBlock*			getStateBlock();

public:
	// Platform specific interface.

protected:
	RsStateBlockGL*			pStateBlock_;
	SysFence				RenderSyncFence_;

	typedef std::map< OsClient*, RsContextGL* > TContextMap;
	typedef TContextMap::iterator TContextMapIterator;

	TContextMap				ContextMap_;
		
};

#endif


