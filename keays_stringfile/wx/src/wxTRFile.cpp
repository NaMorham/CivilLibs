/*
 * Filename: wxTRFile.cpp
 * Date: February 2004
 * Author: Karl Janke, Keays Software
 *
 */

/* wxWindows Includes */
#include <wx/progdlg.h>        // wxProgressDialog
#include <wx/msgdlg.h>        // wxMessageBox
#include <wx/intl.h>        // text internationalization _("")
#include <wx/log.h>            // wxLog

/* My Includes */
#include <wxTRFile.h>        // definition of this class

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

/* Constants */
const char * cTRFILE_KEAYS_ID        = "KEAYS";
const char * cTRFILE_VERSION_2_2    = "Version 2.2 and over";
const char * cTRFILE_HEADER_APP        = "wxTRFile Reader/Writer";
const char * cTRFILE_EOL            = "\r\n";
const int     cTRFILE_BUF_SIZE        = 1000;

/*
 * The implementation of this function is rather inefficient. Therefore
 * it should not be called on a regular basis (i.e inside a loop).
 */
int TRFile::ReadLine( wxFile & file, char * buffer, int bufSize )
{
    char c = '0';
    int curIndex = 0;

    memset( buffer, 0, bufSize );
    while ( curIndex < bufSize && c != '\n' )
    {
        if ( wxInvalidOffset == file.Read( &c, 1 ) )
        {
            memset( buffer, 0, bufSize );
            return -1;
        }

        buffer[curIndex] = c;
        curIndex++;
    }

    return curIndex;
}

