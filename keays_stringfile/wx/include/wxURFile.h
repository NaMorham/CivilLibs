/*
 * Filename: wxURFile.h
 * Date: February 2004
 * Author: Karl Janke, Keays Software
 *
 * This file provides the header definition of a class that can load & save
 * between a UR (Universal Resource) file and a StringData object in memory.
 *
 * It belongs to the keays::stringfile namespace
 */

#ifndef _WX_URFILE_H
#define _WX_URFILE_H

// Includes
#include <string>        // std::string
#include <File.h>        // keays::stringfile::File

namespace keays
{
namespace stringfile
{
namespace wx
{

class URFile : public StringFile
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
}; // class wxURFile

} // namespace wx
} // namespace stringfile
} // namespace keays

#endif