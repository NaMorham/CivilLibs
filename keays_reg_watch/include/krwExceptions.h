/*!
    \file krfExceptions.h
    \brief Exceptions thrown by Keays Software Registry Watch classes.

    \author Andrew Hickey, Keays Software
    \date Friday, January 20th, 2005
 */
#ifndef _KRW_EXCEPTIONS_H_
#define _KRW_EXCEPTIONS_H_

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
    \brief An Exception class for exceptions caused on a per registry key level.
 */
class eRegException
{
public:
    enum R_ExceptionType
    {
        RE_EXCEPTION_TYPE_MIN_LIMIT,    //!< used solely for limit checking when returning string MUST STAY FIRST.

        RE_NULL_FOLDER,                    //!< indicates a NULL folder caused the exception.
        RE_NULL_FILE_NAME,                //!< indicates a NULL file name caused the exception.
        RE_INVALID_FILE,                    //!< indicates that a non-existant or invalid file/folder caused the exception.
        RE_GET_TIMES,                        //!< indicates that trying to get FILETIMES caused the exception.

        RE_EXCEPTION_TYPE_MAX_LIMIT    //!< used solely for limit checking when returning string MUST STAY LAST.
    };

    /*!
        \brief Constuct an eRegException.
        \param type [In]  - a constant R_Exception value indicating the cause of this exception.
     */
    eRegException( const R_ExceptionType type ) : m_type( type ) {}

    /*!
        \brief Return a string describing the cause of the exception.
        \return a constant pointer to a string describing the cause of the exception.
     */
    const char *GetExString() const;

    /*!
        \brief Return the type of exception.
        \return a R_ExceptionType indicating the type (cause) of the exception.
     */
    const R_ExceptionType GetType() const { return m_type; }

private:
    R_ExceptionType m_type;                        //<! the cause of the exception
    static const char *ERROR_CODES[];        //<! an array  of strings describing the cause
};


#if 0
/*!
    \brief An Exception class for exceptions caused on a watcher level.
 */
class eWatchException
{
public:
    enum W_ExceptionType
    {
        WE_EXCEPTION_TYPE_MIN_LIMIT,    //!< used solely for limit checking when returning string MUST STAY FIRST.

        WE_NULL_CALLBACK,                    //!< indicates a NULL function pointer was passed as a callback.
        WE_FILE_INDEX_OUT_OF_RANGE,    //!< an index was passed the was out of range.

        WE_EXCEPTION_TYPE_MAX_LIMIT    //!< used solely for limit checking when returning string MUST STAY LAST.
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
    W_ExceptionType m_type;                    //<! the cause of the exception
    static const char *ERROR_CODES[];    //<! an array  of strings describing the cause
};
#endif


}; // namespace exceptions
}; // namespace fileWatch
}; // namespace keays

#endif _KFW_EXCEPTIONS_H_