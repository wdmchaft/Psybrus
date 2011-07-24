/**************************************************************************
*
* File:		SysJob.cpp
* Author:	Neil Richardson 
* Ver/Date:	6/07/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "SysJob.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
SysJob::SysJob():
	WorkerMask_( 0 )
{
	
}

//////////////////////////////////////////////////////////////////////////
// Dtor
// virtual
SysJob::~SysJob()
{
	
}

//////////////////////////////////////////////////////////////////////////
// internalExecute
// virtual
void SysJob::internalExecute()
{
	execute();
	
	// Clean up.
	delete this;
}
