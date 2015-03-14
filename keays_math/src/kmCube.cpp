/*
 * Filename: kmCube.cpp
 * Date: May 2005
 * Authors: Andrew Hickey, Keays Software
 *
 * Contains implementations of the RectD and Cube classes in the geomery.h file.
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

//-----------------------------------------------------------------------------
RectD::RectD(const double &left /*= -Float::INVALID_DOUBLE*/, const double &right /*= Float::INVALID_DOUBLE*/, 
			  const double &top /*= Float::INVALID_DOUBLE*/, const double &bottom /*= -Float::INVALID_DOUBLE*/)
			  : m_left(left), m_right(right), m_top(top), m_bottom(bottom), 
			    m_numIncludes(0) {}

//-----------------------------------------------------------------------------
RectD::RectD(const VectorD2 &min, const VectorD2 &max) 
			  : m_left(min.x), m_right(max.x), m_top(max.y), m_bottom(min.y), m_numIncludes(0) {}

//-----------------------------------------------------------------------------
RectD::RectD(const VectorD2 &min, const double &width, const double &height)
			  : m_left(min.x), m_right(m_left + fabs(width)), 
			    m_bottom(min.y), m_top(m_bottom + fabs(height)),
				m_numIncludes(0) {}

//-----------------------------------------------------------------------------
RectD::RectD(const RectD &old) 
			  : m_left(old.GetLeft()), m_right(old.GetRight()), 
			    m_top(old.GetTop()), m_bottom(old.GetBottom()),
				m_numIncludes(0) {}

//-----------------------------------------------------------------------------
const double RectD::SetLeft(const double &left) 
{ 
	m_left = left; 
	return m_left; 
}

//-----------------------------------------------------------------------------
const double RectD::SetRight(const double &right) 
{ 
	m_right = right; 
	return m_right; 
}

//-----------------------------------------------------------------------------
const double RectD::SetBottom(const double &bottom) 
{ 
	m_bottom = bottom; 
	return m_bottom; 
}

//-----------------------------------------------------------------------------
const double RectD::SetTop(const double &top) 
{ 
	m_top = top; 
	return m_top; 
}

//-----------------------------------------------------------------------------
const RectD RectD::MakeInvalid()
{
	RectD old = *this;
	m_left = -Float::INVALID_DOUBLE; m_right =   Float::INVALID_DOUBLE;
	m_top =   Float::INVALID_DOUBLE; m_bottom = -Float::INVALID_DOUBLE;
	return old;
}

//-----------------------------------------------------------------------------
bool RectD::IsValid() const
{
	return (m_right >= m_left) && (m_top >= m_bottom);
}

//-----------------------------------------------------------------------------
const double RectD::GetWidth() const		
{ 
	return fabs(m_right - m_left); 
}

//-----------------------------------------------------------------------------
const double RectD::GetHeight() const		
{ 
	return fabs(m_top - m_bottom); 
}

//-----------------------------------------------------------------------------
const double RectD::GetCenterX() const		
{ 
	return m_left + GetWidth()/2; 
}

//-----------------------------------------------------------------------------
const double RectD::GetCenterY() const		
{ 
	return m_bottom + GetHeight()/2; 
}

//-----------------------------------------------------------------------------
const VectorD2 RectD::GetCenter() const	
{ 
	return VectorD2(GetCenterX(), GetCenterY()); 
}

//-----------------------------------------------------------------------------
const VectorD2 RectD::GetMin() const
{
	return VectorD2(m_right, m_top);
}

//-----------------------------------------------------------------------------
const VectorD2 RectD::GetMax() const
{
	return VectorD2(m_left, m_bottom);
}

//-----------------------------------------------------------------------------
const double RectD::CalcArea() const
{
	if (!IsValid())
		return 0.0;

	return GetWidth() * GetHeight();
}

//-----------------------------------------------------------------------------
void RectD::IncludePoint(const double &x, const double &y)
{
	if (x < m_left)	m_left = x;
	if (x > m_right)	m_right = x;
	if (y > m_top)		m_top = y;
	if (y < m_bottom)	m_bottom = y;
	m_numIncludes++;
}

