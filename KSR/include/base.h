/*-----------------------------------------------------------------------
	base.h

	Description: KSR internal header
	Author:		 Julian McKinlay
	---------------------------------------------------------------------*/

#pragma once
#pragma warning(disable: 4786)

#ifdef DEBUG
#define D3D_DEBUG_INFO
#endif

#include <d3d9.h>
#include <d3dx9.h>

#include <fstream.h>
#include <iostream.h>
#include <vfw.h>

#include <list>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <utility>

#include "types.h"
#include "ksrmath.h"

void Logf(LPCTSTR fmt, ...);
void Logf(ubyte level, LPCTSTR fmt, ...);
void DebugPrintf(LPCTSTR fmt, ...);

// EOF