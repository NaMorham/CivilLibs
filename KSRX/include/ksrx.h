/*! \file */
/*-----------------------------------------------------------------------
    ksrx.h

    Description: Keays Simulation & Rendering Extensions API external header
    Author:         Julian McKinlay
    ---------------------------------------------------------------------*/

#pragma once
#pragma warning (disable: 4786)

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "winmm.lib")

#ifdef DEBUG
    #ifdef MT_DLL
        #pragma comment(lib, "d3dx9dt.lib")
    #else
        #pragma comment(lib, "d3dx9d.lib")
    #endif
#else
    #pragma comment(lib, "d3dx9.lib")
#endif

namespace KSRX
{
#include "ksrxfile.h"
#include "ksrxmesh.h"
#include "ksrxmath.h"
#include "ksrxutil.h"
};

// EOF