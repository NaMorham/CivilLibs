#ifndef IMWATCHINGYOULEAK
#define IMWATCHINGYOULEAK

#include <crtdbg.h>

#ifdef _DEBUG	// we don't want this in release
//#define _DO_MEMORY_DEBUG
#endif

#ifdef _DO_MEMORY_DEBUG
inline void* operator new(size_t nSize, const char * lpszFileName, int nLine)
{
    return ::operator new(nSize, 1, lpszFileName, nLine);
}

inline void operator delete(void *v, const char * lpszFileName, int nLine)
{
    ::operator delete(v, 1, lpszFileName, nLine);
}

#define DEBUG_NEW new(THIS_FILE, __LINE__)

#define MALLOC_DBG(x) _malloc_dbg(x, 1, THIS_FILE, __LINE__)
#define malloc(x) MALLOC_DBG(x)

#endif // _DO_MEMORY_DEBUG

#endif // #include guard
