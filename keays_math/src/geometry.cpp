/*
 * Filename: geometry.cpp
 * Date: May 2004
 * Authors: Andrew Hickey, Keays Software
 *			Karl Janke, Keays Software
 *
 * Contains implementations of the functions in the geomery.h file.
 *
 * Part of the keays::maths namespace
 */

#include <assert.h>

#include "..\include\geometry.h"
#include <float.h>
#ifdef _DEBUG
//#include <string>
#include <cstdio>
#include <cstdarg>
//#include <varargs.h> // this is needed for non Unix V compatibility
#endif

#pragma warning(disable : 4786) // ignore the long name warning associated with stl stuff

#include <LeakWatcher.h>

#ifdef _DO_MEMORY_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;		// to get access to std:: stuff
// Functions local to this file
// utility functions ONLY in this file
//--------------------------------------------------------------
int CompareAngCCW(const void *ang1, const void *ang2)
{
	double a, b;
	a = *((const double *)ang1);
	b = *((const double *)ang2);
	return (int)((a - b)*100000.0);
}

//--------------------------------------------------------------
int CompareAngCW(const void *ang1, const void *ang2)
{
	double a, b;
	a = *((const double *)ang1);
	b = *((const double *)ang2);
	return (int)((b - a)*100000.0);
}

