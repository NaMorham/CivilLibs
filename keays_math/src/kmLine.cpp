/*
 * Filename: kmLine.cpp
 * Date: May 2005
 * Authors: Andrew Hickey, Keays Software
 *
 * Contains implementations of the Line Class in the geomery.h file.
 *
 * Part of the keays::maths namespace
 */

#include <assert.h>

#include "..\include\geometry.h"

#include <LeakWatcher.h>

#ifdef _DO_MEMORY_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning(disable : 4786) // ignore the long name warning associated with stl stuff

using namespace std;		// to get access to std:: stuff

namespace keays
{
namespace math
{

//--------------------------------------------------------------
Line::Line()
{
	start = VectorD3(0,0,0);
	end = VectorD3(1,0,0);
}

//--------------------------------------------------------------
Line::Line(const Line &ln)
{
	start = ln.start;
	end = ln.end;
}

//--------------------------------------------------------------
Line::Line(const VectorD2 &st, const VectorD2 &en)
{
	start = st;
	end = en;
}

//--------------------------------------------------------------
Line::Line(const VectorD3 &st, const VectorD3 &en)
{
	start = st;
	end = en;
}

//--------------------------------------------------------------
const Line &Line::operator=(const Line &rhs)
{
	start = rhs.start;
	end = rhs.end;
	return *this;
}

//--------------------------------------------------------------
Line::operator VectorD3() const
{
	return end - start;
}

//--------------------------------------------------------------
Line::operator VectorD2() const
{
	return VectorD2(end.x - start.x, end.y - start.y);
}

//--------------------------------------------------------------
const double Line::GetLength() const
{
	return Distance(start, end);
}

//--------------------------------------------------------------
const double Line::GetLength2D() const
{
	return Dist2D(start, end);
}

//--------------------------------------------------------------
const Line &Line::ExtendEnd(const double &newLength)
{
	if (newLength == 0.0)
	{
		end = start;
	} else
	{
		end = start + ((end-start).GetNormalised() * newLength);
	}
	return *this;
}

//--------------------------------------------------------------
const Line &Line::ExtendStart(const double &newLength)
{
	if (newLength == 0.0)
	{
		start = end;
	} else
	{
		start = end + ((start - end).GetNormalised() * newLength);
	}
	return *this;
}

//--------------------------------------------------------------
const Line &Line::ExtendMid(const double &newLength)
{
	keays::types::VectorD3 mid = GetMidPoint();
	if (newLength == 0.0)
	{
		end = start = mid;
	} else
	{
		double halfLen = newLength / 2.0;
		VectorD3 adjust = (end - start).GetNormalised() * halfLen;
		start = mid - adjust;
		end = mid + adjust;
	}
	return *this;
}

//--------------------------------------------------------------
const Line &Line::ExtendEnd2D(const double &newLength)
{
	if (newLength == 0.0)
	{
		end = start;
	} else
	{
		double grade = GetGrade();
		end = start + ((end - start).XY().GetNormalised() * newLength).VD3(grade * newLength);
	}
	return *this;
}

//--------------------------------------------------------------
const Line &Line::ExtendStart2D(const double &newLength)
{
	if (newLength == 0.0)
	{
		start = end;
	} else
	{
		double grade = GetGrade();
		start = end + ((start - end).XY().GetNormalised() * newLength).VD3(grade * newLength);
	}
	return *this;
}

//--------------------------------------------------------------
const Line &Line::ExtendMid2D(const double &newLength)
{
	keays::types::VectorD3 mid = GetMidPoint();
	if (newLength == 0.0)
	{
		start = end = mid;
	} else
	{
		double halfLen = newLength / 2.0;
		double grade = GetGrade();
		keays::types::VectorD3 adjust = ((end - start).XY().GetNormalised() * halfLen).VD3(grade * halfLen);
		start = mid - adjust;
		end = mid + adjust;
	}
	return *this;
}

//--------------------------------------------------------------
const double Line::GetBearing() const
{
	return Direction(start, end);
}

//--------------------------------------------------------------
const double Line::GetZenith() const
{
	return keays::math::Zenith(start, end);
}

//--------------------------------------------------------------
const double Line::GetGrade() const
{
	return keays::math::Grade(start, end);
}

//--------------------------------------------------------------
const VectorD3 Line::GetMidPoint() const
{
	double x, y, z;
	x = (end.x + start.x) / 2;
	y = (end.y + start.y) / 2;
	z = (end.z + start.z) / 2;
	return VectorD3(x, y, z);
}

//-----------------------------------------------------------------------------
bool Line::PointOnLine(const VectorD2 &pt, const double &tolerance) const
{
	VectorD2 rEnd, rPt;

	rEnd = RotatePoint(end.XY());
	rPt = RotatePoint(pt);

	if (Float::EqualTo((rPt.y - pt.y), 0.0, tolerance))
		return true;
	else
		return false;
}

//--------------------------------------------------------------
bool Line::PointOnSegment(const VectorD2 &pt, const double &tolerance) const
{
	VectorD2 rEnd, rPt;

	rEnd = RotatePoint(end.XY());
	rPt = RotatePoint(pt);

	if (Float::EqualTo((rPt.y - start.y), 0.0, tolerance))
	{
		if (Float::GreaterOrEqual(rPt.x, start.x, tolerance) && Float::LessOrEqual(rPt.x, rEnd.x, tolerance))
			return true;
	}

	return false;
}

//--------------------------------------------------------------
bool Line::PointNear(const VectorD2 &pt, double &distance, const double &tolerance) const
{
	const VectorD2 rPt = RotatePoint(pt);

	return (Float::LessOrEqual((rPt.y - start.y), distance, tolerance));
}

//--------------------------------------------------------------
bool Line::PointNearSegment(const VectorD2 &pt, double &distance, const double &tolerance) const
{
	const VectorD2 rPt = RotatePoint(pt);
	const VectorD2 rEnd = RotatePoint(pt);

	if (Float::LessOrEqual((rPt.y - start.y), distance, tolerance))
	{
		if (Float::GreaterOrEqual(rPt.x, start.x) && Float::LessOrEqual(rPt.x, rEnd.x, tolerance))
		{
			return true;
		} else
		{
			// it might be near the ends
			const double sDistance = Dist2D(start.XY(), pt);
			const double eDistance = Dist2D(end.XY(), pt);
			return (Float::LessOrEqual(sDistance, distance, tolerance) || Float::LessOrEqual(eDistance, distance, tolerance));
		}
	}
	return false;
}

//--------------------------------------------------------------
bool Line::GetPointHeight(const VectorD3 &pt, double &height) const
{
	if (GetLength2D() == 0.0) // we are vertical
	{
		height = ((end.z - start.z) / 2.0) + start.z;
		return true;
	}

	// TODO: redo this using just vector maths
	double grade = 0.0;
	double lineBearing, ptStartBearing;

	grade = GetGrade();
	lineBearing = GetBearing();
	ptStartBearing = Direction(pt.XY(), start.XY());

	int result = AngleIsClockwiseEx(lineBearing, ptStartBearing);

	if (result == ANGLE_EQUAL)
		grade = -1.0 * grade;
	else if (result != ANGLE_OPPOSITE)
		return false;

	double dist = Dist2D(pt, start);

	height = (grade * dist) + start.z;

	return true;
}


//--------------------------------------------------------------
const VectorD2 Line::RotatePoint(const VectorD2 &pt) const
{
	return RotatePointXY(start.XY(), end.XY(), pt);
}

//--------------------------------------------------------------
const VectorD2 Line::UnrotatePoint(const VectorD2 &pt) const
{
	return UnRotatePointXY(start.XY(), end.XY(), pt);
}

//--------------------------------------------------------------
const Line Line::CalcOffset(const double &dist, const eSideSelections side)
{
	if (side == SIDE_NONE)
		return *this;

	VectorD2 perp((end - start).GetNormalised().XY().GetPerpendicular(side == SIDE_RIGHT) * dist);

	return Line(start + perp, end + perp);
}

//--------------------------------------------------------------
const Line Line::CalcOffset(const double &dist, const VectorD2 &pt)
{
	double testDist;

	testDist = GetPerpendicularDist(start.XY(), end.XY(), pt);
	if (0 == testDist)
		return *this;
	else if (0 < testDist)
		return CalcOffset(dist, SIDE_RIGHT);
	else
		return CalcOffset(dist, SIDE_LEFT);
}

}	// namespace keays
}	// namespace math
