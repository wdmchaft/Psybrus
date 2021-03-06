/**************************************************************************
*
* File:		OsCore.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "OsCore.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
OsCore::OsCore()
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
OsCore::~OsCore()
{
	
}

//////////////////////////////////////////////////////////////////////////
// registerClient
void OsCore::registerClient( OsClient* pClient )
{
	ClientList_.push_back( pClient );
}

//////////////////////////////////////////////////////////////////////////
// unregisterClient
void OsCore::unregisterClient( OsClient* pClient )
{
	for( TClientListIterator It( ClientList_.begin() ); It != ClientList_.end(); ++It )
	{
		if( (*It) == pClient )
		{
			ClientList_.erase( It );
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// getNoofClients
BcU32 OsCore::getNoofClients() const
{
	return ClientList_.size();
}

//////////////////////////////////////////////////////////////////////////
// getClient
OsClient* OsCore::getClient( BcU32 Index )
{
	if( Index < ClientList_.size() )
	{
		return ClientList_[ Index ];
	}

	return NULL;
}
