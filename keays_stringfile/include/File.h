/*
 * Filename: StringFile.h
 * Date: May 2004
 * Author: Karl Janke, Keays Software
 *
 * This file provides a header file that defines the interface
 * that all loaders for the Data object must implement.
 *
 * It belongs to the keays::stringfile namespace
 */

#ifndef _STRING_FILE_H
#define _STRING_FILE_H

// Includes
#include <string>		// std::string

#include <Data.h>		// keays::stringfile::Data

namespace keays
{
namespace stringfile
{

// constants
const int cPROGRESS_DIALOG_WIDTH	=	300;
const int cPROGRESS_DIALOG_HEIGHT	=	160;

/*
 * The StringFile class is a abstract class. Any class that inherits
 * from it must implement all its methods.
 */
class StringFile
{
public:
	/*** These must be implemented in child classes ***/
		
	/*
	 * Read from fname. If progress is true than show a progress bar. Return 
	 * false on failure or user abort. True on success.
	 */
	static bool Read ( const std::string & filename, 
						keays::stringfile::Data * pStringData,
						bool progress, bool userAbort );
	
	/*
	 * Save to fname. If progress is true than a progress bar will be displayed.
	 * Return false on failure, true on success.
	 */
	static bool Save ( const std::string & filename, 
						keays::stringfile::Data * pStringData,
						bool progress, bool userAbort );

	/*
	 * Return true if we can load files with this extension. False otherwise
	 */
	static bool CanLoadExt( const std::string & extension );
};

} // namespace stringfile
} // namespace keays

#endif