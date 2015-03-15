/*! \file */
/*-----------------------------------------------------------------------
    timer.h

    Description: Timer structure declaration
    Author:         Julian McKinlay
    ---------------------------------------------------------------------*/

#pragma once

struct Timer : public MemObject
{
public:
    Timer();

    void Start();
    void Stop();
    void Update();
    void Step(float timeDelta);

    const float GetTime() const;
    const float GetDeltaTime() const;
    const float GetFramesPerSecond() const;

private:
    DWORD m_currentTime,
          m_thisFrameTime,
          m_fpsCurrent,
          m_fpsLast,
          m_lastSecond;

    float m_deltaTime,
          m_timeScale;

    bool m_useHFCounter;
};

typedef Timer *PTIMER;

// EOF