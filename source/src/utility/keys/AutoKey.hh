// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file   utility/keys/AutoKey.hh
/// @brief  Automatic hidden index key abstract base class
/// @author Stuart G. Mentzer (Stuart_Mentzer@objexx.com)
///
/// @note
///  @li Object (O) parameter: The type of object being keyed
///  @li Super (S) parameter: The super Key class (== or derived from Key)
///  @li Client (C) parameter: The client (user) of these keys
///  @li There is a distinct Key type for each Object+Super+Client combination
///  @li Hidden index is set automatically at construction in the order keys are constructed
///  @li Default-constructed keys are assigned a hidden index of zero: Use indentifier
///      argument(s) when constructing keys meant for collections
///  @li Key-constructed keys copy the index of the source key so they compare as equal
///  @li Can derive privately from an AutoKey to share the index set of another Key type
///      without allowing convertibility (for type safety)
///  @li Designed for collections of keys of a common abstract or concrete Key type


#ifndef INCLUDED_utility_keys_AutoKey_hh
#define INCLUDED_utility_keys_AutoKey_hh


// Unit headers
#include <utility/keys/AutoKey.fwd.hh>

// Package headers
#include <utility/down_cast.hh>
#include <utility/keys/Key.hh>
#include <utility/keys/KeyLookup.hh> // DO NOT AUTO-REMOVE (needed for template expansion)

// C++ headers
#include <utility/assert.hh>

#ifdef    SERIALIZATION
#include <cereal/types/string.hpp>
#endif // SERIALIZATION

