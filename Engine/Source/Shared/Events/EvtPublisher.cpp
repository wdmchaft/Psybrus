/**************************************************************************
*
* File:		EvtPublisher.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Event dispatcher.
*		
*		
*
* 
**************************************************************************/

#include "Events/EvtPublisher.h"

////////////////////////////////////////////////////////////////////////////////
// Ctor
EvtPublisher::EvtPublisher()
{
	pParent_ = NULL;
	pProxy_ = NULL;
	pBridge_ = NULL;
}

////////////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
EvtPublisher::~EvtPublisher()
{
	
}

////////////////////////////////////////////////////////////////////////////////
// EvtPublisher::unsubscribe
void EvtPublisher::unsubscribe( EvtID ID, void* pOwner )
{
	UnsubscribeByOwnerList_.push_back( TOwnerPair( ID, pOwner ) );
}

////////////////////////////////////////////////////////////////////////////////
// unsubscribeAll
void EvtPublisher::unsubscribeAll( void* pOwner )
{
	for( TBindingListMapIterator BindingListMapIterator = BindingListMap_.begin(); BindingListMapIterator != BindingListMap_.end(); ++BindingListMapIterator )
	{
		unsubscribeByOwner( BindingListMapIterator->first, pOwner );
	}
}

////////////////////////////////////////////////////////////////////////////////
// clearParent
void EvtPublisher::clearParent()
{
	pParent_ = NULL;
}

////////////////////////////////////////////////////////////////////////////////
// setParent
void EvtPublisher::setParent( EvtPublisher* pParent )
{
	BcAssert( pParent_ == NULL );
	pParent_ = pParent;
}

////////////////////////////////////////////////////////////////////////////////
// clearProxy
void EvtPublisher::clearProxy()
{
	pProxy_ = NULL;
}

////////////////////////////////////////////////////////////////////////////////
// setProxy
void EvtPublisher::setProxy( EvtProxy* pProxy )
{
	BcAssert( pProxy_ == NULL );
	pProxy_ = pProxy;
}

////////////////////////////////////////////////////////////////////////////////
// clearBridge
void EvtPublisher::clearBridge()
{
	pBridge_ = NULL;
}

////////////////////////////////////////////////////////////////////////////////
// setBridge
void EvtPublisher::setBridge( EvtBridge* pBridge )
{
	BcAssert( pBridge_ == NULL );
	pBridge_ = pBridge;
}

////////////////////////////////////////////////////////////////////////////////
// publishInternal
BcBool EvtPublisher::publishInternal( EvtID ID, const EvtBaseEvent& EventBase, BcSize EventSize, BcBool AllowBridge, BcBool AllowProxy )
{
	BcAssert( BcIsGameThread() );
	BcUnusedVar( EventSize );
	
	/*
	{
		BcChar PrefixA = ( ID >> 24 ) & 0xff;
		BcChar PrefixB = ( ID >> 16 ) & 0xff;
		BcU32 Group = ( ID >> 8 ) & 0xff;
		BcU32 Item = ( ID ) & 0xff;
		
		BcPrintf( "EvtPublish: %x, \"%c%c\": Group=%u Item=%u\n", ID, PrefixA, PrefixB, Group, Item );
	}
	 //*/

	// Proxy event if we need to.
	if( AllowProxy && pProxy_ )
	{	
		pProxy_->proxy( ID, EventBase, EventSize );
		return BcTrue;
	}

	// Bridge event if we need to.
	if( AllowBridge && pBridge_ )
	{
		pBridge_->bridge( ID, EventBase, EventSize );
	}
	
	// Update binding map before going ahead.
	updateBindingMap();
	
	// If we have a parent, publish to them first.
	BcBool ShouldPublish = BcTrue;
	
	if( pParent_ != NULL )
	{
		ShouldPublish = pParent_->publishInternal( ID, EventBase, EventSize, AllowBridge, AllowProxy );
	}

	// Only publish if the previous call to our parent allows us to.
	if( ShouldPublish == BcTrue )
	{
		// Find the appropriate binding list.
		TBindingListMapIterator BindingListMapIterator = BindingListMap_.find( ID );
		
		// Add list if we need to, and grab iterator.
		if( BindingListMapIterator != BindingListMap_.end() )
		{
			// Iterate over all bindings in list and call.
			TBindingList& BindingList = BindingListMapIterator->second;
			TBindingListIterator Iter = BindingList.begin();
			
			while( Iter != BindingList.end() )
			{
				EvtBinding& Binding = (*Iter);
				
				// Call binding and handle it's return.
				eEvtReturn RetVal = Binding( ID, EventBase );
				switch( RetVal )
				{
					case evtRET_PASS:
						++Iter;
						break;

					case evtRET_BLOCK:
						return BcFalse;
						break;

					case evtRET_REMOVE:
						Iter = BindingList.erase( Iter );
						break;
						
					default:
						BcBreakpoint;
						break;
				}
			}
		}
	}

	return BcTrue;
}

