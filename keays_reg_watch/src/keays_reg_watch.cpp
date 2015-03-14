// keays_reg_watch.cpp : Defines the entry point for the DLL application.
//

#include "..\include\stdafx.h"
#include "..\include\keays_reg_watch.h"

BOOL APIENTRY DllMain( HANDLE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}

/*
// This is an example of an exported variable
KEAYS_REG_WATCH_API int nKeays_reg_watch=0;

// This is an example of an exported function.
KEAYS_REG_WATCH_API int fnKeays_reg_watch(void)
{
	return 42;
}

// This is the constructor of a class that has been exported.
// see keays_reg_watch.h for the class definition
CKeays_reg_watch::CKeays_reg_watch()
{
	return;
}
//*/
