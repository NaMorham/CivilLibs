/*!
    \file FileChangeEvent.h
    \brief Classes and Structures used in managing file and directory watch.

    \author Andrew Hickey, Keays Software
    \date Thursday, October 28th, 2004
 */
#ifndef _FILE_CHANGE_EVENT_H_
#define _FILE_CHANGE_EVENT_H_

#include <windows.h>
#include <vector>
#include <string>
#include <exception>


namespace keays
{
namespace watch
{

/*!
    \brief Watch thread handler.

    \pararm lpParam [In]  - a void pointer to a CFileWatcher(internal), MUST NOT BE NULL.
    \return a DWORD representing the exit code of the thread.
 */
static DWORD WINAPI FileChangeWatch( void *lpParam );

/*!
    \brief File/Folder to watch information.
 */
struct sFileWatchFile
{
    enum {
        INVALID_ID = ~(0),    //!< indicates an invalid ID number
    };

    /*!
        \brief Default constructor
        Creates an invalid object.
     */
    sFileWatchFile();
    /*!
        \brief Cconstruct a folder to watch.
        \param     id [In]  - an unsigned integer as an identifier for this object.
        \param folder [In]  - a constant char pointer to the folder to watch.
     */
    sFileWatchFile( UINT id, const char *folder );
    /*!
        \brief Cconstruct a folder to watch.
        \param     id [In]  - an unsigned integer as an identifier for this object.
        \param folder [In]  - a constant char pointer to the folder containing the file to watch.
        \param   file [In]  - a constant char pointer to the name of the file to watch.
     */
    sFileWatchFile( UINT id, const char *folder, const char *file );
    /*!
        \brief Copy Constructor.
        Construct a sFileWatchFile Object as a duplicate of an existing object.
        \param orig [In]  - a constant reference to an existing sFileWatchFile object.
     */
    sFileWatchFile( const sFileWatchFile &orig );
    /*!
        \brief Duplicate an existing sFileWatchFile Object
        \param rhs [In]  - a constant reference to an existing sFileWatchFile object.
     */
    const sFileWatchFile &operator=( const sFileWatchFile &rhs );

    /*!
        \brief Test if this is a folder.
        \return true if this is a folder.
     */
    bool WatchIsFolder() const { return m_fileName.empty(); }
    /*!
        \brief Test if this is a file.
        \return true if this is a file.
     */
    bool WatchIsFile() const { return !m_fileName.empty(); }

    /*!
        \brief Get the full path and name of the file being watched.
        \return a std::string containing the full path and name of the file
     */
    const std::string GetFullFilePath() const;

    UINT            m_ID;                    //!< a unique id for the file being watched
    FILETIME        m_dtModifyDate;    //!< the last modify time for the file being watched
    FILETIME        m_dtCreateDate;    //!< the creation time for the file being watched
    std::string m_fileName;            //!< the name of the file being watched
    std::string m_filePath;            //!< the path to the file being watched (if this is null we are watching a folder)
};

/*!
    \typedef A simple typedef of a std::vector of sFileWatchFile Objects.
 */
typedef std::vector<sFileWatchFile> sVector;

/*!
    \brief A WatchManager class for multiple files/folders.
 */
class WatchManager
{
public:
    /*!
        \brief A simple typedef for the callback function to indicate which file has changed

        \param a UINT    to receive the file ID of the file/folder that caused the notification.
        \param a constant reference to a std::string to receive the full path and name of the file/folder sending the notification.
        \return a UINT as the return code.
     */
    typedef UINT (*pFnNotify)( UINT, const std::string & );

    /*!
        \brief The constructor

        \param pfnNotifyFn [In]  - a pointer to the callback function to called when a file has changed.  It must take a
        <b>UINT</b> and a <b>const std::string &</b> and return a UINT.
        \exception eWatchException( WE_NULL_CALLBACK ) { description }
     */
    WatchManager( pFnNotify pfnNotifyFn );
    /*!
        \brief The destructor
        Stops the watch running, and frees any objects and handles associated with it.
     */
    ~WatchManager();

    /*!
        \brief Add another file to the watch list
        Pauses the watch and adds the specified file to the list if it (the file) exists

        \param file [In]  - a constant reference to a std::string refrencing the full path and name of the file to add.

        \return true if the add was successful
     */
    bool AddFile( const sFileWatchFile &file );