namespace keays
{
namespace math
{
using namespace keays::types;

// Constant definitions
const double KM_PI      = 3.1415926535897932384626433832795;
const double KM_2PI		= (2 * KM_PI);
const double KM_PI_ON2	= (KM_PI * 0.5);
const double KM_3PI_ON2	= (3.0 * KM_PI_ON2);

const double KM_PI_ON3	= (KM_PI / 3.0);
const double KM_PI_ON4	= (KM_PI * 0.25);
const double KM_PI_ON6	= (KM_PI / 6.0);
const double KM_PI_ON8	= (KM_PI * 0.125);

const double ROOT_2		= 1.4142135623730950488016887242097;
const double ROOT_2_ON2	= (ROOT_2/2);


// Debug Logging Functions
#ifdef _DEBUG
FILE **g_pLogFile = NULL;
void KEAYS_MATH_EXPORTS_API AttachLogFile(FILE **pLogFile)
{
	if (pLogFile && *pLogFile)
	{
		FPrintf(*pLogFile, "--- Attaching keays::math to log file ---\n");
		fflush(*pLogFile);
	}
	g_pLogFile = pLogFile;
}
void KEAYS_MATH_EXPORTS_API DetachLogFile()
{
	if (g_pLogFile && *g_pLogFile)
	{
		FPrintf(*g_pLogFile, "--- Detaching keays::math from log file ---\n");
		fflush(*g_pLogFile);
	}
	g_pLogFile = NULL;
}
void WriteDebugLog(LPCTSTR fmt, ...)
{
	if (!g_pLogFile)
		return;
	if (!(*g_pLogFile))
		return;

	static TCHAR buf[1024];
	buf[1023] = 0;
	va_list argsList;
	va_start(argsList, fmt);

	VsnPrintf(buf, 1023, fmt, argsList);
	va_end(argsList);

	FPrintf(*g_pLogFile, buf);
	fflush(*g_pLogFile);
}
#else
void KEAYS_MATH_EXPORTS_API AttachLogFile(FILE **pLogFile) {}
void KEAYS_MATH_EXPORTS_API DetachLogFile() {}
void WriteDebugLog(LPCTSTR fmt, ...) {}
#endif

const char *IRETURN_TEXT[] =
// Text interpretation of intersection return values
{
	"SUCCESS",						// S_INTERSECT
	"SAME POINT",					// E_SAME_POINT
	"NO INTERSECT",					// E_NO_INTERSECT
	"SAME CIRCLE",					// E_SAME_CIRCLE
	"CIRCLE1 INSIDE CIRCLE2",		// E_CIRC1_IN_CIRC2
	"CIRCLE2 INSIDE CIRCLE1",		// E_CIRC2_IN_CIRC1
	"NO LINE1",						// E_NO_LINE1
	"NO LINE2",						// E_NO_LINE2
	"LINES PARALLEL",				// E_LINES_PARALLEL
	"RECTANGLE1 INSIDE RECTANGLE2", // E_RECT1_INSIDE
	"RECTANGLE2 INSIDE RECTANGLE1", // E_RECT2_INSIDE
	"RECTANGLE1 SAME AS RECTANGLE2",// E_SAME_RECT
	"LINES ARE IN LINE",			// ES_LINES_IN_LINE
	"SEGMENTS DO NOT INTERSECT",	// E_NO_SEGMENT_INTERSECT
	"FAILURE - OTHER",				// E_FAIL_OTHER
	"POINT 1 OF LINE 1 TOUCHES",	// SS_L1P1_CONTACT
	"POINT 2 OF LINE 1 TOUCHES",	// SS_L1P2_CONTACT
	"POINT 1 OF LINE 2 TOUCHES",	// SS_L2P1_CONTACT
	"POINT 2 OF LINE 2 TOUCHES" 	// SS_L2P2_CONTACT
};

//-----------------------------------------------------------------------------
// keays math types
// tGradeSegment struct
tGradeSegment::tGradeSegment(const double &dist /*= 0*/, const double &wd /*= 0*/, const double &gr /*= 0*/)
{
	m_distance = dist;
	m_width = wd;
	m_grade = gr;
}

tGradeSegment::tGradeSegment(const tGradeSegment &orig)
{
	*this = orig;
}

const tGradeSegment &tGradeSegment::operator=(const tGradeSegment &rhs)
{
	m_distance = rhs.m_distance;
	m_width = rhs.m_width;
	m_grade = rhs.m_grade;
	return *this;
}

const bool tGradeSegment::operator<(const tGradeSegment &rhs) const
{
	return m_distance < rhs.m_distance;
}

const bool tGradeSegment::operator<=(const tGradeSegment &rhs) const
{
	return m_distance <= rhs.m_distance;
}

const bool tGradeSegment::operator>(const tGradeSegment &rhs) const
{
	return m_distance > rhs.m_distance;
}

const bool tGradeSegment::operator>=(const tGradeSegment &rhs) const
{
	return m_distance >= rhs.m_distance;
}

const bool tGradeSegment::operator==(const tGradeSegment &rhs) const
{
	return m_distance == rhs.m_distance;
}

const bool tGradeSegment::operator!=(const tGradeSegment &rhs) const
{
	return m_distance != rhs.m_distance;
}

const bool tGradeSegment::DeltaH(const double &distToPoint, double *deltaHeight) const
{
	if (deltaHeight == NULL)
		return false;	// this is bad

	double segDist = distToPoint - m_distance;
	if ((segDist <= 0.0) || (segDist > m_width))
		return false;	// its outside our segment

	(*deltaHeight) = segDist * m_grade;
	return true;
}

//-----------------------------------------------------------------------------
// tGradeSegmentList struct
tGradeSegmentList::tGradeSegmentList()
{
	m_pSegments = new std::list<tGradeSegment>;
	m_pSegments->clear();
}

tGradeSegmentList::tGradeSegmentList(const tGradeSegmentList &orig)
{
	m_pSegments = new std::list<tGradeSegment>;
	m_pSegments->clear();
	*this = orig;
}

tGradeSegmentList::~tGradeSegmentList()
{
	m_pSegments->clear();
	delete m_pSegments;
	m_pSegments = NULL;
}

const bool tGradeSegmentList::AddSegment(const tGradeSegment &newSegment, std::list<tGradeSegment>::iterator *it)
{
	if (newSegment.m_width <= 0.0)
		return false;

	std::list<tGradeSegment>::iterator newSegIt;
	newSegIt = m_pSegments->insert(m_pSegments->end(), newSegment);
	m_pSegments->sort();
	if (it)
		*it = newSegIt;
	return true;
}

const size_t tGradeSegmentList::Size() const
{
	return m_pSegments->size();
}

void tGradeSegmentList::Clear()
{
	m_pSegments->clear();
}

const double tGradeSegmentList::DeltaH(const double &distToPoint) const
{
	// assumes that the list is sorted (which it should be) - TODO make this a class so it has to be?
	double deltaHeight = 0.0, temp;
	std::list<tGradeSegment>::const_iterator it;
	for (it = m_pSegments->begin(); it != m_pSegments->end(); it++)
	{
		if ((*it).DeltaH(distToPoint, &temp))
		{
			deltaHeight += temp;
		}
	}
	return deltaHeight;
}

const tGradeSegment &tGradeSegmentList::Last() const
{
	std::list<tGradeSegment>::const_iterator last;
	last = m_pSegments->end();
	last--;
	return (*last);
}

//--------------------------------------------------------------
// Angle Conversion Functions
// Get the degrees, minutes and seconds from a decimal angle
KEAYS_MATH_EXPORTS_API void
GetDegreesMinutesSeconds(const double &angle, int &degrees, int &minutes, double &seconds,
						  const double &tolerance /*= keays::types::Float::TOLERANCE*/)
{
	double ang = ReduceAngleDegrees(LimitPrecision(angle, tolerance));

	degrees = (int)ang;
	ang -= degrees;
	ang *= 60;

	minutes = (int)ang;
	ang -= minutes;
	ang *= 60;

	seconds = ang;
}

//--------------------------------------------------------------
// Angle Conversion Functions
// Get the degrees, minutes and seconds from a decimal angle
KEAYS_MATH_EXPORTS_API void
GetDegreesMinutesSeconds(const double &angle, int &degrees, int &minutes, int &seconds,
						  const double &tolerance /*= keays::types::Float::TOLERANCE*/)
{
	double ang = ReduceAngleDegrees(LimitPrecision(angle, tolerance));

	degrees = (int)ang;
	ang -= degrees;
	ang *= 60;

	minutes = (int)ang;
	ang -= minutes;
	ang *= 60;

	seconds = DToI(ang);

	if (seconds >= 60)
	{
		seconds = 0;
		minutes++;

		if (minutes >= 60)
		{
			minutes = 0;
			degrees++;

			if (degrees >= 360)
				degrees -= 360;
		}
	}
}

//--------------------------------------------------------------
// Get the decimal degrees of an angle from degrees, minutes and seconds.
KEAYS_MATH_EXPORTS_API const double &
GetDecimalDegrees(const int degrees, const int minutes, const double &seconds, double &angle)
{
	return (angle = (double)degrees + (double)minutes / 60 + seconds / 60);
}

//--------------------------------------------------------------
// Angle Comparison Functions
//--------------------------------------------------------------
KEAYS_MATH_EXPORTS_API const eAngleComparisons
AngleIsClockwiseEx(const double &refAngle, const double &testAngle, const double &tolerance /*= Float::TOLERANCE*/,
				    double *diffAngle /*= NULL*/)
{
	double rAng = GetAbsoluteAngle(refAngle);
	double tAng = GetAbsoluteAngle(testAngle);
	double dAng;

	dAng = GetAbsoluteAngle(tAng - rAng);
	if (diffAngle)
		*diffAngle = dAng;

	if (Float::EqualTo(tAng, rAng))
	{
		return ANGLE_EQUAL;
	}

	if (Float::EqualTo(dAng, KM_PI, tolerance))
	{
		return ANGLE_OPPOSITE;
	} else if (Float::Greater(dAng, KM_PI, tolerance))
	{
		return ANGLE_CW;
	} else
	{
		return ANGLE_CCW;
	}
}


//--------------------------------------------------------------
// Generation Functions
//--------------------------------------------------------------
KEAYS_MATH_EXPORTS_API bool
GenArcPointsRad(const VectorD3 &center, const double &radius, const double &startAngleRad,
				 const double &endAngleRad, Polyline3D &result, const double &intervalRadians,
				 const bool clearResult, const bool doEndPoint, eAngleDirections direction,
				 double *importantAnglesList /*= NULL*/, const int numImportantAngles /*= 0*/, bool isCircle /*= false*/)
{
	int angPos = 0;

	if (clearResult)
		result.clear();

	if (fabs(intervalRadians) < 0.0001)
		return false;

	if ((startAngleRad == INVALID_ANGLE) || (endAngleRad == INVALID_ANGLE))
		return false;

	if (importantAnglesList && numImportantAngles)
		SortAngleList(importantAnglesList, numImportantAngles, startAngleRad, direction);

	if (radius == 0.0)
		return false;

	if (direction == CW)
		return GenArcPointsRadCWEx(center, radius, startAngleRad, intervalRadians, startAngleRad,
									endAngleRad, isCircle, true, doEndPoint, result,
									importantAnglesList, numImportantAngles);
	else
		return GenArcPointsRadCCWEx(center, radius, startAngleRad, intervalRadians, startAngleRad,
									endAngleRad, isCircle, true, doEndPoint, result,
									importantAnglesList, numImportantAngles);
}

#if 0
//--------------------------------------------------------------
const bool AddAngle(const VectorD3 center, const double &radius, std::vector<double> angles, const double &angle,
					 const double &nextAngle, const double &endAngle, const eAngleDirections dir, Polyline3D &pts)
{
	bool added = false;
	if (!angles.size())
		return false;

	pts.clear();
	std::vector<double>::iterator it;
	for (it = angles.begin(); it != angles.end(); it++)
	{
		if (dir == CW)
		{
			if ((((*it) <= angle) && ((*it) >= nextAngle)) && ((*it) >= endAngle))
			{	// its in the right range
				if	((((*it) < angle) && ((*it) > nextAngle)) && ((*it) > endAngle))
				{
					pts.push_back(GenPolarPosRad(center.XY(), radius, (*it)).VD3(center.z));
					added = true;
				}
			}
		} else if (dir == CCW) // if (dir == CW)
		{
			if ((((*it) >= angle) && ((*it) <= nextAngle)) && ((*it) <= endAngle))
			{
				// its in the right range
				if	((((*it) > angle) && ((*it) < nextAngle)) && ((*it) < endAngle))
				{
					pts.push_back(GenPolarPosRad(center.XY(), radius, (*it)).VD3(center.z));
					added = true;
				}
			}
		}
	} // for (it = angles.begin(); it != angles.end(); it++)

	return added;
}
#endif

//--------------------------------------------------------------
KEAYS_MATH_EXPORTS_API bool
GenArcPointsRadCWEx(const VectorD3 &center, const double &radius, const double &circleStartAngleRad,
					 const double &intervalRad, const double &arcStartAngleRad, const double &arcEndAngleRad,
					 const bool isCircle, const bool genStartPoint, const bool genEndPoint,
					 Polyline3D &pts, double *importantAngles /*= NULL*/,
					 const int numImportantAngles /*= 0*/)
{
	double cStartAngle;
	double aStartAngle;
	double aEndAngle;
	double *iAngs = NULL;
	int i, iAngIdx = 0;
	VectorD3 pt;

	// recalculate the angles so they are ALL less than the start angle
	cStartAngle = GetAbsoluteAngle(circleStartAngleRad);
	aStartAngle = GetAbsoluteAngle(arcStartAngleRad);
	aEndAngle = GetAbsoluteAngle(arcEndAngleRad);

	if (aStartAngle > cStartAngle)
	{
		aStartAngle -= KM_2PI;
		aEndAngle -= KM_2PI;
	}

	if (!isCircle)
	{
		if (aEndAngle > aStartAngle)
		{
			aEndAngle -= KM_2PI;
		}
	} else
	{
		aEndAngle = aStartAngle - KM_2PI;
	}

	// now adjust and sort the importantAngles if present
	if (importantAngles && (numImportantAngles >= 1))
	{
		iAngs = new double[numImportantAngles];
		for (i = 0; i < numImportantAngles; i++)
		{
			iAngs[i] = GetAbsoluteAngle(importantAngles[i]); // we need to do this anyway, so we might as well copy at the same time
			if (iAngs[i] > cStartAngle)
				iAngs[i] -= KM_2PI;
			if (iAngs[i] > aStartAngle)
				iAngs[i] -= KM_2PI;
		}
		qsort((void *)iAngs, (size_t)numImportantAngles, sizeof(double), &CompareAngCW);
	}

	// now we set up for the begining
	// find out the first interval before the start
	int numPreIntervals = int((cStartAngle - aStartAngle) / intervalRad);
	double startAng = cStartAngle - (numPreIntervals * intervalRad);
	double firstAng = startAng - intervalRad;
	double curAng, nextAng;

	if (genStartPoint || isCircle)
	{
		pt = GenPolarPosRad(center, radius, aStartAngle);
		pts.push_back(pt);
	}

	while ((iAngIdx < numImportantAngles) && (iAngs[iAngIdx] >= firstAng))
	{
		double angle = iAngs[iAngIdx++];
		if (angle == aStartAngle)
			continue;
		if (angle == firstAng)
			continue;
		pt = GenPolarPosRad(center, radius, angle);
		pts.push_back(pt);
	}

	curAng = firstAng;
	while (curAng > aEndAngle)
	{
		nextAng = curAng - intervalRad;

		pt = GenPolarPosRad(center, radius, curAng);
		pts.push_back(pt);

		while ((iAngIdx < numImportantAngles) && (iAngs[iAngIdx] > nextAng))
		{
			double angle = iAngs[iAngIdx++];
			//if (angle == curAng)
			if (fabs(angle - curAng) < 0.00001)
				continue;
			pt = GenPolarPosRad(center, radius, angle);
			pts.push_back(pt);
		}

		curAng -= intervalRad;
	}

	if (genEndPoint || isCircle)
	{
		pt = GenPolarPosRad(center, radius, aEndAngle);
		pts.push_back(pt);
	}

	if (iAngs)
		delete[] iAngs;

	return true;
}

//--------------------------------------------------------------
KEAYS_MATH_EXPORTS_API bool
GenArcPointsRadCCWEx(const VectorD3 &center, const double &radius, const double &circleStartAngleRad,
					  const double &intervalRad, const double &arcStartAngleRad, const double &arcEndAngleRad,
					  const bool isCircle, const bool genStartPoint, const bool genEndPoint,
					  Polyline3D &pts, double *importantAngles /*= NULL*/,
					  const int numImportantAngles /*= 0*/)
{

	double cStartAngle;
	double aStartAngle;
	double aEndAngle;
	double *iAngs = NULL;
	int i, iAngIdx = 0;
	VectorD3 pt;

	// recalculate the angles so they are ALL greater than the start angle
	cStartAngle = GetAbsoluteAngle(circleStartAngleRad);
	aStartAngle = GetAbsoluteAngle(arcStartAngleRad);
	aEndAngle = GetAbsoluteAngle(arcEndAngleRad);

	if (aStartAngle < cStartAngle)
	{
		aStartAngle += KM_2PI;
		aEndAngle += KM_2PI;
	}

	if (!isCircle)
	{
		if (aEndAngle <= aStartAngle)
		{
			aEndAngle += KM_2PI;
		}
	} else
	{
		aEndAngle = aStartAngle + KM_2PI;
	}

	// now adjust and sort the importantAngles if present
	if (importantAngles && (numImportantAngles >= 1))
	{
		iAngs = new double[numImportantAngles];
		for (i = 0; i < numImportantAngles; i++)
		{
			iAngs[i] = GetAbsoluteAngle(importantAngles[i]); // we need to do this anyway, so we might as well copy at the same time
			if (iAngs[i] < cStartAngle)
				iAngs[i] += KM_2PI;
			if (iAngs[i] < aStartAngle)
				iAngs[i] += KM_2PI;
		}
		qsort((void *)iAngs, (size_t)numImportantAngles, sizeof(double), &CompareAngCCW);
	}

	// now we set up for the begining
	// find out the first interval before the start
	int numPreIntervals = int((aStartAngle - cStartAngle) / intervalRad);
	double startAng = (numPreIntervals * intervalRad) + cStartAngle;
	double firstAng = startAng + intervalRad;
	double curAng, nextAng;

	if (genStartPoint || isCircle)
	{
		pt = GenPolarPosRad(center, radius, aStartAngle);
		pts.push_back(pt);
	}

	while ((iAngIdx < numImportantAngles) && (iAngs[iAngIdx] <= firstAng))
	{
		double angle = iAngs[iAngIdx++];
		if (angle == aStartAngle)
			continue;
		if (angle == firstAng)
			continue;
		pt = GenPolarPosRad(center, radius, angle);
		pts.push_back(pt);
	}

	curAng = firstAng;
	while (curAng < aEndAngle)
	{
		nextAng = curAng + intervalRad;

		pt = GenPolarPosRad(center, radius, curAng);
		pts.push_back(pt);

		while ((iAngIdx < numImportantAngles) && (iAngs[iAngIdx] < nextAng))
		{
			double angle = iAngs[iAngIdx++];
			if (fabs(angle - curAng) < 0.00001)
				continue;
			if (angle >= aEndAngle)
			{
				iAngIdx = numImportantAngles;
				continue;
			}
			pt = GenPolarPosRad(center, radius, angle);
			pts.push_back(pt);
		}

		curAng += intervalRad;
	}

	if (genEndPoint || isCircle)
	{
		pt = GenPolarPosRad(center, radius, aEndAngle);
		pts.push_back(pt);
	}
	if (iAngs)
		delete[] iAngs;

	return true;
}

//--------------------------------------------------------------
KEAYS_MATH_EXPORTS_API bool
GenArcPointsRadEx(const VectorD3 &center, const double &radius, const double &circleStartAngleRad,
				   const double &intervalRad, const double &arcStartAngleRad, const double &arcEndAngleRad,
				   const bool isCircle, const bool genStartPoint, const bool genEndPoint,
				   const eAngleDirections dir, Polyline3D &pts, double *importantAngles /*= NULL*/,
				   const int numImportantAngles /*= 0*/)
{
	double cStartAngle;

	cStartAngle = GetAbsoluteAngle(circleStartAngleRad);

	if (dir == CW)
	{
		return GenArcPointsRadCWEx(center, radius, cStartAngle, intervalRad, arcStartAngleRad, arcEndAngleRad, isCircle,
										  genStartPoint, genEndPoint, pts, importantAngles, numImportantAngles);
	} else if (dir == CCW)
	{
		return GenArcPointsRadCCWEx(center, radius, cStartAngle, intervalRad, arcStartAngleRad, arcEndAngleRad, isCircle,
											genStartPoint, genEndPoint, pts, importantAngles, numImportantAngles);
	} else
	{
		return false;
	}
}

//--------------------------------------------------------------
KEAYS_MATH_EXPORTS_API bool
GenPointsHeight(const VectorD3 &center, const double &fallAngle, const double &fallPct, Polyline3D &pts)
{

	// get angle to each pt ensure it is greater than the fall angle
	double ang, angDiff, dist, fall;
	size_t i, nPts;

	if (!pts.size())
		return false;

	nPts = pts.size();
	for (i = 0; i < nPts; i++)
	{
		ang = Direction(center, pts[i]);
		dist = Dist2D(center, pts[i]);

		if (ang < 0)
			ang += KM_2PI;

		angDiff = fallAngle - ang;

		if (angDiff < 0)
			angDiff += KM_2PI;

		fall = dist * cos(angDiff) * fallPct; // assume a sane value here, if worried check elsewhere

		pts[i].z = center.z + fall;
	}

	return true;
}

//--------------------------------------------------------------
KEAYS_MATH_EXPORTS_API bool
GenPointsHeight(const VectorD3 &center, const double &fallAngle, const double &fallPct, Polyline3D &pts,
				 const tGradeSegmentList *p_cGrades)
{
	// get angle to each pt ensure it is greater than the fall angle
	double ang, angDiff, dist, fall, deltaHeight;
	size_t i, nPts;
	bool haveCrossGrades = false;

	if (!pts.size())
		return false;

	if (p_cGrades && p_cGrades->Size())
		haveCrossGrades = true;

	nPts = pts.size();
	for (i = 0; i < nPts; i++)
	{
		deltaHeight = 0;
		ang = Direction(center, pts[i]);
		dist = Dist2D(center, pts[i]);

		if (ang < 0)
			ang += KM_2PI;

		angDiff = fallAngle - ang;

		if (angDiff < 0)
			angDiff += KM_2PI;

		fall = dist * cos(angDiff) * fallPct; // the fall due to the plane

		if (haveCrossGrades)
			deltaHeight = p_cGrades->DeltaH(dist);

		pts[i].z = center.z + fall + deltaHeight;
	} // for (i = 0; i < nPts; i++)

	return true;
}

//--------------------------------------------------------------
KEAYS_MATH_EXPORTS_API bool
GenPointHeight(const VectorD3 &center, const double &fallAngle, const double &fallPct, VectorD3 &pt)
{

	// get angle to each pt ensure it is greater than the fall angle
	double ang, angDiff, dist, fall;

	ang = Direction(center, pt);
	dist = Dist2D(center, pt);

	if (ang < 0) ang += KM_2PI;

	angDiff = fallAngle - ang;

	if (angDiff < 0) angDiff += KM_2PI;

	fall = dist * cos(angDiff) * fallPct;

	pt.z = center.z + fall;

	return true;
}

//--------------------------------------------------------------
KEAYS_MATH_EXPORTS_API bool
GenPointHeight(const VectorD3 &center, const double &fallAngle, const double &fallPct, VectorD3 &pt, tGradeSegmentList *p_cGrades)
{
	// get angle to each pt ensure it is greater than the fall angle
	double ang, angDiff, dist, fall, deltaHeight;
	bool haveCrossGrades = false;

	deltaHeight = 0.0;
	if (p_cGrades && p_cGrades->Size())
		haveCrossGrades = true;

	ang = Direction(center, pt);
	dist = Dist2D(center, pt);

	if (ang < 0) ang += KM_2PI;

	angDiff = fallAngle - ang;

	if (angDiff < 0) angDiff += KM_2PI;

	fall = dist * cos(angDiff) * fallPct;

	if (haveCrossGrades)
		deltaHeight = p_cGrades->DeltaH(dist);

	pt.z = center.z + fall + deltaHeight;

	return true;
}

//--------------------------------------------------------------
KEAYS_MATH_EXPORTS_API bool
GenDivisons(const Polyline3D &polyline, const double &interval, Polyline3D &generatedPts,
			 const bool bIncludeOriginal, const double &minDistanceFromOriginal /*= 0.01*/)
{
	Polyline3D::const_iterator startSeg, endSeg;
	VectorD3	curPoint, startPoint, endPoint;
	double		currentPos = 0.0;
	double		lineLength = Length3D(polyline, polyline.size(), true);
	double		segmentEnd;
	double		intervalChange = 0.0;
	int			pointCnt = 1;
	double		bearing, zenith;

	// sanity checks
	assert(interval > 0);
	assert(polyline.size() > 1);
	assert(lineLength > 0);

	startSeg = polyline.begin();
	endSeg = polyline.begin();
	endSeg++;

	if (bIncludeOriginal) generatedPts.push_back(*startSeg);

	while (endSeg != polyline.end())
	{
		startPoint = *startSeg;
		endPoint = *endSeg;

		segmentEnd = (Length3D(polyline, pointCnt, true) - interval);
		bearing = Direction(startPoint.XY(), endPoint.XY());
		zenith = Zenith(startPoint, endPoint);

		curPoint = startPoint;
		while (currentPos <= segmentEnd)
		{
			curPoint = GenPolarPosRad(curPoint, interval + intervalChange, bearing, zenith);

			if (curPoint != endPoint && curPoint != startPoint) generatedPts.push_back(curPoint);

			currentPos += interval;
			intervalChange = 0.0;
		} // while (currentPos <= segmentEnd)

		intervalChange = currentPos - (segmentEnd + interval);

		if (bIncludeOriginal) generatedPts.push_back(*endSeg);

		startSeg++;
		endSeg++;
		pointCnt++;
	} // while (endSeg != polyline.end())

	return true;
}

//--------------------------------------------------------------
KEAYS_MATH_EXPORTS_API const VectorD2
GenPolarPosRad(const VectorD2 &base, const double &distance, const double &radians)
{
	if (distance == 0)
		return base;

	double x, y;

	x = distance * cos(radians) + base.x;
	y = distance * sin(radians) + base.y;

	return VectorD2(x, y);
}

//--------------------------------------------------------------
KEAYS_MATH_EXPORTS_API const VectorD3
GenPolarPosRad(const VectorD3 &base, const double &distance, const double &radians)
{
	if (distance == 0)
		return base;

	return VectorD3(distance * cos(radians) + base.x, distance * sin(radians) + base.y, base.z);
}

//--------------------------------------------------------------
// Calculate a position in 3D space, given a bearing (radians), an zenith (radians) and a distance.
KEAYS_MATH_EXPORTS_API const VectorD3
GenPolarPosRad(const VectorD3 &base, const double &distance, const double &bearing, const double &zenith)
{
	VectorD3 result;
	if (distance == 0)
		return base;

	double xyLen;

	xyLen = distance * cos(zenith);
	result = GenPolarPosRad(base.XY(), xyLen, bearing).VD3(distance * sin(zenith) + base.z);

	return result;
}

//--------------------------------------------------------------
KEAYS_MATH_EXPORTS_API const VectorD2
GenPolarPos(const VectorD2 &base, const double &distance, const double &degrees)
{
	if (distance == 0)
		return base;

	return GenPolarPosRad(base, distance, DTR(degrees));
}

//--------------------------------------------------------------
KEAYS_MATH_EXPORTS_API const Polyline3D *
GeneratePerpendicularVectors(const Polyline3D &polyline, Polyline3D *pPerpVectors,
							  bool averageVectors /*= true*/, bool isClosed /*= false*/)
{
	if (!pPerpVectors)
		return NULL;

	// check for a valid polyline >= 2 points
	if (polyline.size() < 2)
		return NULL;

	// clear pPerpVector
	pPerpVectors->clear();

	// for each segment of a polyline
	std::vector<int> indices;
	int idx = 0;

	VectorD3 perpVector;

	Polyline3D tmpPerpVectors;
	Polyline3D::const_iterator itr, prevItr;

	itr = prevItr = polyline.begin();
	itr++;

	for (; itr != polyline.end(); itr++)
	{
		// get the point, and the prevPoint
		const VectorD3 &pt = (*itr);
		const VectorD3 &prevPt = (*prevItr);

		indices.push_back(idx);
		// if the point == prevPoint
		if (Float::EqualTo( Dist2D(prevPt, pt), 0.0))
		{
			// continue, increment point
			prevItr++;
			continue;
		}
		idx++;

		VectorD3 seg((pt.XY() - prevPt.XY()).VD3(0).GetNormalised());

		perpVector.Set(seg.y, -seg.x, 0);
		tmpPerpVectors.push_back(perpVector);

		// increment prevPoint
		prevItr++;
	} // end for
	indices.push_back(idx);

	Polyline3D perps;

	// now average them if required
	if (averageVectors)
	{
		VectorD3 startVect;
		vector<VectorD3>::iterator resItr, resPrevItr;
		resItr = resPrevItr = tmpPerpVectors.begin();
		resItr++;
		if (isClosed)
		{
			vector<VectorD3>::reverse_iterator lastItr = tmpPerpVectors.rbegin();
			startVect = (((*resPrevItr) + (*lastItr)) / 2.0).GetNormalised();
			perps.push_back(startVect);
		} else
		{
			startVect = resPrevItr->GetNormalised();
			perps.push_back(*resPrevItr);
		}

		for (; resItr != tmpPerpVectors.end(); resItr++, resPrevItr++)
		{
			VectorD3 tmp = ((*resItr) + (*resPrevItr)) / 2.0;
			perps.push_back(tmp.GetNormalised());
		}

		if (isClosed)
		{
			perps.push_back(startVect);
		} else
		{
			perps.push_back(*resPrevItr);
		}
	}

	// go through the index list and original polyline
	std::vector<int>::iterator idxItr;
	for (idxItr = indices.begin(); idxItr != indices.end(); idxItr++)
	{
		pPerpVectors->push_back(perps[*idxItr]);
	}

	return pPerpVectors;
}

//-----------------------------------------------------------------------------
KEAYS_MATH_EXPORTS_API bool
InsertPoint(Polyline3D *pPolyline, const double &chainage, size_t *pInsertIndex /*= NULL*/)
{
	if (!pPolyline)
		return false;

	if (pPolyline->size() < 2)
		return false;

	if (chainage < 0.0)
		return false;

	if (chainage == 0.0)
	{
		if (pInsertIndex)
			*pInsertIndex = 0;
		return true;
	}

	bool found = false;
	double curChainage = 0.0;
	double offsetChainage = 0.0;
	double segmentLength = 0.0;
	double grade = 0.0;
	VectorD2 normal;
	size_t insertIndex = 0;

	Polyline3D::iterator itr(pPolyline->begin()), nextItr(itr);
	nextItr++;

	for (; nextItr != pPolyline->end() && itr != pPolyline->end(); nextItr++, itr++, insertIndex++)
	{
		VectorD3 &pt = *itr;
		VectorD3 &nextPt = *nextItr;

		segmentLength = Dist2D(pt, nextPt);
		if (segmentLength <= 0.0)
			continue;

		normal = (nextPt - pt).XY().GetNormalised();
		double segEndChainage = curChainage + segmentLength;
		grade = (nextPt.z - pt.z) / segmentLength;

		if (curChainage < chainage && segEndChainage > chainage)
		{
			offsetChainage = chainage - curChainage;
			VectorD3 newPt(pt + (normal * offsetChainage));
			newPt.z += offsetChainage * grade;
			pPolyline->insert(nextItr, newPt);
			if (pInsertIndex)
				*pInsertIndex = insertIndex+1;
			return true;
		}
		else if (curChainage == chainage)
		{
			if (pInsertIndex)
				*pInsertIndex = insertIndex;
			return true;
		}

		curChainage = segEndChainage;
	}

	// if we get here it may be the last point
	if (curChainage == chainage)
	{
		if (pInsertIndex)
			*pInsertIndex = insertIndex;
		return true;
	}

	// TODO: determine if we should do this.
/*	offsetChainage = chainage - curChainage;
	VectorD3 newPt;	*/

	return false;
}

//-----------------------------------------------------------------------------
KEAYS_MATH_EXPORTS_API bool
InsertPoint(Polyline2D *pPolyline, const double &chainage, size_t *pInsertIndex /*= NULL*/)
{
	if (!pPolyline)
		return false;

	if (pPolyline->size() < 2)
		return false;

	if (chainage < 0.0)
		return false;

	if (chainage == 0.0)
	{
		if (pInsertIndex)
			*pInsertIndex = 0;
		return true;
	}

	bool found = false;
	double curChainage = 0.0;
	double offsetChainage = 0.0;
	double segmentLength = 0.0;
	double grade = 0.0;
	VectorD2 normal;
	size_t insertIndex = 0;

	Polyline2D::iterator itr(pPolyline->begin()), nextItr(itr);
	nextItr++;

	for (; nextItr != pPolyline->end() && itr != pPolyline->end(); nextItr++, itr++, insertIndex++)
	{
		VectorD2 &pt = *itr;
		VectorD2 &nextPt = *nextItr;

		segmentLength = Dist2D(pt, nextPt);
		if (segmentLength <= 0.0)
			continue;

		normal = (nextPt - pt).GetNormalised();
		double segEndChainage = curChainage + segmentLength;

		if (curChainage < chainage && segEndChainage > chainage)
		{
			offsetChainage = chainage - curChainage;
			VectorD2 newPt(pt + (normal * offsetChainage));
			pPolyline->insert(nextItr, newPt);
			if (pInsertIndex)
				*pInsertIndex = insertIndex+1;
			return true;
		}
		else if (curChainage == chainage)
		{
			if (pInsertIndex)
				*pInsertIndex = insertIndex;
			return true;
		}

		curChainage = segEndChainage;
	}

	// if we get here it may be the last point
	if (curChainage == chainage)
	{
		if (pInsertIndex)
			*pInsertIndex = insertIndex;
		return true;
	}

	// TODO: determine if we should do this.
/*	offsetChainage = chainage - curChainage;
	VectorD2 newPt;	*/

	return false;
}

//-----------------------------------------------------------------------------
KEAYS_MATH_EXPORTS_API bool
InsertPoints(const Polyline3D &srcPolyline, Polyline3D *pDestPolyline, const double &interval, int *pNumPtsAdded /*= NULL*/)
{
	if (!pDestPolyline)
		return false;
	if (interval <= 0.0)
		return false;
	if (srcPolyline.size() < 2)
		return false;

	int count = 0;
	double length = 0.0;
	Polyline3D result;

	Polyline3D::const_iterator itr(srcPolyline.begin()), nextItr(itr);
	nextItr++;

	for (; itr!= srcPolyline.end() && nextItr != srcPolyline.end(); itr++, nextItr++)
	{
		const VectorD3 &pt = *itr;
		const VectorD3 &nextPt = *nextItr;
		result.push_back(pt);

		double dist = Dist2D(pt, nextPt);
		if (dist <= 0.0)
			continue;

		VectorD3 deltaVec(nextPt - pt);
		VectorD2 normal = deltaVec.XY().GetNormalised();
		double grade = deltaVec.Grade();
		double endChainage = length + dist;
		double deltaLength = interval;

		while (deltaLength < dist)
		{
			VectorD3 newPt((pt.XY() + (normal * deltaLength)).VD3(pt.z + (grade * deltaLength)));
			result.push_back(newPt);
			count++;
			deltaLength += interval;
		}

		length = endChainage;
	}

	// add the last point as well
	result.push_back(*(srcPolyline.rbegin()));

	pDestPolyline->assign(result.begin(), result.end());

	if (pNumPtsAdded)
		*pNumPtsAdded = count;

	return true;
}

//-----------------------------------------------------------------------------
KEAYS_MATH_EXPORTS_API bool
InsertPoints(const Polyline2D &srcPolyline, Polyline2D *pDestPolyline, const double &interval, int *pNumPtsAdded /*= NULL*/)
{
	if (!pDestPolyline)
		return false;
	if (interval <= 0.0)
		return false;
	if (srcPolyline.size() < 2)
		return false;

	int count = 0;
	double length = 0.0;
	Polyline2D result;

	Polyline2D::const_iterator itr(srcPolyline.begin()), nextItr(itr);
	nextItr++;

	for (; itr!= srcPolyline.end() && nextItr != srcPolyline.end(); itr++, nextItr++)
	{
		const VectorD2 &pt = *itr;
		const VectorD2 &nextPt = *nextItr;
		result.push_back(pt);

		double dist = Dist2D(pt, nextPt);
		if (dist <= 0.0)
			continue;

		VectorD2 deltaVec(nextPt - pt);
		VectorD2 normal = deltaVec.GetNormalised();
		double endChainage = length + dist;
		double deltaLength = interval;

		while (deltaLength < dist)
		{
			VectorD2 newPt(pt + (normal * deltaLength));
			result.push_back(newPt);
			count++;
			deltaLength += interval;
		}

		length = endChainage;
	}

	// add the last point as well
	result.push_back(*(srcPolyline.rbegin()));

	pDestPolyline->assign(result.begin(), result.end());

	if (pNumPtsAdded)
		*pNumPtsAdded = count;

	return true;
}

//--------------------------------------------------------------
// Utility Functions
//--------------------------------------------------------------
KEAYS_MATH_EXPORTS_API const VectorD2
RotatePointXY(const VectorD2 &base, const VectorD2 &ref, const VectorD2 &origPt)
{
	VectorD2 rPt;

	double deltaX, deltaY, len;
	double cosTheta, sinTheta;

	if ((base.x == ref.x) && (base.y == ref.y))
		return VectorD2(-999, -999);

	deltaX = ref.x - base.x;
	deltaY = ref.y - base.y;
	len = sqrt((deltaX * deltaX) + (deltaY * deltaY));
	cosTheta = deltaX/len;
	sinTheta = deltaY/len;

	// TRANSPOSE origPt TO NEW AXIS (align base, ref to x axis)
	rPt.x = cosTheta * (origPt.x - base.x) + sinTheta * (origPt.y - base.y) + base.x;
	rPt.y = cosTheta * (origPt.y - base.y) - sinTheta * (origPt.x - base.x) + base.y;

	return rPt;
}

//--------------------------------------------------------------
KEAYS_MATH_EXPORTS_API const VectorD2
UnRotatePointXY(const VectorD2 &base, const VectorD2 &ref, const VectorD2 &origPt)
{
	VectorD2 rPt;

	double deltaX, deltaY, len;
	double cosTheta, sinTheta;

	if ((base.x == ref.x) && (base.y == ref.y))
		return VectorD2(-999, -999);

	deltaX = ref.x - base.x;
	deltaY = ref.y - base.y;
	len = sqrt(pow(deltaX, 2) + pow(deltaY, 2));
	cosTheta = deltaX/len;
	sinTheta = deltaY/len;

	// TRANSPOSE origPt TO NEW AXIS (align base, ref to x axis)

	rPt.x = cosTheta*(origPt.x-base.x) + sinTheta*(origPt.y-base.y);
	rPt.y = cosTheta*(origPt.y-base.y) - sinTheta*(origPt.x-base.x);

	return rPt;
}

//--------------------------------------------------------------
KEAYS_MATH_EXPORTS_API const eSideSelections
PointOnSide(const VectorD3 &start, const VectorD3 &end, const VectorD3 &pt, const double &tolerance /*= Float::TOLERANCE*/, double *pDistance /*= NULL*/)
{
	double testDist = GetPerpendicularDist(start.XY(), end.XY(), pt.XY());

	if (pDistance)
		*pDistance = testDist;

	if (0 == testDist)
		return SIDE_NONE;
	else if (0 < testDist)
		return SIDE_RIGHT;
	else
		return SIDE_LEFT;
}

//--------------------------------------------------------------
KEAYS_MATH_EXPORTS_API const eSideSelections
PointOnSide(const VectorD2 &start, const VectorD2 &end, const VectorD2 &pt, const double &tolerance /*= Float::TOLERANCE*/, double *pDistance /*= NULL*/)
{
	double testDist = GetPerpendicularDist(start, end, pt);

	if (pDistance)
		*pDistance = testDist;

	if (0 == testDist)
		return SIDE_NONE;
	else if (0 < testDist)
		return SIDE_RIGHT;
	else
		return SIDE_LEFT;
}

//--------------------------------------------------------------
// Calculate the 2D Direction between 2 points.
KEAYS_MATH_EXPORTS_API const double
Direction(const VectorD2 &start, const VectorD2 &end)
{
	double dx = end.x - start.x;
	double dy = end.y - start.y;

	if (dx == 0) // vertical
	{
		if (dy == 0)
			return 0.0;
		else
			return (dy < 0 ? 3*KM_PI_ON2 : KM_PI_ON2);
	} else
	{
		return atan(dy/dx) + (dx < 0 ? KM_PI : 0);
	}
}

//--------------------------------------------------------------
//	Calculate the 2D Direction (Bearing) between 2 3D points in the X/Y plane.
KEAYS_MATH_EXPORTS_API const double
Direction(const VectorD3 &start, const VectorD3 &end)
{
	double dx = end.x - start.x;
	double dy = end.y - start.y;

	if (dx == 0) // vertical
	{
		if (dy == 0)
			return 0.0;
		else
			return (dy < 0 ? 3*KM_PI_ON2 : KM_PI_ON2);
	} else
	{
		return atan(dy/dx) + (dx < 0 ? KM_PI : 0);
	}
}

//--------------------------------------------------------------
// Calculate the Sin of the Zenith between 2 points.
KEAYS_MATH_EXPORTS_API const double
ZenithSin(const VectorD3 &start, const VectorD3 &end)
{
	double len, ht;

	len = Distance(start, end);

	if (len == 0.0) // we use a normal == since it works if there is ANY distance
		return 0.0;

	ht = end.z - start.z;

	return (ht / len);
}

//--------------------------------------------------------------
// Calculate the Grade between 2 points.
KEAYS_MATH_EXPORTS_API const double
Grade(const VectorD3 &start, const VectorD3 &end)
{
	double len2D, height, grade;

	len2D = Dist2D(start, end);
	if (len2D == 0.0) // we use a == comparison since ANY distace works
		return 0.0;

	height = end.z - start.z;
	grade = height/len2D;

	return grade;
}

//--------------------------------------------------------------
// Calculate the 2D Length of a polyline.
KEAYS_MATH_EXPORTS_API const double
Length2D(const Polyline3D &pts, const size_t index, const bool forwards)
{
	double len = 0.0;
	size_t i, size;
	if ((pts.size() <= 1) || ((index < 1) && forwards) || ((index >= (int)pts.size()) && !forwards))
		return len;

	// we handle if the index is greater than the size of the vector, but we cannot handle negatives, and an index of 0 is stupid
	size = pts.size();
	if (forwards)
	{
		for (i = 1; i <= index && i < size; ++i)
			len += Dist2D(pts[i-1].XY(), pts[i].XY());
	} else
	{
		for (i = size-2; i >= 0 && i >= index; --i)
			len += Dist2D(pts[i+1].XY(), pts[i].XY());
	}
	return len;
}

//--------------------------------------------------------------
// Calculate the 3D Length of a polyline.
KEAYS_MATH_EXPORTS_API const double
Length3D(const Polyline3D &pts, const size_t index, const bool forwards)
{
	double len = 0.0;
	size_t i, size;
	if ((pts.size() <= 1) || ((index < 1) && forwards) || ((index >= pts.size()) && !forwards))
		return len;

	// we handle if the index is greater than the size of the vector, but we cannot handle negatives, and an index of 0 is stupid
	size = pts.size();
	if (forwards)
	{
		for (i = 1; i <= index && i < size; ++i)
			len += Distance(pts[i-1], pts[i]);
	} else
	{
		for (i = size-2; i >= 0 && i >= index; --i)
			len += Distance(pts[i+1], pts[i]);
	}
	return len;
}

//--------------------------------------------------------------
KEAYS_MATH_EXPORTS_API const double
Length2D(const Polyline2D &pts, const size_t index, const bool forwards)
{
	double len = 0.0;
	size_t i, size;
	if ((pts.size() <= 1) || ((index < 1) && forwards) || ((index >= pts.size()) && !forwards))
		return len;

	// we handle if the index is greater than the size of the vector, but we cannot handle negatives, and an index of 0 is stupid
	size = pts.size();
	if (forwards)
	{
		for (i = 1; i <= index && i < size; ++i)
			len += Dist2D(pts[i-1], pts[i]);
	} else
	{
		for (i = size-2; i >= 0 && i >= index; --i)
			len += Dist2D(pts[i+1], pts[i]);
	}
	return len;
}

//--------------------------------------------------------------
// Normalise a VectorD3
KEAYS_MATH_EXPORTS_API const VectorD3
NormaliseD(const VectorD3 vec)
{
	double mag = Magnitude(vec);

	if (0 == mag)
		return VectorD3(0, 0, 0);

	return VectorD3(vec.x/mag, vec.y/mag, vec.z/mag);
}

//--------------------------------------------------------------
// Normalise the Vector given by the two points specified
KEAYS_MATH_EXPORTS_API const VectorD3
NormaliseD(const VectorD3 base, const VectorD3 tip)
{
	VectorD3 vec((base.x - tip.x), (base.y - tip.y), (base.z - tip.z));
	double mag = Magnitude(vec);

	if (0 == mag)
		return VectorD3(0, 0, 0);

	return VectorD3(vec.x/mag, vec.y/mag, vec.z/mag);
}

//--------------------------------------------------------------
// Calculate the crossproduct of 2 VectorD3's
KEAYS_MATH_EXPORTS_API const VectorD3
Cross(const VectorD3 &A, const VectorD3 &B)
{
	VectorD3 result;
	result.x = (A.y * B.z) - (B.y * A.z);
	result.y = (A.z * B.x) - (B.z * A.x);
	result.z = (A.x * B.y) - (B.x * A.y);

	return result;
}

//--------------------------------------------------------------
// Calculate the Dotproduct of 2 VectorD2's
KEAYS_MATH_EXPORTS_API const double
Dot(const VectorD2 &A, const VectorD2 &B)
{
	return (A.x*B.x + A.y*B.y);
}

//--------------------------------------------------------------
// Calculate the Dotproduct of 2 VectorD3's
KEAYS_MATH_EXPORTS_API const double
Dot(const VectorD3 &A, const VectorD3 &B)
{
	return (A.x*B.x + A.y*B.y + A.z*B.z);
}

//--------------------------------------------------------------
KEAYS_MATH_EXPORTS_API bool
PointHeightOnPlane(const VectorD3 &normal, const VectorD3 &pointOnPlane, const VectorD2 &point, double &result)
{
	if (normal.z == 0.0)
		return false; // the plane is vertical

	result = (normal.x / normal.z) * (pointOnPlane.x - point.x);	// x component of resulting height
	result += (normal.y / normal.z) * (pointOnPlane.y - point.y);	// y component of resulting height
	result += pointOnPlane.z;

	return true;
}

//--------------------------------------------------------------
KEAYS_MATH_EXPORTS_API const double
GetPerpendicularDist(const VectorD2 &lnStart, const VectorD2 &lnEnd, const VectorD2 &pt)
{
	double dist, lnAng, lnPtAng;

	// get angles and distance
	lnAng = ReduceAngle(Direction(lnStart, lnEnd));
	lnPtAng = ReduceAngle(Direction(lnStart, pt));
	dist = Dist2D(lnStart, pt);

	// get the difference in the angles
	lnPtAng = lnAng - lnPtAng;

	return dist * sin(lnPtAng);
}

//--------------------------------------------------------------
// Calculate the perpendicular distance from an arbitary point to a line.
KEAYS_MATH_EXPORTS_API const int
GetPerpendicularIntersect(const VectorD2 &lnStart, const VectorD2 &lnEnd, const VectorD2 &pt, VectorD2 &result, double *pDistance)
{
	double dist, lnAng, lnPtAng;

	if (lnStart == lnEnd)
	{
		result = VectorD2(BAD_POINT);
		return E_NO_LINE1;
	}

	// get angles and distance
	lnAng = ReduceAngle(Direction(lnStart, lnEnd));
	lnPtAng = ReduceAngle(Direction(lnStart, pt));
	dist = Dist2D(lnStart, pt);

	// get the difference in the angles
	lnPtAng = lnAng - lnPtAng;

	dist = dist * cos(lnPtAng);
	result = GenPolarPosRad(lnStart, dist, lnAng);

	if (pDistance)
	{
		*pDistance = Dist2D(pt, result);
	}

	return S_INTERSECT;
}

KEAYS_MATH_EXPORTS_API const bool
GetPerpendicularIntersect(const Polyline2D &polyline, const VectorD2 &pt, VectorD2 &result, double *pDistance /*= NULL*/, double *pChainage /*= NULL*/)
{
	if (polyline.size() < 2)
		return false;
	// cycle throught the polyline, testing for a perpendicular point
	double chainage = 0.0,
		   minPerpDist = DBL_MAX,
		   perpDist = 0.0;
	Polyline2D::const_iterator     itr = polyline.begin(),
							   nextItr = itr;
	VectorD2 iPt;
	int numFound = 0;

	nextItr++;

	for (; nextItr != polyline.end(); itr++, nextItr++)
	{
		const VectorD2 &plinePt = *itr;
		const VectorD2 &nextPt = *nextItr;

		double dist = Dist2D(plinePt, nextPt);

		if (dist <= 0.0)
			continue;

		if (S_INTERSECT == GetPerpendicularIntersect(plinePt, nextPt, pt, iPt, &perpDist))
		{

			// we have a point, test if it is in the segment
			if (PointOnSegment(plinePt, nextPt, iPt))
			{
				// if so, test if it is closer
				if (perpDist < minPerpDist)
				{
					minPerpDist = perpDist;
					result = iPt;
					++numFound;
				}
			}
		}

		chainage += dist;
	}

	if (numFound >= 0)
	{
		if (pChainage)
			*pChainage = chainage;
		if (pDistance)
			*pDistance = minPerpDist;
		return true;
	}
	else
	{
		return false;
	}
}

KEAYS_MATH_EXPORTS_API const bool
GetPerpendicularIntersect(const Polyline3D &polyline, const VectorD2 &pt, VectorD3 &result, double *pDistance /*= NULL*/, double *pChainage /*= NULL*/)
{
	if (polyline.size() < 2)
		return false;
	// cycle throught the polyline, testing for a perpendicular point
	double chainage = 0.0,
		   minPerpDist = DBL_MAX,
		   perpDist = 0.0;
	Polyline3D::const_iterator     itr = polyline.begin(),
							   nextItr = itr;
	VectorD2 iPt;
	int numFound = 0;

	nextItr++;

	for (; nextItr != polyline.end(); itr++, nextItr++)
	{
		const VectorD3 &plinePt = *itr;
		const VectorD3 &nextPt = *nextItr;

		double dist = Dist2D(plinePt, nextPt);

		if (dist <= 0.0)
			continue;

		if (S_INTERSECT == GetPerpendicularIntersect(plinePt.XY(), nextPt.XY(), pt, iPt, &perpDist))
		{

			// we have a point, test if it is in the segment
			if (PointOnSegment(plinePt.XY(), nextPt.XY(), iPt))
			{
				// if so, test if it is closer
				if (perpDist < minPerpDist)
				{
					minPerpDist = perpDist;
					result = iPt.VD3(plinePt.z + (nextPt - plinePt).Grade() * dist);
					++numFound;
				}
			}
		}

		chainage += dist;
	}

	if (numFound >= 0)
	{
		if (pChainage)
			*pChainage = chainage;
		if (pDistance)
			*pDistance = minPerpDist;
		return true;
	}
	else
	{
		return false;
	}
}

//--------------------------------------------------------------
// Calculate the mid point on a line.
KEAYS_MATH_EXPORTS_API const int
GetMidPoint(const VectorD3 & lnStart, const VectorD3 & lnEnd, VectorD3 & mid)
{
	if (lnStart == lnEnd)
	{
		mid = BAD_POINT;
		return E_NO_LINE1;
	}

	mid.x = (lnStart.x + lnEnd.x);
	mid.y = (lnStart.y + lnEnd.y);
	mid.z = (lnStart.z + lnEnd.z);

	return S_INTERSECT;
}

//--------------------------------------------------------------
/*
	Calculate the area, it will be -ve if the polygon is wound CW. Returns the area in units
	squared.
	polygon [In]  - a constant reference to a Polyline2D points describing a 2D polygon.
	*/
KEAYS_MATH_EXPORTS_API const double
CalcPolygonArea(const Polyline2D &polygon, bool needsClose /*= false*/)
{
	int numPts, i;
	double area;

	if ((numPts = (int)polygon.size()) < 3)
	{
		// its not a polygon
		return 0.0;
	}

	area = 0.0;
	// go through from 0 to numPts-1
	for (i = 0; i < numPts-1; i++)
		area += ((polygon[i].x * polygon[i+1].y) - (polygon[i+1].x * polygon[i].y));

	if (needsClose)
		area += ((polygon[numPts-1].x * polygon[0].y) - (polygon[0].x * polygon[numPts-1].y));

	area /= 2.0;

	return area;
}

//--------------------------------------------------------------
KEAYS_MATH_EXPORTS_API const double
CalcPolygonArea(const std::list<VectorD2> &polygon, bool needsClose /*= false*/)
{
	double area;

	if (polygon.size() < 3)
	{
		// its not a polygon
		return 0.0;
	}

	std::list<VectorD2>::const_iterator itr, next;

	area = 0.0;
	// go through from 0 to numPts-1
	next = itr = polygon.begin();
	next++;
	for (; next != polygon.end(); itr++, next++)
		area += (((*itr).x * (*next).y) - ((*next).x * (*itr).y));

	itr = polygon.begin();
	next = polygon.end();
	next--;
	if (needsClose)
		area += (((*next).x * (*itr).y) - ((*itr).x * (*next).y));

	area /= 2.0;

	return area;
}

//--------------------------------------------------------------
/*!
	\overload
	\param polygon [In]  - a constant pointer to an array of VectorD2 points describing a 2D polygon.
 */
KEAYS_MATH_EXPORTS_API const double
CalcPolygonArea(const VectorD2* polygon, const int numPoints, bool needsClose /*= false*/)
{
	int i;
	double area;

	if (numPoints < 3)
	{
		// its not a triangle
		return 0.0;
	}

	area = 0.0;
	// go through from 0 to numPoints-1
	for (i = 0; i < numPoints-1; i++)
		area += ((polygon[i].x * polygon[i+1].y) - (polygon[i+1].x * polygon[i].y));

	if (needsClose)
		area += ((polygon[numPoints-1].x * polygon[0].y) - (polygon[0].x * polygon[numPoints-1].y));

	area /= 2.0;

	return area;
}

//--------------------------------------------------------------
/*!
	\brief Calculate the area of an arbitary triangle, it will be -ve if the polygon is wound CW.

	\param     triPt1 [In]  - a constant reference to a VectorD2 representing the first point of a CCW wound triangle.
	\param     triPt2 [In]  - a constant reference to a VectorD2 representing the second point of a CCW wound triangle.
	\param     triPt3 [In]  - a constant reference to a VectorD2 representing the third point of a CCW wound triangle.
	\return The area in units squared, the answer will be -ve if the polygon is wound
			in a clockwise direction.
 */
KEAYS_MATH_EXPORTS_API const double
CalcTriangleArea(const VectorD2 &triPt1, const VectorD2 &triPt2, const VectorD2 &triPt3)
{
	double area;

	area = ((triPt1.x * triPt2.y) - (triPt2.x * triPt1.y)) +
		    ((triPt2.x * triPt3.y) - (triPt3.x * triPt2.y)) +
			 ((triPt3.x * triPt1.y) - (triPt1.x * triPt3.y));
	area /= 2.0;

	return area;
}


//--------------------------------------------------------------
// Intersection Functions
//--------------------------------------------------------------
KEAYS_MATH_EXPORTS_API const int
CircleCircleIntersect(const VectorD2 &center1, const double &radius1, const VectorD2 &center2, const double &radius2,
					   const VectorD2 &refPoint, VectorD2 &result, VectorD2 *result2)
{
	double deltaX, deltaY, radiusRadiusDist, cosTheta, sinTheta, SX, SY, YSP;

	result = BAD_POINT;
	if (result2)
		*result2 = BAD_POINT;
	//
	// ROTATE TILL XC1,YC1 =0,0 AND  XC2,YC2 = RAD,0
	//
	deltaX = center2.x - center1.x;
	deltaY = center2.y - center1.y;
	radiusRadiusDist = sqrt((deltaX*deltaX) + (deltaY*deltaY));
	if (Float::Less(radiusRadiusDist, 0.0))
		return E_SAME_POINT;

	cosTheta = deltaX/radiusRadiusDist;
	sinTheta = deltaY/radiusRadiusDist;

	//
	// CHECK IF SOLUTION  1) CIRCLES DONOT TOUCH 2) INSIDE EACH OTHER
	//
	if (Float::Less((radius1+radius2), radiusRadiusDist))
		return E_NO_INTERSECT;

	if (Float::Less((radius1+radiusRadiusDist), radius2))
		return E_CIRC1_IN_CIRC2;

	if (Float::Less((radius2+radiusRadiusDist), radius1))
		return E_CIRC2_IN_CIRC1;

	//
	// NOW CALCULATE POINTS
	//
	SX = ((radius1*radius1) + (radiusRadiusDist*radiusRadiusDist) - (radius2*radius2))/(2*radiusRadiusDist);
	double dodgyNum = radius1-SX;
	SY = sqrt((radius1+SX)*(dodgyNum < 0 ? 0 : dodgyNum));

	//
	// ROTATE SEARCH POINT XP,YP TO NEW AXIS
	// IF YPT +VE TAKE SY + VE  IF YPT -VE TAKE SY -VE
	//
	YSP = cosTheta*(refPoint.y - center1.y) - sinTheta*(refPoint.x - center1.x);
	if (Float::Less(YSP, 0))
		SY = -SY;

	//
	// NOW TRANSLATE POSITION
	//
	result.x = center1.x + cosTheta*SX - sinTheta*SY;
	result.y = center1.y + cosTheta*SY + sinTheta*SX;
	if (result2)
	{
		result2->x = center1.x + cosTheta*SX + sinTheta*-SY;
		result2->y = center1.y - cosTheta*-SY + sinTheta*SX;
	}

	return S_INTERSECT;
}

//--------------------------------------------------------------
KEAYS_MATH_EXPORTS_API const int
LineCircleIntersect(const VectorD2 &pt1, const VectorD2 &pt2, const VectorD2 &center, const double &radius,
					 const VectorD2 &refPoint, VectorD2 &result, const double &lineOffset /*= 0.0*/, VectorD2 *result2 /*= NULL*/)
{
	//
	// LINE X1,Y1/ X2,Y2  TO INTERSECT XC,YC RADIUS R
	//
	// ROTATE TO pt1= 0,0 AND  pt2=radius,0
	//
	double deltaX, deltaY, len;
	double cosTheta, sinTheta;
	double rotXC, rotYC;
	double xIntercept, xsa, xsb;
	VectorD2 sol1, sol2;

	if ((pt1.x == pt2.x) && (pt1.y == pt2.y))
		return E_SAME_POINT;

	deltaX = pt2.x - pt1.x;
	deltaY = pt2.y - pt1.y;
	len = sqrt((deltaX * deltaX) + (deltaY * deltaY));
	cosTheta = deltaX/len;
	sinTheta = deltaY/len;

	//
	// TRANSPOSE center TO NEW AXIS (align pt1, pt2 to x axis)
	//

	rotXC = cosTheta*(center.x-pt1.x) + sinTheta*(center.y-pt1.y);
	rotYC = cosTheta*(center.y-pt1.y) - sinTheta*(center.x-pt1.x) + lineOffset;
	xIntercept = pow(radius, 2)-pow(rotYC,2);
	if (Float::Less(xIntercept, 0.0))
		return E_NO_INTERSECT;

	xIntercept = Max(xIntercept, 0.0);
	xIntercept = sqrt(xIntercept);
	xsa = rotXC - xIntercept;
	xsb = rotXC + xIntercept;

	//
	// TRANSLATE BACK TO NORMAL AXIS
	//
	sol1.x = pt1.x + cosTheta*xsa + sinTheta*lineOffset;
	sol1.y = pt1.y + sinTheta*xsa - cosTheta*lineOffset;
	sol2.x = pt1.x + cosTheta*xsb + sinTheta*lineOffset;
	sol2.y = pt1.y + sinTheta*xsb - cosTheta*lineOffset;

	if (Dist2D(sol1, refPoint) < Dist2D(sol2, refPoint))
	{
		result = sol1;
		if (result2)
			*result2 = sol2;
	} else
	{
		result = sol2;
		if (result2)
			*result2 = sol1;
	}

	return S_INTERSECT;
}

//--------------------------------------------------------------
KEAYS_MATH_EXPORTS_API const int
LineLineIntersect(const VectorD2 &l1p1, const VectorD2 &l1p2, const VectorD2 &l2p1, const VectorD2 &l2p2, VectorD2 &result)
{
	double dx1, dy1;
	double dx2, dy2;
	double denom, top, s;

	result = BAD_POINT;

/* // --------------
      DXL1  = XK - XJ
      DYL1  = YK - YJ
      DXL2  = XM - XL
      DYL2  = YM - YL
      if (DXEQY(DXL1,ZERO) .AND. DXEQY(DYL1,ZERO)) THEN
C
C length line 1 = 0  (invalid)
C
        IERR = 0
        RETURN
      ELSE if (DXEQY(DXL2,ZERO) .AND. DXEQY(DYL2,ZERO)) THEN
C
C length line 2 = 0  (invalid)
C
        IERR = 0
        RETURN
      END IF
 */
	if ((l1p1.x == l1p2.x) && (l1p1.y == l1p2.y)) // no line
		return E_NO_LINE1;
	if ((l2p1.x == l2p2.x) && (l2p1.y == l2p2.y)) // no line
		return E_NO_LINE2;

	dx1 = l1p2.x - l1p1.x;
	dy1 = l1p2.y - l1p1.y;

	dx2 = l2p2.x - l2p1.x;
	dy2 = l2p2.y - l2p1.y;

/* // --------------
      DENOM = DYL1*DXL2 - DXL1*DYL2
      if (DXEQY(DENOM,ZERO)) THEN
C
C lines are parrallel
C
        IERR = 0
        RETURN
      END IF
*/
	denom = dy1*dx2 - dx1*dy2;
	if (Float::EqualTo(denom, 0)) // denom == 0)
	{
		/*
		double bl1 = Line(l1p1, l1p2).Bearing();
		double bl2 = Line(l1p1, l2p2).Bearing();
		if ((bl1 == bl2) || (bl1 == (bl2 + KM_PI)))
		{
			// they're parallel AND in line, so there is an intersect
			int min = 0;
			double	d	 = Dist2D(l1p1, l2p2),
					minD = Dist2D(l1p1, l2p1);
			if (d < minD)
			{
				minD = d;
				min = 1;
			}
			d = Dist2D(l1p2, l2p1);
			if (d < minD)
			{
				minD = d;
				min = 2;
			}
			d = Dist2D(l1p2, l2p2);
			if (d < minD)
			{
				min = 3;
			}

			switch (min)
			{
			case 0:
				result = Line(l1p1, l2p1).MidPoint(); break;
			case 1:
				result = Line(l1p1, l2p2).MidPoint(); break;
			case 2:
				result = Line(l1p2, l2p1).MidPoint(); break;
			case 3:
				result = Line(l1p2, l2p2).MidPoint(); break;
			}
			return ES_LINES_IN_LINE;
		} else
		/*/
		if (Float::EqualTo(GetPerpendicularDist(l1p1, l1p2, l2p1), 0.0))
		{
			// they're parallel AND in line, so there is an intersect

			//flip if required

			return ES_LINES_IN_LINE;
		}
		//*/
		{
			return E_LINES_PARALLEL;
		}
	}
/* // --------------
C
C now find intersection point
C
      TOP = DXL2*(YL - YJ) - DYL2*(XL - XJ)
      S   = TOP/DENOM
      XI  = XJ + S*DXL1
      YI  = YJ + S*DYL1
      RETURN
      END
*/
	top = dx2*(l2p1.y - l1p1.y) - dy2*(l2p1.x - l1p1.x);
	s = top/denom;
	result.x = l1p1.x + s*dx1;
	result.y = l1p1.y + s*dy1;

	return S_INTERSECT;
}

//--------------------------------------------------------------
// Find the point of intersection of 2 line segments.
KEAYS_MATH_EXPORTS_API const int
LineSegLineSegIntersect(const VectorD2 &l1p1, const VectorD2 &l1p2, const VectorD2 &l2p1, const VectorD2 &l2p2, VectorD2 &result)
{
	int iResult = LineLineIntersect(l1p1, l1p2, l2p1, l2p2, result);

	if (iResult != S_INTERSECT)
		return iResult;

	double line1Len, l1p1ptDist, l1p2ptDist;
	line1Len = Dist2D(l1p1, l1p2);
	l1p1ptDist = Dist2D(l1p1, result);
	l1p2ptDist = Dist2D(l1p2, result);
	if (Float::EqualTo(l1p1ptDist, 0.0))
	{
		return SS_L1P1_CONTACT;
	} else if (Float::EqualTo(l1p2ptDist, 0.0))
	{
		return SS_L1P2_CONTACT;
	} else if (Float::NotEqual(line1Len, (l1p1ptDist + l1p2ptDist)))
	{
		return E_NO_SEGMENT_INTERSECT;
	}

	double line2Len, l2p1ptDist, l2p2ptDist;
	line2Len = Dist2D(l2p1, l2p2);
	l2p1ptDist = Dist2D(l2p1, result);
	l2p2ptDist = Dist2D(l2p2, result);
	if (Float::EqualTo(l2p1ptDist, 0.0))
	{
		return SS_L2P1_CONTACT;
	} else if (Float::EqualTo(l2p2ptDist, 0.0))
	{
		return SS_L2P2_CONTACT;
	} else if (Float::NotEqual(line2Len, (l2p1ptDist + l2p2ptDist)))
	{
		return E_NO_SEGMENT_INTERSECT;
	}

	return S_INTERSECT;
}

//--------------------------------------------------------------
int LineLineIntersectAz()
{
/*
      SUBROUTINE LINAZ(X1,Y1,AZ1,X2,Y2,AZ2,XS,YS,D1,D2,FLAG)
C                -------------------------------------------
C
C  ROUTINE TO CALCULATE THE INTERSECTION OF TWO LINES
C  LINE 1  X1,Y1   AT  AZ1
C  LINE 2  X2,Y2   AT  AZ2
C  SOLUTION XS,YS  AT D1 FROM X1,Y1  D2 FROM X2,Y2
C  FLAG  IF PARALLEL LINES
C
C
      DOUBLE PRECISION DABS,DCOS,DSIN
      INTRINSIC        DABS,DCOS,DSIN
      DOUBLE PRECISION X1,Y1,AZ1,X2,Y2,AZ2,XS,YS,D1,D2
      LOGICAL FLAG
      DOUBLE PRECISION XSOL,X2T,Y2T,AZDEF,S1,C1,S2,C2,X2R,Y2R
      DOUBLE PRECISION SMALL
      DATA SMALL/1.0D-8/
C
C
      X2T = X2-X1
      Y2T = Y2-Y1
      AZDEF = AZ2-AZ1
      S1    = DSIN(AZDEF)
      if (DABS(S1) .LE. SMALL) GOTO 100
      C1    = DCOS(AZDEF)
      S2    = DSIN(AZ1)
      C2    = DCOS(AZ1)
      X2R   = X2T*C2 + Y2T*S2
      Y2R   = -X2T*S2 + Y2T*C2
      XSOL = X2R-C1*Y2R/S1
      XS   = X1+XSOL*C2
      YS   = Y1+XSOL*S2
      D1    = X2R -C1*Y2R/S1
      D2    = DABS(Y2R/S1)
      FLAG = .TRUE.
      RETURN
C
C PARALLEL
  100 FLAG=.FALSE.
      RETURN
      END
*/
	return S_INTERSECT;
}

//--------------------------------------------------------------
KEAYS_MATH_EXPORTS_API const int
MultipleLineIntersectAverage(const std::vector<Line> &lines, VectorD2 *result)
{
	size_t nLines = 0;
	size_t i, j;
	int rc;
	Polyline2D intersects;
	VectorD2 centroid;

	if (!result)
		return LC_ERR_INVALID_RESULT_PTR;

	nLines = lines.size();
	if (nLines < 2)
		return LC_ERR_TO_FEW_LINES;

	// for each line we calc the intersect of it and the others that have not been done
	intersects.clear();
	for (i = 0; i < nLines; ++i)
	{
		for (j = i+1; j < nLines; ++j)
		{
			VectorD2 ip;

			rc = LineLineIntersect(lines[i].start.XY(), lines[i].end.XY(), lines[j].start.XY(), lines[j].end.XY(), ip);
			if (rc != S_INTERSECT)
			{
				if (rc == E_LINES_PARALLEL)
				{
					continue;
				} else if (rc != ES_LINES_IN_LINE) // inline is still good in a way
				{
					rc = rc >> 4;
					rc |= LC_ERR_NO_VALID_INTERSECT;
					return rc;
				}
			}
			intersects.push_back(ip);
		} // for (j = i+1; j < nLines; ++j)
	} // for (i = 0; i < nLines; ++i)

	if (rc = AveragePoints(intersects, &centroid) != PAV_SUCCESS)
	{
		printf("ERROR averaging points\n");
		rc = rc >> 4;
		rc |= LC_ERR_PROBLEM_AVERAGING_POINTS;
		return rc;
	}
	*result = centroid;

	return LC_SUCCESS;
}

//--------------------------------------------------------------
KEAYS_MATH_EXPORTS_API const int
AveragePoints(const Polyline2D &points, VectorD2 *result)
{
	size_t nPoints = 0;
	VectorD2 avePt(0, 0);
	if (!result)
		return PAV_ERR_INVALID_RESULT_PTR;

	nPoints = points.size();
	if (!nPoints)
		return PAV_ERR_NO_POINTS;

	for (size_t i = 0; i < nPoints; ++i)
	{
		avePt.x += points[i].x;
		avePt.y += points[i].y;
	}
	avePt.x /= nPoints;
	avePt.y /= nPoints;

	*result = avePt;

	return PAV_SUCCESS;
}

//--------------------------------------------------------------
KEAYS_MATH_EXPORTS_API const int
AveragePoints(const Polyline3D &points, VectorD3 *result)
{
	size_t nPoints = 0;
	VectorD3 avePt(0, 0, 0);
	if (!result)
		return PAV_ERR_INVALID_RESULT_PTR;

	nPoints = points.size();
	if (!nPoints)
		return PAV_ERR_NO_POINTS;

	for (size_t i = 0; i < nPoints; ++i)
	{
		avePt.x += points[i].x;
		avePt.y += points[i].y;
		avePt.z += points[i].z;
	}
	avePt.x /= nPoints;
	avePt.y /= nPoints;
	avePt.z /= nPoints;

	*result = avePt;

	return PAV_SUCCESS;
}

//--------------------------------------------------------------
KEAYS_MATH_EXPORTS_API const int
RectRectIntersect(const VectorD2 & topLeft1, const VectorD2 & botRight1,
				   const VectorD2 & topLeft2, const VectorD2 & botRight2,
				   VectorD2 & resultTopLeft, VectorD2 & resultBotRight)
{

	/* Sanity Check */

	if (topLeft1.x >= botRight1.x || topLeft1.y <= botRight1.y ||
		 topLeft2.x >= botRight2.x || topLeft2.y <= botRight2.y)
	{
		return E_FAIL_OTHER;
	}

	// check if rect1 & rect2 are the same
	if ((topLeft1.x == topLeft2.x) || (botRight1.x == botRight2.x) ||
		 (topLeft1.y == topLeft2.y) || (botRight1.y == botRight2.y)
	  )
	{
		resultTopLeft = topLeft1;
		resultBotRight = botRight1;
		return E_SAME_RECT;
	}

	// check if rect1 & rect2 don't intersect
	if ((botRight1.x <= topLeft2.x) || (topLeft1.x >= botRight2.x) ||
		 (topLeft1.y <= botRight2.y) || (botRight1.y >= topLeft2.y)
	  )
	{
		return E_NO_INTERSECT;
	}

	bool bRect1XInside = false,
		 bRect1YInside = false,
		 bRect2XInside = false,
		 bRect2YInside = false;

	// check if rect1 is contained inside rect2
	if ((topLeft1.x >= topLeft2.x) && (botRight1.x <= botRight2.x)) bRect1XInside = true;
	if ((topLeft1.y <= topLeft2.y) && (botRight1.y >= botRight2.y)) bRect1YInside = true;
	if (bRect1XInside && bRect1YInside)
	{
		resultTopLeft = topLeft1;
		resultBotRight = botRight1;
		return E_RECT1_INSIDE;
	}

	// check if rect2 is contained inside rect1
	if ((topLeft2.x >= topLeft1.x) && (botRight2.x <= botRight1.x)) bRect2XInside = true;
	if ((topLeft2.y <= topLeft1.y) && (botRight2.y >= botRight1.y)) bRect2YInside = true;

	if (bRect2XInside && bRect2YInside)
	{
		resultTopLeft = topLeft2;
		resultBotRight = botRight2;
		return E_RECT2_INSIDE;
	}

	// ok they MUST intersect then. Perform intersection calculations
	bool doneX = false, doneY = false;

	// X dimension
	if (bRect1XInside || bRect2XInside)
	{
		doneX = true;

		if (bRect1XInside)
		{
			resultTopLeft.x = topLeft1.x;
			resultBotRight.x = botRight1.x;
		}

		if (bRect2XInside)
		{
			resultTopLeft.x = topLeft2.x;
			resultBotRight.x = botRight2.x;
		}
	}

	/* // --------------
	 * 1 = big rectangle
	 * 2 = small rectangle
	 *
	 * -----------
	 * |         |
	 *---        |
	 *| |        |
	 *---        |
	 * |         |
	 */
	else if (topLeft1.x < topLeft2.x && botRight1.x > topLeft2.x && botRight2.x < botRight1.x)
	{
		resultTopLeft.x = topLeft1.x;
		resultBotRight.x = botRight2.x;
		doneX = true;
	}

	/* // --------------
	 * 1 = small rectangle
	 * 2 = big rectangle
	 *
	 * -----------
	 * |         |
	 *---        |
	 *| |        |
	 *---        |
	 * |         |
	 */
	else if (topLeft2.x < topLeft1.x && botRight2.x > topLeft1.x && botRight1.x < botRight2.x)
	{
		resultTopLeft.x = topLeft2.x;
		resultBotRight.x = botRight1.x;
		doneX = true;
	}

	/* // --------------
	 * 1 = small rectangle
	 * 2 = big rectangle
	 *
	 * -----------
	 * |         |
	 * |        ---
	 * |        | |
	 * |        ---
	 * |         |
	 */
	else if (topLeft1.x < botRight2.x && botRight1.x > botRight2.x && topLeft1.x > topLeft2.x)
	{
		resultTopLeft.x = topLeft1.x;
		resultBotRight.x = botRight2.x;
		doneX = true;
	}

	/* // --------------
	 * 1 = big rectangle
	 * 2 = small rectangle
	 *
	 * -----------
	 * |         |
	 * |        ---
	 * |        | |
	 * |        ---
	 * |         |
	 */
	else if (topLeft2.x < botRight1.x && botRight2.x > botRight1.x && topLeft2.x > topLeft1.x)
	{
		resultTopLeft.x = topLeft2.x;
		resultBotRight.x = botRight1.x;
		doneX = true;
	}


	// Y dimension
	if (bRect1YInside || bRect2YInside)
	{
		doneY = true;

		if (bRect1YInside)
		{
			resultTopLeft.y = topLeft1.y;
			resultBotRight.y = botRight1.y;
		}

		if (bRect2YInside)
		{
			resultTopLeft.y = topLeft2.y;
			resultBotRight.y = botRight2.y;
		}
	}

	/* // --------------
	 * 1 = small rectangle
	 * 2 = big rectangle
	 *     ---
	 * ----| |----
	 * |   ---   |
	 *
	 */
	else if (topLeft1.y > topLeft2.y && botRight1.y < topLeft2.y && botRight1.y > botRight2.y)
	{
		resultTopLeft.y = topLeft2.y;
		resultBotRight.y = botRight1.y;
		doneY = true;
	}

	/* // --------------
	 * 1 = big rectangle
	 * 2 = small rectangle
	 *     ---
	 * ----| |----
	 * |   ---   |
	 *
	 */
	else if (topLeft2.y > topLeft1.y && botRight2.y < topLeft1.y && botRight2.y > botRight1.y)
	{
		resultTopLeft.y = topLeft1.y;
		resultBotRight.y = botRight2.y;
		doneY = true;
	}

	/* // --------------
	 * 1 = small rectangle
	 * 2 = big rectangle
	 *
	 * |   ---   |
	 * ----| |----
	 *     ---
	 */
	else if (topLeft1.y > botRight2.y && botRight1.y < botRight2.y && topLeft1.y < topLeft2.y)
	{
		resultTopLeft.y = topLeft1.y;
		resultBotRight.y = botRight2.y;
		doneY = true;
	}

	/* // --------------
	 * 1 = big rectangle
	 * 2 = small rectangle
	 *
	 * |   ---   |
	 * ----| |----
	 *     ---
	 */
	else if (topLeft2.y > botRight1.y && botRight2.y < botRight1.y && topLeft2.y < topLeft1.y)
	{
		resultTopLeft.y = topLeft2.y;
		resultBotRight.y = botRight1.y;
		doneY = true;
	}

	if (doneX && doneY) return S_INTERSECT;

	// shouldn't ever get here, but you never know...
	return E_FAIL_OTHER;
}

//--------------------------------------------------------------
// Caculate the Y value for a given X position on a specified line
KEAYS_MATH_EXPORTS_API const double *
YfromXPt(const VectorD2 &pt1, const VectorD2 &pt2, const double &x, double *pY, const double &tolerance /*= Float::TOLERANCE*/)
{
	assert(pY != NULL);

	double dx, dy, m;

	dx = pt2.x - pt1.x;
	dy = pt2.y - pt1.y;

	if (Float::EqualTo(dx, 0.0))
	{
		// its a vertical line, and thus Y is indeterminate
		(*pY) = Float::INVALID_DOUBLE;
		return NULL;
	}

	if (Float::EqualTo(dy, 0.0))
	{
		// its a horizontal line, Y is constant
		(*pY) = pt1.y;
	} else
	{
		m = dy / dx;
		(*pY) = (m * x) - (m * pt1.x) + pt1.y;
	}
	return pY;
}

//--------------------------------------------------------------
// Caculate the X value for a given Y position on a specified line
KEAYS_MATH_EXPORTS_API const double *
XfromYPt(const VectorD2 &pt1, const VectorD2 &pt2, const double &y, double *pX, const double &tolerance /*= Float::TOLERANCE*/)
{
	assert(pX != NULL);

	double dx, dy, invM;

	dx = pt2.x - pt1.x;
	dy = pt2.y - pt1.y;

	if (Float::EqualTo(dy, 0.0))
	{
		// its a vertical line, and thus Y is indeterminate
		(*pX) = Float::INVALID_DOUBLE;
		return NULL;
	}

	if (Float::EqualTo(dx, 0.0))
	{
		// its a horizontal line, Y is constant
		(*pX) = pt1.x;
	} else
	{
		invM = dx / dy;
		(*pX) = (invM * y) - (invM * pt1.y) + pt1.x;
	}
	return pX;
}

//--------------------------------------------------------------
// Get a string representation of the return value from the LineCrosses* and LineSegCrosses functions
KEAYS_MATH_EXPORTS_API const char *
LineTriReturnString(const int val)
{
	static const char *_eLCTErrText[] =
	{
		"FAILURE",
		"LINE CROSSES",
		"LINE TOUCHES",
		"LINE IS CONTAINED",
		"LINE ENTERS",
		"LINE EXITS"
	};

	if ((val < 0) || (val > S_LINE_EXITS))
		return "UNKNOWN";
	else
		return _eLCTErrText[val];
}

//--------------------------------------------------------------
// Calculate the intersection points of a line and a rectangle.
KEAYS_MATH_EXPORTS_API const int
LineCrossesRect(const RectD &rect, const Line &line, VectorD2 *pLineStart, VectorD2 *pLineEnd, const double &tolerance /*= Float::TOLERANCE*/)
{
	assert(pLineStart != NULL);
	assert(pLineEnd != NULL);

	if (!rect.IsValid())
		return E_FAILURE;
	if (Float::EqualTo(line.GetLength2D(), 0.0))
		return E_FAILURE;

	// intercepts
	double yLeft, yRight; // these are y values at the specified x
	double xTop, xBottom; // these are x values at the specified y

	const double &rLeft = rect.GetLeft();
	const double &rTop = rect.GetTop();
	const double &rRight = rect.GetRight();
	const double &rBottom = rect.GetBottom();

	// test xTop and yLeft first
	if (!YfromXPt(line.start.XY(), line.end.XY(), rLeft, &yLeft, tolerance))
	{
		// if this fails the line is vertical, so we test against the yLeft and yRight rect edges to see if it is contained
		if (Float::GreaterOrEqual(line.start.x, rLeft) && Float::LessOrEqual(line.start.x, rRight))
		{
			if (line.start.y > line.end.y)
			{
				pLineStart->Set(line.start.x, rTop);
				pLineEnd->Set(line.end.x, rBottom);
			} else
			{
				pLineStart->Set(line.start.x, rBottom);
				pLineEnd->Set(line.end.x, rTop);
			}

			return S_LINE_CROSSES;
		} else
		{
			return E_FAILURE;
		}
	} // if (!YfromXPt(line.start.XY(), line.end.XY(), rLeft, &yLeft, tolerance))
	if (!XfromYPt(line.start.XY(), line.end.XY(), rTop, &xTop, tolerance))
	{
		// if this fails the line is horizontal, so we test against the xTop and xBottom rect edges to see if it is contained
		if (Float::LessOrEqual(line.start.y, rTop) && Float::GreaterOrEqual(line.start.y, rBottom))
		{
			if (line.start.x < line.end.x)
			{
				pLineStart->Set(rLeft, line.start.y);
				pLineEnd->Set(rRight, line.end.y);
			} else
			{
				pLineStart->Set(rRight, line.start.y);
				pLineEnd->Set(rLeft, line.end.y);
			}

			return S_LINE_CROSSES;
		} else
		{
			return E_FAILURE;
		}
	} // if (!XfromYPt(line.start.XY(), line.end.XY(), rTop, &xTop, tolerance))

	// if we get here we have to find the other edge intersects
	// no need to test for NULL returns, if we get here, it cannot be pure vertical or horizontal
	YfromXPt(line.start.XY(), line.end.XY(), rRight, &yRight, tolerance);
	XfromYPt(line.start.XY(), line.end.XY(), rBottom, &xBottom, tolerance);

	bool touchesLeft = Float::Less(yLeft, rTop) && Float::GreaterOrEqual(yLeft, rBottom);
	bool touchesTop = Float::GreaterOrEqual(xTop, rLeft) && Float::Less(xTop, rRight);
	bool touchesRight = Float::LessOrEqual(yRight, rTop) && Float::Greater(yRight, rBottom);
	bool touchesBottom = Float::Greater(xBottom, rLeft) && Float::LessOrEqual(xBottom, rRight);
	int numSolutions = 0;
	bool haveStart = false;
	bool haveEnd = false;

	if (touchesLeft)
	{
		if (line.start.x <= line.end.x)
		{
			pLineStart->Set(rLeft, yLeft);
			haveStart = true;
		} else
		{
			pLineEnd->Set(rLeft, yLeft);
			haveEnd = true;
		}
		numSolutions++;
	}
	if (touchesTop)
	{
		if ((line.start.y >= line.end.y) && !haveStart)
		{
			haveStart = true;
			pLineStart->Set(xTop, rTop);
		} else
		{
			pLineEnd->Set(xTop, rTop);
			haveEnd = true;
		}
		numSolutions++;
	}
	if ((numSolutions < 2) && touchesRight)
	{
		if ((line.start.x > line.end.x) && !haveStart)
		{
			pLineStart->Set(rRight, yRight);
			haveStart = true;
		} else
		{
			pLineEnd->Set(rRight, yRight);
			haveEnd = true;
		}
		numSolutions++;
	}
	if ((numSolutions < 2) && touchesBottom)
	{
		if ((line.start.y < line.end.y) && !haveStart)
		{
			pLineStart->Set(xBottom, rBottom);
			haveStart = true;
		} else
		{
			pLineEnd->Set(xBottom, rBottom);
			haveEnd = true;
		}
		numSolutions++;
	}

	if (!numSolutions)
		return E_FAILURE;

	if (!haveStart || !haveEnd)
	{
		if (!haveEnd)
			(*pLineEnd) = (*pLineStart);
		else
			(*pLineStart) = (*pLineEnd);
	}

	return S_LINE_CROSSES;
}

//--------------------------------------------------------------
/*!
	\brief Calculate the intersection points of a line segment and a rectangle.

	\param                rect [In]  - a constant reference to the rectangle to test, must be valid or the function returns failure.
	\param                line [In]  - a constant reference to the line to test with, must have some 2D length or the function returns failure.
	\param pLineStart [Out] - A pointer to a VectorD2 for the intersection point for the start of the line, MUST NOT BE NULL.
	\param   pLineEnd [Out] - A pointer to a vectorD2 for the intersection point at the end of the line, MUST NOT BE NULL.
	\param           tolerance [In]  - a constant double representing the tolerance to use for testing.

	\return a constant integer representing the success of the operation. One of the following values from eLCRVals -
	E_FAILURE, S_LINE_CROSSES, S_LINE_TOUCHES, S_LINE_CONTAINED, S_LINE_ENTERS or S_LINE_EXITS.
 */
KEAYS_MATH_EXPORTS_API const int
LineSegCrossesRect(const RectD &rect, const Line &line, VectorD2 *pLineStart, VectorD2 *pLineEnd, const double &tolerance /*= Float::TOLERANCE*/)
{
	// We need to make these duplicates to handle the case where the line's points are the pointers
	// passed in as return values (THIS SHOULDN'tHAPPEN but...)
	VectorD3 origStart(line.start);
	VectorD3 origEnd(line.end);
	int result = LineCrossesRect(rect, line, pLineStart, pLineEnd, tolerance);

	if (result != S_LINE_CROSSES)
		return result; // return as is

	// if we are here it crosses
	bool startInside = rect.PointInside(origStart.XY(), tolerance);
	bool endInside = rect.PointInside(origEnd.XY(), tolerance);

	if (startInside && endInside)
	{
		// if the start and end are contained, we return the original points, and indicate that the line is contained
		(*pLineStart) = origStart.XY();
		(*pLineEnd) = origEnd.XY();
		result = S_LINE_CONTAINED;
	} else if (startInside)
	{
		// if the start is contained and the end is not, we return the original start, and indicate that the line exits
		(*pLineStart) = origStart.XY();
		result = S_LINE_EXITS;
	} else if (endInside)
	{
		// if the end is contained and the start is not, we return the original end, and indicate that the line enters
		(*pLineEnd) = origEnd.XY();
		result = S_LINE_ENTERS;
	}

	return result;
}

//--------------------------------------------------------------
/*!
	\brief test if a point is inside a triangle
	\param     triPt1 [In]  - a constant reference to a VectorD2 representing the first point of a CCW wound triangle.
	\param     triPt2 [In]  - a constant reference to a VectorD2 representing the second point of a CCW wound triangle.
	\param     triPt3 [In]  - a constant reference to a VectorD2 representing the third point of a CCW wound triangle.
	\param         pt [In]  - a constant reference to a VectorD2 representing the point to test.
	\param  tolerance [In]  - a constant double representing the tolerance to use for testing.

	\return true if the point is inside or touching the triangle the triangle
 */
KEAYS_MATH_EXPORTS_API bool
PointInTriangle(const VectorD2 &triPt1, const VectorD2 &triPt2, const VectorD2 &triPt3, const VectorD2 &pt, const double &tolerance /*= Float::TOLERANCE*/)
{
	bool isCCW = true, isCCW1, isCCW2, isCCW3;
	double area = CalcTriangleArea(triPt1, triPt2, triPt3);
	if (area == 0)
		return false;

	isCCW = area > 0.0;

	area = CalcTriangleArea(triPt1, triPt2, pt);
	isCCW1 = (isCCW ? area >= 0.0 : area <= 0.0);
	area = CalcTriangleArea(triPt1, pt, triPt3);
	isCCW2 = (isCCW ? area >= 0.0 : area <= 0.0);
	area = CalcTriangleArea(pt, triPt2, triPt3);
	isCCW3 = (isCCW ? area >= 0.0 : area <= 0.0);

	return 	isCCW1 && isCCW2 && isCCW3;
}

//-----------------------------------------------------------------------------
// Test to see if a specified point is on the segment.
KEAYS_MATH_EXPORTS_API bool
PointOnSegment(const VectorD2 &pt1, const VectorD2 &pt2, const VectorD2 &testPt, const double &tolerance /*= Float::TOLERANCE*/)
{
	VectorD2 pt1ToTestPt((testPt - pt1).GetNormalised());
	VectorD2 testPtToPt2((pt2 - testPt).GetNormalised());

	return Float::GreaterOrEqual(pt1ToTestPt.Dot(testPtToPt2), 0.0, tolerance);
}

//-----------------------------------------------------------------------------
// Test to see if a specified point is on the segment.
KEAYS_MATH_EXPORTS_API bool
PointOnSegment(const VectorD3 &pt1, const VectorD3 &pt2, const VectorD3 &testPt, const double &tolerance /*= Float::TOLERANCE*/)
{
	VectorD3 pt1ToTestPt((testPt - pt1).GetNormalised());
	VectorD3 testPtToPt2((pt2 - testPt).GetNormalised());

	return Float::GreaterOrEqual(pt1ToTestPt.Dot(testPtToPt2), 0.0, tolerance);
}

//--------------------------------------------------------------
const int edgeFollowingPoint[] =
{
	2, 0, 1
};

//--------------------------------------------------------------
KEAYS_MATH_EXPORTS_API const int
LineCrossesTriangle(const VectorD3 &triPt1, const VectorD3 &triPt2, const VectorD3 &triPt3,
					 const Line &line, VectorD3 *pLineStart, VectorD3 *pLineEnd,
					 int *pStartEdge /*= NULL*/, int *pEndEdge /*= NULL*/,
					 const double &tolerance  /*= Float::TOLERANCE*/)
{
	assert(pLineStart != NULL);
	assert(pLineEnd != NULL);
	assert(pStartEdge != NULL);
	assert(pEndEdge != NULL);

	(*pStartEdge) = -1;
	(*pEndEdge) = -1;

	int startEdge = -1;
	int endEdge = -1;

	if (Float::EqualTo(line.GetLength2D(), 0.0))
	{
		WriteDebugLog(_T("    MATH: %5d: Line has 0 length in the XY plane\n"), __LINE__);
		return E_FAILURE;
	}

	double area = CalcTriangleArea(triPt1.XY(), triPt2.XY(), triPt3.XY());
	if (area <= 0.0) // wound clockwise or no area
	{
		WriteDebugLog(_T("    MATH: %5d: Triangle has 0 area in the XY plane\n"), __LINE__);
		return E_FAILURE;
	}

	int result = S_LINE_CROSSES;

	// rotate the triangle so the line is flat
	VectorD2 rotPt[3] =
	{
		(RotatePointXY(line.start.XY(), line.end.XY(), triPt1.XY())),
		(RotatePointXY(line.start.XY(), line.end.XY(), triPt2.XY())),
		(RotatePointXY(line.start.XY(), line.end.XY(), triPt3.XY()))
	};

	// a touch is considered a crossing
	int numAbove = 0,
		numBelow = 0,
		numOn = 0,
		numCrossing = 0;
	int ptsLoc[3] = { 0, 0, 0 };
	int i;

	for (i = 0; i < 3; i++)
	{
		if (rotPt[i].y < line.start.y)
		{
			ptsLoc[i] = -1;
			numBelow++;
		} else if (rotPt[i].y > line.start.y)
		{
			ptsLoc[i] = 1;
			numAbove++;
		}
	} // for (i = 0; i < 3; i++)

	if ((numAbove == 3) || (numBelow == 3)) // we do not even touch
	{
		WriteDebugLog(_T("    MATH: %5d: LineCrossesTriangle - FAILS, does not touch\n"), __LINE__);
		return E_FAILURE;
	}

	numOn = 3 - (numAbove + numBelow);
	if (ptsLoc[0] && ptsLoc[0] == -ptsLoc[1]) numCrossing++;
	if (ptsLoc[1] && ptsLoc[1] == -ptsLoc[2]) numCrossing++;
	if (ptsLoc[2] && ptsLoc[2] == -ptsLoc[0]) numCrossing++;

	const VectorD2 refs[3] = { triPt1, triPt2, triPt3 };

	if (numOn && !numCrossing)
	{
		if (numOn == 3)
		{
			WriteDebugLog(_T("    MATH: %5d: 3 Triangle points lie on line - FAILS\n"), __LINE__);
			return E_FAILURE; // not set up for this
		}

		result = S_LINE_TOUCHES;

		// find the intersection points
		if (numOn == 1)
		{
			WriteDebugLog(_T("    MATH: %5d: LineCrossesTriangle - Line touches a corner\n"), __LINE__);
			// we touch a corner
			if (ptsLoc[0] == 0)
			{
				(*pLineStart) = (*pLineEnd) = triPt1;
				startEdge = endEdge = edgeFollowingPoint[0];
			} else if (ptsLoc[1] == 0)
			{
				(*pLineStart) = (*pLineEnd) = triPt2;
				startEdge = endEdge = edgeFollowingPoint[1];
			} else
			{
				(*pLineStart) = (*pLineEnd) = triPt3;
				startEdge = endEdge = edgeFollowingPoint[2];
			}
		} else
		{
			WriteDebugLog(_T("    MATH: %5d: LineCrossesTriangle - Line runs along side\n"), __LINE__);
			// we run along a side
			for (i = 0; i < 3; i++)
			{
				if (ptsLoc[i] == 0)
				{
					(*pLineStart) = refs[i];
					startEdge = edgeFollowingPoint[i];
					i++;
					break;
				} // if (ptsLoc[i] == 0)
			} // for (i = 0; i < 3; i++)
			for (; i < 3; i++)
			{
				if (ptsLoc[i] == 0)
				{
					(*pLineEnd) = refs[i];
					endEdge = edgeFollowingPoint[i];
					i++;
					break;
				} // if (ptsLoc[i] == 0)
			} // for (; i < 3; i++)
		}
	} else
	{
		int numFound = 0;
		VectorD2 intPoints[2];

		// if we get here we have a triangle that crosses the line
		// test 0 - 1
		WriteDebugLog(_T("    MATH: %5d: LineCrossesTriangle - ptsLoc[] = { %d, %d, %d };\n"),
					   __LINE__, ptsLoc[0], ptsLoc[1], ptsLoc[2]);
		if (ptsLoc[0] == -ptsLoc[1])
		{
			WriteDebugLog(_T("    MATH: %5d: LineCrossesTriangle - Side 0 - 1, numFound = %d\n"), __LINE__, numFound);
			if (LineLineIntersect(triPt1.XY(), triPt2.XY(), line.start.XY(), line.end.XY(), intPoints[numFound]) != S_INTERSECT)
			{
				WriteDebugLog(_T("    MATH: %5d: tri pts[0] && [1] do not intersect with line\n"), __LINE__);
				return E_FAILURE;
			}
			startEdge = 2;
			numFound++;
		}
		// test 1 - 2
		if (ptsLoc[1] == -ptsLoc[2])
		{
			WriteDebugLog(_T("    MATH: %5d: LineCrossesTriangle - Side 1 - 2, numFound = %d\n"), __LINE__, numFound);
			if (LineLineIntersect(triPt2.XY(), triPt3.XY(), line.start.XY(), line.end.XY(), intPoints[numFound]) != S_INTERSECT)
			{
				WriteDebugLog(_T("    MATH: %5d: tri pts[1] && [2] do not intersect with line\n"), __LINE__);
				return E_FAILURE;
			}
			if (numFound)
				endEdge = 0;
			else
				startEdge = 0;
			numFound++;
		}
		// test 2 - 0
		if ((numFound < 2) && (ptsLoc[2] == -ptsLoc[0]))
		{
			WriteDebugLog(_T("    MATH: %5d: LineCrossesTriangle - Side 2 - 0, numFound = %d\n"), __LINE__, numFound);
			if (LineLineIntersect(triPt3.XY(), triPt1.XY(), line.start.XY(), line.end.XY(), intPoints[numFound]) != S_INTERSECT)
			{
				WriteDebugLog(_T("    MATH: %5d: tri pts[2] && [0] do not intersect with line\n"), __LINE__);
				return E_FAILURE;
			}
			if (numFound)
				endEdge = 1;
			else
				startEdge = 1;
			numFound++;
		}

		// if we get here and do not have 2 points, we need to locate one
		if (numFound < 2)
		{
			WriteDebugLog(_T("    MATH: %5d: LineCrossesTriangle - NEED TO LOCATE SECOND POINT\n"), __LINE__);
			for (i = 0; i < 3; i++)
			{
				if (ptsLoc[i] == 0)
				{
					intPoints[1] = refs[i];
					endEdge = edgeFollowingPoint[i];
					break;
				} // if (ptsLoc[i] == 0)
			} // for (i = 0; i < 3; i++)
		}

		// we should have 2 points by now
		WriteDebugLog(_T("    MATH: %5d: LineCrossesTriangle - \n"
						 "\t\t\tintersectPts[0] = [%.3f, %.3f]\n"
						 "\t\t\tintersectPts[1] = [%.3f, %.3f]\n"),
						 __LINE__, intPoints[0].x, intPoints[0].y, intPoints[1].x, intPoints[1].y);
		(*pLineStart) = intPoints[0];
		(*pLineEnd) = intPoints[1];
	}

	if ((result != S_LINE_TOUCHES) || 1)
	{
		// now ensure if is in right direction
		double bearing = line.GetBearing();	// TODO: determine if this should be trig or vector maths
		double backBearing = GetAbsoluteAngle(bearing + KM_PI);
		double resBearing = GetAbsoluteAngle(Direction((*pLineStart), (*pLineEnd)));
		WriteDebugLog(_T("    MATH: %5d: LineCrossesTriangle - Test for swap\n"
						 "\t\t\tbackBearing = %.3f rad (%.3f%c)\n"
						 "\t\t\t resBearing = %.3f rad (%.3f%c)\n"),
						 __LINE__,
						 backBearing, RTD(backBearing), DEG_CHAR,
						 resBearing, RTD(resBearing), DEG_CHAR);
		if (Float::EqualTo(resBearing , backBearing, tolerance))
		{
			WriteDebugLog(_T("    MATH: %5d: LineCrossesTriangle - Swap Points\n"), __LINE__);
			// we need to swap them
			VectorD2 temp(*pLineStart);
			(*pLineStart) = (*pLineEnd);
			(*pLineEnd) = temp;

			int iTemp = startEdge;
			startEdge = endEdge;
			endEdge = iTemp;
		}
	}

	// get the heights
	double ht;
	PointHeightOnPlaneTri(triPt1, triPt2, triPt3, (*pLineStart).XY(), ht);
	(*pLineStart).z = ht;
	PointHeightOnPlaneTri(triPt1, triPt2, triPt3, (*pLineEnd).XY(), ht);
	(*pLineEnd).z = ht;

	(*pStartEdge) = startEdge;
	(*pEndEdge) = endEdge;

	WriteDebugLog(_T("    MATH: %5d: tri intersects with line\n"), __LINE__);
	return result;
}

KEAYS_MATH_EXPORTS_API const int
LineSegCrossesTriangle(const VectorD3 &triPt1, const VectorD3 &triPt2, const VectorD3 &triPt3,
							   const Line &line, VectorD3 *pLineStart, VectorD3 *pLineEnd,
								int *pStartEdge, int *pEndEdge,
								const double &tolerance /*= Float::TOLERANCE*/)
{
	VectorD3 origStart(line.start);
	VectorD3 origEnd(line.end);

	int result = LineCrossesTriangle(triPt1, triPt2, triPt3, line, pLineStart, pLineEnd, pStartEdge, pEndEdge, tolerance);

	if ((result != S_LINE_CROSSES) && (result != S_LINE_TOUCHES))
		return result; // return as is

	// if we are here it crosses
	bool startInside = PointInTriangle(triPt1.XY(), triPt2.XY(), triPt3.XY(), origStart.XY(), tolerance);
	bool endInside = PointInTriangle(triPt1.XY(), triPt2.XY(), triPt3.XY(), origEnd.XY(), tolerance);

	WriteDebugLog(_T("    MATH: %5d: LineSegCrossesTriangle - test segment\n"
					 "\t\t\tOrigStart = [%.3f, %.3f], origEnd = [%.3f, %.3f]\n"
					 "\t\t\t    start = [%.3f, %.3f],     end = [%.3f, %.3f]\n"),
					 __LINE__, origStart.x, origStart.y, origEnd.x, origEnd.y,
					 pLineStart->x, pLineStart->y, pLineEnd->x, pLineEnd->y);
	if (startInside && endInside)
	{
		// if the start and end are contained, we return the original points, and indicate that the line is contained
		(*pLineStart) = origStart.XY();
		(*pLineEnd) = origEnd.XY();
		result = S_LINE_CONTAINED;
		(*pStartEdge) = -1;
		(*pEndEdge) = -1;
		WriteDebugLog(_T("    MATH: %5d: LineSegCrossesTriangle - Line is contained\n"), __LINE__);
	} else if (startInside)
	{
		// if the start is contained and the end is not, we return the original start, and indicate that the line exits
		WriteDebugLog(_T("    MATH: %5d: LineSegCrossesTriangle - Line Exits PRE start = [%.3f, %.3f] edge = %d\n"),
					  __LINE__, pLineStart->x, pLineStart->y, *pStartEdge);
		result = S_LINE_EXITS;
//		if (Float::EqualTo(Dist2D(pLineEnd->XY(), origStart.XY()), 0.0))
//		{
//			(*pLineStart) = origEnd.XY();
//			(*pStartEdge) = -1;
//		} else
		{
			(*pLineStart) = origStart.XY();
			(*pStartEdge) = -1;
		}
		WriteDebugLog(_T("    MATH: %5d: LineSegCrossesTriangle - Line Exits PRE start = [%.3f, %.3f] edge = %d\n"),
					  __LINE__, pLineStart->x, pLineStart->y, *pStartEdge);
	} else if (endInside)
	{
		// if the end is contained and the start is not, we return the original end, and indicate that the line enters
		(*pLineEnd) = origEnd.XY();
		result = S_LINE_ENTERS;
		(*pEndEdge) = -1;
		WriteDebugLog(_T("    MATH: %5d: LineSegCrossesTriangle - Line Enters\n"), __LINE__);
	} else
	{
		double bearing = line.GetBearing();
		double lsBearing = Line(line.start, (*pLineStart)).GetBearing();
		double startDist = Dist2D(line.start, (*pLineStart));
		double leBearing = Line((*pLineEnd), line.end).GetBearing();
		if (
			 !(
			   Float::EqualTo(bearing, lsBearing, tolerance) ||
				Float::EqualTo(startDist, 0.0, tolerance)
			) ||
			 !Float::EqualTo(bearing, leBearing, tolerance))
		{
			// neither
			result = E_FAILURE;
			(*pStartEdge) = -1;
			(*pEndEdge) = -1;
			WriteDebugLog(_T("    MATH: %5d: LineSegCrossesTriangle - Line Seg does not cross\n"), __LINE__);
		}
	}

	return result;
}

//--------------------------------------------------------------
KEAYS_MATH_EXPORTS_API const bool
SegmentTriangleIntersect(const VectorD3 &triPt1, const VectorD3 &triPt2, const VectorD3 &triPt3,
						  const Line &line, VectorD3 *pIntersect, const double &tolerance /*= Float::TOLERANCE*/)
{
	if (!pIntersect)
		return false;

	/* Note: source code from http://softsurfer.com.
    Vector    u, v, n;             // triangle vectors
    Vector    dir, w0, w;          // ray vectors
    float     r, a, b;             // params to calc ray-plane intersect

    // get triangle edge vectors and plane normal
    u = T.V1 - T.V0;
    v = T.V2 - T.V0;
    n = u * v;             // cross product
    if (n == (Vector)0)            // triangle is degenerate
        return -1;                 // do not deal with this case

    dir = R.P1 - R.P0;             // ray direction vector
    w0 = R.P0 - T.V0;
    a = -dot(n,w0);
    b = dot(n,dir);
    if (fabs(b) < SMALL_NUM) {     // ray is parallel to triangle plane
        if (a == 0)                // ray lies in triangle plane
            return 2;
        else return 0;             // ray disjoint from plane
    }

    // get intersect point of ray with triangle plane
    r = a / b;
    if (r < 0.0)                   // ray goes away from triangle
        return 0;                  // => no intersect
    // for a segment, also test if (r > 1.0) => no intersect

    *I = R.P0 + r * dir;           // intersect point of ray and plane

    // is I inside T?
    float    uu, uv, vv, wu, wv, D;
    uu = dot(u,u);
    uv = dot(u,v);
    vv = dot(v,v);
    w = *I - T.V0;
    wu = dot(w,u);
    wv = dot(w,v);
    D = uv * uv - uu * vv;

    // get and test parametric coords
    float s, t;
    s = (uv * wv - vv * wu) / D;
    if (s < 0.0 || s > 1.0)        // I is outside T
        return 0;
    t = (uv * wu - uu * wv) / D;
    if (t < 0.0 || (s + t) > 1.0)  // I is outside T
        return 0;

    return 1;                      // I is in T
	*/

	VectorD3 v0 = triPt2 - triPt1;
	VectorD3 v1 = triPt3 - triPt1;
	VectorD3 normal = v0.Cross(v1);

	if (Float::EqualTo(normal.Magnitude(), 0.0, tolerance))
		return false;

	VectorD3 dir = line.end - line.start;
	VectorD3 t = line.start - triPt1;

	double a = -normal.Dot(t);
	double b = normal.Dot(dir);

	if (Float::EqualTo(b, 0.0, tolerance))
		return false;

	double r = a / b;

	if (r < 0.0)
		return false;

	*pIntersect = line.start + r * dir;

	double uu, uv, vv, wu, wv, d;

	uu = v0.Dot(v0);
	uv = v0.Dot(v1);
	vv = v1.Dot(v1);

	VectorD3 w = *pIntersect - triPt1;
	wu = w.Dot(v0);
	wv = w.Dot(v1);
	d = uv * uv - uu * vv;

	return true;
}

//--------------------------------------------------------------
KEAYS_MATH_EXPORTS_API const bool
RayTriangleIntersect(const VectorD3 &triPt1, const VectorD3 &triPt2, const VectorD3 &triPt3,
					  const Line &line, VectorD3 *pIntersect, const double &tolerance /*= Float::TOLERANCE*/)
{
	VectorD3 v0 = triPt2 - triPt1;
	VectorD3 v1 = triPt3 - triPt1;
	VectorD3 normal = v0.Cross(v1);

	if (Float::EqualTo(normal.Magnitude(), 0.0, tolerance))
		return false;

	VectorD3 dir = line.end - line.start;
	VectorD3 w = line.start - triPt1;

	double a = -normal.Dot(w);
	double b = normal.Dot(dir);

	if (Float::EqualTo(b, 0.0, tolerance))
		return false;

	double r = a / b;

	if (r < 0.0)
		return false;

	if (pIntersect)
		*pIntersect = line.start + r * dir;

	return true;
}

//--------------------------------------------------------------
KEAYS_MATH_EXPORTS_API bool
SimplifyPolyline(const Polyline3D &polyline, const double &bearingTol, const double &zenithTol,
				  const double &minimumChainage, Polyline3D &simpleLine)
{
	Polyline3D::const_iterator startIt, middleIt, endIt;
	VectorD3	start, middle, end;
	double		bearing1, bearing2, zenith1, zenith2, bTol, aTol;
	bool		bBearingRemove, bZenithRemove;

	assert(polyline.size() > 2);
	assert(bearingTol >= 0.0);
	assert(zenithTol >= 0.0);

	startIt = middleIt = endIt = polyline.begin();
	middleIt++;
	endIt++;
	endIt++;

	bTol = (bearingTol * KM_2PI);
	aTol = (zenithTol * KM_PI_ON2);

	simpleLine.push_back(*startIt);

	while (endIt != polyline.end())
	{
		start = *startIt;
		middle = *middleIt;
		end = *endIt;

		bBearingRemove = false;
		bZenithRemove = false;

		bearing1 = Direction(start.XY(), middle.XY());
		bearing2 = Direction(middle.XY(), end.XY());

		if ((start == middle) || (middle == end))
		{
			bZenithRemove = true;
			bBearingRemove = true;
		}

		if (start != middle)
			zenith1 = Zenith(start, middle);
		if (middle != end)
			zenith2 = Zenith(middle, end);

		if (fabs(bearing1-bearing2) <= bTol)
		{	// eliminate
			bBearingRemove = true;
		}

		if (fabs(zenith1-zenith2) <= aTol )
		{	// eliminate
			bZenithRemove = true;
		}

		double dist = keays::math::Dist2D(start, middle);

		if (!bZenithRemove || !bBearingRemove || Float::GreaterOrEqual(dist, minimumChainage))
		{
			simpleLine.push_back(middle);
			startIt = middleIt;
			middleIt++;
			endIt++;
		}
		else
		{
			middleIt++;
			endIt++;
		}
	}

	simpleLine.push_back(*middleIt);

	return true;
}

//--------------------------------------------------------------
// Modify point heights to make a vertical curve.
KEAYS_MATH_EXPORTS_API const int
VerticalCurve(Polyline3D &pts, const double &sChain /*= 0.0*/, const double &length /*= -1*/,
			   const VectorD2 *pArcCenter /*= NULL*/, const double *pRadius /*= NULL*/,
			   VectorD2 *pStartIndices /*= NULL*/, VectorD3 *pIndices /*= NULL*/, double *pTotalLength /*= NULL*/)
{
	double curChainage;
	double dist, startChainage, endChainage, totalDist;
	double segmentBearing, segmentGrade;
	size_t idx;

	size_t startPtIdx, endPtIdx, midPointIdx, TP1Idx, TP2Idx;
	double startGrade, endGrade, midPointGrade;
	double quarterDist, midPointDist;
	double TP1Chainage, midPointChainage, TP2Chainage;
	double curveLen;

	double dTemp;

	if (pts.size() < 4)
	{
		// not much point so drop out
// TODO: Some kind of non KSR, non MFC, non WX logging system.
		return E_VC_TOO_FEW_POINTS;
	}

	if (sChain < 0)
	{
		startChainage = 0;
	} else
	{
		startChainage = sChain;
	}

	totalDist = Length2D(pts, pts.size()-1, true);
	endChainage = startChainage + length;
	curveLen = length;

	/*if (endChainage > totalDist)
	{
		// BAD, it doesn't fit
		return E_VC_CURVE_LENGTH; // TODO: Error constant for polyline too short
	}*/

	if ((length < 0) || (length > totalDist) || (endChainage > totalDist))
	{
		// we go to the full length
		endChainage = totalDist;
		curveLen = totalDist - startChainage;
	}
	if (startChainage <= 0.0)
	{
		if (Dist2D(pts[0], pts[1]) <= 0.0)
		{
			startGrade = 999e99;
		} else
		{
			startGrade = Grade(pts[0], pts[1]);
		}
	}

	midPointDist = curveLen / 2.0;
	midPointChainage = midPointDist + startChainage;
	quarterDist = midPointDist / 2.0;
	TP1Chainage = quarterDist + startChainage;
	TP2Chainage = midPointDist + TP1Chainage;

	dist = curChainage = dTemp = 0;
	Polyline3D::iterator itr, next;
	next = itr = pts.begin();
	next++;
	idx = startPtIdx = 0;
	endPtIdx = pts.size() - 1;

	bool haveStart		= false,
		 haveEnd		= false,
		 haveTP1		= false,
		 haveTP2		= false,
		 haveMidPoint	= false;

	if (startChainage == 0.0)
	{
		haveStart = true;
	}

	for (; next != pts.end(); itr++, next++)
	{
		VectorD3 &start = (*itr);
		VectorD3 &end = (*next);

		dist = Dist2D(start.XY(), end.XY());

		if (dist <= 0.0)
		{
			idx++;
			continue;
		}

		if (!haveStart &&
			Float::GreaterOrEqual(curChainage + dist, startChainage))
		{
			// we need to add the start point if it does not already exist
			startPtIdx = idx+1;
			if (pStartIndices)
				pStartIndices->x = double(startPtIdx);
			startGrade = (end.z - start.z) / dist;
			haveStart = true;

			if (Float::Greater(curChainage + dist, startChainage))
			{
				// generate the extra point
				VectorD3 newStartPt;
				segmentBearing = Direction(start.XY(), end.XY());
				segmentGrade = Grade(start, end);//Zenith(start, end);

				dTemp = dist - ((curChainage+dist) - startChainage);

				newStartPt = GenPolarPosRad(start.XY(), dTemp, segmentBearing).VD3(dTemp * segmentGrade + start.z);

				next = pts.insert(next, newStartPt);
				itr = next;
				itr--;

				idx++;
				curChainage += dTemp;
				continue;
			}
		}
		if (!haveTP1 &&
			Float::GreaterOrEqual(curChainage + dist, TP1Chainage))
		{
			TP1Idx = idx+1;
			if (pIndices)
				pIndices->x = (double)TP1Idx;
			haveTP1 = true;

			if (Float::Greater(curChainage + dist, TP1Chainage))
			{
				VectorD3 tp1;
				segmentBearing = Direction(start.XY(), end.XY());

				dTemp = dist - ((curChainage + dist) - TP1Chainage);

				tp1 = GenPolarPosRad(start.XY(), dTemp, segmentBearing).VD3
					(dTemp * startGrade + pts[startPtIdx].z);
				next = pts.insert(next, tp1);
				itr = next;
				itr--;

				idx++;
				curChainage += dTemp;
				continue;
			}
		}
		if (!haveMidPoint &&
			Float::GreaterOrEqual(curChainage + dist, midPointChainage))
		{
			midPointIdx = idx+1;
			if (pIndices)
				pIndices->y = (double)midPointIdx;
			haveMidPoint = true;

			if (Float::Greater(curChainage + dist, midPointChainage))
			{
				VectorD3 midPoint;
				segmentBearing = Direction(start.XY(), end.XY());
				// DO not get the grade here as it is dependant on the end grade which we do not know

				dTemp = dist - ((curChainage + dist) - midPointChainage);

				midPoint = GenPolarPosRad(start.XY(), dTemp, segmentBearing).VD3();
				next = pts.insert(next, midPoint);
				itr = next;
				itr--;

				idx++;
				curChainage += dTemp;
				continue;
			}
		}
		if (!haveTP2 &&
			Float::GreaterOrEqual(curChainage + dist, TP2Chainage))
		{
			TP2Idx = idx+1;
			if (pIndices)
				pIndices->z = (double)TP2Idx;
			haveTP2 = true;

			if (Float::Greater(curChainage + dist, TP2Chainage))
			{
				VectorD3 tp2;
				segmentBearing = Direction(start.XY(), end.XY());
				// DO not get the grade here as it is dependant on the end grade which we do not know

				dTemp = dist - ((curChainage + dist) - TP2Chainage);

				tp2 = GenPolarPosRad(start.XY(), dTemp, segmentBearing).VD3(end.z);//.VD3(dTemp * segmentGrade + start.z);
				next = pts.insert(next, tp2);
				itr = next;
				itr--;

				idx++;
				curChainage += dTemp;
				continue;
			}
		}
		if (!haveEnd &&
			Float::GreaterOrEqual(curChainage + dist, endChainage))
		{
			endPtIdx = idx+1;
			if (pStartIndices)
				pStartIndices->y = double(endPtIdx);
			endGrade = (end.z - start.z) / dist;
			haveEnd = true;

			if (Float::Greater(curChainage + dist, endChainage))
			{
				// generate the extra point
				VectorD3 newEndPt;
				segmentBearing = Direction(start.XY(), end.XY());
				//segmentGrade = Grade(start, end);//Zenith(start, end);

				dTemp = ((curChainage + dist) - endChainage);

				newEndPt = GenPolarPosRad(end.XY(), -dTemp, segmentBearing).VD3(-dTemp * endGrade + end.z);

				next = pts.insert(next, newEndPt);
				itr = next;
				itr--;

				idx++;
				curChainage += dTemp;
				continue;
			}
		}

		curChainage += dist;
		idx++;
	}
	pts[TP1Idx].z = startGrade * quarterDist + pts[startPtIdx].z;
	pts[TP2Idx].z = endGrade * -quarterDist + pts[endPtIdx].z;
	midPointGrade = ((pts[TP2Idx].z - pts[TP1Idx].z) / midPointDist);
	pts[midPointIdx].z = midPointGrade * quarterDist + pts[TP1Idx].z;

	// now we generate the curve
	double deltaGrade, instGrade, distRatio, yDefl;
	curChainage = 0;
	deltaGrade = midPointGrade - startGrade;
	for (idx = startPtIdx+1; idx <= midPointIdx; idx++)
	{
		VectorD3 &a = pts[idx-1];
		VectorD3 &b = pts[idx];

		curChainage += keays::math::Dist2D(a, b);
		distRatio = curChainage / midPointDist;

		// G0   = G1 + DG*DR1 <-- instantaneous grade
		instGrade = startGrade + (deltaGrade * distRatio);

		// DEFL = HALF*(DG*DX1*DR1)	<-- deflection from
		yDefl = 0.5 * (deltaGrade * distRatio) * curChainage + (startGrade * curChainage);
		pts[idx].z = pts[startPtIdx].z + yDefl;
	}
	curChainage = 0;
	deltaGrade = endGrade - midPointGrade;
	for (idx = midPointIdx+1; idx <= endPtIdx; idx++)
	{
		VectorD3 &a = pts[idx-1];
		VectorD3 &b = pts[idx];

		curChainage += keays::math::Dist2D(a, b);
		distRatio = curChainage / midPointDist;

		// G0   = G1 + DG*DR1 <-- instantaneous grade
		instGrade = midPointGrade + (deltaGrade * distRatio);

		// DEFL = HALF*(DG*DX1*DR1)	<-- deflection from
		yDefl = 0.5 * (deltaGrade * distRatio) * curChainage + (midPointGrade * curChainage);
		pts[idx].z = pts[midPointIdx].z + yDefl;
	}

	return S_VC_SUCCESS;
}

KEAYS_MATH_EXPORTS_API const int
VerticalCurve(Polyline3D &pts, const double &sGrade, const double &sHeight, const double &eGrade, const double &eHeight,
			   const VectorD2 *pArcCenter /*= NULL*/, const double *pRadius /*= NULL*/, VectorD3 *pIndices /*= NULL*/, double *pTotalLength /*= NULL*/)
{
	size_t idx, endPtIdx, midPointIdx, TP1Idx, TP2Idx;
	double curChainage, dist, TP1Chainage, midPointChainage, TP2Chainage, endChainage;
	double midPointGrade, segmentBearing, dTemp;

	if (pts.size() < 4)
	{
		// not much point so drop out
// TODO: Some kind of non KSR, non MFC, non WX logging system.
		return E_VC_TOO_FEW_POINTS;
	}

	endChainage = Length2D(pts, pts.size()-1, true);

	midPointChainage = endChainage / 2.0;
	TP1Chainage = midPointChainage / 2.0;
	TP2Chainage = endChainage - TP1Chainage;

	dist = curChainage = dTemp = 0;
	Polyline3D::iterator itr, next;
	next = itr = pts.begin();
	next++;
	idx = 0;
	endPtIdx = pts.size() - 1;

	pts[0].z = sHeight;
	pts[endPtIdx].z = eHeight;

	bool haveTP1		= false,
		 haveTP2		= false,
		 haveMidPoint	= false;
	bool useCircularPos = (pArcCenter != NULL) && (pRadius != NULL);

	for (; next != pts.end(); itr++, next++)
	{
		VectorD3 &start = (*itr);
		VectorD3 &end = (*next);

		dist = Dist2D(start.XY(), end.XY());

		if (dist <= 0.0)
		{
			idx++;
			continue;
		}

		if (!haveTP1 &&
			Float::GreaterOrEqual(curChainage + dist, TP1Chainage))
		{
			TP1Idx = idx+1;
			haveTP1 = true;
			if (pIndices)
				pIndices->x = (double)TP1Idx;

			if (Float::Greater(curChainage + dist, TP1Chainage))
			{
				VectorD3 tp1;
				if (useCircularPos)
				{
					// reuse segment bearing but be aware that it represents the bearing from the center to the start of the polyline
					segmentBearing = Direction(*pArcCenter, pts.begin()->XY()) + (TP1Chainage / *pRadius);
					tp1 = GenPolarPosRad(*pArcCenter, *pRadius, segmentBearing).VD3();
				} else
				{
					segmentBearing = Direction(start.XY(), end.XY());

					dTemp = dist - ((curChainage + dist) - TP1Chainage);

					tp1 = GenPolarPosRad(start.XY(), dTemp, segmentBearing).VD3();
				}

				next = pts.insert(next, tp1);
				itr = next;
				itr--;

				idx++;
				curChainage += dTemp;
				endPtIdx++;
				continue;
			}
		}
		if (!haveMidPoint &&
			Float::GreaterOrEqual(curChainage + dist, midPointChainage))
		{
			midPointIdx = idx+1;
			haveMidPoint = true;
			if (pIndices)
				pIndices->y = (double)midPointIdx;

			if (Float::Greater(curChainage + dist, midPointChainage))
			{
				VectorD3 midPoint;
				if (useCircularPos && 0)
				{
					// reuse segment bearing but be aware that it represents the bearing from the center to the start of the polyline
					segmentBearing = Direction(*pArcCenter, pts.begin()->XY()) + (midPointChainage / *pRadius);
					midPoint = GenPolarPosRad(*pArcCenter, *pRadius, segmentBearing).VD3();
				} else
				{
					segmentBearing = Direction(start.XY(), end.XY());
					// DO not get the grade here as it is dependant on the end grade which we do not know
					dTemp = dist - ((curChainage + dist) - midPointChainage);

					midPoint = GenPolarPosRad(start.XY(), dTemp, segmentBearing).VD3();
				}
				next = pts.insert(next, midPoint);
				itr = next;
				itr--;

				idx++;
				curChainage += dTemp;
				endPtIdx++;
				continue;
			}
		}
		if (!haveTP2 &&
			Float::GreaterOrEqual(curChainage + dist, TP2Chainage))
		{
			TP2Idx = idx+1;
			haveTP2 = true;
			if (pIndices)
				pIndices->z = (double)TP2Idx;

			if (Float::Greater(curChainage + dist, TP2Chainage))
			{

				VectorD3 tp2;
				if (useCircularPos && 0)
				{
					// reuse segment bearing but be aware that it represents the bearing from the center to the start of the polyline
					segmentBearing = Direction(*pArcCenter, pts.begin()->XY()) + (TP2Chainage / *pRadius);
					tp2 = GenPolarPosRad(*pArcCenter, *pRadius, segmentBearing).VD3();
				} else
				{
					segmentBearing = Direction(start.XY(), end.XY());

					dTemp = dist - ((curChainage + dist) - TP2Chainage);

					tp2 = GenPolarPosRad(start.XY(), dTemp, segmentBearing).VD3();
				}
				next = pts.insert(next, tp2);
				itr = next;
				itr--;

				idx++;
				curChainage += dTemp;
				endPtIdx++;
				continue;
			}
		}

		curChainage += dist;
		idx++;
	}

	endPtIdx = pts.size() - 1;
	pts[TP1Idx].z = sGrade * TP1Chainage + sHeight;
	pts[TP2Idx].z = eGrade * -TP1Chainage + eHeight;
	midPointGrade = ((pts[TP2Idx].z - pts[TP1Idx].z) / midPointChainage);
	pts[midPointIdx].z = midPointGrade * TP1Chainage + pts[TP1Idx].z;

	// now we generate the curve
	double deltaGrade, instGrade, distRatio, yDefl;
	curChainage = 0;
	deltaGrade = midPointGrade - sGrade;
	for (idx = 1; idx <= midPointIdx; idx++)
	{
		VectorD3 &a = pts[idx-1];
		VectorD3 &b = pts[idx];

		curChainage += keays::math::Dist2D(a, b);
		distRatio = curChainage / midPointChainage;

		// G0   = G1 + DG*DR1 <-- instantaneous grade
		instGrade = sGrade + (deltaGrade * distRatio);

		// DEFL = HALF*(DG*DX1*DR1)	<-- deflection from
		yDefl = 0.5 * (deltaGrade * distRatio) * curChainage + (sGrade * curChainage);
		pts[idx].z = sHeight + yDefl;
	}
	curChainage = 0;
	deltaGrade = eGrade - midPointGrade;
	for (idx = midPointIdx+1; idx <= endPtIdx; idx++)
	{
		VectorD3 &a = pts[idx-1];
		VectorD3 &b = pts[idx];

		curChainage += keays::math::Dist2D(a, b);
		distRatio = curChainage / midPointChainage;

		// G0   = G1 + DG*DR1 <-- instantaneous grade
		instGrade = midPointGrade + (deltaGrade * distRatio);

		// DEFL = HALF*(DG*DX1*DR1)	<-- deflection from
		yDefl = 0.5 * (deltaGrade * distRatio) * curChainage + (midPointGrade * curChainage);
		pts[idx].z = pts[midPointIdx].z + yDefl;
	}

//	OutputDebugString("VerticalCurve - SUCCESS\n\n");

	return S_VC_SUCCESS;
}

//-----------------------------------------------------------------------------
const size_t INVALID_VC_INDEX = size_t(-1);
KEAYS_MATH_EXPORTS_API bool
VerticalCurve(const Polyline3D &source, Polyline3D *pDest, const double &startChainage, const double &endChainage,
			   const double &curveLength, const unsigned int flags)
{
	if (!pDest)
		return false;

	pDest->clear();

	if (source.size() < 3)
		return false;

	Polyline3D intermediate(source);
	Polyline3D::const_iterator itr(source.begin()),
							   nextItr(itr);
	nextItr++;

	double startChainageAdj = Max(startChainage, 0.0);
	bool needsEnd = true;
	double endChainageAdj = endChainage;
	double totalPolylineLength = Length2D(intermediate, intermediate.size(), true);
	if (endChainageAdj < startChainageAdj)
		needsEnd = false;

	size_t startIndex = 0;
	size_t endIndex = 0;
	if (!InsertPoint(&intermediate, startChainageAdj, &startIndex))
		return false;
	if (needsEnd)
	{
		if (!InsertPoint(&intermediate, endChainageAdj, &endIndex))
		{
			endIndex = intermediate.size()-1;
			endChainageAdj = totalPolylineLength;
		}
	} else
	{
		endIndex = intermediate.size()-1;
		endChainageAdj = totalPolylineLength;
	}

	// now we have start locations we can get the start and end grades
	itr = intermediate.begin();
	std::advance(itr, startIndex);
	nextItr = itr;
	double dist = 0.0;
	if (startIndex == 0)
	{
		// this is a special case so we take the grade FROM this point

		// advance until we find a pt at some distance
		nextItr++;
		while ((nextItr != intermediate.end()) && ((dist = Distance(*itr, *nextItr)) == 0))
			nextItr++;

		if (nextItr == intermediate.end())
			return false;
	} else
	{
		// take the grade to this point
		// retreat until we find a pt at some distance
		itr--;
		while ((itr != intermediate.end()) && ((dist = Distance(*itr, *nextItr)) == 0))
			itr--;

		if (itr == intermediate.end())
			return false;
	}
	VectorD3 startNormal((*nextItr - *itr).GetNormalised());
	double startGrade = startNormal.Grade();

	itr = intermediate.begin();
	std::advance(itr, endIndex);
	nextItr = itr;
	if (endIndex == intermediate.size()-1)
	{
		// this is a special case so we take the grade FROM this point
		// retreat until we find a pt at some distance
		itr--;
		while ((itr != intermediate.end()) && ((dist = Distance(*itr, *nextItr)) == 0))
			itr--;

		if (itr == intermediate.end())
			return false;
	} else
	{
		// take the grade to this point
		// advance until we find a pt at some distance
		nextItr++;
		while ((nextItr != intermediate.end()) && ((dist = Distance(*itr, *nextItr)) == 0))
			nextItr++;

		if (nextItr == intermediate.end())
			return false;
	}
	VectorD3 endNormal((*nextItr - *itr).GetNormalised());
	double endGrade = endNormal.Grade();

	// since we now have a start and end location as well as start and end grades we can determine if we need a single or double curve
	double totalCurveLen = endChainageAdj - startChainageAdj;
	double intersectDist = 0.0;
	double intersectRL = 0.0;
	double halfVCLen = curveLength * 0.5;

	if (totalCurveLen < 0.0)
		return false;

	bool isDoubleVC = false;
	if (totalCurveLen < curveLength)
	{
		// it's too short so it has to be a double vc
		isDoubleVC = true;
	} else
	{
		// x = (m2*p3_x + p3_y + m1*p1_x - p1_y) / (m1-m2)
		double denom = (startGrade - endGrade);

		if (denom == 0.0)
		{
			isDoubleVC = true;
		}
		else
		{
			// x = (y2 - y1 + m1*x1 - m2*x2) / (m1-m2)
			intersectDist = (intermediate[endIndex].z - intermediate[startIndex].z + (startGrade * startChainageAdj) - (endGrade * endChainageAdj));
			intersectDist /= denom;

			if ((Float::Greater(halfVCLen, 0.0)) && (Float::Less(intersectDist, (startChainageAdj + halfVCLen)) ||
				Float::Greater(intersectDist, (endChainageAdj - halfVCLen))))
			{
				isDoubleVC = true;
			} else
			{
				intersectRL = intermediate[startIndex].z + ((intersectDist - startChainageAdj) * startGrade);
			}
		}
	}

	size_t preIntermediateSize = intermediate.size();

	const VectorD3 startPt(intermediate[startIndex]);
	const VectorD3 endPt(intermediate[endIndex]);

	double chainage = 0;
	double deltaG = 0.0;
	double distRatio = 0.0;
	double Gx = 0.0;
	double y = 0.0;

	size_t index = 1;

	if (isDoubleVC)
	{
		size_t midTPIndex = INVALID_VC_INDEX;
		size_t TP1Index = INVALID_VC_INDEX;
		size_t TP2Index = INVALID_VC_INDEX;
		double vcLen = (endChainageAdj - startChainageAdj) * 0.5;
		double midTPChainage = (endChainageAdj + startChainageAdj) * 0.5;
		double qPointDistance = vcLen * 0.5;

		VectorD2 qp1(startChainageAdj + qPointDistance, intermediate[startIndex].z + qPointDistance * startGrade);
		VectorD2 qp2(endChainageAdj - qPointDistance, intermediate[endIndex].z + qPointDistance * -endGrade);

		double midGrade = (qp2.y - qp1.y) / (qp2.x - qp1.x);
		double midRL = (qp1.y + qp2.y) * 0.5;
		double deltaGrade1 = midGrade - startGrade;
		double deltaGrade2 = endGrade - midGrade;
		double TP1Dist = (-vcLen * startGrade) / deltaGrade1 + startChainageAdj;
		double TP2Dist = (-vcLen * midGrade) / deltaGrade2 + midTPChainage;

		if (TP1Dist > startChainageAdj && TP1Dist < midTPChainage)
		{
			if (!InsertPoint(&intermediate, TP1Dist, &TP1Index))
				return false;
			if (preIntermediateSize != intermediate.size())
				endIndex++;
		}

		if (!InsertPoint(&intermediate, midTPChainage, &midTPIndex))
			return false;
		if (preIntermediateSize != intermediate.size())
			endIndex++;

		VectorD3 midPoint(intermediate[midTPIndex]);

		if (TP2Dist > midTPChainage && TP2Dist < endChainageAdj)
		{
			if (!InsertPoint(&intermediate, TP2Dist, &TP2Index))
				return false;
			if (preIntermediateSize != intermediate.size())
				endIndex++;
		}

		// TODO:    radius eqn: vcLen / deltaGrade
		pDest->push_back(*intermediate.begin());
		itr = intermediate.begin();
		while (itr != intermediate.end() && chainage < totalPolylineLength)
		{
			const VectorD3 &pt = *itr;

			nextItr = itr;
			nextItr++;

			if (nextItr == intermediate.end())
			{
				// special case, just add the point and break;
				pDest->push_back(pt);
				break;
			} else
			{
				const VectorD3 &nextPt = *nextItr;

				double segGrade = (nextPt - pt).Grade();

				dist = Dist2D(pt, nextPt);

				if (index < startIndex)
				{
					// before the curve domain
					// just copy as is
					pDest->push_back(nextPt);
				} else if (index < midTPIndex)
				{
					// in the first curve (TP should by definition be in here if it is valid)
					// grade is based on distance from the start of the curve
					double x = chainage + dist - startChainageAdj;
					distRatio = x / vcLen;
					Gx = /*startGrade + */(distRatio * deltaGrade1);
					y = (0.5 * Gx * x) + (startGrade * x) + startPt.z;
					VectorD3 newPt(nextPt.XY().VD3(y));
					pDest->push_back(newPt);
				} else if (index == midTPIndex)
				{
					// special case - transition between the curves
					// just copy as is
					VectorD3 newPt(nextPt.XY().VD3(midRL));
					pDest->push_back(newPt);//nextPt);
				} else if (index < endIndex)
				{
					// in the second curve (TP should by definition be in here if it is valid)
					// grade is based on distance from the start of the curve
					double x = chainage + dist - midTPChainage;
					distRatio = x / vcLen;
					Gx = /*midGrade + */(distRatio * deltaGrade2);
					y = (0.5 * Gx * x) + (midGrade * x) + midRL;
					VectorD3 newPt(nextPt.XY().VD3(y));
					pDest->push_back(newPt);
				} else
				{
					// after the curve domain
					// just copy as is
					pDest->push_back(nextPt);
				}

				chainage += dist;
			}

			itr++;
			index++;
		}
	} else
	{
		double curveStartChainage = intersectDist - halfVCLen;
		double curveEndChainage = intersectDist + halfVCLen;
		double deltaG = endGrade - startGrade;
		if (deltaG == 0.0)
			return false;
		double TPDist = (-curveLength * startGrade) / deltaG + curveStartChainage;	// distance from the start of the curve

		size_t curveStartIndex = INVALID_VC_INDEX;
		size_t curveEndIndex = INVALID_VC_INDEX;
		size_t curveTPIndex = INVALID_VC_INDEX;

		if (!InsertPoint(&intermediate, curveStartChainage, &curveStartIndex))
			return false;
		if (preIntermediateSize != intermediate.size())
			endIndex++;

		if (TPDist > curveStartChainage && TPDist < curveEndChainage)
		{
			if (!InsertPoint(&intermediate, TPDist, &curveTPIndex))
				return false;
			if (preIntermediateSize != intermediate.size())
				endIndex++;
		}

		if (!InsertPoint(&intermediate, curveEndChainage, &curveEndIndex))
			return false;
		if (preIntermediateSize != intermediate.size())
			endIndex++;

		double startRL = (startPt.z + (startGrade * (curveStartChainage - startChainageAdj)));
		pDest->push_back(*intermediate.begin());
		itr = intermediate.begin();
		while (itr != intermediate.end() && chainage < totalPolylineLength)
		{
			const VectorD3 &pt = *itr;

			nextItr = itr;
			nextItr++;

			if (nextItr == intermediate.end())
			{
				// special case, just add the point and break;
				pDest->push_back(pt);
				break;
			} else
			{
				const VectorD3 &nextPt = *nextItr;

				double segGrade = (nextPt - pt).Grade();

				dist = Dist2D(pt, nextPt);

				if (index < startIndex)
				{
					// before the curve domain
					// just copy as is
					pDest->push_back(nextPt);
				} else if (index < curveStartIndex)
				{
					// before the curve
					// RL's should be a constant grade from the start point
					VectorD3 newPt(nextPt.XY().VD3(startPt.z + (startGrade * (chainage + dist - startChainageAdj))));
					pDest->push_back(newPt);
				} else if (index < curveEndIndex)
				{
					// in the curve (TP should be definition be in here if it is valid)
					// grade is based on distance from the start of the curve
					/*
					deltaG = G2 - G1;

					distRatio = x / L;
					Gx = G1 + (distRatio * deltaG);
					y = (0.5 * Gx * x) + (G1 * x) + y1;

			  		yDeflection = 0.5 * (deltaGrade * distRatio) * dist + (startGrade * dist);

					TPx = (-L * G1) / deltaG;
					*/

					double x = chainage + dist - curveStartChainage;
					distRatio = x / curveLength;
					Gx = /*startGrade + */(distRatio * deltaG);
					y = (0.5 * Gx * x) + (startGrade * x) + startRL;
					VectorD3 newPt(nextPt.XY().VD3(y));
					pDest->push_back(newPt);
				} else if (index < endIndex)
				{
					// after the curve
					// RL should be a constant grade (-endGrade) from the end point
					VectorD3 newPt(nextPt.XY().VD3(endPt.z + (-endGrade * (endChainageAdj - (chainage + dist)))));
					pDest->push_back(newPt);
				} else
				{
					// after the curve domain
					// just copy as is
					pDest->push_back(nextPt);
				}

				chainage += dist;
			}

			itr++;
			index++;
		}
	}

	return true;
}

//--- DRAWING GENERATION FUNCTIONS ---
//--------------------------------------------------------------
KEAYS_MATH_EXPORTS_API const double
AngleFromChordLength(const double &radius, const double &chordLen, const eAngleDirections direction /*= CCW*/,
					  const double &startAngle /*= 0*/, const double &endAngle /*= KM_2PI*/, const bool evenNumDivs /*= true*/)
{
	double includedAngle;
	double arcLen;
	double noDivisions;
	double dir = (direction == CCW ? 1 : -1);

	double s, e;

	s = startAngle;
	e = endAngle;

	if (direction != CCW)
	{
		if (s < 0)
			s += KM_2PI;
		else if (e > (s + KM_2PI))
			e -= KM_2PI;
	} else
	{
		if (s > (e + KM_2PI))
			s -= KM_2PI;
		else if (e < 0)
			e += KM_2PI;
	}

	includedAngle = (e - s);
	if (includedAngle < 0)
		includedAngle = -includedAngle;
	arcLen = includedAngle * radius;

	if (chordLen < arcLen)
	{
		if (evenNumDivs)
		{
			noDivisions = (int)ceil(arcLen / chordLen);
		} else
		{
			noDivisions = (int)floor(arcLen / chordLen);
		}
	} else
	{
		noDivisions = 1;
	}

	return includedAngle / noDivisions;
}

//--------------------------------------------------------------
/* // -------------- GetIntervalFromWidth BROKEN --------------
// TODO: Fix this properly
//--------------------------------------------------------------
// Generate an angular interval (in radians), based on viewport width
KEAYS_MATH_EXPORTS_API const float GetIntervalFromWidth(const float width)
{
	//float mult;
	if (width > 300)
	{
		return DTRF(15.0f);
	}
	else if (width > 100)
	{
		//mult = (float)((int)width / 100);
		return DTRF(Max(((float)width / 100.0f) * 5.0f, 1.0f));
	}
	else if (width > 10)
	{
		//mult = (float)((int)width / 100);
		return DTRF(Max(((float)width / 100.0f) * 0.5f, 1.0f));
	}
	else
	{
		return DTRF(1.0f);
	}
}
*/

//--------------------------------------------------------------
void SortAngleList(double *importantAngleList, const int numImportantAngles,
				    const double &startAngleRad /*= 0*/, const eAngleDirections direction /*= CCW*/)
{
	int idx;
	double stAng;

	if (!importantAngleList || !numImportantAngles)
		return;

	stAng = GetAbsoluteAngle(startAngleRad);
	//adjust the angles for the start angle
	for (idx = 0; idx < numImportantAngles; ++idx)
	{
		importantAngleList[idx] = GetAbsoluteAngle(importantAngleList[idx]);
		if (direction == CW)
		{
			// any angle greater than start must be reduced by KM_2PI
			if (importantAngleList[idx] > stAng)
				importantAngleList[idx] -= KM_2PI;
		} else if (direction == CCW)
		{
			// any angle less than start must be increased by KM_2PI
			if (importantAngleList[idx] < stAng)
				importantAngleList[idx] += KM_2PI;
		}
	}

	if (direction == CCW)
		qsort((void *)importantAngleList, (size_t)numImportantAngles, sizeof(double), &CompareAngCCW);
	else
		qsort((void *)importantAngleList, (size_t)numImportantAngles, sizeof(double), &CompareAngCW);
}

const double DEFAULT_INTERVAL_DEG = 5.0;	//!< A default interval angle in degrees
const double DEFAULT_INTERVAL_RAD = DTR(DEFAULT_INTERVAL_DEG); //!< A default interval angle in radians
const double DEFAULT_INTERVAL_LEN = 3;		//!< A default interval chord length

//--------------------------------------------------------------
KEAYS_MATH_EXPORTS_API bool
ParallelPolylineOffset(const Polyline3D &points, Polyline3D &result,
					    const double &distance, const eSideSelections side, const double &zDistance, bool closed /*= false*/)
{
	if (points.size() < 2)
		return false; // need at least 2 points

	using namespace keays::types;

	size_t resultSize = result.size();

	Polyline3D::const_iterator  ptItr = points.begin(),
								nextPtItr;

	vector<Line> offsets;

	bool skipped = false;
	do
	{
		nextPtItr = ptItr;
		nextPtItr++;

		if ((nextPtItr == points.end()) || (ptItr == points.end()))
		{
			break;
		}

		const VectorD3 &pt = *ptItr;
		const VectorD3 &nextPt = *nextPtItr;

		if (Float::LessOrEqual(Dist2D(pt.XY(), nextPt.XY()), 0.0))
		{
			ptItr++;
			continue;
		}

		Line ln = Offset(pt.XY(), nextPt.XY(), distance, side);
		ln.start.z = pt.z + zDistance;
		ln.end.z = nextPt.z + zDistance;

		offsets.push_back(ln);

		// advance ptItr, nextPtItr is adjusted as required.  This enables bypassing of duplicate points
		ptItr++;
	} while (1);

	if (offsets.size() < 1)
		return false;

    vector<Line>::const_iterator segItr = offsets.begin();
	vector<Line>::const_iterator nextSegItr = segItr;
	nextSegItr++;

	result.push_back(segItr->start);

	double rl;
	int rc;
	VectorD2 intersectPoint;

	for (; nextSegItr != offsets.end(); segItr++, nextSegItr++)
	{
		rc = LineLineIntersect(*segItr, *nextSegItr, intersectPoint);

		if (rc == S_INTERSECT)
		{
			rl = 0;

			if (segItr->GetPointHeight(intersectPoint.VD3(), rl))
			{
				result.push_back(intersectPoint.VD3(rl));
			}
		}
		else if (rc == ES_LINES_IN_LINE)
		{
			result.push_back(segItr->end);
		}
	}

	if (closed)
	{
		segItr = offsets.begin();
		vector<Line>::reverse_iterator endSegItr = offsets.rbegin();

		rc = LineLineIntersect(*segItr, *endSegItr, intersectPoint);

		if (rc == S_INTERSECT)
		{
			rl = 0;

			if (endSegItr->GetPointHeight(intersectPoint.VD3(), rl))
			{
				result.push_back(intersectPoint.VD3(rl));
				result[resultSize] = intersectPoint.VD3(rl);
			}
		}
		else if (rc == ES_LINES_IN_LINE)
		{
			result.push_back(endSegItr->end);

			result[resultSize] = endSegItr->end;
		}
	}
	else
	{
		result.push_back(offsets.rbegin()->end);
	}

	//return true;
	return (result.size()-resultSize) >= 2;
}
/*
{
	if (points.size() < 2)
		return false;	// need at least 2 points

	typedef Polyline3D VD3Vector;

	VD3Vector					tmpVec;
	VD3Vector::const_iterator	it;
	VectorD3 *s1, *s2;
	VectorD3 tempStore;
	Line l;
	unsigned int cnt = 1;

	it = points.begin();
	tmpVec.push_back(*it);
	it++;

	// Loop through the specified points, generating the parallel offset and
	// also doubling every point but the first & last. This is required
	// to generate intersection points in the loop below.
	for (; it != points.end(); it++)
	{
		tmpVec.push_back(*it);

		s2 = &(tmpVec.at(tmpVec.size() - 1));
		s1 = &(tmpVec.at(tmpVec.size() - 2));

		l = Offset(	VectorD2(s1->x, s1->y), VectorD2(s2->x, s2->y), distance, side);
		if ((s1->x == s2->x) && (s1->y == s2->y) && (s1->z == s2->z))
		{
			cnt++;
			continue;
		}

 		if (((cnt == points.size() - 1) && (points.size() > 2)	// handle special cases
			   || (cnt == points.size() && cnt == 3)))
		{
			s1->x = l.start.x;
			s1->y = l.start.y;

			s2->x = l.end.x;
			s2->y = l.end.y;
		}
		else	// general case
		{
			tempStore = *s2;

			s1->x = l.start.x;
			s1->y = l.start.y;

			s2->x = l.end.x;
			s2->y = l.end.y;

			if (points.size() > 2)
			{
				tmpVec.push_back(tempStore);
			}
		}

		cnt++;
	} // for (; it != points.end(); it++)

	// add in the start point
	result.push_back(tmpVec.at(0));

	VD3Vector::iterator	sIt1, sIt2, sIt3, sIt4;
	// Loop through performing a Line-Line intersection for each pair of points
	// and adding these to the result.
	cnt = 2;
	if (tmpVec.size() >= 4)
	{
		sIt1 = sIt2= sIt3 = sIt4 = tmpVec.begin();
		sIt2++;
		sIt3++; sIt3++;
		sIt4++; sIt4++; sIt4++;
		VectorD2 secPoint;

		while (cnt < tmpVec.size())
		{
			LineLineIntersect((*sIt1).XY(), (*sIt2).XY(),	// line #1
									 (*sIt3).XY(), (*sIt4).XY(),	// line #2
									 secPoint);

			double z = ((*sIt2).z + (*sIt3).z) / 2.0;
			result.push_back(VectorD3(secPoint.x, secPoint.y, z+zDistance));

			sIt1++; sIt1++;
			sIt2++; sIt2++;
			sIt3++; sIt3++;
			sIt4++; sIt4++;
			cnt += 2;
		}
	}

	// add in the end point
	result.push_back(tmpVec.at(tmpVec.size()-1));

	return true;
}
*/

//--------------------------------------------------------------
/*
KEAYS_MATH_EXPORTS_API bool
ParallelPolylineOffset(const Polyline2D &points, Polyline2D &result,
					    const double distance, const int side)
{
	if (points.size() < 2) 	// need at least 2 points
		return false;

	typedef Polyline2D VD2Vector;

	VD2Vector					tmpVec;
	VD2Vector::const_iterator	it;
	VectorD2 *s1, *s2;
	VectorD2 tempStore;
	Line l;
	unsigned int cnt = 1;

	it = points.begin();
	tmpVec.push_back(*it);
	it++;

	// Loop through the specified points, generating the parallel offset and
	// also doubling every point but the first & last. This is required
	// to generate intersection points in the loop below.
	for (; it != points.end(); it++)
	{
		tmpVec.push_back(*it);

		s2 = &(tmpVec.at(tmpVec.size() - 1));
		s1 = &(tmpVec.at(tmpVec.size() - 2));

		l = Offset(	VectorD2(s1->x, s1->y),
					VectorD2(s2->x, s2->y),
					distance, side);

 		if (((cnt == points.size() - 1) && (points.size() > 2)	// handle special cases
			   || (cnt == points.size() && cnt == 3)))
		{
			s1->x = l.start.x;
			s1->y = l.start.y;

			s2->x = l.end.x;
			s2->y = l.end.y;
		}
		else	// general case
		{
			tempStore = *s2;

			s1->x = l.start.x;
			s1->y = l.start.y;

			s2->x = l.end.x;
			s2->y = l.end.y;

			if (points.size() > 2)
			{
				tmpVec.push_back(tempStore);
			}
		}

		cnt++;
	} // for (; it != points.end(); it++)

	// add in the start point
	result.push_back(tmpVec.at(0));

	VD2Vector::iterator			sIt1,
								sIt2,
								sIt3,
								sIt4;
	// Loop through performing a Line-Line intersection for each pair of points
	// and adding these to the result.
	cnt = 2;
	if (tmpVec.size() >= 4)
	{
		sIt1 = sIt2= sIt3 = sIt4 = tmpVec.begin();
		sIt2++;
		sIt3++; sIt3++;
		sIt4++; sIt4++; sIt4++;
		VectorD2 secPoint;

		while (cnt < tmpVec.size())
		{
			LineLineIntersect(VectorD2((*sIt1).x, (*sIt1).y),
									 VectorD2((*sIt2).x, (*sIt2).y),	// line #1
									 VectorD2((*sIt3).x, (*sIt3).y),
									 VectorD2((*sIt4).x, (*sIt4).y),	// line #2
									 secPoint);

			result.push_back(secPoint);

			sIt1++; sIt1++;
			sIt2++; sIt2++;
			sIt3++; sIt3++;
			sIt4++; sIt4++;
			cnt += 2;
		}
	}

	// add in the end point
	result.push_back(tmpVec.at(tmpVec.size()-1));

	return true;
}*/

//-----------------------------------------------------------------------------
KEAYS_MATH_EXPORTS_API const Polyline3D
RemoveDuplicates(const Polyline3D &sourcePolyline, const double &tolerance /*= Float::TOLERANCE*/ )
{
	Polyline3D result;
	result.assign(sourcePolyline.begin(), sourcePolyline.end());

	return *RemoveDuplicates(&result);
}

//-----------------------------------------------------------------------------
KEAYS_MATH_EXPORTS_API const Polyline3D *
RemoveDuplicates(Polyline3D *sourcePolyline, const double &tolerance /*= Float::TOLERANCE*/ )
{
	if (sourcePolyline)
	{
		Polyline3D::iterator i = sourcePolyline->begin();

		for (; i != sourcePolyline->end(); i++)
		{
			Polyline3D::iterator next = i;
			next++;
			while (i != sourcePolyline->end() && next != sourcePolyline->end() &&
				Float::LessOrEqual(Dist2D(*i, *next), 0.0, tolerance))
			{
				i = sourcePolyline->erase(i);
				next = i;
				next++;
			}
		}
	}

	return sourcePolyline;
}

//-----------------------------------------------------------------------------
double PerpTestData::ms_tolerance = Float::TOLERANCE;

//-----------------------------------------------------------------------------
KEAYS_MATH_EXPORTS_API bool
GetNearestPoint(const Polyline3D &testPolyline, const VectorD2 &testPoint,
				 VectorD3 *pPerpPoint, double *pChainage /*= NULL*/,  double *pPerpDist /*= NULL*/,
				 bool interpolateAtEnd /*= true*/, std::list<PerpTestData> *pTestData /*= NULL*/,
				 const double &tolerance /*= Float::TOLERANCE*/)
{
	if (testPolyline.size() < 2)
		return false;

	if (!pPerpPoint)
		return false;

	Polyline3D::const_iterator i = testPolyline.begin();
	Polyline3D::const_iterator prev = i;
	Polyline3D::const_iterator next = i;
	next++;

	list< PerpTestData > testData;

	double chainage = 0;
	double perpDist = 0;

	bool firstSegment = true;
	bool lastSegment = false;

	VectorD3 testPoint3D = testPoint;
	VectorD3 lastPoint;

	for (int index = 0; next != testPolyline.end(); i++, next++, index++)
	{
		if (index == testPolyline.size() - 2)
		{
			lastSegment = true;
		}

		VectorD3 start(i->x, i->y, i->z);
		VectorD3 end(next->x, next->y, next->z);

		VectorD2 resultPoint;

		if (S_INTERSECT == GetPerpendicularIntersect(start.XY(), end.XY(), testPoint, resultPoint))
		{
			VectorD2 a = (resultPoint - start.XY()).GetNormalised();
			VectorD2 b = (end.XY() - resultPoint).GetNormalised();
			VectorD2 c = (testPoint - start.XY()).GetNormalised();

			if (a == b)
			{
				VectorD3 resultVec = resultPoint.VD3(start.z) - start;

				double segmentLength = Dist2D(end.XY(), start.XY());

				if (segmentLength == 0.0)
					continue;

				double startPointLength = Dist2D(resultPoint, start.XY());
				double distFactor = startPointLength / segmentLength;

				VectorD3 result3D(resultPoint.VD3(start.z + (end.z - start.z) * distFactor));

				perpDist = abs(GetPerpendicularDist(start.XY(), end.XY(), testPoint));

				double pointChainage = chainage + Dist2D(start, result3D);

				testData.push_back(PerpTestData(index, result3D, pointChainage, perpDist, true));
			} else
			{
				bool resolved = false;

				if (interpolateAtEnd)
				{
					if (firstSegment)
					{
						VectorD2 firstA = end.XY() - start.XY();
						VectorD2 firstB = testPoint - start.XY();

						if (Dot(firstA.Normalise(), firstB.Normalise()) < 0.0)
						{
							resolved = true;

							VectorD3 resultVec = resultPoint.VD3(start.z) - start;

							double segmentLength = Dist2D(end.XY(), start.XY());

							if (segmentLength == 0.0)
								continue;

							double startPointLength = -Dist2D(resultPoint, start.XY());
							double distFactor = startPointLength / segmentLength;

							VectorD3 result3D(resultPoint.VD3(start.z + (end.z - start.z) * distFactor));

							perpDist = abs(GetPerpendicularDist(start.XY(), end.XY(), testPoint));

							double pointChainage = chainage + Dist2D(start, result3D);

							testData.push_back(PerpTestData(index, result3D, pointChainage, perpDist, false));
						}
					}
					else if (lastSegment)
					{
						VectorD2 lastA = start.XY() - end.XY();
						VectorD2 lastB = testPoint - end.XY();

						if (Dot(lastA.Normalise(), lastB.Normalise()) < 0.0)
						{
							resolved = true;

							VectorD3 resultVec = resultPoint.VD3(start.z) - start;

							double segmentLength = Dist2D(end.XY(), start.XY());

							if (segmentLength == 0.0)
								continue;

							double startPointLength = Dist2D(resultPoint, start.XY());
							double distFactor = startPointLength / segmentLength;

							VectorD3 result3D(resultPoint.VD3(start.z + (end.z - start.z) * distFactor));

							perpDist = abs(GetPerpendicularDist(start.XY(), end.XY(), testPoint));

							double pointChainage = chainage + Dist2D(start, result3D);

							testData.push_back(PerpTestData(index, result3D, pointChainage, perpDist, false));
						}
					}
				}

				if (!resolved)
				{
					VectorD2 lastA = start.XY() - end.XY();
					VectorD2 lastB = testPoint - end.XY();

					if (lastSegment && Dot(lastA.Normalise(), lastB.Normalise()) < 0.0)
					{
						perpDist = Dist2D(end.XY(), testPoint);

						testData.push_back(PerpTestData(index, end, chainage, perpDist, false));
					}
					else
					{
						VectorD2 prevSegment(start.XY() - VectorD2(prev->x, prev->y));
						VectorD2 nextSegment(end.XY() - start.XY());

						prevSegment.Normalise();
						nextSegment.Normalise();

						double cosPrevC = Dot(prevSegment.VD3(), c.VD3());
						double cosNextC = Dot(nextSegment.VD3(), c.VD3());

						if ((cosPrevC >= 0) && (cosNextC <= 0))
						{
							perpDist = Dist2D(start.XY(), testPoint);

							testData.push_back(PerpTestData(index, start, chainage, perpDist, false));
						}
					}
				}
			}
				} // end if (S_INTERSECT == GetPerpendicularIntersect(start.XY(), end.XY(), testPoint, resultPoint)) else

		chainage += Dist2D(start, end);

		if (!firstSegment)
		{
			lastPoint = start;
			prev++;
		}

		firstSegment = false;
		} // end for (int index = 0; next != testPolyline.end(); i++, next++, index++)

	if (testData.size() < 1)
		return false;

	double oldTolerance = PerpTestData::ms_tolerance;
	PerpTestData::ms_tolerance = tolerance; // just in case we store the old tolerance value
	testData.sort();
	PerpTestData::ms_tolerance = oldTolerance;

	*pPerpPoint = testData.begin()->m_intersectPoint;

	if (pChainage)
		*pChainage = testData.begin()->m_chainage;

	if (pPerpDist)
		*pPerpDist = testData.begin()->m_distance;

	if (pTestData)
	{
		pTestData->clear();
		pTestData->assign(testData.begin(), testData.end());
	}

	return true;
}

}	// namespace keays
}	// namespace math
