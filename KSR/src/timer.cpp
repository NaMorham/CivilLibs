#include "../include/ksr.h"

#include "../include/leakwatcher.h"

#ifdef _DO_MEMORY_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace KSR;

//-----------------------------------------------------------------------
	Timer::Timer()
	//-------------------------------------------------------------------
	{
		AddUsedMemory(sizeof(Timer), "Timer::Timer()");

		INT64 timerFrequency;
		if (QueryPerformanceFrequency((LARGE_INTEGER*)&timerFrequency))
		{
			m_useHFCounter = true;
			m_timeScale = 1.0f / timerFrequency;
		}
		else
		{
			m_timeScale = 1.0f / 1000;
			m_useHFCounter = false;
		}

		Stop();
	}


//-----------------------------------------------------------------------
	void Timer::Start()
	//-------------------------------------------------------------------
	{
		if (m_useHFCounter)
		{
			INT64 newTime;
			QueryPerformanceCounter((LARGE_INTEGER *)&newTime);
			m_currentTime = (DWORD)newTime;
		}
		else
		{
			m_currentTime = timeGetTime();
		}

		m_thisFrameTime = m_currentTime;
		m_lastSecond = m_currentTime;
	}


//-----------------------------------------------------------------------
	void Timer::Update()
	//-------------------------------------------------------------------
	{
		if (m_currentTime == 0)
			return;

		if (m_useHFCounter)
		{
			INT64 newTime;
			QueryPerformanceCounter((LARGE_INTEGER *)&newTime);
			m_currentTime = (DWORD)newTime;
		}
		else
		{
			m_currentTime = timeGetTime();
		}

		m_deltaTime = (m_currentTime - m_thisFrameTime) * m_timeScale;
		m_thisFrameTime = m_currentTime;

		m_fpsCurrent++;

		if ((m_currentTime - m_lastSecond) * m_timeScale > 1.0f)
		{
			m_lastSecond = m_currentTime;
			m_fpsLast = m_fpsCurrent;
			m_fpsCurrent = 0;
		}
	}


//-----------------------------------------------------------------------
	void Timer::Stop()
	//-------------------------------------------------------------------
	{
		m_currentTime = m_thisFrameTime = m_fpsCurrent = m_fpsLast = m_lastSecond = 0;
		m_deltaTime = 0.0f;
	}


//-----------------------------------------------------------------------
	void Timer::Step(float timeDelta)
	//-------------------------------------------------------------------
	{
		m_deltaTime = timeDelta;
	}


//-----------------------------------------------------------------------
	const float Timer::GetTime() const
	//-------------------------------------------------------------------
	{
		return (float)m_currentTime;
	}


//-----------------------------------------------------------------------
	const float Timer::GetDeltaTime() const
	//-------------------------------------------------------------------
	{
		return m_deltaTime;
	}


//-----------------------------------------------------------------------
	const float Timer::GetFramesPerSecond() const
	//-------------------------------------------------------------------
	{
		return (float)m_fpsLast;
	}

// EOF