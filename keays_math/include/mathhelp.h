/*! 
	\file mathhelp.h
	\brief	General use maths routines.
	General maths functions. Part of the keays::math namespace.
	Retarded name is to deal with issues including math.h

	\date May 2004.
	\author Andrew Hickey, Keays Software.
	\author Karl Janke, Keays Software.
 */

#pragma once

#include <keays_types.h>	// keays types includes
#include <math.h>				// system math includes (sin/cos/abs etc)
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
#endif


namespace keays
{
namespace math
{

// included for compatibility with Visual Studio 2005
#ifndef SnPrintf
	#if _MSC_VER < 1400
		#define SnPrintf _sntprintf
	#else
		#define SnPrintf _sntprintf_s
	#endif
#endif

#ifndef VsnPrintf
	#if _MSC_VER < 1400
		#define VsnPrintf _vsntprintf
	#else
		#define VsnPrintf _vsntprintf_s
	#endif
#endif

#ifndef FPrintf
	#if	_MSC_VER <1400
		#define FPrintf _ftprintf
	#else
		#define FPrintf _ftprintf_s
	#endif
#endif

inline FILE *FileOpen(LPCTSTR filename, LPCTSTR mode)
{
#if _MSC_VER < 1400
	return _tfopen(filename, mode);
#else
	FILE *pFile = NULL;
	int errNo = _tfopen_s(&pFile, filename, mode);
	return errNo == 0 ? NULL : pFile;
#endif
}

inline bool SplitPath(LPCTSTR path, LPTSTR drive, LPTSTR dir, LPTSTR name, LPTSTR extension)
{
#if _MSC_VER < 1400
	_tsplitpath(path, drive, dir, name, extension);
	return true;
#else
	return _tsplitpath_s(path, drive, _MAX_DRIVE, dir, _MAX_DIR, name, _MAX_FNAME, extension, _MAX_EXT) == 0;
#endif
}

inline bool MakePath(LPTSTR path, LPCTSTR drive, LPCTSTR dir, LPCTSTR name, LPCTSTR extension)
{
#if _MSC_VER < 1400
	_tmakepath(path, drive, dir, name, extension);
	return true;
#else
	return _tmakepath_s(path, _MAX_PATH, drive, dir, name, extension) == 0;
#endif
}

//-----------------------------------------------------------------------------
/*!
	\name Value limiting functions
	@{
 */
//-----------------------------------------------------------------------------
/*! 
	\name Min limiting functions
	@{
 */

/*!
	\brief Limit a value to a specified minimum.
	\param   value [In]  - a constant of type T specifying the value to test.
	\param minimum [In]  - a constant of type T specifying the minimum value to test against.

	\return value if it is greater than the specified minimum, otherwise return the minimum.
 */
template<class T>
inline const T LimitToMin(const T &value, const T &minimum)
{ return (value > minimum ? value : minimum); }

/*!
	\brief Return the minimum of 2 values
	\param value1 [In]  - a constant value of type T to test.
	\param value2 [In]  - a constant value of type T to test against.
	\return the minimum of the 2 values.
 */
template<class T>
inline const T Min(const T &value1, const T &value2)
{ return (value1 > value2 ? value2 : value1); }
//! @}

//-----------------------------------------------------------------------------
/*! 
	\name Max limiting functions
	@{
 */

/*!
	\brief Limit a value to a specified maximum.
	\param   value [In]  - a constant of type T specifying the value to test.
	\param maximum [In]  - a constant of type T specifying the maximum value to test against.

	\return value if it is greater than the specified maximum, otherwise return the maximum.
 */
template<class T>
inline const T LimitToMax(const T &value, const T &maximum)
{ return (value < maximum ? value : maximum); }

/*!
	\brief Return the maximum of 2 values
	\param value1 [In]  - a constant value of type T to test.
	\param value2 [In]  - a constant value of type T to test against.
	\return the maximum of the 2 values.
 */
template<class T>
inline const T Max(const T &value1, const T &value2)
{ return (value1 > value2 ? value1 : value2); }
//! @}

//-----------------------------------------------------------------------------
/*! 
	\name Range limiting functions
	@{
 */
/*!
	\brief Limit a value to a specified range.
	\param   value [In]  - a constant of type T specifying the value to test.
	\param minimum [In]  - a constant of type T specifying the minimum value to test against.
	\param maximum [In]  - a constant of type T specifying the maximum value to test against.

	\return value if it is in the range specified by minimum and maximum, otherwise return the limit.
 */
template<class T>
inline const T Limit(const T &value, const T &minimum, const T &maximum)
{ return Min(Max(value, minimum), maximum); }

/*!
	\brief Limit the precision on a value.
	Removes extraneous precision data from a value.
	
	\param       val [In]  - a constant double specifying the value to limit precision on.
	\param tolerance [In]  - a constant double specifying the precision tolerance to use.
	\return a constant double with the limited precision value.
 */
inline const double LimitPrecision(const double &val, const double &tolerance = keays::types::Float::TOLERANCE)
{
	double invTol = 1/tolerance;
	double result;
	int temp;

	temp = int(val * invTol);
	//result = double(temp) * tolerance;
	result = (temp * tolerance);
	return result;
}

/*!
	\brief Limit the precision on a value based on the number of decimal places.
	Removes extraneous precision data from a value, based on base 10.
	
	\param       val [In]  - a constant double specifying the value to limit precision on.
	\param numPlaces [In]  - a constant int specifying the numer of decimal places to keep.
	\return a constant double with the limited precision value.
 */
inline const double LimitVal(const double &val, const int numPlaces = 3)
{
	static char buf[64];
	SnPrintf(buf, 64, "%.*f", numPlaces, val);
	return atof(buf);
}

/*!
	\brief Limit the precision of a 2D point based on the number of decimal places to keep.
	Removes extraneous precision data from a point.
	
	\param       pos [In]  - a constant reference to a VectorD2 specifying the position to limit precision on.
	\param numPlaces [In]  - a constant int specifying the numer of decimal places to keep.
	\return a constant VectorD2 with the limited precision point.
 */
inline const keays::types::VectorD2 LimitVal(const keays::types::VectorD2 &pos, const int numPlaces = 3)
{
	keays::types::VectorD2 limitPos;
	limitPos.x = LimitVal(pos.x, numPlaces);
	limitPos.y = LimitVal(pos.y, numPlaces);
	return limitPos;
}

/*!
	\brief Limit the precision of a 3D point based on the number of decimal places to keep.
	Removes extraneous precision data from a point.
	
	\param       pos [In]  - a constant reference to a VectorD3 specifying the position to limit precision on.
	\param numPlaces [In]  - a constant int specifying the numer of decimal places to keep.
	\return a constant VectorD3 with the limited precision point.
 */
inline const keays::types::VectorD3 LimitVal(const keays::types::VectorD3 &pos, const int numPlaces = 3)
{
	keays::types::VectorD3 limitPos;
	limitPos.x = LimitVal(pos.x, numPlaces);
	limitPos.y = LimitVal(pos.y, numPlaces);
	limitPos.z = LimitVal(pos.z, numPlaces);
	return limitPos;
}
//! @}
//! @} - end group - Value Limiting Functions

/*!
	\brief Return the absolute value.
	\param val [In]  - a constant value to return the absolute of.
	\return the absolute value of val.
 */
template<class T>
inline const T abs(const T &val)
{ return (val < 0 ? -val : val); }

/*!
	\brief Find the number of decimal place given a required precision

	\param precision [In]  - a constant double representing the required precision to test.

	\return An integer representing the number of decimal places.
 */
inline const int FindNumDecimalPlaces(const double &precision)
{
	double dblDecOnly = precision - (int)precision;
	double invDecOnly = 1.0/dblDecOnly;
	int result = 0;
	int testVal = (int)invDecOnly;

	while (testVal)
	{
		++result;
		testVal /= 10;
	}

	return result;
}

/*!
	\brief Convert and round a double to an integer.

	\param val [In] - a constant double representing the value to round.

	\return An integer representing the rounded value.
 */
inline const int DToI(const double &val)
{
	double fraction = val - double(int(val));
	if (fraction >= 0.5)
	{
		return int(ceil(val));
	} else
	{
		return int(floor(val));
	}
}

/*!
	\brief Convert and round a float to an integer.

	\param val [In] - a constant float representing the value to round.

	\return An integer representing the rounded value.
 */
inline const int FToI(const float &val)
{
	float fraction = val - float(int(val));
	if (fraction >= 0.5)
	{
		return int(ceil(val));
	} else
	{
		return int(floor(val));
	}
}
} // namespace math
} // namespace keays

