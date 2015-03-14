#include "..\include\krwExceptions.h"

namespace keays
{	//#region
namespace watch
{	//#region
namespace exceptions
{	//#region

const char *eRegException::ERROR_CODES[] = 
{	//#region
	"INVALID EXCEPTION TYPE",
	"Path specified is NULL",
	"File Name specified is NULL",
	"An invalid file or folder was specified",
	"Unable to get file times for file specified",
	//#endregion
};

const char *eRegException::GetExString() const
{	//#region
	if( ( m_type <= RE_EXCEPTION_TYPE_MIN_LIMIT ) || ( m_type >= RE_EXCEPTION_TYPE_MAX_LIMIT ) )
	{
		return ERROR_CODES[0];
	} else
	{
		return ERROR_CODES[m_type];
	}
	//#endregion
}

//-----------------------------------------------------------------------------
#if 0
const char *eWatchException::ERROR_CODES[] = 
{	//#region
	"INVALID EXCEPTION TYPE",
	"Callback function specified is NULL",
	"File Index out of range",
	//#endregion
};

const char *eWatchException::GetExString() const
{	//#region
	if( ( m_type <= WE_EXCEPTION_TYPE_MIN_LIMIT ) || ( m_type >= WE_EXCEPTION_TYPE_MAX_LIMIT ) )
	{
		return ERROR_CODES[0];
	} else
	{
		return ERROR_CODES[m_type];
	}
	//#endregion
}
#endif

//#endregion
}; // namespace exceptions
//#endregion
}; // namespace watch
//#endregion
}; // namespace keays
