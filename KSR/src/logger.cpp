#include "../include/ksr.h"

#include "../include/leakwatcher.h"

#ifdef _DO_MEMORY_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace KSR;

Logger *g_pLogger = NULL;

//-----------------------------------------------------------------------
	void KSR::Logf(LPCTSTR fmt, ...)
	//-------------------------------------------------------------------
	{
		va_list args;
		TCHAR buf1[1024];
		TCHAR buf2[1024];
		TCHAR timeBuf[32];

		SYSTEMTIME time;
		GetLocalTime(&time);
		_stprintf(timeBuf, _T("%02hu:%02hu:%02hu"), time.wHour, time.wMinute, time.wSecond);

		va_start(args, fmt);
		_vstprintf(buf1, fmt, args);
		va_end(args);

		_stprintf(buf2, _T("%s %s"), timeBuf, buf1);

		if (g_pLogger)
			g_pLogger->Log(LL_LOWEST, buf2);
	}


//-----------------------------------------------------------------------
	void KSR::Logf(ubyte level, LPCTSTR fmt, ...)
	// Note: Logf Duplicated to allow arguments list
	//-------------------------------------------------------------------
	{
		va_list args;
		TCHAR buf1[1024];
		TCHAR buf2[1024];
		TCHAR timeBuf[32];

		SYSTEMTIME time;
		GetLocalTime(&time);
		_stprintf(timeBuf, _T("%02hu:%02hu:%02hu"), time.wHour, time.wMinute, time.wSecond);

		va_start(args, fmt);
		_vstprintf(buf1, fmt, args);
		va_end(args);

		_stprintf(buf2, _T("%s %s"), timeBuf, buf1);

		if (g_pLogger)
			g_pLogger->Log(level, buf2);
	}


//-----------------------------------------------------------------------
	void Logger::Log(const ubyte level, LPCTSTR text)
	//-------------------------------------------------------------------
	{
		if (!m_maxSize && !m_fileEnabled)
			return;

		if (!(level >= m_minLogLevel && level <= m_maxLogLevel))
			return;

		String str = text;
		m_log.push_back(str);
		str += _T("\n");

		String debugStr(_T("[KSR] "));
		debugStr += str;
		DebugPrintf(debugStr.c_str());

		if (m_maxSize > -1)
		{
			while (m_log.size() > (size_t)m_maxSize)
				m_log.pop_front();
		}

		if (m_fileEnabled)
		{
			fwrite(str.c_str(), str.size(), sizeof(TCHAR), m_file);
			fflush(m_file);
		}
	}


//-----------------------------------------------------------------------
	Logger::Logger()
		:	m_minLogLevel(LL_LOWEST),
			m_maxLogLevel(LL_HIGHEST),
			m_fileEnabled(false),
			m_filePath(_T("")),
			m_file(NULL)
	//-------------------------------------------------------------------
	{
		g_pLogger = this;
	}


//-----------------------------------------------------------------------
	Logger::~Logger()
	//-------------------------------------------------------------------
	{
		m_log.clear();
		m_filePath.erase();

		if (m_file)
			fclose(m_file);
	}


//-----------------------------------------------------------------------
	void Logger::GetLog(StringList::iterator *pBegin, StringList::iterator *pEnd)
	//-------------------------------------------------------------------
	{
#ifdef _DEBUG
		int numLogSize = m_log.size();
#endif
		(*pBegin) = m_log.begin();
		(*pEnd) = m_log.end();
	}


//-----------------------------------------------------------------------
	void Logger::SetFile(bool enabled, LPCTSTR filename)
	//-------------------------------------------------------------------
	{
		m_fileEnabled = enabled;

		if (m_fileEnabled)
		{
			m_filePath = filename;

			m_file = _tfopen(m_filePath.c_str(), _T("w+t"));
/*
			SYSTEMTIME time;
			GetLocalTime(&time);
			Logf("Keays Simulation & Rendering API Log");			
			Logf("Generated at %02hu:%02hu:%02hu on %02hu/%02hu/%04hu",
				 time.wHour, time.wMinute, time.wSecond,
				 time.wDay, time.wMonth, time.wYear);
			Logf("------------------------------------\n");
*/
			fclose(m_file);
			m_file = _tfopen(m_filePath.c_str(), _T("a+t"));
		}
	}

// EOF