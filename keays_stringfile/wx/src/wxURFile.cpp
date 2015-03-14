/*
 * Filename: wxURFile.cpp
 * Date: February 2004
 * Author: Karl Janke, Keays Software
 *
 */

/* wxWindows Includes */
#include <wx/log.h>			// logging
#include <wx/progdlg.h>		// wxProgressDialog
#include <wx/msgdlg.h>		// wxMessageBox
#include <wx/file.h>		// wxFile
#include <wx/intl.h>		// text internationalization _("")

#include <string>			// std::string

/* My Includes */
#include <wxURFile.h>		// definition of this class

#include <LeakWatcher.h>

#ifdef _DO_MEMORY_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;

namespace keays
{
namespace stringfile
{
namespace wx
{

const char * cURFILE_VERSION_2_2	= "Version 2.2 and over";

bool URFile::Read ( const std::string & filename, 
					keays::stringfile::Data * stringData,
					bool progress, bool userAbort ) 
{
	char real_buf [cRECORD_LENGTH];
	char tmpBuf[21];
	void *buffer = &real_buf;
	unsigned long lastStringNo = 0;
	unsigned long numRecs = 0;	
	unsigned long numStrings = 0;
	unsigned long maxStringNum = 0;
	wxFile			file;	// file object
	
	
	userAbort = false;

	/* Check that we can open and read the file. 
	 * Fail if we can't.
	 */
	file.Open ( filename.c_str(), wxFile::read );

	if ( !wxFile::Access( filename.c_str(), wxFile::read) )
	{
		wxLogVerbose("We can't access this file!");
	}

	if ( !file.IsOpened() ) return false;

	for ( unsigned int i = 0; i < 4; i++ ) {
		memset(real_buf, '\0', cRECORD_LENGTH);
		if ( file.Read(real_buf, cRECORD_LENGTH) == wxInvalidOffset )
		{
			wxLogVerbose("Failure in reading from file!");
			return false;
		}

		switch (i) {
			case 0: 
				{					
					memcpy((void *)tmpBuf, real_buf, 20);
					tmpBuf[20] = '\0';
					string bob ( (char *)tmpBuf );
					if ( bob != 
						cURFILE_VERSION_2_2 )  {
						wxLogVerbose( "Version Mismatch. Expected <%s>, got <%s>",
									  cURFILE_VERSION_2_2, tmpBuf );
						return false;
					}
				}; break;

			case 1:
				{
					memcpy((void *)&numRecs, buffer, 4);					
				}; break;

			case 2:
				{
					memset( (&(real_buf[cTITLE_LENGTH])), 
							0, cRECORD_LENGTH - cTITLE_LENGTH );
					stringData->SetTitle( std::string((char *)(buffer)) );
				}; break;

			case 3:
				{	
					memset( (&(real_buf[cTITLE_LENGTH])), 
							0, cRECORD_LENGTH - cTITLE_LENGTH );
					stringData->SetCompany( std::string((char *)(buffer)) );
				}; break;
		}
	}
	

	wxProgressDialog pDlg ("Loading String File...", "Loading String File", numRecs,
							NULL, wxPD_AUTO_HIDE | wxPD_APP_MODAL | wxPD_CAN_ABORT);
	pDlg.SetSize(cPROGRESS_DIALOG_WIDTH, cPROGRESS_DIALOG_HEIGHT);

	
	for ( i = 0; i < numRecs; i++ ) {
		memset(buffer, '\0', cRECORD_LENGTH);
		file.Read(buffer, cRECORD_LENGTH); 

		Point *tmp = GetPoint();

		memcpy((void*)tmp, buffer, cRECORD_LENGTH );

		if ( tmp == NULL ) 
		{
			::wxMessageBox ( _("The file cannot be loaded as the system cannot allocate anymore\n"
								"memory. The program will now terminate"), _("Out Of Memory"), 
								wxICON_ERROR|wxOK );
			exit(1);
		}		

		if (tmp->stringNo != lastStringNo && tmp->stringNo != 0)
		{
			numStrings++;
			lastStringNo = tmp->stringNo;
		}
		
		stringData->Append(tmp);	
		if ( tmp->stringNo > maxStringNum ) maxStringNum = tmp->stringNo;

		if (pDlg.Update( i ) == FALSE)
		{
			userAbort = TRUE;
			stringData->Clear();
			file.Close();
			return false;
		}
	}
	
	stringData->SetMaxStringNum( maxStringNum );
	stringData->SetNumStrings( numStrings );	
	stringData->SetChanged( false );
	file.Close();

	return true;
}

bool URFile::Save ( const std::string & filename, 
					keays::stringfile::Data * stringData,
					bool progress, bool userAbort )
{
	wxASSERT( NULL != stringData );

	iterator it;
	char real_buf [cRECORD_LENGTH];
	wxFile			file;	// file object
	int numRecs;

	userAbort = false;

	file.Open( filename.c_str(), wxFile::write );
	if ( !file.IsOpened() ) return false;	

	numRecs = stringData->size();

	for ( int i = 0; i < 4; i++ )
	{
		memset((void *)&real_buf, '\0', cRECORD_LENGTH);
		switch (i) {
			case 0: 
				{					
					memcpy((void *)&real_buf, cURFILE_VERSION_2_2, 20);					
				}; break;

			case 1:
				{			
					memcpy((void *)&real_buf, (void *)&numRecs, 4);	
				}; break;

			case 2:
				{
					memcpy((void *)&real_buf, 
						   (void *)(stringData->GetTitle().c_str()), 
						   stringData->GetTitle().size());
					memset( (&(real_buf[cTITLE_LENGTH])), 
							' ', cRECORD_LENGTH - cTITLE_LENGTH );
				}; break;

			case 3:
				{	
					memcpy((void *)&real_buf, 
							(void *)(stringData->GetCompany().c_str()),
							stringData->GetCompany().size());
					memset( (&(real_buf[cTITLE_LENGTH])), 
							' ', cRECORD_LENGTH - cTITLE_LENGTH );
				}; break;
		}

		file.Write(real_buf, cRECORD_LENGTH);

	}

	wxProgressDialog pDlg ("Saving String File...", "Saving String File", numRecs,
							NULL, wxPD_AUTO_HIDE | wxPD_APP_MODAL);
	pDlg.SetSize(cPROGRESS_DIALOG_WIDTH, cPROGRESS_DIALOG_HEIGHT);


	i = 0;
	for ( it = stringData->begin(); it != stringData->end(); it++ ) 
	{
		memset((void *)&real_buf, '\0', cRECORD_LENGTH);
		memcpy((void *)&real_buf, (*it), cRECORD_LENGTH);		

		file.Write(real_buf, cRECORD_LENGTH);
		pDlg.Update(i++);

	}
	
	stringData->SetChanged( false );

	return true;
}

bool URFile::CanLoadExt( const std::string & extension )
{
	if ( extension.substr( 0, 2 ) == "ur" ) return true;

	return false;
}

} // namespace wx
} // namespace stringfile
} // namespace keays