/**************************************************************************
*
* File:		BcMemory.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __BCMEMORY_H__
#define __BCMEMORY_H__

#include <new>
#include <memory.h>

//////////////////////////////////////////////////////////////////////////
// BcMemSet
inline void BcMemSet( void* pMemory, BcU8 Value, BcSize Bytes )
{
	memset( pMemory, Value, Bytes );
}

//////////////////////////////////////////////////////////////////////////
// BcMemCopy
inline void BcMemCopy( void* pDst, const void* pSrc, BcSize Bytes )
{
	memcpy( pDst, pSrc, Bytes );
}

//////////////////////////////////////////////////////////////////////////
// BcMemCompare
inline BcBool BcMemCompare( const void* pA, const void* pB, BcSize Bytes )
{
	int Result = memcmp( pA, pB, Bytes );
	return Result == 0;
}

//////////////////////////////////////////////////////////////////////////
// BcMemZero
inline void BcMemZero( void* pMemory, BcSize Bytes )
{
	BcMemSet( pMemory, 0, Bytes );
}

#endif