//-----------------------------------------------------------------------------
void RectD::IncludePoint(const VectorD2 &pt)
{
	if (pt.x < m_left)		m_left = pt.x;
	if (pt.x > m_right)		m_right = pt.x;
	if (pt.y > m_top)		m_top = pt.y;
	if (pt.y < m_bottom)	m_bottom = pt.y;
	m_numIncludes++;
}

//-----------------------------------------------------------------------------
bool RectD::PointInside(const double &x, const double &y, const double &tolerance /*= Float::TOLERANCE*/) const
{
	return ((Float::GreaterOrEqual(x, m_left, tolerance) && Float::LessOrEqual(x, m_right, tolerance)) &&
			 (Float::GreaterOrEqual(y, m_bottom, tolerance) && Float::LessOrEqual(y, m_top, tolerance)));
}

//-----------------------------------------------------------------------------
const RectD &RectD::operator=(const RectD &rhs)
{ 
	m_left = rhs.m_left;	m_right = rhs.m_right; 
	m_top = rhs.m_top;		m_bottom = rhs.m_bottom; 
	return *this; 
}

//-----------------------------------------------------------------------------
void RectD::SetWidth(const double &d)		
{ 
	m_right = m_left + d; 
}

//-----------------------------------------------------------------------------
void RectD::SetHeight(const double &d)		
{ 
	m_top = m_bottom + d; 
}

//-----------------------------------------------------------------------------
void RectD::SetX(const double &d) { double w = fabs(GetWidth()); m_left = d; SetWidth(w); }

//-----------------------------------------------------------------------------
void RectD::SetY(const double &d) { double h = fabs(GetHeight()); m_bottom = d; SetHeight(h); }

//-----------------------------------------------------------------------------
void RectD::Expand(const double &left, const double &right, const double &top, const double &bottom)
{
	m_left   -= left;
	m_right  += right;
	m_bottom -= bottom;
	m_top    += top;
}

//-----------------------------------------------------------------------------
void RectD::Expand(const double &width, const double &height)
{
	double exp;
	exp =  width / 2.0; m_left   -= exp; m_right += exp;
	exp = height / 2.0; m_bottom -= exp; m_top   += exp;
}
//-----------------------------------------------------------------------------
void RectD::Expand(const double &expansion)
{
	double exp = expansion / 2.0;
	m_left -= exp;
	m_right += exp;
	m_bottom -= exp; 
	m_top += exp;
}

//=============================================================================
Cube::Cube(const double &left /*= -Float::INVALID_DOUBLE*/, const double &right /*=  Float::INVALID_DOUBLE*/,
		    const double &top /*=  Float::INVALID_DOUBLE*/, const double &bottom /*= -Float::INVALID_DOUBLE*/,
			const double &base /*= -Float::INVALID_DOUBLE*/, const double &roof /*=  Float::INVALID_DOUBLE*/)
			: RectD(left, right, top, bottom), 
			  m_base(base), m_roof(roof) {}

//-----------------------------------------------------------------------------
Cube::Cube(const VectorD3 &min, const VectorD3 &max)
			: RectD(min.x, max.x, max.y, min.y),
			  m_base(min.z), m_roof(max.z) {}

//-----------------------------------------------------------------------------
Cube::Cube(const VectorD3 &min, const double &width, const double &height, const double &depth)
			: RectD(min.x, min.x + width, min.y, min.y + height), 
			  m_base(min.z), m_roof(min.z + depth) {}

//-----------------------------------------------------------------------------
Cube::Cube(const Cube &old)
			: RectD(old.XY()),
			  m_base(old.GetBase()), m_roof(old.GetRoof()) {}

//-----------------------------------------------------------------------------
const double Cube::SetBase(const double &base) 
{ 
	m_base = base; 
	return m_base; 
}

//-----------------------------------------------------------------------------
const double Cube::SetRoof(const double &roof) 
{ 
	m_roof = roof; 
	return m_roof; 
}

//-----------------------------------------------------------------------------
Cube Cube::MakeInvalid()
{
	Cube old = *this;
	m_left = -Float::INVALID_DOUBLE;
	m_right = Float::INVALID_DOUBLE;
	m_top = Float::INVALID_DOUBLE;	
	m_bottom = -Float::INVALID_DOUBLE;
	m_base = -Float::INVALID_DOUBLE;	
	m_roof = Float::INVALID_DOUBLE;
	m_numIncludes = 0;
	return old;
}

