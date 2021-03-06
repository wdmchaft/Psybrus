/**************************************************************************
*
* File:		RsProgramGL.cpp
* Author:	Neil Richardson 
* Ver/Date:	28/02/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Renderer/GL/RsProgramGL.h"

////////////////////////////////////////////////////////////////////////////////
// Ctor
RsProgramGL::RsProgramGL( RsShaderGL* pVertexShader, RsShaderGL* pFragmentShader ):
	ParameterBufferSize_( 0 ),
	pVertexShader_( pVertexShader ),
	pFragmentShader_( pFragmentShader )
{
	
}

////////////////////////////////////////////////////////////////////////////////
// Dtor
//virtua
RsProgramGL::~RsProgramGL()
{

}

////////////////////////////////////////////////////////////////////////////////
// create
void RsProgramGL::create()
{
	// Some checks to ensure validity.
	BcAssert( pVertexShader_ != NULL );
	BcAssert( pFragmentShader_ != NULL );
	BcAssert( pVertexShader_->hasHandle() );
	BcAssert( pFragmentShader_->hasHandle() );	

	// Create program.
	GLuint Handle = glCreateProgram();
	BcAssert( Handle != 0 );

	// Attach shaders.
	glAttachShader( Handle, pVertexShader_->getHandle< GLuint >() );
	glAttachShader( Handle, pFragmentShader_->getHandle< GLuint >() );
	
	// Bind default vertex attributes.
	bindAttribute( Handle, rsVC_POSITION,		"aPosition" );
	bindAttribute( Handle, rsVC_NORMAL,			"aNormal" );
	bindAttribute( Handle, rsVC_TANGENT,		"aTangent" );
	bindAttribute( Handle, rsVC_TEXCOORD0,		"aTexCoord0" );
	bindAttribute( Handle, rsVC_TEXCOORD1,		"aTexCoord1" );
	bindAttribute( Handle, rsVC_TEXCOORD2,		"aTexCoord2" );
	bindAttribute( Handle, rsVC_TEXCOORD3,		"aTexCoord3" );
	bindAttribute( Handle, rsVC_COLOUR,			"aColour" );
	
	// Link program.
	glLinkProgram( Handle );	


	// Catch error.
	RsGLCatchError;
	
	// Clear parameter list and buffer.
	ParameterList_.clear();
	ParameterBufferSize_ = 0;

	// Attempt to find uniform names.
	GLint ActiveUniforms = 0;
	glGetProgramiv( Handle, GL_ACTIVE_UNIFORMS, &ActiveUniforms );
	
	for( BcU32 Idx = 0; Idx < (BcU32)ActiveUniforms; ++Idx )
	{
		// Uniform information.
		GLchar UniformName[ 256 ];
		GLsizei UniformNameLength = 0;
		GLint Size = 0;
		GLenum Type = GL_INVALID_VALUE;

		// Get the uniform.
		glGetActiveUniform( Handle, Idx, sizeof( UniformName ), &UniformNameLength, &Size, &Type, UniformName );
		
		// Add it as a parameter.
		if( UniformNameLength > 0 && Type != GL_INVALID_VALUE )
		{
			GLint UniformLocation = glGetUniformLocation( Handle, UniformName );
			addParameter( UniformName, UniformLocation, Type );
		}
	}
	
	// Catch error.
	RsGLCatchError;
	
	// Set handle.
	setHandle( Handle );}

////////////////////////////////////////////////////////////////////////////////
// update
void RsProgramGL::update()
{

}

////////////////////////////////////////////////////////////////////////////////
// destroy
void RsProgramGL::destroy()
{
	GLuint Handle = getHandle< GLuint >();
	if( Handle != 0 )
	{
		glDeleteProgram( Handle );
		setHandle< GLuint >( 0 );
	}
}

////////////////////////////////////////////////////////////////////////////////
// findParameter
//virtual
BcU32 RsProgramGL::getParameterBufferSize() const
{
	return ParameterBufferSize_;
}

////////////////////////////////////////////////////////////////////////////////
// findParameterOffset
//virtual
BcU32 RsProgramGL::findParameterOffset( const BcChar* Name, eRsShaderParameterType& Type, BcU32& Offset ) const
{
	for( TParameterListConstIterator It( ParameterList_.begin() ); It != ParameterList_.end(); ++It )
	{
		if( (*It).Name_ == Name )
		{
			Type = (*It).Type_;
			Offset = (*It).Offset_;
			return BcTrue;
		}
	}
	
	return BcFalse;
}

////////////////////////////////////////////////////////////////////////////////
// bind
//virtual
void RsProgramGL::bind( void* pParameterBuffer )
{
	GLuint Handle = getHandle< GLuint >();
	glUseProgram( Handle );
	RsGLCatchError;
	
	// Bind parameters from buffer if we have been given one.
	// TODO: Only set a value if it isn't equal to the last set one.
	if( pParameterBuffer != NULL )
	{
		BcF32* pFloatParameter = (BcF32*)pParameterBuffer;
		BcS32* pIntParameter = (BcS32*)pParameterBuffer;
		for( TParameterListIterator It( ParameterList_.begin() ); It != ParameterList_.end(); ++It )
		{
			TParameter& Parameter = (*It);
			const GLint ParamHandle = Parameter.Handle_;
			const BcU32 Offset = Parameter.Offset_;
#ifdef PSY_DEBUG
			BcF32* pFloatParameterOffset = &pFloatParameter[ Offset ];
			BcS32* pIntParameterOffset = &pIntParameter[ Offset ];
#endif
			
			switch( Parameter.Type_ )
			{
				case rsSPT_FLOAT:
					glUniform1fv( ParamHandle, 1, &pFloatParameter[ Offset ] );
					break;
				case rsSPT_FLOAT_VEC2:
					glUniform2fv( ParamHandle, 1, &pFloatParameter[ Offset ] );
					break;
				case rsSPT_FLOAT_VEC3:
					glUniform3fv( ParamHandle, 1, &pFloatParameter[ Offset ] );
					break;
				case rsSPT_FLOAT_VEC4:
					glUniform4fv( ParamHandle, 1, &pFloatParameter[ Offset ] );
					break;
				case rsSPT_FLOAT_MAT2:
					glUniformMatrix2fv( ParamHandle, 1, GL_FALSE, &pFloatParameter[ Offset ] );
					break;
				case rsSPT_FLOAT_MAT3:
					glUniformMatrix3fv( ParamHandle, 1, GL_FALSE, &pFloatParameter[ Offset ] );
					break;
				case rsSPT_FLOAT_MAT4:
					glUniformMatrix4fv( ParamHandle, 1, GL_FALSE, &pFloatParameter[ Offset ] );
					break;
				case rsSPT_INT:
				case rsSPT_BOOL:
				case rsSPT_SAMPLER_1D:
				case rsSPT_SAMPLER_2D:
				case rsSPT_SAMPLER_3D:
				case rsSPT_SAMPLER_CUBE:
				case rsSPT_SAMPLER_1D_SHADOW:
				case rsSPT_SAMPLER_2D_SHADOW:
					glUniform1iv( ParamHandle, 1, &pIntParameter[ Offset ] );
					break;
				case rsSPT_INT_VEC2:
				case rsSPT_BOOL_VEC2:
					glUniform2iv( ParamHandle, 1, &pIntParameter[ Offset ] );
					break;
				case rsSPT_INT_VEC3:
				case rsSPT_BOOL_VEC3:
					glUniform3iv( ParamHandle, 1, &pIntParameter[ Offset ] );
					break;
				case rsSPT_INT_VEC4:
				case rsSPT_BOOL_VEC4:
					glUniform4iv( ParamHandle, 1, &pIntParameter[ Offset ] );
					break;
					break;
			}
			
#ifdef PSY_DEBUG
			int Error = glGetError();
			if( Error != 0 )
			{
				BcPrintf( "Error setting parameter \"%s\". Handle=%u, f=%f, i=%u\n", Parameter.Name_.c_str(), ParamHandle, *pFloatParameterOffset, *pIntParameterOffset );
			}
#endif
		}
	}

}

////////////////////////////////////////////////////////////////////////////////
// bindAttribute
void RsProgramGL::bindAttribute( GLuint ProgramHandle, eRsVertexChannel Channel, const BcChar* Name )
{
	glBindAttribLocation( ProgramHandle, Channel, Name );
	if( glGetError() != GL_NO_ERROR )
	{
		BcPrintf( "WARNING: RsProgramGL: Could not bind attribute \"%s\"\n", Name );
	}	
}

////////////////////////////////////////////////////////////////////////////////
// addParameter
void RsProgramGL::addParameter( const GLchar* pName, GLint Handle, GLenum Type )
{
	// Calculate number of bytes it needs and size.
	eRsShaderParameterType InternalType;
	BcU32 Bytes = 0;
	switch( Type )
	{
		case GL_FLOAT:
			InternalType = rsSPT_FLOAT;
			Bytes = 4;
			break;
		case GL_FLOAT_VEC2:
			InternalType = rsSPT_FLOAT_VEC2;
			Bytes = 8;
			break;
		case GL_FLOAT_VEC3:
			InternalType = rsSPT_FLOAT_VEC3;
			Bytes = 12;
			break;
		case GL_FLOAT_VEC4:
			InternalType = rsSPT_FLOAT_VEC4;
			Bytes = 16;
			break;
		case GL_INT:
			InternalType = rsSPT_INT;
			Bytes = 4;
			break;
		case GL_INT_VEC2:
			InternalType = rsSPT_INT_VEC2;
			Bytes = 8;
			break;
		case GL_INT_VEC3:
			InternalType = rsSPT_INT_VEC3;
			Bytes = 12;
			break;
		case GL_INT_VEC4:
			InternalType = rsSPT_INT_VEC4;
			Bytes = 16;
			break;
		case GL_BOOL:
			InternalType = rsSPT_BOOL;
			Bytes = 4;
			break;
		case GL_BOOL_VEC2:
			InternalType = rsSPT_BOOL_VEC2;
			Bytes = 8;
			break;
		case GL_BOOL_VEC3:
			InternalType = rsSPT_BOOL_VEC3;
			Bytes = 12;
			break;
		case GL_BOOL_VEC4:
			InternalType = rsSPT_BOOL_VEC4;
			Bytes = 16;
			break;
		case GL_FLOAT_MAT2:
			InternalType = rsSPT_FLOAT_MAT2;
			Bytes = 16;
			break;
		case GL_FLOAT_MAT3:
			InternalType = rsSPT_FLOAT_MAT3;
			Bytes = 36;
			break;
		case GL_FLOAT_MAT4:
			InternalType = rsSPT_FLOAT_MAT4;
			Bytes = 64;
			break;
			/* NOTE: GL2.1 or later, ignore for now!
		case GL_FLOAT_MAT2x3:
			Bytes = 24;
			break;
		case GL_FLOAT_MAT2x4:
			Bytes = 32;
			break;
		case GL_FLOAT_MAT3x2:
			Bytes = 24;
			break;
		case GL_FLOAT_MAT3x4:
			Bytes = 48;
			break;
		case GL_FLOAT_MAT4x2:
			Bytes = 32;
			break;
		case GL_FLOAT_MAT4x3:
			Bytes = 48;
			break;
			*/
		case GL_SAMPLER_1D:
			InternalType = rsSPT_SAMPLER_1D;
			Bytes = 4;
			break;
		case GL_SAMPLER_2D:
			InternalType = rsSPT_SAMPLER_2D;
			Bytes = 4;
			break;
		case GL_SAMPLER_3D:
			InternalType = rsSPT_SAMPLER_3D;
			Bytes = 4;
			break;
		case GL_SAMPLER_CUBE:
			InternalType = rsSPT_SAMPLER_CUBE;
			Bytes = 4;
			break;
		case GL_SAMPLER_1D_SHADOW:
			InternalType = rsSPT_SAMPLER_1D_SHADOW;
			Bytes = 4;
			break;
		case GL_SAMPLER_2D_SHADOW:
			InternalType = rsSPT_SAMPLER_2D_SHADOW;
			Bytes = 4;
			break;
		default:
			InternalType = rsSPT_INVALID;
			Bytes = 0;
			break;
	}

	// If parameter is valid, add it.
	if( InternalType != rsSPT_INVALID )
	{
		TParameter Parameter = 
		{
			pName,
			Handle,
			ParameterBufferSize_ >> 2,
			InternalType
		};
		
		// Add parameter.
		ParameterList_.push_back( Parameter );
	
		// Increate parameter buffer size.
		ParameterBufferSize_ += Bytes;
		
		// Log.
		BcPrintf( "RsProgramGL::Adding parameter \"%s\". Handle=%u\n", pName, Handle );

	}
}
