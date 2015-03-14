/*! \file */
/*-----------------------------------------------------------------------
	logger.h

	Description: Logger class declaration
	Author:		 Julian McKinlay
	---------------------------------------------------------------------*/

#pragma once

//! \enum
enum LOGLEVEL
{
	LL_LOWEST = 0,	/**< Lowest log level available. */
	LL_KSR,			/**< KSR internal logging. */
	LL_APP0,		/**< Application defined level 0. */
	LL_APP1,		/**< Application Defined level 1. */
	LL_APP2,		/**< Application Defined level 2. */
	LL_APP3,		/**< Application Defined level 3. */
	LL_APP4,		/**< Application Defined level 4. */
	LL_APP5,		/**< Application Defined level 5. */
	LL_APP6,		/**< Application Defined level 6. */
	LL_APP7,		/**< Application Defined level 7. */
	LL_HIGHEST,		/**< Highest log level available. */
};

struct Logger
{
public:
	Logger();
	~Logger();

	void Log(const ubyte level, LPCTSTR text);
	void SetMaxSize(const char size) { m_maxSize = size; };
	void SetMinLevel(const char level) { m_minLogLevel = level; };
	void SetMaxLevel(const char level) { m_maxLogLevel = level; };
	void SetFile(bool enabled, LPCTSTR filename);
	void GetLog(StringList::iterator *pBegin,
				StringList::iterator *pEnd);

private:
	StringList m_log;
	int m_maxSize,
		m_minLogLevel,
		m_maxLogLevel;

	bool m_fileEnabled;
	String m_filePath;
	FILE *m_file;
};

// EOF