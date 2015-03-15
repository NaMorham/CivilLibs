/*! \file */
/*-----------------------------------------------------------------------
    ksr.h

    Description: Keays Simulation & Rendering API external header
    Author:         Julian McKinlay
    ---------------------------------------------------------------------*/

#pragma once
#pragma warning (disable: 4786)

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "winmm.lib")

#ifdef _DEBUG
    #pragma comment(lib, "d3dx9d.lib")
    #if 0
        #ifdef MT_DLL
            #pragma comment(lib, "d3dx9dt.lib")
        #else
            #pragma comment(lib, "d3dx9d.lib")
        #endif
    #endif
#else
    #pragma comment(lib, "d3dx9.lib")
#endif

#ifdef _DEBUG
#define D3D_DEBUG_INFO
#endif

#include <d3d9.h>
#include <d3dx9.h>

#ifdef _MSVC_6_
#include <fstream.h>
#include <iostream.h>
#else
#include <fstream>
#include <iostream>
#endif

#include <vfw.h>

#include <list>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <utility>
#include <algorithm>
#include <tchar.h>
#include <assert.h>

namespace KSR
{
#define VERSION 0x0101

//#define _SHOW_KSR_SIZES_ 1 // USED FOR DEBUGGING PURPOSES ONLY

#include "types.h"

//! \brief Logs a string
/*! \param fmt Format-control string
    \param [argument] Optional arguments */
void Logf(LPCTSTR fmt, ...);

//! \brief Logs a level-determined string
/*! \param level Log level of this string
    \param fmt Format-control string
    \param [argument] Optional arguments */
void Logf(ubyte level, LPCTSTR fmt, ...);

//! \brief Outputs a debug string
/*! \param fmt Format-control string
    \param [argument] Optional arguments */
void DebugPrintf(LPCTSTR fmt, ...);

#include "ksrmath.h"
#include "memobject.h"
#include "settings.h"
#include "camera.h"
#include "geometry.h"
#include "effect.h"
#include "visual.h"
#include "resource.h"
#include "entity.h"
#include "light.h"
#include "font.h"
#include "swapchain.h"
#include "viewport.h"
// #include "patch.h"
#include "scene.h"
#include "renderer.h"
#include "logger.h"
#include "timer.h"
#include "serialiser.h"
#include "interface.h"

/*
    \brief Typedef progress function for use in function that may take to long to process.
    Takes:
        - a float as the percentage of total loading
        - a LPCTSTR (const char/w_char_t *) as a message
        - a void * for progress payload to be passed to the callback function
 */
typedef unsigned int (*pProgressUpdateFn)(const float, LPCTSTR, void *);

/*
#ifdef _DEBUG
   #pragma comment(lib, "MyLib_d")
#elseif
   #pragma comment(lib, "MyLib")
#endif
*/

    //! \brief Creates an Interface
    /*! returns S_OK on success and E_FAIL on failure.
    \param hWnd Handle to the top level window of the application.
    \param logFilename [in] Pointer to a string containing the name of the log file to log to.
    \param maxLogSize Maximum number of entries in the log before it is front truncated.
    \param pCoreSettings [in] Pointer to a CoreSettings structure containing creation parameters.
    \param pKSR [out] Address of a pointer to a Interface structure, representing the created interface. */
    HRESULT CreateInterface(HWND hWnd, LPCTSTR logFilename, int maxLogSize,
                            PCORESETTINGS pCoreSettings, PINTERFACE *pKSR);

    //! \brief Destroys a KSRInterface
    /*! returns S_OK on success and E_FAIL on failure.
    \param pKSR [in] Pointer to an Interface structure, representing the interface to be destroyed. */
    HRESULT DestroyInterface(PINTERFACE pKSR);
};

// EOF