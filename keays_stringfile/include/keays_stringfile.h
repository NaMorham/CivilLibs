/*
 * Filename: keays_stringfile.h
 * Date: September 2004
 * Author: Karl Janke, Keays Software
 *
 * This file handles inclusion of the required headers for the
 * keays::stringfile namespace and library.
 */

#ifndef _KEAYS_STRINGFILE_H
#define _KEAYS_STRINGFILE_H

#include <Data.h>
#include <File.h>

#ifdef __WXMSW__

#include "../wx/include/wxBSPFile.h"
#include "../wx/include/wxTRFile.h"
#include "../wx/include/wxURFile.h"

#else

#include <TRFile.h>
#include <URFile.h>

#endif

#endif	// #ifndef _KEAYS_STRINGFILE_H