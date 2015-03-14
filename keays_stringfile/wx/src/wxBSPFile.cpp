/*
 * Filename: wxBSPFile.cpp
 * Date: May 2004
 * Author: Karl Janke, Keays Software
 *
 */

/* wxWindows Includes */
#include <wx/progdlg.h>		// wxProgressDialog
#include <wx/msgdlg.h>		// wxMessageBox
#include <wx/intl.h>		// text internationalization _("")
#include <wx/log.h>			// wxLog
#include <wx/file.h>		// wxFile

/* My Includes */
#include <wxBSPFile.h>		// definition of this class

#include <LeakWatcher.h>

#ifdef _DO_MEMORY_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/* Namespace usage */
using namespace std;

namespace keays
{
namespace stringfile
{
namespace wx
{

bool BSPFile::Read ( const std::string & filename, 
					 keays::stringfile::Data * stringData,
					 bool progress, bool userAbort ) 
{
	unsigned long	maxStringNo = 0;				// maximum string number
	unsigned long	lastStringNo = 0;				// last string number seen
	unsigned long	lastPointNo = 0;				// last point number seen
	unsigned long	numStrings = 0;					// total number of strings
	wxFile			file;							// file object	
	int				i = 0;							// counter

	userAbort = false;

	// Check that we can open and read the file. Fail if we can't.
	file.Open ( filename.c_str(), wxFile::read );
	if ( !file.IsOpened() ) 
	{
		::wxMessageBox( _("Unable to open the selected file for reading. Please check that "
						  "it not in use "
						  "by another program and that you have permission to access it"), 
						  _("File Open Failed"), wxOK );
		return false;	
	}
	
	
	int numVerts,					// number of verts in BSP file
		numBSPFaces;				// number of faces in BSP file

	BSPVertex   *pVerts = NULL;		// tempory array of vertices
	BSPFace	    *pFaces = NULL;		// tempory array of faces

	BSPHeader header;				// header of the BSP file
	BSPLump lumps[kMaxLumps] = {0};	// lumps in BSP file

	// Get index information about BSP file
	file.Read( &header, sizeof(BSPHeader) );
	
	if ( strncmp(header.id, "IBSP", 4) != 0 || header.version != 46 )
	{
		wxString tmpStr;
		tmpStr.Printf( "This is not a BSP File supported by this loader!\n"
					   "Header is: %s\n"
					   "Version is: %d\n", header.id, header.version );
		::wxMessageBox( tmpStr, _("BSP Loader Unable to Open File"), wxOK );
		file.Close();
		return false;
	}

	file.Read( &lumps, sizeof(BSPLump) * kMaxLumps );

	numVerts = lumps[kVertices].length / sizeof(BSPVertex);
	numBSPFaces = lumps[kFaces].length / sizeof(BSPFace);

	pVerts    = new BSPVertex[numVerts];
	pFaces    = new BSPFace[numBSPFaces];

	// Find the face (to be converted to string) information
	file.Seek( lumps[kFaces].offset, wxFromStart );
	file.Read( pFaces, sizeof(BSPFace) * numBSPFaces );	

	file.Seek( lumps[kVertices].offset, wxFromStart );

	// Show progress dialog
	wxProgressDialog pDlg ("Loading BSP File...", "Loading BSP File", numVerts * 2 + 50,
							NULL, wxPD_AUTO_HIDE | wxPD_APP_MODAL | wxPD_CAN_ABORT);
	pDlg.SetSize(cPROGRESS_DIALOG_WIDTH, cPROGRESS_DIALOG_HEIGHT);

	// Read in the raw X,Y,Z data first
	for (i = 0; i < numVerts; i++)
	{
		file.Read(&pVerts[i], sizeof(BSPVertex) );

		float temp = pVerts[i].position.y;
		pVerts[i].position.x = -pVerts[i].position.x;
		pVerts[i].position.y = pVerts[i].position.z;
		pVerts[i].position.z = -temp;

		if (pDlg.Update( i ) == FALSE)
		{
			userAbort = TRUE;
			stringData->Clear();
			file.Close();
			return false;
		}
	}

	// Don't need file any longer
	file.Close();	

	// Process the vertex data into strings
	lastStringNo = 1;
	lastPointNo = 0;

	Point *pFirst;	// required to repeat the first point of the string as the last

	wxString tmpStr;				// tempory working string
	int faceIndex, vertIndex;		// count variables

	for ( faceIndex = 0; faceIndex < numBSPFaces; faceIndex++ )
	{
		if ( pFaces[faceIndex].type != 1 && pFaces[faceIndex].type != 3 ) 
		{
			/*wxLogDebug("Face Type: %d, startVertex: %d, numVerts: %d",
						pFaces[faceIndex].type, pFaces[faceIndex].startVertex,
						pFaces[faceIndex].numVerts );*/
			continue;
		}		

		lastPointNo = 0;
		Point *tmp = keays::stringfile::GetPoint();
		pFirst = tmp;

		for ( vertIndex = pFaces[faceIndex].startVertex; 
			  vertIndex < pFaces[faceIndex].startVertex + pFaces[faceIndex].numVerts;
			  vertIndex++ )
		{			
			tmp->x = pVerts[vertIndex].position.x;
			tmp->y = pVerts[vertIndex].position.z;
			tmp->z = pVerts[vertIndex].position.y;
			tmp->stringNo = lastStringNo;
			tmp->pointNo = lastPointNo;
			tmp->contourPlot = 'Y';
			tmp->featurePlot = 'Y';
			tmp->contourString = 'S';
			tmp->featureString = 'S';			
		
			tmpStr.Printf("%d", pFaces[faceIndex].type);
			tmpStr.Pad(4);
		
			strncpy( tmp->plotCode, tmpStr.c_str(), 4 );
			stringData->Append( tmp );

			lastPointNo++;
			tmp = keays::stringfile::GetPoint();
		}

		// add in the last point
		memcpy( tmp, pFirst, cRECORD_LENGTH );
		tmp->pointNo = lastPointNo;
		stringData->Append( tmp );
		
		lastStringNo++;		

		if ( pDlg.Update( numVerts + vertIndex ) == FALSE )
		{
			userAbort = TRUE;
			delete[] pVerts;
			delete[] pFaces;
			stringData->Clear();
			return false;
		}
	}

	// Fix up meta data for string file
	stringData->SetMaxStringNum( lastStringNo );
	stringData->SetNumStrings( lastStringNo );	
	
	stringData->SetTitle( std::string(filename.c_str()).substr( 0, cTITLE_LENGTH ) );
	stringData->SetCompany( std::string("Unknown - From BSP File") );
	stringData->SetChanged( false );

	delete[] pVerts;
	delete[] pFaces;

	return true;
}

bool BSPFile::Save ( const std::string & filename, keays::stringfile::Data * data,
					   bool progress, bool userAbort )
{
	return false;
}

bool BSPFile::CanLoadExt( const std::string & extension )
{
	if ( extension.substr( 0, 3 ) == "bsp" ) return true;
	return false;
}

} // namespace wx
} // namespace stringfile
} // namespace keays