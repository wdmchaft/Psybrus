/**************************************************************************
*
* File:		BcName.cpp
* Author: 	Neil Richardson
* Ver/Date:	27/11/10
* Description:
*		
*		
*		
*		
* 
**************************************************************************/

#include "Base/BcName.h"
#include "Base/BcMisc.h"

//////////////////////////////////////////////////////////////////////////
// Statics
BcName BcName::INVALID;
BcName BcName::NONE( "None" );

BcNameEntryList* BcName::pStringEntries_ = NULL;

//////////////////////////////////////////////////////////////////////////
// Ctor
BcName::BcName():
	EntryIndex_( BcErrorCode ),
	ID_( BcErrorCode )
{
	
}

//////////////////////////////////////////////////////////////////////////
// Ctor
BcName::BcName( const std::string& String )
{
	BcAssertMsg( BcIsGameThread(), "Only safe for use on game thread!" );
	setInternal( String, BcErrorCode );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
BcName::BcName( const std::string& String, BcU32 ID )
{
	BcAssertMsg( BcIsGameThread(), "Only safe for use on game thread!" );
	setInternal( String, ID );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
BcName::BcName( const BcChar* pString )
{
	BcAssertMsg( BcIsGameThread(), "Only safe for use on game thread!" );
	setInternal( pString, BcErrorCode );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
BcName::BcName( const BcChar* pString, BcU32 ID )
{
	BcAssertMsg( BcIsGameThread(), "Only safe for use on game thread!" );
	setInternal( pString, ID );
}

//////////////////////////////////////////////////////////////////////////
// Copy Ctor
BcName::BcName( const BcName& Other )
{
	EntryIndex_ = Other.EntryIndex_;
	ID_ = Other.ID_;
}

//////////////////////////////////////////////////////////////////////////
// operator =
BcName& BcName::operator = ( const BcName& Other )
{
	EntryIndex_ = Other.EntryIndex_;
	ID_ = Other.ID_;

	return (*this);
}

//////////////////////////////////////////////////////////////////////////
// Convert into string
std::string BcName::operator * () const
{
	BcAssertMsg( BcIsGameThread(), "Only safe for use on game thread!" );
	BcVerifyMsg( EntryIndex_ != BcErrorCode, "BcName: Converting an invalid name to a string!" );
	const BcNameEntryList& StringEntries( getStringEntries() );

	if( EntryIndex_ < StringEntries.size() )
	{
		const BcNameEntry& Entry = StringEntries[ EntryIndex_ ];

		if( ID_ != BcErrorCode )
		{
			// Generate "value_id" string.
			static BcChar Buffer[ BcNameEntry::MAX_STRING_LENGTH + 12 ];

			BcSPrintf( Buffer, "%s_%u", &Entry.Value_[ 0 ], ID_ );
			return Buffer;
		}
		else
		{
			// No ID, just return raw string.
			return Entry.Value_;	
		}
	}

	return "invalid";
}

//////////////////////////////////////////////////////////////////////////
// getValue.
std::string BcName::getValue() const
{
	BcAssertMsg( BcIsGameThread(), "Only safe for use on game thread!" );
	BcVerifyMsg( EntryIndex_ != BcErrorCode, "BcName: Converting an invalid name to a string!" );
	const BcNameEntryList& StringEntries( getStringEntries() );

	if( EntryIndex_ < StringEntries.size() )
	{
		return StringEntries[ EntryIndex_ ].Value_;	
	}

	return "invalid";
}

//////////////////////////////////////////////////////////////////////////
// getID.
BcU32 BcName::getID() const
{
	return ID_;
}

//////////////////////////////////////////////////////////////////////////
// getUnique
BcName BcName::getUnique() const
{
	BcAssertMsg( BcIsGameThread(), "Only safe for use on game thread!" );
	BcAssert( isValid() );
	BcNameEntryList& StringEntries( getStringEntries() );
	BcNameEntry& StringEntry( StringEntries[ EntryIndex_ ] );

	// Create a new name with passed in ID.
	BcName UniqueName( StringEntry.Value_, ID_ );

	// If we haven't got an ID assigned already, then create one.
	if( ID_ == BcErrorCode )
	{
		UniqueName.ID_ = StringEntry.ID_;
	
		// Advance ID for name.
		StringEntry.ID_++;
	}

	// Return new unique name.
	return UniqueName;
}

//////////////////////////////////////////////////////////////////////////
// isValid
BcBool BcName::isValid() const
{
	return EntryIndex_ != BcErrorCode;
}

//////////////////////////////////////////////////////////////////////////
// operator ==
bool BcName::operator == ( const BcName& Other ) const
{
	return EntryIndex_ == Other.EntryIndex_ && ID_ == Other.ID_;
}

//////////////////////////////////////////////////////////////////////////
// operator !=
bool BcName::operator != ( const BcName& Other ) const
{
	return EntryIndex_ != Other.EntryIndex_ || ID_ != Other.ID_;
}

//////////////////////////////////////////////////////////////////////////
// operator <
bool BcName::operator < ( const BcName& Other ) const
{
	return EntryIndex_ < Other.EntryIndex_;
}

//////////////////////////////////////////////////////////////////////////
// setInternal.
void BcName::setInternal( const std::string& Value, BcU32 ID )
{
	BcAssertMsg( BcIsGameThread(), "Only safe for use on game thread!" );

	// Check validity.
	BcVerifyMsg( isNameValid( Value ), "BcName: String (%s) contains invalid characters.", Value.c_str() );
	if( isNameValid( Value ) == BcFalse )
	{
		EntryIndex_ = BcErrorCode;
		ID_ = BcErrorCode;
		return;
	}

	// Entry index.
	BcU32 EntryIndex = BcErrorCode;

	// Find '_'
	std::string::size_type Position = Value.find_last_of( '_' );
		
	// If we have a trailing '_', strip out and use as ID.
	if( Position != std::string::npos )
	{
		std::string NewValue = Value.substr( 0, Position );
		EntryIndex = getEntryIndex( NewValue );

		std::string NewID = Value.substr( Position + 1, Value.length() - ( Position + 1 ) );
		ID = BcStrAtoi( NewID.c_str() );
	}
	else
	{
		EntryIndex = getEntryIndex( Value );
	}


	if( EntryIndex != BcErrorCode )
	{
		// Store index and ID.
		EntryIndex_ = EntryIndex;

		// Store, and advance index.
		ID_ = ID;
	}
	else
	{
		EntryIndex_ = BcErrorCode;
		ID_ = BcErrorCode;
	}
}

//////////////////////////////////////////////////////////////////////////
// getStringEntries.
//static
BcNameEntryList& BcName::getStringEntries()
{
	BcAssertMsg( BcIsGameThread(), "Only safe for use on game thread!" );

	// Check if we've been initialised.
	if( pStringEntries_ == NULL )
	{
		pStringEntries_ = new BcNameEntryList();
		pStringEntries_->reserve( ENTRY_RESERVE_COUNT );
	}

	// Dereference and return.
	return *pStringEntries_;
}

//////////////////////////////////////////////////////////////////////////
// getEntryIndex.
//static
BcU32 BcName::getEntryIndex( const std::string& Value )
{
	BcAssertMsg( BcIsGameThread(), "Only safe for use on game thread!" );

	// If string is too long, return invalid index.
	BcVerifyMsg( Value.length() < BcNameEntry::MAX_STRING_LENGTH, "BcName: String(%s) too long to store in name table.", Value.c_str() );
	if( Value.length() >= BcNameEntry::MAX_STRING_LENGTH )
	{
		return BcErrorCode;
	}

	// If string is too short, return invalid index. Don't warn.
	if( Value.length() == 0 )
	{
		return BcErrorCode;
	}

	// TODO: Store in a hash map.
	// Iterate over array to find if string exists.
	BcNameEntryList& StringEntries( getStringEntries() );
	for( BcU32 Idx = 0; Idx < StringEntries.size(); ++Idx )
	{
		BcNameEntry& Entry = StringEntries[ Idx ];

		// If we find the entry, return it's index.
		if( BcStrCompare( Value.c_str(), Entry.Value_ ) )
		{
			return Idx;
		}
	}

	// If we make it here, add a new entry.
	BcNameEntry NewEntry;
	BcStrCopyN( &NewEntry.Value_[ 0 ], Value.c_str(), BcNameEntry::MAX_STRING_LENGTH );
	NewEntry.ID_ = 0;
	StringEntries.push_back( NewEntry );

	// Check if entry count has went over reserve list size. Not a critical failure.
	BcVerifyMsg( StringEntries.size() < ENTRY_RESERVE_COUNT, "BcName: Reached end of reserve size." );

	// Return new entry index.
	return StringEntries.size() - 1;	
}

//////////////////////////////////////////////////////////////////////////
// isNameValid.
//static
BcBool BcName::isNameValid( const std::string& Value )
{
	BcAssertMsg( BcIsGameThread(), "Only safe for use on game thread!" );

	// Find '_'
	std::string::size_type Position = Value.find_last_of( '_' );
		
	// If we have a trailing '_', strip out and use as ID.
	if( Position != std::string::npos )
	{
		std::string NewValue = Value.substr( 0, Position );

		// Allow more valid characters.
		for( BcU32 Idx = 0; Idx < NewValue.length(); ++Idx )
		{
			if( ( NewValue[ Idx ] >= 'a' && NewValue[ Idx ] <= 'z' ) ||
				( NewValue[ Idx ] >= 'A' && NewValue[ Idx ] <= 'Z' ) ||
				( NewValue[ Idx ] >= '0' && NewValue[ Idx ] <= '9' ) )
			{
				continue;
			}
			else
			{
				return BcFalse;
			}
	
		}

		// And only numbers are valid for the trailing.
		std::string NewID = Value.substr( Position + 1, Value.length() - ( Position + 1 ) );
		if( BcStrIsNumber( NewID.c_str() ) == BcFalse )
		{
			return BcFalse;
		}
	}
	else
	{
		// Only 'a'-'z', 'A'-'Z' & '0'-'9' are valid.
		for( BcU32 Idx = 0; Idx < Value.length(); ++Idx )
		{
			if( ( Value[ Idx ] >= 'a' && Value[ Idx ] <= 'z' ) ||
				( Value[ Idx ] >= 'A' && Value[ Idx ] <= 'Z' ) ||
				( Value[ Idx ] >= '0' && Value[ Idx ] <= '9' ) )
			{
				continue;
			}
			else
			{
				return BcFalse;
			}
		}
	}

	return BcTrue;
}
 
