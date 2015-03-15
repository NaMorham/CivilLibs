#include "..\include\kfwExceptions.h"

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
namespace exceptions
{

const char *eFileException::ERROR_CODES[] =
{
    "INVALID EXCEPTION TYPE",
    "Path specified is NULL",
    "File Name specified is NULL",
    "An invalid file or folder was specified",
    "Unable to get file times for file specified",
};

const char *eFileException::GetExString() const
{
    if( ( m_type <= FE_EXCEPTION_TYPE_MIN_LIMIT ) || ( m_type >= FE_EXCEPTION_TYPE_MAX_LIMIT ) )
        return ERROR_CODES[0];
    else
        return ERROR_CODES[m_type];
}

//-----------------------------------------------------------------------------
const char *eWatchException::ERROR_CODES[] =
{
    "INVALID EXCEPTION TYPE",
    "Callback function specified is NULL",
    "File Index out of range",
};

const char *eWatchException::GetExString() const
{
    if( ( m_type <= WE_EXCEPTION_TYPE_MIN_LIMIT ) || ( m_type >= WE_EXCEPTION_TYPE_MAX_LIMIT ) )
        return ERROR_CODES[0];
    else
        return ERROR_CODES[m_type];
}


}; // namespace exceptions
}; // namespace watch
}; // namespace keays
