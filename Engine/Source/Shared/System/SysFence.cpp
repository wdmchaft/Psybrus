/**************************************************************************
*
* File:		SysFence.cpp
* Author:	Neil Richardson 
* Ver/Date:	15/12/11	
* Description:
*		Fence for synchronisation of jobs.
*		
*
*
* 
**************************************************************************/

#include "System/SysFence.h"
#include "System/SysKernel.h"

////////////////////////////////////////////////////////////////////////////////
// Ctor
SysFence::SysFence():
	Count_( 0 )
{

}

////////////////////////////////////////////////////////////////////////////////
// Dtor
SysFence::~SysFence()
{

}

////////////////////////////////////////////////////////////////////////////////
// increment
void SysFence::increment()
{
	++Count_;
}

////////////////////////////////////////////////////////////////////////////////
// decrement
void SysFence::decrement()
{
	--Count_;
}

////////////////////////////////////////////////////////////////////////////////
// queue
void SysFence::queue( BcU32 WorkerMask )
{
	for(BcU32 Idx = 0; Idx < SysKernel::pImpl()->workerCount(); ++Idx)
	{
		BcU32 Mask = 1 << Idx;

		if((WorkerMask & Mask) != 0)
		{
			// Increment the count.
			increment();
			
			// Queue job on the mask to decrement.
			BcDelegate< void(*)() > Delegate( BcDelegate< void(*)() >::bind< SysFence, &SysFence::decrement >( this ) );
			SysKernel::pImpl()->enqueueDelegateJob( Mask, Delegate );
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// wait
void SysFence::wait( BcU32 Value )
{
	while( Count_ > Value )
	{
		BcYield();
	}
}
