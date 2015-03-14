/*
 * Filename: wxTRFile.h
 * Date: February 2004
 * Author: Karl Janke, Keays Software
 *
 * This file provides the header definition of a class that can load & save
 * between a TR (Text Resource) file and a StringData object in memory.
 *
 * It belongs to the keays::stringfile namespace
 */

#ifndef _WX_TRFILE_H
#define _WX_TRFILE_H

// Includes
#include <string>		// std::string

#include <wx/file.h>	// wxFile

#include <File.h>		// keays::stringfile::File


namespace keays
{
namespace stringfile
{
namespace wx
{

class TRFile : public StringFile
{
public:
	/* Implementation of StringFile interface */
	static bool Read ( const std::string & filename, 
					   keays::stringfile::Data * stringData,
					   bool progress, bool userAbort );
	
	static bool Save ( const std::string & filename, 
					   keays::stringfile::Data * stringData,
					   bool progress, bool userAbort );

	static bool CanLoadExt( const std::string & extension );
	/* End Implementation of StringFile interface */

	static bool PointToString( const keays::stringfile::Point & point,
		  					   wxString & result );
private:
	
	/*
	 * Read a line (until the next \n) from the current position in file, 
	 * putting the read data in buffer, up to a maximum size of bufSize. 
	 * Return the number of bytes read or -1 on failure.
	 */
	static int ReadLine( wxFile & file, char * buffer, int bufSize );	
};

} // namespace wx
} // namespace stringfile
} // namespace keays

#endif