//-----------------------------------------------------------------------------
bool Cube::IsValid() const
{
	return (m_right >= m_left) && (m_top >= m_bottom) && (m_roof >= m_base);
}

//-----------------------------------------------------------------------------
const double Cube::GetCenterZ() const 
{ 
	return m_base + GetDepth()/2; 
}

//-----------------------------------------------------------------------------
const VectorD3 Cube::GetCenter() const 
{ 
	return VectorD3(GetCenterX(), GetCenterY(), GetCenterZ()); 
}

//-----------------------------------------------------------------------------
const VectorD3 Cube::GetMin() const
{
	return VectorD3(m_right, m_top, m_roof);
}

//-----------------------------------------------------------------------------
const VectorD3 Cube::GetMax() const
{
	return VectorD3(m_left, m_bottom, m_base);
}

//-----------------------------------------------------------------------------
const RectD Cube::XY() const 
{ 
	return RectD(m_left, m_right, m_top, m_bottom); 
}

//-----------------------------------------------------------------------------
const RectD Cube::XZ() const 
{ 
	return RectD(m_left, m_right, m_roof, m_base); 
}

//-----------------------------------------------------------------------------
const RectD Cube::YZ() const 
{ 
	return RectD(m_top, m_bottom, m_roof, m_base); 
}

//-----------------------------------------------------------------------------
void Cube::IncludePoint(const double &x, const double &y, const double &z) 
{
	if (x < m_left)	m_left = x;
	if (x > m_right)	m_right = x;
	if (y > m_top)		m_top = y;
	if (y < m_bottom)	m_bottom = y;
	if (z < m_base)	m_base = z;
	if (z > m_roof)	m_roof = z;
	m_numIncludes++;
}

//-----------------------------------------------------------------------------
bool Cube::PointInside(const double &x, const double &y, const double &z, 
					    const double &tolerance /*= Float::TOLERANCE*/) const
{
	return ((Float::GreaterOrEqual(x, m_left,   tolerance) && Float::LessOrEqual(x, m_right, tolerance)) &&
				(Float::GreaterOrEqual(y, m_bottom, tolerance) && Float::LessOrEqual(y, m_top,   tolerance)) &&
				(Float::GreaterOrEqual(z, m_base,   tolerance) && Float::LessOrEqual(z, m_roof,  tolerance)));
}

//-----------------------------------------------------------------------------
const Cube &Cube::operator+=(const Cube &rhs)
{
	IncludePoint(VectorD3(rhs.m_left,  rhs.m_top,    rhs.m_roof));
	IncludePoint(VectorD3(rhs.m_right, rhs.m_bottom, rhs.m_base));
	m_numIncludes += rhs.m_numIncludes;
	return *this;
}

//-----------------------------------------------------------------------------
const Cube Cube::operator+(const Cube &rhs) const
{
	Cube temp(*this);
	temp.IncludePoint(VectorD3(rhs.m_left,  rhs.m_top,    rhs.m_roof));
	temp.IncludePoint(VectorD3(rhs.m_right, rhs.m_bottom, rhs.m_base));
	return temp;
}

//-----------------------------------------------------------------------------
void Cube::Expand(const double &left, const double &right, const double &top,
				   const double &bottom, const double &base, const double &roof)
{
	m_left   -= left;
	m_right  += right;
	m_bottom -= bottom;
	m_top    += top;
	m_base   -= base;
	m_roof   += roof;
}

//-----------------------------------------------------------------------------
void Cube::Expand(const double &width, const double &height, const double &depth)
{
	double exp = width / 2.0;
	m_left -= exp;
	m_right += exp;

	exp = height / 2.0;
	m_bottom -= exp;
	m_top += exp;

	exp = depth / 2.0;	
	m_base -= exp;
	m_roof += exp;
}
//-----------------------------------------------------------------------------
void Cube::Expand(const double &expansion)
{
	double exp = expansion / 2.0;
	m_left -= exp;
	m_right += exp;
	m_bottom -= exp;
	m_top += exp;
	m_base -= exp;
	m_roof += exp;
}

//-----------------------------------------------------------------------------

}	// namespace keays
}	// namespace math
