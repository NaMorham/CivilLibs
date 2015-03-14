/*!
	\file kwfExceptions.h
	\brief Exceptions thrown by Keays Software File & Registry Watch classes.

	\author Andrew Hickey, Keays Software
	\date Thursday, October 28th, 2004
 */
#ifndef _KFW_EXCEPTIONS_H_
#define _KFW_EXCEPTIONS_H_

/*!
	\brief General Keays Software Namespace
	Used to prevent conflicts with existing functions/ variables
 */
namespace keays
{
/*!
	\brief file watch classes and structures for use in Keays Software applications
 */
namespace watch
{
/*!
	\brief exceptions thrown by Keays Software File & Registry Watch classes.
 */
namespace exceptions
{

/*!
	\brief An Exception class for exceptions caused on a per file level.
 */
class eFileException
{
public:
	enum F_ExceptionType
	{
		FE_EXCEPTION_TYPE_MIN_LIMIT,	//!< used solely for limit checking when returning string MUST STAY FIRST.

		FE_NULL_FOLDER,					//!< indicates a NULL folder caused the exception.
		FE_NULL_FILE_NAME,				//!< indicates a NULL file name caused the exception.
		FE_INVALID_FILE,					//!< indicates that a non-existant or invalid file/folder caused the exception.
		FE_GET_TIMES,						//!< indicates that trying to get FILETIMES caused the exception.

		FE_EXCEPTION_TYPE_MAX_LIMIT	//!< used solely for limit checking when returning string MUST STAY LAST.
	};

	/*!
		\brief Constuct an eFileException.
		\param type [In]  - a constant F_Exception value indicating the cause of this exception.
	 */ 
	eFileException( const F_ExceptionType type ) : m_type( type ) {}

	/*!
		\brief Return a string describing the cause of the exception.
		\return a constant pointer to a string describing the cause of the exception.
	 */
	const char *GetExString() const;

	/*!
		\brief Return the type of exception.
		\return a F_ExceptionType indicating the type (cause) of the exception.
	 */
	const F_ExceptionType GetType() const { return m_type; }

private:
	F_ExceptionType m_type;					//<! the cause of the exception
	static const char *ERROR_CODES[];		//<! an array  of strings describing the cause
};


/*!
	\brief An Exception class for exceptions caused on a watcher level.
 */
class eWatchException
{
public:
	enum W_ExceptionType
	{
		WE_EXCEPTION_TYPE_MIN_LIMIT,	//!< used solely for limit checking when returning string MUST STAY FIRST.

		WE_NULL_CALLBACK,					//!< indicates a NULL function pointer was passed as a callback.
		WE_FILE_INDEX_OUT_OF_RANGE,	//!< an index was passed the was out of range.

		WE_EXCEPTION_TYPE_MAX_LIMIT	//!< used solely for limit checking when returning string MUST STAY LAST.
	};

	/*!
		\brief Constuct an eWatchException.
		\param type [In]  - a constant W_Exception value indicating the cause of this exception.
	 */ 
	eWatchException( const W_ExceptionType type ) : m_type( type ) {}

	/*!
		\brief Return a string describing the cause of the exception.
		\return a constant pointer to a string describing the cause of the exception.
	 */
	const char *GetExString() const;

	/*!
		\brief Return the type of exception.
		\return a W_ExceptionType indicating the type (cause) of the exception.
	 */
	const W_ExceptionType GetType() const { return m_type; }

private:
	W_ExceptionType m_type;					//<! the cause of the exception
	static const char *ERROR_CODES[];	//<! an array  of strings describing the cause
};

}; // namespace exceptions
}; // namespace fileWatch
}; // namespace keays

#endif _KFW_EXCEPTIONS_H_