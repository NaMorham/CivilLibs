// someone that cares about this loader can fix it
#if 0

/*
 * Filename: URFile.h
 * Date: January 2004
 * Author: Karl Janke, Keays Software
 *
 */

#ifndef URFILE_H
#define URFILE_H

#include <vector>
#include <stdio.h>

class StringData;

class URFile 
{
public:

	URFile ( std::string fullname, std::string path, std::string filename,
			 StringData *stringData );
	~URFile ( );

	bool Read ( bool progress );
	
	bool SaveAs ( std::string fname, bool progress );
	bool Save ( bool progress );

	std::string GetPath() { return m_Path; };
	std::string GetFullPath() { return m_Fullpath; };
	std::string GetFilename() { return m_Filename; };
	
private:
	
	std::string		m_Fullpath;	// full path, including filename
	std::string		m_Path;		// just the path
	std::string		m_Filename;	// just the filename

	StringData		*m_StringData;
};

#endif

#endif