    /*!
        \brief Add another file to the watch list
        Pauses the watch and adds the specified file to the list if it (the file) exists

        \param filePath [In]  - a constant char * pointing to the path of the file to add.
        \param fileName [In]  - a constant char * pointing to the name of the file to add.

        \return true if the add was successful
     */
    bool AddFile( UINT uniqueID, const char *filePath, const char *fileName );

    /*!
        \brief Add another folder to the watch list
        Pauses the watch and adds the specified folder to the list if it (the folder) exists

        \param filePath [In]  - a constant char * pointing to the path of the file to add.

        \return true if the add was successful
     */
    bool AddFolder( UINT uniqueID, const char *path );

    /*!
     */
    bool RemoveFile( const UINT fileID, bool firstOnly = false );
    /*!
     */
    bool RemoveFile( const char *filePath, bool firstOnly = false );
    /*!
     */
    bool RemoveFile( const char *filePath, const char *fileName, bool firstOnly = false );
    /*!
     */
    //bool RemoveFileEx( const char *fullPath, bool firstOnly = false );

    bool RemoveAll();

    /*!
     */
    bool StartWatch();
    /*!
     */
    void StopWatch();

    /*!
     */
    const sFileWatchFile &RefFileByIndex( const unsigned int index );

    /*!
     */
    bool IsRunning() const
    {
        EnterCriticalSection( &m_CriticalSection );        m_lockCount++;
        bool b = m_isRunning;
        LeaveCriticalSection( &m_CriticalSection );        m_lockCount--;
        return b;
    }

    /*!
     */
    int GetNumToWatch() const
    {
        EnterCriticalSection( &m_CriticalSection );        m_lockCount++;
        int num = m_files.size();
        LeaveCriticalSection( &m_CriticalSection );        m_lockCount--;
        return num;
    }

    /*!
     */
    void NotifyParent( unsigned int filesIdx );

    /*!
     */
    const HANDLE SetExitHandle( const HANDLE exHandle );//    { return ( m_exitHandle = exHandle ); }

    /*!
     */
    const HANDLE GetExitHandle() const    { return m_exitHandle; }

//--- These are only for use by the thread function
    /*!
     */
    bool DieOff() const
    {
        EnterCriticalSection( &m_CriticalSection );        m_lockCount++;
        bool b = m_kill;
        LeaveCriticalSection( &m_CriticalSection );        m_lockCount--;
        return b;
    }

    /*!
     */
    void FlagRunning( const bool isRunning )
    {
        EnterCriticalSection( &m_CriticalSection );        m_lockCount++;
        m_isRunning = isRunning;
        LeaveCriticalSection( &m_CriticalSection );        m_lockCount--;
    }

    /*!
     */
    void UpdateFileTimesByIndex( const unsigned int fileIndex, FILETIME createTime, FILETIME modifyTime );
//---

    bool PauseWatch();

    bool ResumeWatch();

    bool IsPaused() const;

private:
    /*
     */
    int FindFirstFile( const UINT fileID, sVector::iterator *pItr );

    /*
     */
    int FindNextFile( const UINT fileID, sVector::iterator &prevItr );

    /*
     */
    int FindFirstFile( const std::string &filePath, sVector::iterator *pItr );

    /*
     */
    int FindNextFile( const std::string &filePath, sVector::iterator &prevItr );

    /*
     */
    int FindFirstFile( const std::string &filePath, const std::string &fileName, sVector::iterator *pItr );

    /*
     */
    int FindNextFile( const std::string &filePath, const std::string &fileName, sVector::iterator &prevItr );

    /*
     */
    void StopWatchInt();

    HANDLE        m_hWatchThread;            //!< the handle to the thread.
    DWORD            m_dwWatchThreadID;        //!< the ID for the thread.
    sVector        m_files;                        //!< an std::vector of sFileWatchFile objects to watch.
    HANDLE        m_exitHandle;                //!< the handle for the exit event.
    bool            m_isRunning;                //!< flag to indicate that the thread has entered its running loop.
    bool            m_suspended;                //!< flag to indicate that the thread has been suspended.
    bool            m_kill;                        //!< flag to indicate to the thread that it should die.
    pFnNotify    m_pfnNotify;                //!< the function pointer for the callback function.

    mutable CRITICAL_SECTION
                    m_CriticalSection;        //!< local CRITICAL_SECTION object to control access where needed.

    mutable DWORD
                    m_lockCount;
};

}; // namespace watch
}; // namespace keays

#endif    // _FILE_CHANGE_EVENT_H_