bool TRFile::Read ( const std::string & filename,
                    keays::stringfile::Data * stringData,
                    bool progress, bool userAbort )
{
    unsigned long    maxStringNo = 0;                // maximum string number
    unsigned long    lastStringNo = 0;                // last string number seen
    unsigned long    numStrings = 0;                    // total number of strings
    wxFile            file;                            // file object
    int                i = 0,                            // counter
                    readSize = 0,                    // how much of the file we have read
                    lineLen;                        // length of current line of text
    wxString        curLine,                        // current line
                    curChunk;                        // current part of line
    char            recBuffer[cTRFILE_BUF_SIZE];    // tempory char buffer

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

    lineLen = ReadLine( file, (char *)&recBuffer, cTRFILE_BUF_SIZE );
    curLine = recBuffer;

    // Check version
    /*
     * At Johns request I have removed this check. If you attempt to open
     * _anything_ that is renamed tr the code will probably happily proceed
     * to crash..
     */
#if 0
    if ( -1 == curLine.Find( cTRFILE_VERSION_2_2 ) )
    {
        if ( ::wxMessageBox( _("This does not appear to be a valid Keays Transfer (TR) text file.\n"
                          "Click on Yes to continue loading, at the risk of causing program \n"
                          "instability, or click No to abort loading the file." ),
                          _("Not A Valid Text Transfer File"), wxYES_NO ) == wxNO )
        {
            return false;
        }
    }
#endif

    // Show progress dialog
    wxProgressDialog pDlg ("Loading String Text File...", "Loading String Text File", file.Length(),
                            NULL, wxPD_AUTO_HIDE | wxPD_APP_MODAL | wxPD_CAN_ABORT);
    pDlg.SetSize(cPROGRESS_DIALOG_WIDTH, cPROGRESS_DIALOG_HEIGHT);

    int tmpSize;
    // Loop until the end of the file, reading in the data
    while ( !file.Eof() )
    {
        /* Adaptive read length algorithm -
         *
         * Assume that the inital lineLen is correct. After each read check that
         * the character is \n. If not then rewind the file & do a scan of the
         * line. Set the line length to be the correct value.
         */
        tmpSize = file.Read( (char *)&recBuffer, lineLen );
        curLine = recBuffer;

        if ( curLine.Length() == 0 ) continue;                // sanity check

        if ( curLine.GetChar( curLine.Length() - 1 ) != '\n' )
        {
            file.Seek( -1 * lineLen, wxFromCurrent );
            lineLen = ReadLine( file, (char *)&recBuffer, cTRFILE_BUF_SIZE );
            curLine = recBuffer;
            wxLogMessage("WARNING: wxTRFile::Read() - Length of record has changed");
            tmpSize = lineLen;
        }

        readSize += tmpSize;

        Point *tmp = GetPoint( NULL );

        if ( tmp != NULL )
        {
        } else {
            ::wxMessageBox ( _("The file cannot be loaded as the system cannot allocate anymore\n"
                                "memory. The program will now terminate"), _("Out Of Memory"), wxOK );
            exit(1);
        }

        // numbers here are pretty evil
        curChunk = curLine.Mid( 0, 6 );
        curChunk.ToLong( (long *)(&(tmp->stringNo)) );
        curChunk = curLine.Mid( 6, 6 );
        curChunk.ToLong( (long *)(&(tmp->pointNo)) );

        curChunk = curLine.Mid( 12, 13 );
        curChunk.ToDouble( &(tmp->x) );
        curChunk = curLine.Mid( 25, 13 );
        curChunk.ToDouble( &(tmp->y) );
        curChunk = curLine.Mid( 38, 10 );
        curChunk.ToDouble( &(tmp->z) );

        curChunk = curLine.Mid( 49, 4 );
        strncpy( tmp->plotCode, curChunk.c_str(), 4 );

        curChunk = curLine.Mid( 54, 12 );
        strncpy( tmp->notes, curChunk.c_str(), 12 );

        curChunk = curLine.Mid( 66, 4 );
        if ( curChunk.Length() >= 4 )
        {
            tmp->contourPlot = curChunk[0];
            tmp->contourString = curChunk[1];
            tmp->featurePlot = curChunk[2];
            tmp->featureString = curChunk[3];
        }

        if (tmp->stringNo != lastStringNo && tmp->stringNo != 0)
        {
            numStrings++;
            lastStringNo = tmp->stringNo;
        }
        stringData->Append(tmp);

        if ( tmp->stringNo > maxStringNo ) maxStringNo = tmp->stringNo;

        if (pDlg.Update( readSize ) == FALSE)

        {
            userAbort = TRUE;
            stringData->Clear();
            file.Close();
            return false;
        }

        i++;
    }

    stringData->SetMaxStringNum( maxStringNo );
    stringData->SetNumStrings( numStrings );

    stringData->SetTitle( std::string(filename.c_str()).substr( 0, cTITLE_LENGTH ) );
    stringData->SetCompany( std::string("Unknown - From Transfer File") );
    stringData->SetChanged( false );

    file.Close();

    return true;
}

