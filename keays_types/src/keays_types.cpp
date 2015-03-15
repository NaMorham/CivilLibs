#include "..\include\keays_types.h"

#include <LeakWatcher.h>
#include <float.h>
#include <cmath>

#ifdef _DO_MEMORY_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning(disable : 4786) // ignore the long name warning associated with stl stuff

namespace keays
{
namespace types
{

//-----------------------------------------------------------------------------
// Float namespace - wrapper function for reliable boolean operations on doubles
namespace Float
{
double TOLERANCE = 0.0001;

KEAYS_TYPES_EXPORTS_API bool Less(const double &lhs, const double &rhs, const double &tol /*= TOLERANCE*/)
{
    return ((lhs < (rhs + tol)) && (lhs < (rhs - tol)));
}

KEAYS_TYPES_EXPORTS_API bool LessOrEqual(const double &lhs, const double &rhs, const double &tol /*= TOLERANCE*/)
{
    return (lhs <= (rhs + tol));
}

KEAYS_TYPES_EXPORTS_API bool EqualTo(const double &lhs, const double &rhs, const double &tol /*= TOLERANCE*/)
{
    return ((lhs >= (rhs-tol)) && (lhs <= (rhs+tol)));
}

KEAYS_TYPES_EXPORTS_API bool Greater(const double &lhs, const double &rhs, const double &tol /*= TOLERANCE*/)
{
    return ((lhs > (rhs - tol)) && (lhs > (rhs + tol)));
}

KEAYS_TYPES_EXPORTS_API bool GreaterOrEqual(const double &lhs, const double &rhs, const double &tol /*= TOLERANCE*/)
{
    return (lhs >= (rhs-tol));
}

KEAYS_TYPES_EXPORTS_API bool NotEqual(const double &lhs, const double &rhs, const double &tol /*= TOLERANCE*/)
{
    return !EqualTo(lhs, rhs);
}

KEAYS_TYPES_EXPORTS_API const double SafeFloatToDouble(float fl, const double &tol /*= TOLERANCE*/)
{
    double invTol = 1.0/tol;
    return double(int(fl*invTol)) / invTol;
}

const double LimitPrecisionRound(const double &val, const double &tolerance)
{
    double invTol = 1.0/tolerance;
    double result;
    double fraction;
    int temp;

    temp = int(val * invTol * 10.0);
    //result = double(temp) * tolerance;
    result = (temp * tolerance);
    fraction = result - double(int(result));

    if (fraction >= 0.5)
        return ceil(result) / 10.0;
    else
        return floor(result) / 10.0;
}

const int ToInt(const double &val)
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

const double INVALID_DOUBLE = -999e99;
};

//-----------------------------------------------------------------------------
KEAYS_TYPES_EXPORTS_API const VectorD3 operator*(const double &scale, const VectorD3 &vector)
{
    VectorD3 vec(vector);
    return vec * scale;
}

KEAYS_TYPES_EXPORTS_API const VectorD2 operator*(const double &scale, const VectorD2 &vector)
{
    VectorD2 vec(vector);
    return vec * scale;
}

KEAYS_TYPES_EXPORTS_API const VectorD3 operator/(const double &scale, const VectorD3 &vector)
{
    VectorD3 vec(vector);
    return vec / scale;
}

KEAYS_TYPES_EXPORTS_API const VectorD2 operator/(const double &scale, const VectorD2 &vector)
{
    VectorD2 vec(vector);
    return vec / scale;
}

//-----------------------------------------------------------------------------
// VectorD3 class implementation
double VectorD3::ms_tolerance = Float::TOLERANCE;

VectorD3::VectorD3(const VectorD2 &vd2, const double &zPos /*= 0*/)  : x(vd2.x), y(vd2.y), z(zPos)
{
}

const VectorD3 &VectorD3::operator=(const VectorD3 &rhs)
{
    x = rhs.x;
    y = rhs.y;
    z = rhs.z;
    return *this;
}


const VectorD3 &VectorD3::operator=(const VectorD2 &rhs)
{
    x = rhs.x; y = rhs.y;
    return *this;
}

const VectorD3 VectorD3::operator+(const VectorD3 &rhs) const
{
    VectorD3 result;
    result.x = x + rhs.x;
    result.y = y + rhs.y;
    result.z = z + rhs.z;
    return result;
}

const VectorD3 VectorD3::operator+(const VectorD2 &rhs) const
{
    VectorD3 result;
    result.x = x + rhs.x;
    result.y = y + rhs.y;
    result.z = z;
    return result;
}

const VectorD3 &VectorD3::operator+=(const VectorD3 &rhs)
{
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    return *this;
}

const VectorD3 &VectorD3::operator+=(const VectorD2 &rhs)
{
    x += rhs.x;
    y += rhs.y;
    return *this;
}

const VectorD3 VectorD3::operator-(const VectorD3 &rhs) const
{
    VectorD3 result;
    result.x = x - rhs.x;
    result.y = y - rhs.y;
    result.z = z - rhs.z;
    return result;
}

const VectorD3 VectorD3::operator-(const VectorD2 &rhs) const
{
    VectorD3 result;
    result.x = x - rhs.x;
    result.y = y - rhs.y;
    result.z = z;
    return result;
}

const VectorD3 &VectorD3::operator-=(const VectorD3 &rhs)
{
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
    return *this;
}

const VectorD3 &VectorD3::operator-=(const VectorD2 &rhs)
{
    x -= rhs.x;
    y -= rhs.y;
    return *this;
}

const VectorD3 VectorD3::operator*(const double &scale) const
{
    VectorD3 result;
    result.x = x * scale;
    result.y = y * scale;
    result.z = z * scale;
    return result;
}

const VectorD3 &VectorD3::operator*=(const double &scale)
{
    x *= scale;
    y *= scale;
    z *= scale;
    return *this;
}

const VectorD3 VectorD3::operator/(const double &scale) const
{
    VectorD3 result;
    if (scale == 0.0)
    {
        result.Set(0, 0, 0);
    } else
    {
        result.x = x / scale;
        result.y = y / scale;
        result.z = z / scale;
    }
    return result;
}

const VectorD3 &VectorD3::operator/=(const double &scale)
{
    if (scale == 0.0)
    {
        Set(0, 0, 0);
    } else
    {
        x /= scale;
        y /= scale;
        z /= scale;
    }
    return *this;
}

bool VectorD3::operator==(const VectorD3 &rhs) const
{
    return(Float::EqualTo(x, rhs.x, ms_tolerance) &&
            Float::EqualTo(y, rhs.y, ms_tolerance) &&
            Float::EqualTo(z, rhs.z, ms_tolerance));
}

bool VectorD3::operator==(const VectorD2 &rhs) const
{
    return(Float::EqualTo(x, rhs.x, ms_tolerance) &&
            Float::EqualTo(y, rhs.y, ms_tolerance));
}


bool VectorD3::operator!=(const VectorD3 &rhs) const    { return !(*this == rhs); }
bool VectorD3::operator!=(const VectorD2 &rhs) const    { return !(*this == rhs); }

//    Normalise the VectorD3 (for use when it is representing a VECTOR rather than a POINT
const VectorD3 &VectorD3::Normalise()
{
    double mag = sqrt(x*x + y*y + z*z);
    if (0.0 != mag)
    {
        x = x/mag;
        y = y/mag;
        z = z/mag;
    }

    return *this;
}

const VectorD3 VectorD3::GetNormalised() const
{
    return GetNormalised(x, y, z);
}

const VectorD3 VectorD3::GetNormalised(const double &xSize, const double &ySize, const double &zSize)
{
    return VectorD3(xSize, ySize, zSize).Normalise();
}

// Calculate the crossproduct
const VectorD3 VectorD3::Cross(const VectorD3 &vec) const
{
    VectorD3 result;
    result.x = (y * vec.z) - (vec.y * z);
    result.y = (z * vec.x) - (vec.z * x);
    result.z = (x * vec.y) - (vec.x * y);

    return result;
}

const double VectorD3::Dot(const VectorD3 &vec) const
{
    return (x*vec.x + y*vec.y + z*vec.z);
}

const double VectorD3::AngleBetween(const VectorD3 &vec) const
{
    return acos(GetNormalised().Dot(vec.GetNormalised()));
}

// calc the magnitude
const double VectorD3::Magnitude() const
{
    return sqrt(x*x + y*y + z*z);
}

const double VectorD3::Magnitude2D() const
{
    return sqrt(x*x + y*y);
}

const double VectorD3::Grade() const
{
    double mag2D = Magnitude2D();
    //if (Float::EqualTo(mag2D, 0.0, ms_tolerance))
    if (mag2D == 0.0)
        return DBL_MAX;

    return z / mag2D;
}

VectorD3::operator VectorD2() const
{
    return VectorD2(x, y);
}

const VectorD2 VectorD3::XY() const
{
    return VectorD2(x, y);
}

const VectorD2 VectorD3::XZ() const
{
    return VectorD2(x, z);
}

const VectorD2 VectorD3::YZ() const
{
    return VectorD2(y, z);
}

//-----------------------------------------------------------------------------
const VectorD3 BAD_POINT = VectorD3(Float::INVALID_DOUBLE, Float::INVALID_DOUBLE, Float::INVALID_DOUBLE);

const double INVALID_ANGLE = Float::INVALID_DOUBLE;

//-----------------------------------------------------------------------------
// VectorD2 class
double VectorD2::ms_tolerance = Float::TOLERANCE;

const VectorD2 &VectorD2::operator=(const VectorD3 &rhs)
{
    x = rhs.x; y = rhs.y;
    return *this;
}

const VectorD2 &VectorD2::operator=(const VectorD2 &rhs)
{
    x = rhs.x; y = rhs.y;
    return *this;
}

bool VectorD2::operator==(const VectorD2 &rhs) const
{
    return(Float::EqualTo(x, rhs.x, ms_tolerance) &&
            Float::EqualTo(y, rhs.y, ms_tolerance));
}

bool VectorD2::operator==(const VectorD3 &rhs) const
{
    return(Float::EqualTo(x, rhs.x, ms_tolerance) &&
            Float::EqualTo(y, rhs.y, ms_tolerance));
}

const VectorD2 VectorD2::operator+(const VectorD2 &rhs) const
{
    return VectorD2(x + rhs.x, y + rhs.y);
}

const VectorD2 &VectorD2::operator+=(const VectorD2 &rhs)
{
    x += rhs.x;
    y += rhs.y;
    return *this;
}

const VectorD2 VectorD2::operator-(const VectorD2 &rhs) const
{
    return VectorD2(x - rhs.x, y - rhs.y);
}

const VectorD2 &VectorD2::operator-=(const VectorD2 &rhs)
{
    x -= rhs.x;
    y -= rhs.y;
    return *this;
}

const VectorD2 VectorD2::operator*(const double &scale) const
{
    return VectorD2(x * scale, y * scale);
}

const VectorD2 &VectorD2::operator*=(const double &scale)
{
    x *= scale;
    y *= scale;
    return *this;
}

const VectorD2 VectorD2::operator/(const double &scale) const
{
    if (scale == 0.0)
    {
        return VectorD2(0, 0);
    } else
    {
        return VectorD2(x / scale, y / scale);
    }
}

const VectorD2 &VectorD2::operator/=(const double &scale)
{
    if (scale == 0.0)
    {
        Set(0, 0);
    } else
    {
        x /= scale;
        y /= scale;
    }
    return *this;
}

const double VectorD2::Dot(const VectorD2 &vec) const
{
    return (x*vec.x + y*vec.y);
}

const double VectorD2::AngleBetween(const VectorD2 &vec) const
{
    return acos(GetNormalised().Dot(vec.GetNormalised()));
}

VectorD2::operator VectorD3() { return VectorD3(x, y, 0); }

const VectorD2 VectorD2::GetNormalised() const
{
    return GetNormalised(x, y);
}

const VectorD2 &VectorD2::Normalise()
{
    double mag = sqrt(x*x + y*y);
    if (0.0 != mag)
    {
        x = x/mag;
        y = y/mag;
    }

    return *this;
}

const VectorD2 VectorD2::GetNormalised(const double &xSize, const double &ySize)
{
    return VectorD2(xSize, ySize).Normalise();
}

const VectorD2 VectorD2::GetPerpendicular(bool rightSide /*= true*/) const
{
    return rightSide ? VectorD2(y, -x) : VectorD2(-y, x);
}

//-----------------------------------------------------------------------------
// Local Axes class
LocalAxes::LocalAxes()
{
    m_i.Set(1, 0, 0);
    m_j.Set(0, 1, 0);
    m_k.Set(0, 0, 1);
}

LocalAxes::LocalAxes(const VectorD3 &i, const VectorD3 &j, const VectorD3 &k)
{
    m_i = i;
    m_j = j;
    m_k = k;
}

// mutators
const VectorD3 &LocalAxes::I(const VectorD3 &i)
{
    m_i = i;
    return m_i;
}

const VectorD3 &LocalAxes::J(const VectorD3 &j)
{
    m_j = j;
    return m_j;
}

const VectorD3 &LocalAxes::K(const VectorD3 &k)
{
    m_k = k;
    return m_k;
}

const VectorD3 &LocalAxes::CalcI()
{
    m_i = m_j.Cross(m_k).GetNormalised();
    return m_i;
}

const VectorD3 &LocalAxes::CalcJ()
{
    m_j = m_i.Cross(m_k).GetNormalised();
    return m_j;
}

const VectorD3 &LocalAxes::CalcK()
{
    m_k = m_i.Cross(m_j).GetNormalised();
    return m_k;
}

const LocalAxes &LocalAxes::operator=(const LocalAxes &rhs)
{
    m_i = rhs.m_i;
    m_j = rhs.m_j;
    m_k = rhs.m_k;

    return *this;
}

/*
size_t LocalAxes::WriteFile(FILE *file) const
{
    double temp;
    size_t bytes = 0;

    if (!file)
        return 0;

    temp = m_i.x;
    bytes += fwrite(&temp, sizeof(temp), 1, file);
    temp = m_i.y;
    bytes += fwrite(&temp, sizeof(temp), 1, file);
    temp = m_i.z;
    bytes += fwrite(&temp, sizeof(temp), 1, file);

    temp = m_j.x;
    bytes += fwrite(&temp, sizeof(temp), 1, file);
    temp = m_j.y;
    bytes += fwrite(&temp, sizeof(temp), 1, file);
    temp = m_j.z;
    bytes += fwrite(&temp, sizeof(temp), 1, file);

    temp = m_k.x;
    bytes += fwrite(&temp, sizeof(temp), 1, file);
    temp = m_k.y;
    bytes += fwrite(&temp, sizeof(temp), 1, file);
    temp = m_k.z;
    bytes += fwrite(&temp, sizeof(temp), 1, file);

    return bytes;
}
//*/

}    // namespace types
}    // namespace keays