////////////////////////////////////////////////////////////////////////////////
// subscribeInternal
void EvtPublisher::subscribeInternal( EvtID ID, const EvtBinding& Binding )
{
	// Find the appropriate binding list.
	TBindingListMapIterator BindingListMapIterator = BindingListMap_.find( ID );
	
	// Add list if we need to, and grab iterator.
	if( BindingListMapIterator == BindingListMap_.end() )
	{
		BindingListMap_[ ID ] = TBindingList();
		BindingListMapIterator = BindingListMap_.find( ID );
	}
	
	// Append binding to list.
	TBindingList& BindingList = BindingListMapIterator->second;
	BindingList.push_back( Binding );
}


////////////////////////////////////////////////////////////////////////////////
// unsubscribeInternal
void EvtPublisher::unsubscribeInternal( EvtID ID, const EvtBinding& Binding )
{
	// Find the appropriate binding list.
	TBindingListMapIterator BindingListMapIterator = BindingListMap_.find( ID );
	
	// Add list if we need to, and grab iterator.
	if( BindingListMapIterator != BindingListMap_.end() )
	{
		// Find the matching binding.
		TBindingList& BindingList = BindingListMapIterator->second;
		TBindingListIterator Iter = BindingList.begin();
		
		while( Iter != BindingList.end() )
		{
			if( (*Iter) == Binding )
			{
				Iter = BindingList.erase( Iter );
			}
			else
			{
				++Iter;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// unsubscribeByOwner
void EvtPublisher::unsubscribeByOwner( EvtID ID, void* pOwner )
{
	// Find the appropriate binding list.
	TBindingListMapIterator BindingListMapIterator = BindingListMap_.find( ID );
	
	// Add list if we need to, and grab iterator.
	if( BindingListMapIterator != BindingListMap_.end() )
	{
		// Find the matching binding.
		TBindingList& BindingList = BindingListMapIterator->second;
		TBindingListIterator Iter = BindingList.begin();
		
		while( Iter != BindingList.end() )
		{
			if( (*Iter).getOwner() == pOwner )
			{
				Iter = BindingList.erase( Iter );
			}
			else
			{
				++Iter;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// updateBindingMap
void EvtPublisher::updateBindingMap()
{
	// Subscribe.
	for( TBindingPairListIterator Iter = SubscribeList_.begin(); Iter != SubscribeList_.end(); Iter = SubscribeList_.erase( Iter ) )
	{
		subscribeInternal( Iter->first, Iter->second );
	}

	// Unsubscribe.
	for( TBindingPairListIterator Iter = UnsubscribeList_.begin(); Iter != UnsubscribeList_.end(); Iter = UnsubscribeList_.erase( Iter ) )
	{
		unsubscribeInternal( Iter->first, Iter->second );
	}

	// Unsubscribe by owner.
	for( TOwnerPairListIterator Iter = UnsubscribeByOwnerList_.begin(); Iter != UnsubscribeByOwnerList_.end(); Iter = UnsubscribeByOwnerList_.erase( Iter ) )
	{
		unsubscribeByOwner( Iter->first, Iter->second );
	}
}
