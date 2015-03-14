// someone that cares about this loader can fix it
#if 0

/* Includes */
#include <URFile.h>
#include <math.h>
#include <string>
#include <StringData.h>
#include <stdio.h>

/* Memory Leak Debuggery */
#include <LeakWatcher.h>

#ifdef _DO_MEMORY_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;

URFile::URFile ( string fullpath, string path, string filename,
				 StringData *data ) 
{
	m_Fullpath = fullpath;
	m_Path = path;
	m_Filename = filename;
	m_StringData = data;
}

URFile::~URFile ( ) 
{
	
}

bool URFile::Read ( bool progress ) 
{
	char real_buf [REC_LENGTH];
	char tmpBuf[20];
	void *buffer = &real_buf;
	int	lastStringNo = -1;
	int numStrings = 0;
	int numRecs;
	FILE *			file;		// file pointer

	/* 
	 * Check that we can open and read the file. 
	 * Fail if we can't.
	 */

	file = fopen( m_Fullpath.c_str(), "rb");
	if ( NULL == file ) return false;

	for ( int i = 0; i < 4; i++ ) {
		memset(buffer, '\0', REC_LENGTH);
		fread( buffer, REC_LENGTH, 1, file );

		switch (i) 
		{
			case 0: 
				{					
					memcpy((void *)tmpBuf, real_buf, 20);
					tmpBuf[20] = '\0';
					string bob ( (char *)tmpBuf );
					if ( bob != 
						"Version 2.2 and over" )  {
						return false;
					}
				}; break;

			case 1:
				{
					memcpy((void *)&numRecs, buffer, 4);
				}; break;

			case 2:
				{
					// junk, just skip
				}; break;

			case 3:
				{	
					// junk, just skip
				}; break;
		}
	}
	

	for ( i = 0; i < numRecs; i++ ) {
		memset(buffer, '\0', REC_LENGTH);
		fread( buffer, REC_LENGTH, 1, file );

		struct tStringStruct *tmp = (struct tStringStruct *) malloc ( REC_LENGTH );
		memcpy((void *)tmp, buffer, 32);
		memcpy((void *)(tmp->plotCode), &( ((char *)buffer)[32]), CODE_LEN);
		memcpy((void *)(tmp->notes), &( ((char *)buffer)[36]), NOTE_LEN);

		if ( tmp != NULL ) {
			if (tmp->stringNo != lastStringNo && tmp->stringNo != 0)
			{
				numStrings++;
				lastStringNo = tmp->stringNo;
			}
			m_StringData->Append(tmp);	
		} else {

		}

	}
	
	m_StringData->SetNumStrings( numStrings );
	m_StringData->SetChanged( false );
	fclose( file );

	return true;
}

bool URFile::SaveAs ( string fname, bool progress )
{
	vector<tStringStruct *>::iterator it;
	char real_buf [REC_LENGTH];
	int numRecs;
	FILE *			file = NULL;		// file pointer

	file = fopen( fname.c_str(), "wb" );

	numRecs = m_StringData->size();

	for ( int i = 0; i < 4; i++ )
	{
		memset((void *)&real_buf, '\0', REC_LENGTH);
		switch (i) {
			case 0: 
				{					
					memcpy((void *)&real_buf, "Version 2.2 and over", 20);					
				}; break;

			case 1:
				{			
					memcpy((void *)&real_buf, (void *)&numRecs, 4);	
				}; break;

			case 2:
				{
					// junk, just skip
				}; break;

			case 3:
				{	
					// junk, just skip
				}; break;
		}

		fwrite( real_buf, REC_LENGTH, 1, file );

	}

	i = 0;
	for ( it = m_StringData->begin(); it != m_StringData->end(); it++ ) 
	{		
		memset((void *)&real_buf, '\0', REC_LENGTH);
		memcpy((void *)&real_buf, (*it), REC_LENGTH);		

		fwrite( real_buf, REC_LENGTH, 1, file );
	}
	
	/* Update the current paths */	
	m_Fullpath = fname;
	m_Path = fname.substr( 0, fname.find( '\\', true) );
	m_Filename = fname.substr( 0, fname.size() - fname.find( '\\', true) - 1 );
	m_StringData->SetChanged( false );
	
	return true;
}

bool URFile::Save ( bool progress )
{
	return SaveAs ( m_Fullpath, progress );
}

#endif