bool TRFile::Save ( const std::string & filename,
                    keays::stringfile::Data * stringData,
                    bool progress, bool userAbort )
{
    wxFile                    file;    // file object
    wxString                tmpStr;    // tempory buffer to work in
    wxString                outStr;    // output string
    iterator                it;        // to loop through all data in stringData
    int                        i = 0;    // counter to update progress bar
    Point                    *s;        // convience pointer

    userAbort = false;

    // Check that we can open and write the file. Fail if we can't.
    file.Open( filename.c_str(), wxFile::write );
    if ( !file.IsOpened() )
    {
        ::wxMessageBox( _("Unable to open the selected file for writing. Please check that it is "
                          "not in use "
                          "by another program and that you have permission to access it"),
                          _("File Open Failed"), wxOK );
        return false;
    }

    // First write out the header line
    tmpStr.Clear();
    tmpStr << cTRFILE_KEAYS_ID << "\t" << cTRFILE_VERSION_2_2 << "\t"
           << cTRFILE_HEADER_APP << cTRFILE_EOL;
    file.Write( tmpStr );

    wxProgressDialog pDlg ("Saving String File...", "Saving String File",
                            stringData->GetRecordCount(),
                            NULL, wxPD_AUTO_HIDE | wxPD_APP_MODAL );
    pDlg.SetSize(cPROGRESS_DIALOG_WIDTH, cPROGRESS_DIALOG_HEIGHT);


    // approximate format string
    // "%6d%6d%13.4f%13.4f%10.4f %-4.4s %-12.12s%c%c%c%c%s"

    // loop through and write out each record
    for ( it = stringData->begin(); it != stringData->end(); it++ )
    {
        s = (*it);
        outStr = "";

        // need to strip out any newlines. Otherwise they will break the
        // written file
        for ( int k = 0; k < cPLOT_CODE_LENGTH; k++ )
            if ( (s->plotCode)[k] == '\n' )
                (s->plotCode)[k] = ' ';

        for ( k = 0; k < cNOTE_LENGTH; k++ )
            if ( (s->notes)[k] == '\n' )
                (s->notes)[k] = ' ';

        if ( !PointToString( *s, outStr ) )
        {
            file.Close();
            ::wxRemoveFile( wxString(filename.c_str()) );

            wxString tmpMessage;
            tmpMessage.Printf("Unable to save file in TR format. Error at "
                              "record number %d: \n\n%s", i+1, tmpStr );
            wxMessageBox( tmpMessage, "Save Aborted.", wxOK|wxICON_ERROR );
            return false;
        }

        file.Write( outStr );

        pDlg.Update( i++ );
    }

    stringData->SetChanged( false );
    file.Close();

    return true;
}

bool TRFile::CanLoadExt( const std::string & extension )
{
    if ( extension.substr( 0, 2 ) == "tr" ) return true;
    else if ( extension.substr( 0, 2 ) == "tm" ) return true;

    return false;
}

bool TRFile::PointToString( const keays::stringfile::Point & point,
                                     wxString & result )
{
    wxString tmpStr;

    // string number
    tmpStr.Printf( "%6d", point.stringNo );
    if ( tmpStr.Length() > 6 )
    {
        result = _("String No. can't be converted to text without loss of information.");
        return false;
    }
    result += tmpStr;

    // point number
    tmpStr.Printf( "%6d", point.pointNo );
    if ( tmpStr.Length() > 6 )
    {

        result = _("Point No. can't be converted to text without loss of information.");
        return false;
    }
    result += tmpStr;

    // x
    tmpStr.Printf( "%13.4f", point.x );
    if ( tmpStr.Length() > 13 )
    {
        if ( tmpStr.Find( '.', true ) >= 14 )
        {
            result = _("Easting can't be converted to text without loss of information.");
            return false;
        }
        else
        {
            tmpStr = tmpStr.Left( 13 );
        }
    }
    result += tmpStr;

    // y
    tmpStr.Printf( "%13.4f", point.y );
    if ( tmpStr.Length() > 13 )
    {
        if ( tmpStr.Find( '.', true ) >= 14 )
        {
            result = _("Northing can't be converted to text without loss of information.");
            return false;
        }
        else
        {
            tmpStr = tmpStr.Left( 13 );
        }
    }
    result += tmpStr;

    // z
    tmpStr.Printf( "%10.4f", point.z );
    if ( tmpStr.Length() > 10 )
    {
        if ( tmpStr.Find( '.', true ) >= 11 )
        {
            result = _("Altitude can't be converted to text without loss of information.");
            return false;
        }
        else
        {
            tmpStr = tmpStr.Left( 10 );
        }
    }
    result += tmpStr;

    tmpStr.Printf( " %-4.4s %-12.12s%c%c%c%c%s",
                   point.plotCode, point.notes, point.contourPlot,
                   point.contourString, point.featurePlot,
                   point.featureString, cTRFILE_EOL );

    result += tmpStr;

    return true;
}

} // namespace wx
} // namespace stringfile
} // namespace keays