namespace utility {
namespace keys {


/// @brief Automatic hidden index key abstract base class
template< typename O, typename S, typename C >
class AutoKey :
	public S
{


private: // Types


	typedef  S  Super;


public: // Types


	typedef  utility::keys::Key  Key;

	// STL/boost style
	typedef  O  object_type;
	typedef  std::size_t  index_type;
	typedef  std::size_t  size_type;

	// Project style
	typedef  O  Object;
	typedef  std::size_t  Index;
	typedef  std::size_t  Size;


protected: // Creation


	/// @brief Default constructor
	inline
	AutoKey() :
		index_( 0 ) // Key not meant for key collections
	{}


	/// @brief Copy constructor
	inline
	AutoKey( AutoKey const & key ) :
		Super( key ),
		index_( key.index_ ), // Not a new index so we don't increment the index count
		id_( key.id_ ),
		identifier_( key.identifier_ ),
		code_( key.code_ )
	{}


	/// @brief Copy + identifier constructor
	inline
	AutoKey(
		AutoKey const & key,
		std::string const & id_a,
		std::string const & identifier_a = std::string(),
		std::string const & code_a = std::string()
	) :
		Super( key ),
		index_( key.index_ ), // Not a new index so we don't increment the index count
		id_( id_a ),
		identifier_( identifier_a.empty() ? id_a : identifier_a ),
		code_( code_a.empty() ? id_a : code_a )
	{}


	/// @brief Key constructor
	inline
	explicit
	AutoKey( Key const & key ) :
		Super( key ),
		index_( key.index() ), // Not a new index so we don't increment the index count
		id_( key.id() ),
		identifier_( key.identifier() ),
		code_( key.code() )
	{
		debug_assert( dynamic_cast< AutoKey const * >( &key ) );
	}


	/// @brief Key + identifier constructor
	inline
	AutoKey(
		Key const & key,
		std::string const & id_a,
		std::string const & identifier_a = std::string(),
		std::string const & code_a = std::string()
	) :
		Super( key ),
		index_( key.index() ), // Not a new index so we don't increment the index count
		id_( id_a ),
		identifier_( identifier_a.empty() ? id_a : identifier_a ),
		code_( code_a.empty() ? id_a : code_a )
	{
		debug_assert( dynamic_cast< AutoKey const * >( &key ) );
	}


	/// @brief Identifier constructor
	inline
	explicit
	AutoKey(
		std::string const & id_a,
		std::string const & identifier_a = std::string(),
		std::string const & code_a = std::string()
	) :
		index_( next_index() ),
		id_( id_a ),
		identifier_( identifier_a.empty() ? id_a : identifier_a ),
		code_( code_a.empty() ? id_a : code_a )
	{}


public: // Creation

	/// @brief Clone this
	AutoKey *
	clone() const override = 0;

	/// @brief Destructor
	inline
	~AutoKey() override {}

public: // Assignment


	/// @brief Copy assignment
	inline
	AutoKey &
	operator =( AutoKey const & key )
	{
		if ( this != &key ) {
			assign_Key( key );
		}
		return *this;
	}


	/// @brief Key assignment
	inline
	AutoKey &
	operator =( Key const & key )
	{
		if ( this != &key ) {
			assign_Key( key );
		}
		return *this;
	}


protected: // Assignment


	/// @brief Key assignment
	void
	assign_Key( Key const & key ) override
	{
		debug_assert( comparable( key ) );
		index_ = key.index();
		id_ = key.id();
		identifier_ = key.identifier();
		code_ = key.code();
	}


public: // Properties


	/// @brief ID
	std::string const &
	id() const override
	{
		return id_;
	}


	/// @brief ID
	std::string &
	id() override
	{
		return id_;
	}


	/// @brief ID assignment
	AutoKey &
	id( std::string const & id_a ) override
	{
		id_ = id_a;
		return *this;
	}


	/// @brief Identifier
	std::string const &
	identifier() const override
	{
		return identifier_;
	}


	/// @brief Identifier
	std::string &
	identifier() override
	{
		return identifier_;
	}


	/// @brief Identifier assignment
	AutoKey &
	identifier( std::string const & identifier_a ) override
	{
		identifier_ = identifier_a;
		return *this;
	}


	/// @brief Code
	std::string const &
	code() const override
	{
		return code_;
	}


	/// @brief Code
	std::string &
	code() override
	{
		return code_;
	}


	/// @brief Code assignment
	AutoKey &
	code( std::string const & code_a ) override
	{
		code_ = code_a;
		return *this;
	}


	/// @brief Index
	/// @note  Only for use as an optimization: DO NOT WRITE CODE DEPENDING ON THE SPECIFIC INDEX VALUE!
	Index
	private_index() const override
	{
		return index_;
	}


public: // Comparison


	/// @brief AutoKey == AutoKey
	/// @note  Index-based equality
	friend
	inline
	bool
	operator ==( AutoKey const & a, AutoKey const & b )
	{
		return ( a.index_ == b.index_ );
	}


	/// @brief AutoKey != AutoKey
	/// @note  Index-based equality
	friend
	inline
	bool
	operator !=( AutoKey const & a, AutoKey const & b )
	{
		return ( a.index_ != b.index_ );
	}


	/// @brief AutoKey < AutoKey
	/// @note  Index-based ordering
	/// @note  Needed for use as key in associative containers
	friend
	inline
	bool
	operator <( AutoKey const & a, AutoKey const & b )
	{
		return ( a.index_ < b.index_ );
	}


	/// @brief AutoKey <= AutoKey
	/// @note  Index-based ordering
	friend
	inline
	bool
	operator <=( AutoKey const & a, AutoKey const & b )
	{
		return ( a.index_ <= b.index_ );
	}


	/// @brief AutoKey >= AutoKey
	/// @note  Index-based ordering
	friend
	inline
	bool
	operator >=( AutoKey const & a, AutoKey const & b )
	{
		return ( a.index_ >= b.index_ );
	}


	/// @brief AutoKey > AutoKey
	/// @note  Index-based ordering
	friend
	inline
	bool
	operator >( AutoKey const & a, AutoKey const & b )
	{
		return ( a.index_ > b.index_ );
	}


	/// @brief AutoKeys are sequential?
	/// @note  Index-based ordering
	template< typename UO, typename US, typename UC >
	friend
	bool
	sequential( AutoKey<UO, US, UC> const & a, AutoKey<UO, US, UC> const & b );


public: // Static functions


	/// @brief Number of keys with distinct indexes
	inline
	static
	Size
	n_key()
	{
		return n_key_();
	}


#if !(defined _MSC_VER)||(defined __INTEL_COMPILER) // Not Visual C++: Normal case
protected: // Conversion
#else // Visual C++ 2005 bug work-around
public: // Conversion
#endif


	/// @brief Index value conversion
	inline
	operator Index() const
	{
		return index_;
	}


protected: // Properties


	/// @brief Index
	Index
	index() const override
	{
		return index_;
	}


	/// @brief Index
	Index &
	index() override
	{
		return index_;
	}


	/// @brief Index assignment
	AutoKey &
	index( Index const index_a ) override
	{
		index_ = index_a;
		return *this;
	}


protected: // Comparison


	/// @brief Equal to a Key?
	bool
	equals( Key const & key ) const override
	{
		return ( index_ == utility::down_cast< AutoKey const & >( key ).index_ );
	}


	/// @brief Less than a Key?
	bool
	less_than( Key const & key ) const override
	{
		return ( index_ < utility::down_cast< AutoKey const & >( key ).index_ );
	}


	/// @brief Comparable to a Key?
	bool
	comparable( Key const & key ) const override
	{
		return dynamic_cast< AutoKey const * >( &key );
	}


private: // Static functions


	/// @brief Number of keys with distinct indexes
	inline
	static
	Size &
	n_key_()
	{
		static Size n_key_s( 0 ); // Function-local to support globals
		return n_key_s;
	}


	/// @brief Increment the index counter and return it
	static
	Size
	next_index()
	{
		debug_assert( n_key_() < Size( -1 ) ); // Check for overflow
		return ++n_key_();
	}


private: // Fields


	/// @brief Index
	Index index_;

	/// @brief ID: Short identifier
	std::string id_;

	/// @brief Identifier: Long identifier
	std::string identifier_;

	/// @brief Code: Coded identifier
	std::string code_;


#ifdef    SERIALIZATION
public:
	template< class Archive > void save( Archive & arc ) const {
		arc(index_);
		arc(id_);
		arc(identifier_);
		arc(code_);
	}
	template< class Archive > void load( Archive & arc ) {
		arc(index_);
		arc(id_);
		arc(identifier_);
		arc(code_);
	}
#endif // SERIALIZATION

}; // AutoKey


// Friend function namespace declarations


/// @brief AutoKey == AutoKey
template< typename O, typename S, typename C >
bool
operator ==( AutoKey< O, S, C > const & a, AutoKey< O, S, C > const & b );


/// @brief AutoKey != AutoKey
template< typename O, typename S, typename C >
bool
operator !=( AutoKey< O, S, C > const & a, AutoKey< O, S, C > const & b );


/// @brief AutoKey < AutoKey
template< typename O, typename S, typename C >
bool
operator <( AutoKey< O, S, C > const & a, AutoKey< O, S, C > const & b );


/// @brief AutoKey <= AutoKey
template< typename O, typename S, typename C >
bool
operator <=( AutoKey< O, S, C > const & a, AutoKey< O, S, C > const & b );


/// @brief AutoKey >= AutoKey
template< typename O, typename S, typename C >
bool
operator >=( AutoKey< O, S, C > const & a, AutoKey< O, S, C > const & b );


/// @brief AutoKey > AutoKey
template< typename O, typename S, typename C >
bool
operator >( AutoKey< O, S, C > const & a, AutoKey< O, S, C > const & b );


/// @brief AutoKeys are sequential?
template< typename O, typename S, typename C >
bool
sequential( AutoKey< O, S, C > const & a, AutoKey< O, S, C > const & b )
{
	return ( a.index_ + 1 == b.index_ );
}


} // namespace keys
} // namespace utility


#endif // INCLUDED_utility_keys_AutoKey_HH
