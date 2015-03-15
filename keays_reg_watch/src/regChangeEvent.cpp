#include "..\include\RegChangeEvent.h"
#include "..\include\krwExceptions.h"

#include <io.h>

namespace keays
{    //#region
namespace watch
{    //#region

using namespace std;

static DWORD WINAPI threadProc( LPVOID lpParameter );

#ifdef _DEBUG
KEAYS_REG_WATCH_API void OutputDebugStringf( const char *fmt, ... )
{    //#region
    char buf[1024];
    buf[1023] = 0;
    va_list args;
    va_start( args, fmt );
    _vsnprintf( buf, 1023, fmt, args );
    va_end( args );

    OutputDebugString( buf );
    //#endregion
}
#else
KEAYS_REG_WATCH_API void OutputDebugStringf( const char *fmt, ... ) {}
#endif

//-----------------------------------------------------------------------------
//#region KeyData implementation
KeyData::KeyData( const HKEY baseKey, const char *key )
{    //#region
    m_key = NULL;
    m_baseKey = NULL;

    BaseKey( baseKey );
    Key( key );
    //#endregion
}

KeyData::KeyData( const KeyData &orig )
{    //#region
    m_key = NULL;
    m_baseKey = NULL;

    BaseKey( orig.BaseKey() );
    Key( orig.Key() );
    //#endregion
}

KeyData::~KeyData()
{    //#region
    if( m_key )
    {
        delete[] m_key;
        m_key = NULL;
    }
    m_baseKey = NULL;
    //#endregion
}

const KeyData &KeyData::operator=( const KeyData &rhs )
{    //#region
    BaseKey( rhs.BaseKey() );
    Key( rhs.Key() );
    return *this;
    //#endregion
}

bool KeyData::operator==( const KeyData &rhs ) const
{    //#region
    return ( m_baseKey == rhs.BaseKey() ) && ( stricmp( m_key, rhs.Key() ) == 0 );
    //#endregion
}

HKEY KeyData::BaseKey( HKEY baseKey )
{    //#region
    return m_baseKey = baseKey;
    //#endregion
}

const char *KeyData::Key( const char *key )
{    //#region
    UINT len = strlen( key ) + 1;

    if( m_key )
    {
        delete[] m_key;
        m_key = NULL;
    }
    m_key = new char[len];
    memset( m_key, 0, len );
    strncpy( m_key, key, len );

    return m_key;
    //#endregion
}
//#endregion

//-----------------------------------------------------------------------------
//#region WatchKeyData implementation
WatchKeyData::WatchKeyData( UINT id, const HKEY baseKey, const char *key ) : KeyData( baseKey, key )
{    //#region
    ID( id );
    //#endregion
}

bool WatchKeyData::operator==( const WatchKeyData & rhs )
{    //#region
    return ( m_id == rhs.ID() );
    //#endregion
}

const WatchKeyData &WatchKeyData::operator=( const WatchKeyData &rhs )
{    //#region
    ID( rhs.ID() );
    BaseKey( rhs.BaseKey() );
    Key( rhs.Key() );
    return *this;
    //#endregion
}
//#endregion

//-----------------------------------------------------------------------------
//#region RegWatcher implementation
/*!
    \constructor
    \param fn [In]  - a function pointer to the function to call when the watch event is triggered.
 */
RegWatcher::RegWatcher( pFnNotify fn )
{    //#region
    InitializeCriticalSection( &m_CriticalSection );

    m_pKeys = new vector<WatchKeyData>;
    m_exitHandle = NULL;
    m_isRunning = false;
    m_suspended = false;
    m_thread = NULL;
    NotifyFn = fn;
    //#endregion
}

/*!
    \destructor
 */
RegWatcher::~RegWatcher()
{     //#region
    StopWatch();

    if( m_pKeys )
    {
        delete m_pKeys;
        m_pKeys = NULL;
    }
    DeleteCriticalSection( &m_CriticalSection );
    //#endregion
}

/*!
    \brief Add a new key to the list to be watched

    \param a const reference to a KeyData object defining the key to watch.
 */
bool RegWatcher::AddKey( const WatchKeyData &newKey )
{    //#region
    vector<WatchKeyData>::iterator itr;
    bool found = false;
    for( itr = m_pKeys->begin(); itr != m_pKeys->end(); itr++ )
    {
        if( newKey.ID() == (*itr).ID() )
        {
            found = true;
            break;
        }
    }

    if( !found )
    {
        m_pKeys->push_back( newKey );
    }

    return !found;
    //#endregion
}

bool RegWatcher::RemoveKey( const WatchKeyData &key )
{    //#region
    vector<WatchKeyData>::iterator itr;
    for( itr = m_pKeys->begin(); itr != m_pKeys->end(); itr++ )
    {
        if( key.ID() == (*itr).ID() )
        {
            m_pKeys->erase( itr );
            return true;
        }
    }

    return false;
    //#endregion
}

/*!
    \brief Start the watcher running.

    \param    pID [Out] - a pointer to an int for the id of the thread.
    \return true it the thread is running.
 */
bool RegWatcher::StartWatch( int *pID )
{    //#region
    DWORD id = 0;

    EnterCriticalSection( &m_CriticalSection );
    while( m_isRunning )
    {
        StopWatch();
    }

    m_thread = CreateThread( NULL, 0, threadProc, (void*)this, 0, &id );
    m_isRunning = true;

    if( pID )
        *pID = id;

    LeaveCriticalSection( &m_CriticalSection );

    return m_isRunning;
    //#endregion
}

/*!
    \brief End the watcher.

    \return true it the thread is stopped.
 */
bool RegWatcher::StopWatch()
{    //#region
    bool result;
    HANDLE tmpHandle;
    EnterCriticalSection( &m_CriticalSection );
    tmpHandle = m_exitHandle;
    LeaveCriticalSection( &m_CriticalSection );
    if( tmpHandle )
    {
        SetEvent( m_exitHandle );
    }

    while( IsRunning() )
    {
        OutputDebugStringf( "Waiting for reg watcher thread to finish\n" ); // wait for it to end
    }

    result = !IsRunning();
    return result;
    //#endregion
}

bool RegWatcher::PauseWatch()
{    //#region
    EnterCriticalSection( &m_CriticalSection );
    bool    suspended = m_suspended,
            running = m_isRunning;
    LeaveCriticalSection( &m_CriticalSection );

    if( !suspended && running )
    {
        // it is actually in progress
        if( SuspendThread( m_thread ) != 0xFFFFFFFF )
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

bool RegWatcher::ResumeWatch()
{    //#region
    EnterCriticalSection( &m_CriticalSection );
    bool    suspended = m_suspended,
            running = m_isRunning;
    LeaveCriticalSection( &m_CriticalSection );

    if( suspended && running )
    {
        // it is running, but is currently suspended
        if( ::ResumeThread( m_thread ) == 1 )
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

bool RegWatcher::IsRunning() const
{    //#region
    bool running;
    EnterCriticalSection( &m_CriticalSection );
    running = m_isRunning;
    LeaveCriticalSection( &m_CriticalSection );
    return running;
    //#endregion
}

bool RegWatcher::IsPaused() const
{    //#region
    bool suspended;
    EnterCriticalSection( &m_CriticalSection );
    suspended = m_suspended;
    LeaveCriticalSection( &m_CriticalSection );
    return suspended;
    //#endregion
}

HANDLE RegWatcher::SetExitHandle( HANDLE exitHandle )
{    //#region
    EnterCriticalSection( &m_CriticalSection );
    m_exitHandle = exitHandle;
    LeaveCriticalSection( &m_CriticalSection );
    return exitHandle;
    //#endregion
}

const vector<WatchKeyData> &RegWatcher::GetKeysRef() const
{    //#region
    return *m_pKeys;
    //#endregion
}

UINT RegWatcher::Notify( int index ) const
{    //#region
    if( ( index < 0 ) || ( (UINT)index >= m_pKeys->size() ) )
        return 0;
    const WatchKeyData &key = m_pKeys->at( index );
    return NotifyFn( key.ID(), key );
    //#endregion
}
//#endregion

//-----------------------------------------------------------------------------
DWORD WINAPI threadProc( LPVOID lpParameter )
{    //#region
    CRITICAL_SECTION criticalSection;
    DWORD dwFilter = REG_NOTIFY_CHANGE_NAME |
                          REG_NOTIFY_CHANGE_ATTRIBUTES |
                          REG_NOTIFY_CHANGE_LAST_SET |
                          REG_NOTIFY_CHANGE_SECURITY;

    LONG lErrorCode = ERROR_SUCCESS;

    InitializeCriticalSection( &criticalSection );

    RegWatcher *pWatcher = (RegWatcher *)lpParameter;

    if( pWatcher == NULL )
    {
        return 0;
    }

    // no point doing anything if we have no keys to watch

    const vector<WatchKeyData> &rkeys = pWatcher->GetKeysRef();
    if( rkeys.size() < 1 )
    {
        return 0;
    }

    HANDLE *pHandles = new HANDLE[rkeys.size()+1];

    // create the exit event first
    pHandles[0] = CreateEvent( NULL, TRUE, FALSE, "reg_watch_exit_event" );
    pWatcher->SetExitHandle( pHandles[0] );

    int eventCount = 1, i;
    HKEY hKey = NULL;
    vector<HKEY> keys;
    vector<HKEY>::iterator keyItr;
    vector<WatchKeyData>::const_iterator itr;

    EnterCriticalSection( &criticalSection );
    for( itr = rkeys.begin(); itr != rkeys.end(); itr++ )
    {
        // open the key
        const WatchKeyData &kData = (*itr);
        if( RegOpenKeyEx( kData.BaseKey(), kData.Key(), 0, KEY_NOTIFY, &hKey ) == ERROR_SUCCESS )
        {
            keys.push_back( hKey ); // since we have a key we add it and create an event
            pHandles[eventCount++] = CreateEvent( NULL, TRUE, FALSE, NULL );
        } else
        {
            OutputDebugStringf( "Failed to open key\n" );
        }
    }
    LeaveCriticalSection( &criticalSection );

    while( 1 )
    {
        // watch for the events
        for( i = 0; i < eventCount-1; i++ )
        {
            hKey = keys[i];
            lErrorCode = RegNotifyChangeKeyValue( keys[i], TRUE, dwFilter, pHandles[i+1], TRUE );
            if( lErrorCode != ERROR_SUCCESS )
            {
                OutputDebugStringf( "Failed to attach event (%d/%d)\n", i, eventCount );
                break;
            }
        }

        lErrorCode = WaitForMultipleObjects( eventCount, pHandles, FALSE, INFINITE );
        int result = lErrorCode - WAIT_OBJECT_0;
        if( result )
        {
            OutputDebugStringf( "Watch (%d) signalled - %s - \n\t%u\n", result, rkeys[result-1].Key(), pWatcher->Notify( result-1 ) );
        } else
        {
            // it's an exit event
            OutputDebugStringf( "Exit event fired!!!\n" );
            break;
        }
        OutputDebugStringf( "\tresetting...\n" );
    }

    // cleanup
    for( keyItr = keys.begin(); keyItr != keys.end(); keyItr++ )
    {
        hKey = (*keyItr);
        lErrorCode = RegCloseKey( hKey );
        if( lErrorCode != ERROR_SUCCESS )
        {
            OutputDebugStringf( "Failed to close key\n" );
        }
    }

    for( int closeEvent = 0; closeEvent < eventCount; closeEvent++ )
    {
        if( !CloseHandle( pHandles[closeEvent] ) )
        {
            OutputDebugStringf( "Failed to close handle (%d)\n", closeEvent );
        }
    }

    delete[] pHandles;
    pHandles = NULL;

    EnterCriticalSection( &criticalSection );
    pWatcher->SetStopped();
    LeaveCriticalSection( &criticalSection );

    OutputDebugStringf( "Watcher thread is dead\n" );

    return 1;
    //#endregion
}

//#endregion
}; // namespace watch
//#endregion
}; // namespace keays
