
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the KEAYS_REG_WATCH_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// KEAYS_REG_WATCH_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef KEAYS_REG_WATCH_EXPORTS
#define KEAYS_REG_WATCH_API __declspec(dllexport)
#else
#define KEAYS_REG_WATCH_API __declspec(dllimport)
#endif

#include "krwExceptions.h"
#include "RegChangeEvent.h"

/*
// This class is exported from the keays_reg_watch.dll
class KEAYS_REG_WATCH_API CKeays_reg_watch 
{
public:
	CKeays_reg_watch(void);
	// TODO: add your methods here.
};

extern KEAYS_REG_WATCH_API int nKeays_reg_watch;

KEAYS_REG_WATCH_API int fnKeays_reg_watch(void);
//*/
