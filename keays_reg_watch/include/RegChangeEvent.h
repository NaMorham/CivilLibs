/*!
    \file RegChangeEvent.h
    \brief Classes and Structures used in managing file and directory watch.

    \author Andrew Hickey, Keays Software
    \date Thursday, October 28th, 2004
 */
#ifndef _REG_CHANGE_EVENT_H_
#define _REG_CHANGE_EVENT_H_

#include <windows.h>
#include <vector>
#include <string>
#include <exception>

#ifdef KEAYS_REG_WATCH_EXPORTS
#define KEAYS_REG_WATCH_API __declspec(dllexport)
#else
#define KEAYS_REG_WATCH_API __declspec(dllimport)
#endif

namespace keays
{
namespace watch
{

KEAYS_REG_WATCH_API void OutputDebugStringf( const char *fmt, ... );

class KEAYS_REG_WATCH_API KeyData
{
public:
    KeyData( const HKEY baseKey, const char *key );
    KeyData( const KeyData &orig );
    ~KeyData();

    const KeyData &operator=( const KeyData &rhs );

    bool operator==( const KeyData &rhs ) const;

    HKEY BaseKey() const { return m_baseKey; }
    const char *Key() const { return m_key; }

protected:
    HKEY BaseKey( HKEY baseKey );
    const char *Key( const char *key );

private:
    HKEY m_baseKey;
    char *m_key;
};

class KEAYS_REG_WATCH_API WatchKeyData : public KeyData
{
public:
    WatchKeyData( UINT id, const HKEY baseKey, const char *key );

    bool operator==( const WatchKeyData & rhs );

    const WatchKeyData &operator=( const WatchKeyData &rhs );

    const UINT ID() const { return m_id; }

protected:
    const UINT ID( const UINT id ) { return m_id = id; }

private:
    UINT m_id;
};

class KEAYS_REG_WATCH_API RegWatcher
{
public:
    /*!
        \brief A simple typedef for the callback function to indicate which file has changed

        \param a UINT    to receive the file ID of the file/folder that caused the notification.
        \param a constant reference to a std::string to receive the full path and name of the file/folder sending the notification.
        \return a UINT as the return code.
     */
    typedef UINT (*pFnNotify)( UINT, const KeyData );

    /*!
        \constructor
        \param fn [In]  - a function pointer to the function to call when the watch event is triggered.
     */
    RegWatcher( pFnNotify fn );

    /*!
        \destructor
     */
    ~RegWatcher();

    /*!
        \brief Add a new key to the list to be watched

        \param a const reference to a KeyData object defining the key to watch.
     */
    bool AddKey( const WatchKeyData &newKey );

    bool RemoveKey( const WatchKeyData &key );

    /*!
        \brief Start the watcher running.

        \param    pID [Out] - a pointer to an int for the id of the thread.
        \return true it the thread is running.
     */
    bool StartWatch( int *pID );

    /*!
        \brief End the watcher.

        \return true it the thread is stopped.
     */
    bool StopWatch();

    bool PauseWatch();

    bool ResumeWatch();

    bool IsRunning() const;

    bool IsPaused() const;

    HANDLE SetExitHandle( HANDLE exitHandle );

    const std::vector<WatchKeyData> &GetKeysRef() const;

    UINT Notify( int index ) const;

    void SetStopped( bool stopped = true ) { m_isRunning = !stopped; }
private:
    pFnNotify    NotifyFn;
    HANDLE        m_exitHandle;
    std::vector<WatchKeyData>
                    *m_pKeys;
    HANDLE        m_thread;
    bool            m_isRunning;
    bool            m_suspended;

    mutable CRITICAL_SECTION
                    m_CriticalSection;
};

}; // namespace watch
}; // namespace keays

#endif    // _REG_CHANGE_EVENT_H_
