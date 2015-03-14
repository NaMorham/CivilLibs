/*!
	\file keays_version.h
	\author Andrew Hickey - Keays Software
	\date April 2005
 */

#pragma once

#include <string>

/*!
	\brief General Keays Software Namespace
	Used to prevent conflicts with existing functions/ variables
 */
namespace keays
{

/*!
	\brief basic Version type
 */
class Version
{
private:
	unsigned int m_major,	//!< Major version number
				 m_minor,	//!< Minor version number
				 m_patch;	//!< Patch level
	char m_mod;				//!< Version modification, so far only 'a'(Alpha) and 'b'(Beta) are appropriate

	static const char *ALLOWABLE_MODS;

public:
	//! @{ \name Constructors
	/*!
		\constructor
		\brief Default constructor, allows specifying 0 or more of the values.
		Construct a Version object with the values specified, by default will produce Version 0.0.0 (no modification),
		if a preset value is needed (ie. v1.0.22b) All values are needed, if just a major and minor are needed, only 
		the first 2 values are required.

		\param major [In]  - a constant unsigned integer specifying the major version level.
		\param minor [In]  - a constant unsigned integer specifying the minor version level.
		\param patch [In]  - a constant unsigned integer specifying the patch level.
		\param mod   [In]  - a constant char specifying the version modification (should be one of 'a' or 'b').
	 */
	Version( const unsigned int major = 0, const unsigned int minor = 0, const unsigned int patch = 0, const char mod = 0 ) 
						{ m_major = major; m_minor = minor; m_patch = patch; m_mod = tolower( mod ); }
	/*!
		\constructor
		\brief Simple constructor specifying major level and modification.
		Construct a Version object with the values specified.

		\param major [In]  - a constant unsigned integer specifying the major version level.
		\param mod   [In]  - a constant char specifying the version modification (should be one of 'a' or 'b').
	 */
	Version( const unsigned int major, const char mod )
						{ m_major = major; m_minor = 0; m_patch = 0; m_mod = tolower( mod ); }
	/*!
		\constructor
		\brief Simple constructor specifying major and minor levels with modification.
		Construct a Version object with the values specified.

		\param major [In]  - a constant unsigned integer specifying the major version level.
		\param minor [In]  - a constant unsigned integer specifying the minor version level.
		\param mod   [In]  - a constant char specifying the version modification (should be one of 'a' or 'b').
	 */
	Version( const unsigned int major, const unsigned int minor, const char mod ) 
						{ m_major = major; m_minor = minor; m_patch = 0; m_mod = tolower( mod ); }

	/*!
		\copy constructor
		\brief Copy constructor dupicating an existing Version object.

		\param orig [In]  - a constant reference to an exising Version to duplicate.
	 */
	Version( const Version &orig );
	//!< }@

	//! @{ \name Accessors
	/*!
		\brief Get the major version level.
		\return A constant unsigned int representing the major version level.
	 */
	const unsigned int Major() const { return m_major; }
	/*!
		\brief Get the minor version level.
		\return A constant unsigned int representing the minor version level.
	 */
	const unsigned int Minor() const { return m_minor; }
	/*!
		\brief Get the patch level.
		\return A constant unsigned int representing the patch level.
	 */
	const unsigned int Patch() const { return m_patch; }
	/*!
		\brief Get the version modification.
		\return A constant char representing the version modification.
	 */
	const char Mod() const { return m_mod; }

	//!< }@ Accessors

	//!< @{ \name Mutators
	/*!
		\brief Set the major version level.

		\param major [In]  - a constant unsigned integer specifying the new major version level.
		\return A constant unsigned int representing the major version level.
	 */
	const unsigned int Major( const unsigned major );
	/*!
		\brief Set the minor version level.

		\param minor [In]  - a constant unsigned integer specifying the new minor version level.
		\return A constant unsigned int representing the major version level.
	 */
	const unsigned int Minor( const unsigned minor );
	/*!
		\brief Set the patch level.

		\param patch [In]  - a constant unsigned integer specifying the new patch level.
		\return A constant unsigned int representing the patch level.
	 */
	const unsigned int Patch( const unsigned int patch );
	/*!
		\brief Set the version modification.

		\param mod [In]  - a constant char specifying the new version modification.
		\return A constant char representing the version modification.
	 */
	const char Mod( const char mod );
	//!< }@ Mutators

	//! @{ \name Operators
	/*!
		\brief Duplicate an existing Version object.

		\param rhs [In]  - a constant reference to an existing Version to copy data from.
		\return a constant reference to the current object
	 */
	const Version &operator=( const Version &rhs );
	/*!
		\brief Parse a string for version values.

		\param str [In]  - a constant char pointer to the string to parse.
		\return a constant reference to the current object
	 */
	const Version &operator=( const char *str );

	const bool operator<( const Version &rhs );
	const bool operator>( const Version &rhs );
	const bool operator==( const Version &rhs );
	const bool operator<=( const Version &rhs ) { return !(*this > rhs); }
	const bool operator>=( const Version &rhs ) { return !(*this < rhs); }
	const bool operator!=( const Version &rhs ) { return !(*this == rhs); }

	operator const std::string () const;
	//!< }@
};

}; // namespace keays