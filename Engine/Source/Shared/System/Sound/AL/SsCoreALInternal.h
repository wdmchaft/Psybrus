/**************************************************************************
 *
 * File:		SsCoreALInternal.h
 * Author:		Neil Richardson
 * Ver/Date:	
 * Description:
 *		
 *
 *
 *
 * 
 **************************************************************************/

#ifndef __SSCOREIMPLAL_H__
#define __SSCOREIMPLAL_H__

#include "SsCore.h"

#include "SsAL.h"
#include "SsChannelAL.h"

#include "BcCommandBuffer.h"

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class SsCoreALInternal;

//////////////////////////////////////////////////////////////////////////
// Defines
#define MAX_AL_CHANNELS				( 32 )
#define MAX_AL_STEREO_SOURCES		( 2 )
#define MAX_AL_MONO_SOURCES			( MAX_AL_CHANNELS + ( 2 * MAX_AL_STEREO_SOURCES ) )

///////////////////////////////////////////////////////////////////////////
// SsCoreALInternal
class SsCoreALInternal:
	public SsCore
{
private:
	BcBool initEFX();

public:
	virtual void open();
	virtual void update();
	virtual void close();

	
public:
	virtual SsSample* createSample( BcU32 SampleRate, BcU32 Channels, BcBool Looping, void* pData, BcU32 DataSize );
	virtual void updateResource( SsResource* pResource );
	virtual void destroyResource( SsResource* pResource );
	
private:
	void createResource( SsResource* pResource );

public:
	virtual SsChannel* play( SsSample* pSample, SsChannelCallback* pCallback = NULL );
	virtual void setListener( const BcVec3d& Position, const BcVec3d& LookAt, const BcVec3d& Up );

public:
	SsChannelAL* allocChannel();
	void freeChannel( SsChannelAL* pSound );

private:
	BcCommandBuffer			CommandBuffer_;

	//
	const BcChar*			pSelectedDevice_;
	ALCcontext*				ALContext_;
	ALCdevice*				ALDevice_;
	BcU32					InternalResourceCount_;

	BcBool					bEFXEnabled_;

	// Sound.
	BcU32					ChannelCount_;
	
	typedef std::list< SsChannelAL* > TChannelList;
	typedef TChannelList::iterator TChannelListIterator;
	
	TChannelList			FreeChannels_;
	TChannelList			UsedChannels_;
	
	// Listener.
	BcVec3d					ListenerPosition_;
	BcVec3d					ListenerLookAt_;
	BcVec3d					ListenerUp_;

	// Environment.
	ALuint					ALReverbEffectSlot_;
	ALuint					ALReverbEffect_;

};

#endif