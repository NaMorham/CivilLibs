#include "..\include\FileChangeEvent.h"
#include "..\include\kfwExceptions.h"

#include <io.h>

#include <LeakWatcher.h>

#ifdef _DO_MEMORY_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace keays
{
namespace watch
{


sFileWatchFile::sFileWatchFile()
{
    m_ID = INVALID_ID;
    m_fileName.erase();
    m_filePath.erase();
    memset( &m_dtCreateDate, 0, sizeof( FILETIME ) );
    memset( &m_dtModifyDate, 0, sizeof( FILETIME ) );
}

sFileWatchFile::sFileWatchFile( UINT id, const char *folder )
{
    m_ID = id;
    m_fileName.erase();
    m_filePath = folder;

    if( _access( folder, 00 ) == -1 )
    {
        m_ID = INVALID_ID;
        throw exceptions::eFileException( exceptions::eFileException::FE_INVALID_FILE );
    }

    memset( &m_dtCreateDate, 0, sizeof( FILETIME ) );
    memset( &m_dtModifyDate, 0, sizeof( FILETIME ) );
}

sFileWatchFile::sFileWatchFile( UINT id, const char *folder, const char *file )
{
    m_ID = id;
    m_filePath = folder;

    if( file )
    {
        HANDLE f;

        m_fileName = file;

        f = CreateFile( GetFullFilePath().c_str(),
                            0,
                             ( FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE ),
                             NULL,
                             OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL,
                             NULL );
        if( f != INVALID_HANDLE_VALUE )
        {
            if( !GetFileTime( f, &m_dtCreateDate, NULL, &m_dtModifyDate ) )
            {
                m_ID = INVALID_ID;
                throw exceptions::eFileException( exceptions::eFileException::FE_GET_TIMES );
            }

            CloseHandle(f);
        } else
        {
            m_ID = INVALID_ID;
            throw exceptions::eFileException( exceptions::eFileException::FE_INVALID_FILE );
        }
    } else
    {
        m_fileName.erase();
        memset( &m_dtCreateDate, 0, sizeof( FILETIME ) );
        memset( &m_dtModifyDate, 0, sizeof( FILETIME ) );    // THIS IS BAD!!!
    }
}

sFileWatchFile::sFileWatchFile( const sFileWatchFile &orig )
{
    *this = orig;
}

const sFileWatchFile &sFileWatchFile::operator=( const sFileWatchFile &rhs )
{
    m_ID = rhs.m_ID;
    m_dtCreateDate = rhs.m_dtCreateDate;
    m_dtModifyDate = rhs.m_dtModifyDate;
    m_fileName = rhs.m_fileName;
    m_filePath = rhs.m_filePath;

    return *this;
}

const std::string sFileWatchFile::GetFullFilePath() const
{
    std::string buf;
    buf = m_filePath;

    char temp[_MAX_PATH];
    memset( temp, 0, sizeof( char )*_MAX_PATH );

    _makepath( temp, NULL, m_filePath.c_str(), m_fileName.c_str(), NULL );
    buf = std::string( temp );

//    if( WatchIsFile() )
//    {
//        buf += m_fileName;
//    }
    return buf;
}

//-----------------------------------------------------------------------------
//#region WatchManager
WatchManager::WatchManager( pFnNotify pfnNotifyFn/*, UINT id*/ )
{    //#region
    if( pfnNotifyFn == NULL )
        throw exceptions::eWatchException( exceptions::eWatchException::WE_NULL_CALLBACK );
//    m_ID = id;
    m_pfnNotify = pfnNotifyFn;

    m_hWatchThread = NULL;
    m_exitHandle = NULL;

    m_isRunning = false;
    m_suspended = false;
    m_kill = false;

    m_lockCount = 0;

    InitializeCriticalSection( &m_CriticalSection );
    //#endregion
}

WatchManager::~WatchManager()
{    //#region
    m_kill = true;

    if( m_exitHandle  && ( m_exitHandle != INVALID_HANDLE_VALUE ) && IsRunning() )
    {
        StopWatchInt();
        while( m_isRunning )
        {
            OutputDebugString( "Waiting for CFileWatcher to end\n" );
        }

    }

    DeleteCriticalSection( &m_CriticalSection );
    //#endregion
}

bool WatchManager::AddFile( const sFileWatchFile &file )
{    //#region
    OutputDebugString( "--- AddFile( const sFileWatchFile & )\n" );
    EnterCriticalSection( &m_CriticalSection );        m_lockCount++;
    if( file.m_ID == sFileWatchFile::INVALID_ID )
    {
        LeaveCriticalSection( &m_CriticalSection );        m_lockCount--;
        OutputDebugString( "--- AddFile( const sFileWatchFile & ) - false\n" );
        return false;    // we cannot watch an invalid file
    }

    StopWatchInt(); // this waits until the thread has exited

    m_files.push_back( file );
    LeaveCriticalSection( &m_CriticalSection );        m_lockCount--;
    OutputDebugString( "--- AddFile( const sFileWatchFile & ) - true\n" );
    return true;
    //#endregion
}

bool WatchManager::AddFile( UINT uniqueID, const char *filePath, const char *fileName )
{    //#region
    OutputDebugString( "--- AddFile( UINT, const char *, const char * )\n" );
    EnterCriticalSection( &m_CriticalSection );        m_lockCount++;
    sFileWatchFile file;

    try
    {
        file = sFileWatchFile( uniqueID, filePath, fileName );
    }
    catch ( exceptions::eFileException & e )
    {
        OutputDebugString( e.GetExString() ); OutputDebugString( "\n" );
        LeaveCriticalSection( &m_CriticalSection );
        return false;
    }

    if( file.m_ID == sFileWatchFile::INVALID_ID )
    {
        OutputDebugString( "--- AddFile( UINT, const char *, const char * ) - false\n" );
        LeaveCriticalSection( &m_CriticalSection );        m_lockCount--;
        return false;    // we cannot watch an invalid file
    }

    StopWatchInt(); // this waits until the thread has exited

    m_files.push_back( file );

    OutputDebugString( "--- AddFile( UINT, const char *, const char * ) - true\n" );
    LeaveCriticalSection( &m_CriticalSection );        m_lockCount--;
    return true;
    //#endregion
}

bool WatchManager::AddFolder( UINT uniqueID, const char *path )
{    //#region
    OutputDebugString( "--- AddFile( UINT, const char * )\n" );
    EnterCriticalSection( &m_CriticalSection );        m_lockCount++;
    sFileWatchFile folder( uniqueID, path );

    if( folder.m_ID == sFileWatchFile::INVALID_ID )
    {
        LeaveCriticalSection( &m_CriticalSection );        m_lockCount--;
        OutputDebugString( "--- AddFile( UINT, const char * ) - false\n" );
        return false;    // we cannot watch an invalid file
    }

    StopWatchInt(); // this waits until the thread has exited

    m_files.push_back( folder );

    OutputDebugString( "--- AddFile( UINT, const char * ) - true\n" );
    LeaveCriticalSection( &m_CriticalSection );        m_lockCount--;

    return true;
    //#endregion
}

bool WatchManager::RemoveFile( const UINT fileID, bool firstOnly /*= false*/ )
{    //#region
    OutputDebugString( "--- RemoveFile( const UINT, bool )\n" );
    EnterCriticalSection( &m_CriticalSection );        m_lockCount++;
    StopWatchInt();

    sVector::iterator itr;
    if( FindFirstFile( fileID, &itr ) != -1 )
    {
        itr = m_files.erase( itr );

        if( !firstOnly )
        {
            while( FindNextFile( fileID, itr ) != -1 )
            {
                itr = m_files.erase( itr );
            }
        }
        LeaveCriticalSection( &m_CriticalSection );        m_lockCount--;
        OutputDebugString( "--- RemoveFile( const UINT, bool ) - true\n" );
        return true;
    } else
    {
        LeaveCriticalSection( &m_CriticalSection );        m_lockCount--;
        OutputDebugString( "--- RemoveFile( const UINT, bool ) - false\n" );
        return false;
    }
    //#endregion
}

bool WatchManager::RemoveFile( const char *filePath, bool firstOnly /*= false*/ )
{    //#region
    OutputDebugString( "--- RemoveFile( const char *, bool )\n" );
    EnterCriticalSection( &m_CriticalSection );        m_lockCount++;
    StopWatchInt();

    sVector::iterator itr;
    if( FindFirstFile( filePath, &itr ) != -1 )
    {
        itr = m_files.erase( itr );

        if( !firstOnly )
        {
            while( FindNextFile( filePath, itr ) != -1 )
            {
                itr = m_files.erase( itr );
            }
        }
        LeaveCriticalSection( &m_CriticalSection );        m_lockCount--;
        OutputDebugString( "--- RemoveFile( const char *, bool ) - true\n" );
        return true;
    } else
    {
        LeaveCriticalSection( &m_CriticalSection );        m_lockCount--;
        OutputDebugString( "--- RemoveFile( const char *, bool ) - false\n" );
        return false;
    }
    //#endregion
}

bool WatchManager::RemoveFile( const char *filePath, const char *fileName, bool firstOnly /*= false*/ )
{    //#region
    OutputDebugString( "--- RemoveFile( const char *, const char *, bool )\n" );
    EnterCriticalSection( &m_CriticalSection );        m_lockCount++;
    StopWatchInt();

    sVector::iterator itr;
    if( FindFirstFile( filePath, fileName, &itr ) != -1 )
    {
        itr = m_files.erase( itr );

        if( !firstOnly )
        {
            while( FindNextFile( filePath, fileName, itr ) != -1 )
            {
                itr = m_files.erase( itr );
            }
        }
        LeaveCriticalSection( &m_CriticalSection );        m_lockCount--;
        OutputDebugString( "--- RemoveFile( const char *, const char *, bool ) - true\n" );
        return true;
    } else
    {
        LeaveCriticalSection( &m_CriticalSection );        m_lockCount--;
        OutputDebugString( "--- RemoveFile( const char *, const char *, bool ) - false\n" );
        return false;
    }
    //#endregion
}

/*
bool WatchManager::RemoveFileEx( const char *fullPath, bool firstOnly )
{
    EnterCriticalSection( &m_CriticalSection );
    StopWatchInt();
    LeaveCriticalSection( &m_CriticalSection );
    return true;
}
//*/

bool WatchManager::RemoveAll()
{    //#region
    EnterCriticalSection( &m_CriticalSection );        m_lockCount++;
    StopWatchInt();
    m_files.clear();
    LeaveCriticalSection( &m_CriticalSection );        m_lockCount--;
    return true;
    //#endregion
}

const sFileWatchFile &WatchManager::RefFileByIndex( const unsigned int index )
{    //#region
    EnterCriticalSection( &m_CriticalSection );        m_lockCount++;

    if( ( index >= m_files.size() ) )
    {
        LeaveCriticalSection( &m_CriticalSection );        m_lockCount--;
        throw exceptions::eWatchException( exceptions::eWatchException::WE_FILE_INDEX_OUT_OF_RANGE );
    }

    sFileWatchFile & file = m_files[index];
    LeaveCriticalSection( &m_CriticalSection );        m_lockCount--;
    return file;
    //#endregion
}

bool WatchManager::StartWatch()
{    //#region
    EnterCriticalSection( &m_CriticalSection );        m_lockCount++;

    if( IsRunning() )
        StopWatchInt();

    m_kill = false;

    m_hWatchThread = CreateThread( NULL, 0, FileChangeWatch, (LPVOID)this, 0, &m_dwWatchThreadID );
    if( !m_hWatchThread )
    {
        LeaveCriticalSection( &m_CriticalSection );        m_lockCount--;
        return false;
    }

    LeaveCriticalSection( &m_CriticalSection );        m_lockCount--;
    return true;
    //#endregion
}

const HANDLE WatchManager::SetExitHandle( const HANDLE exHandle )
{    //#region
    HANDLE h;
    EnterCriticalSection( &m_CriticalSection );        m_lockCount++;
    h = ( m_exitHandle = exHandle );
    LeaveCriticalSection( &m_CriticalSection );        m_lockCount--;
    return h;
    //#endregion
}

void WatchManager::StopWatch()
{    //#region
    EnterCriticalSection( &m_CriticalSection );        m_lockCount++;
    StopWatchInt();
    LeaveCriticalSection( &m_CriticalSection );        m_lockCount--;
    //#endregion
}

void WatchManager::StopWatchInt()
{    //#endregion
    if( !IsRunning() )
    {
        return;    // nothing to stop;
    }

    m_kill = true;

    OutputDebugString( "See if we have a handle to stop\n" );
    if( m_exitHandle && ( m_exitHandle != INVALID_HANDLE_VALUE ) )
    {
        OutputDebugString( "attemp to stop watch\n" );

        BOOL h = SetEvent( m_exitHandle );

        while( m_isRunning )
        {
            // give other threads a chance to change m_isRunning
            LeaveCriticalSection( &m_CriticalSection );        m_lockCount--;
            EnterCriticalSection( &m_CriticalSection );        m_lockCount++;
        }
        //m_exitHandle = NULL;
        CloseHandle( m_hWatchThread );
        m_hWatchThread = NULL;
        OutputDebugString( "The thread is dead Dave\n" );
    }
    //#endregion
}

void WatchManager::NotifyParent( unsigned int filesIdx )
{    //#region
    EnterCriticalSection( &m_CriticalSection );        m_lockCount++;
    OutputDebugString( "CFileWatcher::NotifyParent( int filesIdx )\n" );
    if( ( filesIdx >= m_files.size() ) )
    {
        LeaveCriticalSection( &m_CriticalSection );        m_lockCount--;
        throw exceptions::eWatchException( exceptions::eWatchException::WE_FILE_INDEX_OUT_OF_RANGE );
    }

    m_pfnNotify( m_files[filesIdx].m_ID, m_files[filesIdx].GetFullFilePath().c_str() );
    LeaveCriticalSection( &m_CriticalSection );        m_lockCount--;
    //#endregion
}

int WatchManager::FindFirstFile( const UINT fileID, sVector::iterator *pItr )
{    //#region
    EnterCriticalSection( &m_CriticalSection );        m_lockCount++;
    sVector::iterator itr;
    int index = -1;
    for( itr = m_files.begin(); itr != m_files.end(); itr++ )
    {
        index++;
        if( (*itr).m_ID == fileID )
        {
            if( pItr ) *pItr = itr;
            LeaveCriticalSection( &m_CriticalSection );        m_lockCount--;
            return index;
        }
    }

    LeaveCriticalSection( &m_CriticalSection );        m_lockCount--;
    return -1;
    //#endregion
}

int WatchManager::FindNextFile( const UINT fileID, sVector::iterator &prevItr )
{    //#region
    EnterCriticalSection( &m_CriticalSection );        m_lockCount++;
    sVector::iterator itr;
    int index = -1;
    for( itr = prevItr; itr != m_files.end(); itr++ )
    {
        index++;
        if( (*itr).m_ID == fileID )
        {
            prevItr = itr;
            LeaveCriticalSection( &m_CriticalSection );        m_lockCount--;
            return index;
        }
    }

    LeaveCriticalSection( &m_CriticalSection );        m_lockCount--;
    return -1;
    //#endregion
}

int WatchManager::FindFirstFile( const std::string &filePath, sVector::iterator *pItr )
{    //#region
    EnterCriticalSection( &m_CriticalSection );        m_lockCount++;
    sVector::iterator itr;
    int index = -1;
    for( itr = m_files.begin(); itr != m_files.end(); itr++ )
    {
        index++;
        if( (*itr).m_filePath == filePath )
        {
            if( pItr ) *pItr = itr;
            LeaveCriticalSection( &m_CriticalSection );        m_lockCount--;
            return index;
        }
    }

    LeaveCriticalSection( &m_CriticalSection );        m_lockCount--;
    return -1;
    //#endregion
}

int WatchManager::FindNextFile( const std::string &filePath, sVector::iterator &prevItr )
{    //#region
    EnterCriticalSection( &m_CriticalSection );        m_lockCount++;

    sVector::iterator itr;
    int index = -1;
    for( itr = m_files.begin(); itr != m_files.end(); itr++ )
    {
        index++;
        if( (*itr).m_filePath == filePath )
        {
            prevItr = itr;
            LeaveCriticalSection( &m_CriticalSection );        m_lockCount--;
            return index;
        }
    }

    LeaveCriticalSection( &m_CriticalSection );        m_lockCount--;
    return -1;
    //#endregion
}

int WatchManager::FindFirstFile( const std::string &filePath, const std::string &fileName, sVector::iterator *pItr )
{    //#region
    EnterCriticalSection( &m_CriticalSection );        m_lockCount++;
    sVector::iterator itr;
    int index = -1;
    for( itr = m_files.begin(); itr != m_files.end(); itr++ )
    {
        index++;
        if( ( (*itr).m_filePath == filePath ) && ( (*itr).m_fileName == fileName ) )
        {
            if( pItr ) *pItr = itr;
            LeaveCriticalSection( &m_CriticalSection );        m_lockCount--;
            return index;
        }
    }

    LeaveCriticalSection( &m_CriticalSection );        m_lockCount--;
    return -1;
    //#endregion
}

int WatchManager::FindNextFile( const std::string &filePath, const std::string &fileName, sVector::iterator &prevItr )
{    //#region
    EnterCriticalSection( &m_CriticalSection );        m_lockCount++;
    sVector::iterator itr;
    int index = -1;
    for( itr = m_files.begin(); itr != m_files.end(); itr++ )
    {
        index++;
        if( ( (*itr).m_filePath == filePath ) && ( (*itr).m_fileName == fileName ) )
        {
            prevItr = itr;
            LeaveCriticalSection( &m_CriticalSection );        m_lockCount--;
            return index;
        }
    }

    LeaveCriticalSection( &m_CriticalSection );        m_lockCount--;
    return -1;
    //#endregion
}

void WatchManager::UpdateFileTimesByIndex( const unsigned int fileIndex, FILETIME createTime, FILETIME modifyTime )
{    //#region
    EnterCriticalSection( &m_CriticalSection );        m_lockCount++;
    if( fileIndex >= m_files.size() )
    {
        LeaveCriticalSection( &m_CriticalSection );        m_lockCount--;
        throw exceptions::eWatchException( exceptions::eWatchException::WE_FILE_INDEX_OUT_OF_RANGE );
    }

    m_files[fileIndex].m_dtCreateDate = createTime;
    m_files[fileIndex].m_dtModifyDate = modifyTime;

    LeaveCriticalSection( &m_CriticalSection );        m_lockCount--;
    //#endregion
}

bool WatchManager::PauseWatch()
{    //#region
    EnterCriticalSection( &m_CriticalSection );
    bool    suspended = m_suspended,
            running = m_isRunning;
    LeaveCriticalSection( &m_CriticalSection );

    if( !suspended && running )
    {
        // it is actually in progress
        if( SuspendThread( m_hWatchThread ) != 0xFFFFFFFF )
        {
            // it is suspended
            EnterCriticalSection( &m_CriticalSection );
            m_suspended = true;
            LeaveCriticalSection( &m_CriticalSection );
            return true;
        }
    }
    return false;
    //#endregion
}

bool WatchManager::ResumeWatch()
{    //#region
    EnterCriticalSection( &m_CriticalSection );
    bool    suspended = m_suspended,
            running = m_isRunning;
    LeaveCriticalSection( &m_CriticalSection );

    if( suspended && running )
    {
        // it is running, but is currently suspended
        if( ::ResumeThread( m_hWatchThread ) == 1 )
        {
            // it is suspended
            EnterCriticalSection( &m_CriticalSection );
            m_suspended = false;
            LeaveCriticalSection( &m_CriticalSection );
            return true;
        }
    }
    return false;
    //#endregion
}

bool WatchManager::IsPaused() const
{    //#region
    bool suspended;
    EnterCriticalSection( &m_CriticalSection );
    suspended = m_suspended;
    LeaveCriticalSection( &m_CriticalSection );
    return suspended;
    //#endregion
}
//#endregion -- WatchManager --

//-----------------------------------------------------------------------------
DWORD WINAPI FileChangeWatch( LPVOID lpParam )
{    //#region
    WatchManager *pWatcher = (WatchManager*)lpParam;

    int i;
    int numHandles = pWatcher->GetNumToWatch() + 1;
    HANDLE *handles = new HANDLE[numHandles];

    OutputDebugString( "Create Exit Handle\n" );
    handles[0] = CreateEvent( NULL, TRUE, FALSE, NULL );
    pWatcher->SetExitHandle( handles[0] );

    OutputDebugString( "Set up file watch handles\n" );
    for( i = 1; i < numHandles; i++ )
    {
        char dbg[512];
        sprintf( dbg, "Watch %s\n", pWatcher->RefFileByIndex( i-1 ).m_filePath.c_str() );
        OutputDebugString( dbg );
        handles[i] = FindFirstChangeNotification( pWatcher->RefFileByIndex( i-1 ).m_filePath.c_str(), TRUE,
                         FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_ATTRIBUTES |
                         FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE );
    }

    DWORD result;

    while( 1 )
    {
        pWatcher->FlagRunning( true );

        if( pWatcher->DieOff() )
        {
            OutputDebugString( "CFileWatcher::Killing thread\n" );
            break;
        }

        result = WaitForMultipleObjects( numHandles, handles, FALSE, INFINITE );

        // Wait for an event to occur.
        if( result == WAIT_OBJECT_0 )
        {
            OutputDebugString( "CFileWatcher::Exit event signalled\n" );
            break;
        } else if( result == WAIT_TIMEOUT )
        {
            OutputDebugString( "CFileWatcher::TIMEOUT\n" );
            break;
        } else if( result == WAIT_ABANDONED )
        {
            OutputDebugString( "CFileWatcher::ABANDONED\n" );
            break;
        } else
        {
            int handleIdx = result - WAIT_OBJECT_0;
            int fileIdx = handleIdx-1;
            if( fileIdx < 0 )
                break;    //!< \todo Handle this properly, throw an exception?

            sFileWatchFile theFile;

            /*
             * !!WARNING!!
             * Because this runs in its own thread this is the only place
             * we can handle this exception. However how do we warn the main
             * app that we have broken????? Not handling the exception just
             * results in a crash!
             * !!WARNING!!
             */
            try
            {
                theFile = pWatcher->RefFileByIndex( fileIdx );
            }
            catch ( exceptions::eWatchException & except )
            {
                OutputDebugString( except.GetExString() ); OutputDebugString( "\n" );
                continue;
            }

            char buf[128];
            sprintf( buf, "fileWatch signalled (idx = %d), %s\n", fileIdx, theFile.GetFullFilePath().c_str() );
            OutputDebugString( buf );

            if( theFile.WatchIsFile() ) // we have a filename as well as a path
            {
                HANDLE f = CreateFile( theFile.GetFullFilePath().c_str(),
                                              0,
                                              ( FILE_SHARE_DELETE | FILE_SHARE_READ /*| FILE_SHARE_WRITE*/ ),
                                              NULL,
                                              OPEN_EXISTING,
                                              NULL,//FILE_ATTRIBUTE_NORMAL,
                                              NULL );
                if ( f != INVALID_HANDLE_VALUE )
                {
                    FILETIME dtModifyDate;
                    FILETIME dtCreateDate;
                    GetFileTime( f, &dtCreateDate, NULL, &dtModifyDate );
                    CloseHandle(f);

                    if( CompareFileTime( &(theFile.m_dtCreateDate), &dtCreateDate ) ||
                         CompareFileTime( &(theFile.m_dtModifyDate), &dtModifyDate ) )
                    {
                        pWatcher->UpdateFileTimesByIndex( fileIdx, dtCreateDate, dtModifyDate );
                        pWatcher->NotifyParent( fileIdx );
                    } else
                    {
                        OutputDebugString( "Folder has changed, but file dates are the same.\n" );
                    }
                } else
                {
                    OutputDebugString( "Failed to get file handle\n" );
                }
            } else
            {
                // its a folder
                pWatcher->NotifyParent( fileIdx );
            }

            if( !FindNextChangeNotification( handles[handleIdx] ) )
            {
                return -1;
            }

/*/
            OutputDebugString( "CFileWatcher::SOME OTHER EVENT SIGNAL\n" );
            break;
//*/
        }
    }

    OutputDebugString( "CFileWatcher::Cleaning up prior to thread exit\n" );

    for( i = 0; i < numHandles; i++ )
    {
        CloseHandle( handles[i] );
        handles[i] = NULL;
    }
    if( handles )
    {
        delete[] handles;
        handles = NULL;
    }
    //pWatcher->SetExitHandle( NULL );
    pWatcher->FlagRunning( false );

    return 0;
    //#endregion
}

}; // namespace watch
}; // namespace keays
