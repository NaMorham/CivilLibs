/*!
    \file geometry.h
    \brief    Geometry routines and objects.
    Functions to handle working with geometry.
    Part of the Keays::Maths namespace.

    \date May 2004.
    \author Andrew Hickey, Keays Software.
    \author Karl Janke, Keays Software.
 */
#pragma once

#include <vector>            // stl vector support
#include <list>

#include "mathhelp.h"        // our math library
#include <keays_types.h>    // keays types library

//#include <stdarg.h>

#ifdef KEAYS_MATH_EXPORTS
#define KEAYS_MATH_EXPORTS_API __declspec(dllexport)
#else
#define KEAYS_MATH_EXPORTS_API __declspec(dllimport)
#endif

// Not sure where ubyte is defined (if anywhere). So redefine here.
#if !defined(ubyte)
#define ubyte unsigned char
#endif

// end hacky defines
/*!
    \brief General Keays Software Namespace
    Used to prevent conflicts with existing functions/ variables
 */
namespace keays
{
/*!
    \brief Maths & Geometry routines for use in Keays Software applications
 */
namespace math
{
/*!
    \name Constants
    @{
 */
//! double precision \f$\pi\f$
extern KEAYS_MATH_EXPORTS_API const double KM_PI;
//! double precision \f$2\pi\f$
extern KEAYS_MATH_EXPORTS_API const double KM_2PI;
//! double precision \f$\pi/2\f$
extern KEAYS_MATH_EXPORTS_API const double KM_PI_ON2;
//! double precision \f$3\pi/2\f$
extern KEAYS_MATH_EXPORTS_API const double KM_3PI_ON2;

//! double precision \f$\pi/3\f$
extern KEAYS_MATH_EXPORTS_API const double KM_PI_ON3;
//! double precision \f$\pi/4\f$
extern KEAYS_MATH_EXPORTS_API const double KM_PI_ON4;
//! double precision \f$\pi/6\f$
extern KEAYS_MATH_EXPORTS_API const double KM_PI_ON6;
//! double precision \f$\pi/8\f$
extern KEAYS_MATH_EXPORTS_API const double KM_PI_ON8;

//! double precision \f$\sqrt{2}\f$
extern KEAYS_MATH_EXPORTS_API const double ROOT_2;
//! double precision \f$\frac{ \sqrt{2} }{ 2 }\f$
extern KEAYS_MATH_EXPORTS_API const double ROOT_2_ON2;
/*! @}*/

const char DEG_CHAR = (char)0xb0;


void KEAYS_MATH_EXPORTS_API AttachLogFile(FILE **pLogFile);
void KEAYS_MATH_EXPORTS_API DetachLogFile();


//! Used in the offset functions to determine which way to offset the line.
enum KEAYS_MATH_EXPORTS_API eSideSelections
{
    SIDE_LEFT  = -1,    //!< indicates that an object (point/line/???) is to the  left of a reference object.
    SIDE_NONE  =  0,    //!< indicates that an object (point/line/???) is on the  left of a reference object.
    SIDE_RIGHT =  1        //!< indicates that an object (point/line/???) is to the right of a reference object.
};

/*!
    \brief The Line class represents a line in 3 dimensional space.
    Effectively a point pair, using keays::types::VectorD3.
 */
class KEAYS_MATH_EXPORTS_API Line
{
public:
    /*!
        \brief Basic Constructor.
        Constructs a simple line with 2 keays::types::VectorD3 points [ 0, 0, 0 ] [ 1, 0, 0 ].
     */
    Line();

    /*!
        \brief The Copy Constructor - duplicates an existing Line.

        \param ln [In] - A constant reference to an existing Line object.
     */
    Line(const Line &ln);

    /*!
        \brief Constructor (2 keays::types::VectorD2 points).

        \param st [In] - A constant reference to a <b>VectorD2</b> point for the start of the line.
        \param en [In] - A constant reference to a <b>VectorD2</b> point for the end of the line.
     */
    Line(const keays::types::VectorD2 &st, const keays::types::VectorD2 &en);

    /*!
        \brief Constructor (2 keays::types::VectorD3 points).

        \param st [In] - A constant reference to a keays::types::VectorD3 point for the start of the line.
        \param en [In] - A constant reference to a keays::types::VectorD3 point for the end of the line.
     */
    Line(const keays::types::VectorD3 &st, const keays::types::VectorD3 &en);

    /*!
        \brief Assignment operator.
        Assign the values from a given Line to the current object.

        \param    rhs [In] - A constant reference to a Line object to copy.

        \return a reference to the current object.
     */
    const Line &operator=(const Line &rhs);

    /*!
        \brief Casting operator to keays::types::VectorD3.

        \return the 3D direction Vector representing the line.
     */
    operator keays::types::VectorD3() const;

    /*!
        \brief Casting operator to keays::types::VectorD2.

        \return the 2D direction Vector representing the line.
     */
    operator keays::types::VectorD2() const;

    /*
        \defgroup lineUtilityFunctions Utility Functions
        @{
     */
    /*!
        \brief Get the length of the line.

        \return a <b>double</b> representing the length of the line.
     */
    const double GetLength() const;

    /*!
        \brief Extend the Line in 3D from the start point to the new length.

        \param newLength [In] - A constant reference to a <b>double</b> specifying the desired length for the line.

        \return a constant reference to the current object.
     */
    const Line &ExtendEnd(const double &newLength);

    /*!
        \brief Extend the Line in 3D from the end point back to the new length.

        \param newLength [In] - A constant reference to a <b>double</b> specifying the desired length for the line.

        \return a constant reference to the current object.
     */
    const Line &ExtendStart(const double &newLength);

    /*!
        \brief Extend the Line in 3D from the mid point back to the new length.

        \param newLength [In] - A constant reference to a <b>double</b> specifying the desired length for the line.

        \return a constant reference to the current object.
     */
    const Line &ExtendMid(const double &newLength);

    /*!
        \brief Extend the Line in 2D from the start point to the new length.
        This will extend the line as it changes the 2D length, this effectively moves the end point backwards and
        forwards along the line, as a result the z-value of the end of the line will change as well.

        \param newLength [In] - A constant reference to a <b>double</b> specifying the desired length for the line.

        \return a constant reference to the current object.
     */
    const Line &ExtendEnd2D(const double &newLength);

    /*!
        \brief Extend the Line in 2D from the end point back to the new length.
        This will extend the line as it changes the 2D length, this effectively moves the start point backwards
        and forwards along the line, as a result the z-value of the start of the line will change as well.

        \param newLength [In] - A constant reference to a <b>double</b> specifying the desired length for the line.

        \return a constant reference to the current object.
     */
    const Line &ExtendStart2D(const double &newLength);

    /*!
        \brief Extend the Line in 2D from the mid point back to the new length.
        This will extend the line as it changes the 2D length, this effectively moves the start and end points
        backwards and forwards along the line, as a result the z-values of both ends of the line will change as well.

        \param newLength [In] - A constant reference to a <b>double</b> specifying the desired length for the line.

        \return a constant reference to the current object.
     */
    const Line &ExtendMid2D(const double &newLength);

    /*!
        \brief Get the length of the line in the X/Y Plane.

        \return a constant <b>double</b> representing the 2D X/Y length of the line.
     */
    const double GetLength2D() const;

    /*!
        \brief Get the bearing (direction) of the line in the X/Y plane.

        \return a constant <b>double</b> representing the bearing of the line in the X/Y Plane.
     */
    const double GetBearing() const;

    /*!
        \brief Get the zenith (elevation) of the line.

        \return a constant <b>double</b> representing the zenith of the line.
     */
    const double GetZenith() const;

    /*!
        \brief Get the grade of the line (gradient - gradians)

        \return a constant <b>double</b> representing the gradient of the line
     */
    const double GetGrade() const;

    /*!
        \brief Get the mid point of the line

        \return a constant <b>VectorD3</b> representing the position of the midpoint
     */
    const keays::types::VectorD3 GetMidPoint() const;

    /*!
        \brief Determine if a point is on the Line in the XY plane

        \param        pt [In] - A constant reference to a keays::types::VectorD2 point to test.
        \param tolerance [In] - A constant reference to a <b>double</b> indicating the tolerance to use in the test.

        \return true if the point is on the line
     */
    bool PointOnLine(const keays::types::VectorD2 &pt, const double &tolerance) const;

    /*!
        \brief Determine if a point is on the Line Segment in the XY plane

        \param        pt [In] - A constant reference to a keays::types::VectorD2 point to test.
        \param tolerance [In] - A constant reference to a <b>double</b> indicating the tolerance to use in the test.

        \return true if the point is on the line
     */
    bool PointOnSegment(const keays::types::VectorD2 &pt, const double &tolerance = keays::types::Float::TOLERANCE) const;

    /*!
        \brief Determine if a point is within a given distance of a line.

        \param        pt [In]  - A constant reference to a keays::types::VectorD3 point to test.
        \param  distance [Out] - A reference to a <b>double</b> specifying the test distance.
        \param tolerance [In]  - A constant reference to a <b>double</b> indicating the tolerance to use in the test.

        \return true if the point is on the line, and the line is not vertical.
     */
    bool PointNear(const keays::types::VectorD2 &pt, double &distance, const double &tolerance = keays::types::Float::TOLERANCE) const;

    /*!
        \brief Determine if a point is within a given distance of a line segment.

        \param        pt [In]  - A constant reference to a keays::types::VectorD3 point to test.
        \param  diatsnce [Out] - A reference to a <b>double</b> specifying the test distance.
        \param tolerance [In]  - A constant reference to a <b>double</b> indicating the tolerance to use in the test.

        \return true if the point is on the line, and the line is not vertical.
     */
    bool PointNearSegment(const keays::types::VectorD2 &pt, double &distance, const double &tolerance = keays::types::Float::TOLERANCE) const;

    /*!
        \brief Calculate the height of a specified point if it lies on the line.

        \param     pt [In]  - A constant reference to a keays::types::VectorD3 point to get the height for.
        \param height [Out] - A reference to a <b>double</b> to receive the height value.

        \return true if the point is on the line, and the line is not vertical.
     */
    bool GetPointHeight(const keays::types::VectorD3 &pt, double &height) const;

    /*!
        \brief Rotate a point using the line as a reference.
        Calculates the rotated position of the point if it is kept relative to the line as the line rotates to lie
        parallel to the X axis, using the start position as a base point.  This is the inverse of the UnrotatePt
        member function.

        \param pt [In] - A constant reference to a keays::types::VectorD2 representing the point to rotate.

        \return A constant keays::types::VectorD2 at the rotated location.
     */
    const keays::types::VectorD2 RotatePoint(const keays::types::VectorD2 &pt) const;

    /*!
        \brief Unrotate a point using the line as a reference.
        Calculates the unrotated position of the point if it is kept relative to the line as the line is rotated
        from lying parallel to the X axis, back to its original position, using the start position as a base
        point.  This is the inverse of the RotatePt member function.

        \param pt [In] - A constant reference to a keays::types::VectorD2 representing the point to unrotate.

        \return A constant keays::types::VectorD2 at the unrotated location.
     */
    const keays::types::VectorD2 UnrotatePoint(const keays::types::VectorD2 &pt) const;

    /*!
        \brief Generate a parallel offset a given distance from an existing line

        \param    dist [In] - A constant reference to a <b>double</b> specifying the distance to offset.
        \param    side [In] - one of SIDE_LEFT or SIDE_RIGHT to determine which side to offset. Side is determined by the direction of the source polyline.

        \return    The generated offset Line.
     */
    const Line CalcOffset(const double &dist, const eSideSelections side);

    /*!
        \brief Generate a parallel offset a given distance from an existing line

        \param    dist [In] - A constant reference to a <b>double</b> specifying the distance to offset.
        \param      pt [In] - A constant reference to a keays::types::VectorD2 as a reference point to determine the side to offset.

        \return    The generated offset Line.
     */
    const Line CalcOffset(const double &dist, const keays::types::VectorD2 &pt);

    //----------------------------------------------
    keays::types::VectorD3 start,    //!< The keays::types::VectorD3 representing the start of the line.
                           end;        //!< The keays::types::VectorD3 representing the end of the line.
};

//!    \brief A rectangle structure for testing points/bounds using doubles.
struct KEAYS_MATH_EXPORTS_API RectD
{
    /*!
        \param     left [In] - A constant reference to a <b>double</b> specifying the left edge of the rectangle.
        \param  right [In] - A constant reference to a <b>double</b> specifying the right edge of the rectangle.
        \param      top [In] - A constant reference to a <b>double</b> specifying the top edge of the rectangle.
        \param bottom [In] - A constant reference to a <b>double</b> specifying the bottom edge of the rectangle.
     */
    RectD(const double &left = -keays::types::Float::INVALID_DOUBLE,
          const double &right = keays::types::Float::INVALID_DOUBLE,
          const double &top = keays::types::Float::INVALID_DOUBLE,
          const double &bottom = -keays::types::Float::INVALID_DOUBLE);

    /*!
        \param min [In] - A constant reference to a keays::types::VectorD2 specifying the bottom-left (minimum) corner of the rectangle.
        \param max [In] - A constant reference to a keays::types::VectorD2 specifying the top-right (maximum) corner of the rectangle.
     */
    RectD(const keays::types::VectorD2 &min, const keays::types::VectorD2 &max);

    /*!
        \param    min [In] - A constant reference to a keays::types::VectorD2 specifying the bottom-left (minimum) corner of the rectangle.
        \param  width [In] - A constant reference to a double specifiying the width of the rectangle.
        \param height [In] - A constant reference to a double specifiying the height of the rectangle.
     */
    RectD(const keays::types::VectorD2 &min, const double &width, const double &height);

    /*!
        \brief The Copy Constructor - duplicates an existing RectD.

        \param old [In] - A constant reference to an existing RectD object.
     */
    RectD(const RectD &old);

    /*!
        \brief Get the current left value.

        \return A constant <b>double</b> with the current value of the left member.
     */
    inline const double GetLeft() const
    {
        return m_left;
    }

    /*!
        \brief Set the current left value.

        \param left [In] - A constant reference to a <b>double</b> specifying the new value for the left member.

        \return A constant <b>double</b> with the new value of the left member.
     */
    const double SetLeft(const double &left);

    /*!
        \brief Get the current right value.

        \return A constant <b>double</b> with the current value of the right member.
     */
    inline const double GetRight() const
    {
        return m_right;
    }

    /*!
        \brief Set the current right value.

        \param left [In] - A constant reference to a <b>double</b> specifying the new value for the right member.

        \return A constant <b>double</b> with the new value of the right member.
     */
    const double SetRight(const double &right);

    /*!
        \brief Get the current bottom value.

        \return A constant <b>double</b> with the current value of the bottom member.
     */
    inline const double GetBottom() const
    {
        return m_bottom;
    }

    /*!
        \brief Set the current bottom value.

        \param bottom [In] - A constant reference to a <b>double</b> specifying the new value for the bottom member.

        \return A constant <b>double</b> with the new value of the bottom member.
     */
    const double SetBottom(const double &bottom);

    /*!
        \brief Get the current top value.

        \return A constant <b>double</b> with the current value of the top member.
     */
    inline const double GetTop() const
    {
        return m_top;
    }

    /*!
        \brief Set the current top value.

        \param bottom [In] - A constant reference to a <b>double</b> specifying the new value for the top member.

        \return A constant <b>double</b> with the new value of the top member.
     */
    const double SetTop(const double &top);

    /*!
        \brief Invalidate the cube.
        Restores the cube to a default state.

        \return A duplicate of the previous state of the RectD.
     */
    const RectD MakeInvalid();

    /*!
        \brief Test is the RectD is valid.

        \return true if the RectD is valid.
     */
    bool IsValid() const;

    //! \brief Get a <b>double</b> representing the width of the rectangle.
    const double GetWidth() const;

    //! \brief Get a <b>double</b> representing the height of the rectangle.
    const double GetHeight() const;

    //! \brief Get a <b>double</b> representing the center of the rectangle in the X dimension.
    const double GetCenterX() const;

    //! \brief Get a <b>double</b> representing the the center of the rectangle in the Y dimension.
    const double GetCenterY() const;

    //! \brief Get a keays::types::VectorD3 representing the center of the RectD.
    const keays::types::VectorD2 GetCenter() const;

    //! \brief Get a keays::types::VectorD2 representing the bottom-left (minimum) corner of the RectD.
    const keays::types::VectorD2 GetMin() const;

    //! \brief Get a keays::types::VectorD2 representing the top-right (maximum) corner of the RectD.
    const keays::types::VectorD2 GetMax() const;

    //! \brief Calculate the Area of the rectangle.
    const double CalcArea() const;

    /*!
        \brief Include a point in the rectangle.
        This expands the rectangle to include the specified point.

        \param x [In] - A constant reference to a <b>double</b> with the x value of the point.
        \param y [In] - A constant reference to a <b>double</b> with the y value of the point.
     */
    void IncludePoint(const double &x, const double &y);

    /*!
        \overload

        \param pt [In] - A constant reference to a keays::types::VectorD2 representing the point.
     */
    void IncludePoint(const keays::types::VectorD2 &pt);

    /*!
        \brief Test to see if a point is inside the rectangle.

        \param         x [In] - A constant reference to a <b>double</b> with the x value of the point.
        \param         y [In] - A constant reference to a <b>double</b> with the y value of the point.
        \param tolerance [In] - A constant reference to a <b>double</b> representing the tolerance to use in the test [optional].

        \return true if the point is inside the rectangle
     */
    bool PointInside(const double &x, const double &y, const double &tolerance = keays::types::Float::TOLERANCE) const;

    /*!
        \overload

        \param        pt [In] - A constant reference to a keays::types::VectorD2 representing the point.
        \param tolerance [In] - A constant reference to a <b>double</b> representing the tolerance to use in the test [optional].
     */
    bool PointInside(const keays::types::VectorD2 &pt, const double &tolerance = keays::types::Float::TOLERANCE) const
    {
        return PointInside(pt.x, pt.y, tolerance);
    }

    /*!
        \brief Assignment operator.
        Assign the values from a given RectD to the current object.

        \param rhs [In] - A constant reference to a RectD object to copy.

        \return A constant reference to the current object.
     */
    const RectD &operator=(const RectD &rhs);

    // hopefully self explanatory
    inline const double GetX() const
    {
        return m_left;
    }

    inline const double GetY() const
    {
        return m_bottom;
    }

    /*!
        \brief Resize the rectangle so that it has the specified width.
        The left value of the rectangle will remain constant.

        \param d [In] - A constant reference to a <b>double</b> specifying the new width of the rectangle.
     */
    void SetWidth(const double &d);

    /*!
        \brief Resize the rectangle so that it has the specified height.
        The top value of the rectangle will remain constant.

        \param d [In] - A constant reference to a <b>double</b> specifying the new height of the rectangle.
     */
    void SetHeight(const double &d);

    /*!
        \brief Translates the rectangle to start at the given x position.
        The width of the rectangle will remain constant.

        \param d [In] - A constant reference to a <b>double</b> specifying the new left x co-ordinate of the rectangle.
     */
    void SetX(const double &d);

    /*!
        \brief Translates the rectangle to start at the given y position.
        The height of the rectangle will remain constant.

        \param d [In] - A constant reference to a <b>double</b> specifying the new bottom y co-ordinate of the rectangle.
     */
    void SetY(const double &d);

    /*!
        \brief Expand the rectangle by the  specified amounts.
        Modify the size  of the rectangle, +ve values increase the edge, -ve values decrease them.

        \param   left [In] - A constant reference to a <b>double</b> indicating the amount to adjust the left.
        \param  right [In] - A constant reference to a <b>double</b> indicating the amount to adjust the right.
        \param bottom [In] - A constant reference to a <b>double</b> indicating the amount to adjust the bottom.
        \param    top [In] - A constant reference to a <b>double</b> indicating the amount to adjust the top.
     */
    void Expand(const double &left, const double &right, const double &top, const double &bottom);

    /*!
        \overload

        \param  width [In] - A constant reference to a <b>double</d> indicating the amount to increase the width of the rectangle  (width/2 to left and right).
        \param height [In] - A constant reference to a <b>double</d> indicating the amount to increase the height of the rectangle (height/2 to top and bottom).
     */
    void Expand(const double &width, const double &height);

    /*!
        \overload

        \param expansion [In] - A constant reference to a <b>double</d> indicating the amount to increase the dimensions of the rectangle (expansion/2 to all members).
     */
    void Expand(const double &expansion);

    //--------------------
    double    m_left,        //!< the left edge of the rectangle.
            m_right,    //!< the right edge of the rectangle.
            m_top,        //!< the top edge of the rectangle.
            m_bottom;    //!< the bottom edge of the rectangle.

    int m_numIncludes;    //!< number of points that have been inserted.
};

//!    \brief A cube structure for testing points/bounds using doubles - inherits from RectD.
struct KEAYS_MATH_EXPORTS_API Cube : public RectD
{
    /*!
        \param   left [In] - A constant reference to a <b>double</b> specifying the left of the Cube.
        \param  right [In] - A constant reference to a <b>double</b> specifying the right of the Cube.
        \param    top [In] - A constant reference to a <b>double</b> specifying the top of the Cube.
        \param bottom [In] - A constant reference to a <b>double</b> specifying the bottom of the Cube.
        \param   base [In] - A constant reference to a <b>double</b> specifying the base of the Cube.
        \param   roof [In] - A constant reference to a <b>double</b> specifying the roof of the Cube.
     */
    Cube(const double &left = -keays::types::Float::INVALID_DOUBLE,
         const double &right = keays::types::Float::INVALID_DOUBLE,
         const double &top = keays::types::Float::INVALID_DOUBLE,
         const double &bottom = -keays::types::Float::INVALID_DOUBLE,
         const double &base = -keays::types::Float::INVALID_DOUBLE,
         const double &roof = keays::types::Float::INVALID_DOUBLE);

    /*!
        \param     min [In] - A constant reference to a keays::types::VectorD3 specifying the bottom-left-base (minimum) corner of the Cube.
        \param     max [In] - A conatsnt reference to a keays::types::VectorD3 specifying the top-right-roof (maximum) corner of the Cube.
     */
    Cube(const keays::types::VectorD3 &min, const keays::types::VectorD3 &max);

    /*!
        \param    min [In] - A keays::types::VectorD3 specifying the bottom-left-base (minimum) corner of the Cube.
        \param  width [In] - A constant reference to a <b>double</b> specifiying the width of the Cube (x-axis).
        \param height [In] - A constant reference to a <b>double</b> specifiying the height of the Cube (y-axis).
        \param  depth [In] - A constant reference to a <b>double</b> specifiying the depth of the Cube (z-axis).
     */
    Cube(const keays::types::VectorD3 &min, const double &width, const double &height, const double &depth);

    /*!
        \brief The Copy Constructor - duplicates an existing RectD.

        \param old [In] - A reference to an existing RectD object.
     */
    Cube(const Cube &old);

    /*!
        \brief Get the current base value.

        \return a constant <b>double</b> with the current value of the base member.
     */
    inline const double GetBase() const
    {
        return m_base;
    }

    /*!
        \brief Set the current base value.

        \param base [In] - A constant reference to a <b>double</b> specifying the new value for the base member.

        \return a constant <b>double</b> with the new value of the base member.
     */
    const double SetBase(const double &base);

    /*!
        \brief Get the current roof value.

        \return a constant <b>double</b> with the current value of the roof member.
     */
    inline const double GetRoof() const
    {
        return m_roof;
    }

    /*!
        \brief Set the current roof value.

        \param base [In] - A constant reference to a <b>double</b> specifying the new value for the roof member.

        \return a constant <b>double</b> with the new value of the roof member.
     */
    const double SetRoof(const double &roof);

    /*!
        \brief Invalidate the cube..
        Restores the cube to a default state.

        \return A duplicate of the previous state of the Cube.
     */
    Cube MakeInvalid();

    /*!
        \brief Test is the Cube is valid.

        \return true if the Cube is valid.
     */
    bool IsValid() const;

    //! \brief Get a <b>double</b> representing the depth of the Cube.
    inline const double GetDepth() const
    {
        return m_roof - m_base;
    }

    //! \brief Get a <b>double</b> representing the center of the Cube in the Z dimension.
    const double GetCenterZ() const;

    //! \brief Get a keays::types::VectorD3 representing the center of the Cube.
    const keays::types::VectorD3 GetCenter() const;

    //! \brief Get a keays::types::VectorD3 representing the bottom-left-base (minimum) corner of the Cube.
    const keays::types::VectorD3 GetMin() const;

    //! \brief Get a keays::types::VectorD3 representing the top-right-roof (maximum) corner of the Cube.
    const keays::types::VectorD3 GetMax() const;

    //! \brief Get a RectD representing the extents in the XY plane.
    const RectD XY() const;

    //! \brief Get a RectD representing the extents in the XZ plane.
    const RectD XZ() const;

    //! \brief Get a RectD representing the extents in the YZ plane.
    const RectD YZ() const;

    /*!
        \brief Include a point in the rectangle.
        This expands the Cube to include the specified point.

        \param x [In] - A constant reference to a <b>double</b> with the x value of the point.
        \param y [In] - A constant reference to a <b>double</b> with the y value of the point.
        \param z [In] - A constant reference to a <b>double</b> with the y value of the point.
     */
    void IncludePoint(const double &x, const double &y, const double &z);

    /*!
        \overload

        \param pt [In] - A constant reference to a keays::types::VectorD3 representing the point.
     */
    inline void IncludePoint(const keays::types::VectorD3 &pt)
    {
        IncludePoint(pt.x, pt.y, pt.z);
    }

    /*!
        \brief Test to see if a point is inside the Cube.

        \param         x [In] - A constant reference to a <b>double</b> with the x value of the point.
        \param         y [In] - A constant reference to a <b>double</b> with the y value of the point.
        \param         z [In] - A constant reference to a <b>double</b> with the z value of the point.
        \param tolerance [In] - A constant reference to a <b>double</b> representing the tolerance to use in the test [optional].

        \return true if the point is inside the Cube
     */
    bool PointInside(const double &x, const double &y, const double &z,
                     const double &tolerance = keays::types::Float::TOLERANCE) const;

    /*!
        \overload

        \param        pt [In] - A constant reference to a keays::types::VectorD3 representing the point.
        \param tolerance [In] - A constant reference to a <b>double</b> representing the tolerance to use in the test [optional].
     */
    inline bool PointInside(const keays::types::VectorD3 &pt,
                            const double &tolerance = keays::types::Float::TOLERANCE) const
    {
        return PointInside(pt.x, pt.y, pt.z, tolerance);
    }

    /*!
        \brief Add the extents of an existing cube to the current one.

        \param rhs [In] - A constant reference to the Cube to add in.

        \return A constant reference to the current Cube.
     */
    const Cube &operator+=(const Cube &rhs);

    /*!
        \brief Add the extents of an existing cube to the current one.

        \param rhs [In] - A constant reference to the Cube to add in.

        \return A constant reference to the current Cube.
     */
    const Cube Cube::operator+(const Cube &rhs) const;

    /*!
        \brief Expand the cube by the specified amounts
        Expand the dimensions of the cude by the specified amounts, +ve values increase the edge, -ve values
        decrease them.

        \param   left [In] - A constant reference to a <b>double</b> indicating the amount to adjust the left.
        \param  right [In] - A constant reference to a <b>double</b> indicating the amount to adjust the right.
        \param bottom [In] - A constant reference to a <b>double</b> indicating the amount to adjust the bottom.
        \param    top [In] - A constant reference to a <b>double</b> indicating the amount to adjust the top.
        \param   base [In] - A constant reference to a <b>double</b> indicating the amount to adjust the base.
        \param   roof [In] - A constant reference to a <b>double</b> indicating the amount to adjust the roof.
     */
    void Expand(const double &left, const double &right, const double &top,
                const double &bottom, const double &base, const double &roof);

    /*!
        \overload

        \param  width [In] - A constant reference to a <b>double</d> indicating the amount to increase the width of the cube  (width/2 to left and right).
        \param height [In] - A constant reference to a <b>double</d> indicating the amount to increase the height of the cube (height/2 to top and bottom).
        \param  depth [In] - A constant reference to a <b>double</d> indicating the amount to increase the depth of the cube  (depth/2 to base and roof).
     */
    void Expand(const double &width, const double &height, const double &depth);

    /*!
        \overload

        \param expansion [In] - A constant reference to a <b>double</d> indicating the amount to increase the dimensions of the cube (expansion/2 to all members).
     */
    void Expand(const double &expansion);

    //-------------------------------------
    double m_roof,    //!< the roof of the Cube.
           m_base;    //!< the base of the Cube.
};

//!    \brief A specialised structure for calculating point heights based on a centeral point and distance.
struct KEAYS_MATH_EXPORTS_API tGradeSegment
{
    double m_distance;    //!< this is the distance to this segment.
    double m_width;        //!< this is the width of this segment.
    double m_grade;        //!< this is the grade in the outwards direction from the ref points (so in +ve distance).

    /*!
        \brief Default constructor.

        \param dist [in] - A constant reference to a <b>double</b> specifying the distance of the segment.
        \param   wd [in] - A constant reference to a <b>double</b> specifying the width of the segment.
        \param   gr [in] - A constant reference to a <b>double</b> specifying the grade of the segment.
     */
    tGradeSegment(const double &dist = 0, const double &wd = 0, const double &gr = 0);

    /*!
        \brief Copy constructor.

        \param orig [In] - A constant refence to a tGradeSegment to copy.
     */
    tGradeSegment(const tGradeSegment &orig);

    /*
        \brief Copy operator.

        \param rhs [In]  - a constant reference to another tGradeSegment to copy.

        \return the current object.
     */
    const tGradeSegment &operator=(const tGradeSegment &rhs);

    /*!
        \brief Less than operator.

        \param rhs [In]  - a constant reference to the tGradeSegment to compare the distance of.

        \return true if the distance of the current segment is less than the distance of rhs.
     */
    const bool operator<(const tGradeSegment &rhs) const;

    /*!
        \brief Less than or equal to operator.

        \param rhs [In]  - a constant reference to the tGradeSegment to compare the distance of.

        \return true if the distance of the current segment is less than or equal to the distance of rhs.
     */
    const bool operator<=(const tGradeSegment &rhs) const;

    /*!
        \brief Greater than operator.

        \param rhs [In]  - a constant reference to the tGradeSegment to compare the distance of.

        \return true if the distance of the current segment is greater than the distance of rhs.
     */
    const bool operator>(const tGradeSegment &rhs) const;

    /*!
        \brief Greater or equal to operator.

        \param rhs [In]  - a constant reference to the tGradeSegment to compare the distance of.

        \return true if the distance of the current segment is greater or equal to the distance of rhs.
     */
    const bool operator>=(const tGradeSegment &rhs) const;

    /*!
        \brief Equal to operator.

        \param rhs [In]  - a constant reference to the tGradeSegment to compare the distance of.

        \return true if the distance of the current segment is equal to the distance of rhs.
     */
    const bool operator==(const tGradeSegment &rhs) const;

    /*!
        \brief Not equal to operator.

        \param rhs [In]  - a constant reference to the tGradeSegment to compare the distance of.

        \return true if the distance of the current segment is not equal to the distance of rhs.
     */
    const bool operator!=(const tGradeSegment &rhs) const;

    const bool DeltaH(const double &distToPoint, double *deltaHeight) const;
};

//!    \brief A wrapper for a list of grade segments.
struct KEAYS_MATH_EXPORTS_API tGradeSegmentList
{
    tGradeSegmentList();
    tGradeSegmentList(const tGradeSegmentList &orig);
    ~tGradeSegmentList();

    const bool AddSegment(const tGradeSegment &newSegment, std::list<tGradeSegment>::iterator *it);
    const double DeltaH(const double &distToPoint) const;
    const size_t Size() const;
    void Clear();
    const tGradeSegment &Last() const;

    std::list<tGradeSegment> *m_pSegments;
};

//=============================================================================
/*!
    \name Angle Conversion Functions
    @{
 */
/*!
    \brief    Get the degrees, minutes and seconds from a decimal angle.

    \param     angle [In]  - A constant reference to a <b>double</b> representing the angle in degrees to get the degrees, minutes and seconds of.
    \param   degrees [Out] - An <b>int</b> to receive the number of degrees.
    \param   minutes [Out] - An <b>int</b> to receive the number of minutes.
    \param   seconds [Out] - A reference to a <b>double</b> to receive the number of seconds.
    \param tolerance [In]  - A constant reference to a <b>double</b> specifying the tolerance to use for calculation.  This is to offset any inaccuracy due to floating point error.
 */
KEAYS_MATH_EXPORTS_API void
GetDegreesMinutesSeconds(const double &angle, int &degrees, int &minutes, double &seconds,
                         const double &tolerance = keays::types::Float::TOLERANCE);

/*!
    \brief    Get the degrees, minutes and seconds from a decimal angle while ensuring appropriate rounding.

    \param     angle [In]  - A constant reference to a <b>double</b> representing the angle in degrees to get the degrees, minutes and seconds of.
    \param   degrees [Out] - An <b>int</b> to receive the number of degrees.
    \param   minutes [Out] - An <b>int</b> to receive the number of minutes.
    \param   seconds [Out] - An <b>int</b> to receive the number of seconds.
    \param tolerance [In]  - A constant reference to a <b>double</b> specifying the tolerance to use for calculation.  This is to offset any inaccuracy due to floating point error.
 */
KEAYS_MATH_EXPORTS_API void
GetDegreesMinutesSeconds(const double &angle, int &degrees, int &minutes, int &seconds,
                         const double &tolerance = keays::types::Float::TOLERANCE);

/*!
    \brief    Get the decimal degrees of an angle from degrees, minutes and seconds.

    \param    degrees [In]  - A constant <b>int</b> representing the number of degrees.
    \param    minutes [In]  - A constant <b>int</b> representing the number of minutes.
    \param    seconds [In]  - A constant reference to a <b>double</b> representing the number of seconds.
    \param    angle   [Out] - A reference to a <b>double</b> to receive the angle in decimal degrees.

    \return A constant reference to a double representing the angle in decimal form.
 */
KEAYS_MATH_EXPORTS_API const double &
GetDecimalDegrees(const int degrees, const int minutes, const double &seconds, double &angle);

/*!
    \brief    Angle conversion functions. Convert from degrees to radians.

    \param    degrees [In] - A <b>double</b> representing the angle in degrees.

    \return A double representing the angle in radians.
 */
inline const double DTR(const double &degrees)
{
    return ((KM_PI) * (degrees/180.0));
}

/*!
    \overload

    \param    degrees [In] - A <b>float</b> representing the angle in degrees.

    \return A float representing the angle in radians.
 */
inline const float DTR(const float degrees)
{
    return (float(KM_PI) * (degrees/180.0f));
}

/*!
    \brief    Angle conversion functions. Convert from radians to degrees.

    \param    radians [In] - A <b>double</b> representing the angle in radians.

    \return A double representing the angle in radians.
 */
inline const double RTD(const double &radians)
{
    return ((radians * 180.0)/(KM_PI));
}

/*!
    \overload

    \param    radians [In] - A <b>float</b> representing the angle in radians.

    \return A float representing the angle in radians.
 */
inline const float RTD(const float radians)
{
    return ((radians * 180.0f)/float(KM_PI));
}

/*!
    \brief    Reduce an angle to satisfy [ \f$-2\pi \le \theta \le 2\pi\f$ ].

    \param    radians [In] - A <b>double</b> representing the angle in radians to get the absolute of.

    \return    A double representing the absolute angle in radians.
 */
inline const double ReduceAngle(const double &radians)
{
    return fmod(radians, KM_2PI);
}

/*!
    \overload

    \param    radians [In] - A <b>float</b> representing the angle in radians to get the absolute of.

    \return    A float representing the absolute angle in radians.
 */
inline const float ReduceAngle(const float &radians)
{
    return fmod(radians, (float)KM_2PI);
}

/*!
    \brief    Reduce an angle to satisfy [ \f$-\frac {\pi} {2} \le \theta \le \frac {\pi} {2}\f$ ].

    \param    radians [In] - A <b>double</b> representing the angle in radians to get the reduced angle of.

    \return    A double representing the absolute angle in radians.
 */
inline const double ReduceAngleHalfPi(const double &radians)
{
    double ang = ReduceAngle(radians);
    return ang < -KM_PI ? ang + KM_2PI : ang > KM_PI ? ang - KM_2PI : ang;
}

/*!
    \overload

    \param    radians [In] - A <b>float</b> representing the angle in radians to get the reduced angle of.

    \return    A float representing the absolute angle in radians.
 */
inline const float ReduceAngleHalfPi(const float &radians)
{
    float ang = ReduceAngle(radians);
    return ang < (float)-KM_PI ? ang + (float)KM_2PI : ang > (float)KM_PI ? ang - (float)KM_2PI : ang;
}

/*!
    \brief    Reduce an angle to satisfy [ \f$360^o \le \theta \le 360^o\f$ ]<!-- 360 < degrees < 360 -->

    \param    degrees [In] - A <b>double</b> representing the angle in degrees to get the absolute of.

    \return    A double representing the absolute angle in degrees.
 */
inline const double ReduceAngleDegrees(const double &degrees)
{
    return fmod(degrees, 360.0);
}

/*!
    \overload

    \param    degrees [In] - A <b>float</b> representing the angle in degrees to get the absolute of.

    \return    A float representing the absolute angle in degrees.
 */
inline const float ReduceAngleDegrees(const float &degrees)
{
    return fmod(degrees, 360.0f);
}

/*!
    \brief    Reduce an angle to satisfy [ \f$-180^o \le \theta \le 180^o\f$ ]<!-- -180 < degrees < 180 -->.

    \param    degrees [In] - A <b>double</b> representing the angle in degrees to get the absolute of.

    \return    A double representing the absolute angle in degrees.
 */
inline const double ReduceAngle180Degrees(const double &degrees)
{
    double ang = ReduceAngleDegrees(degrees);
    return ang < -180.0 ? ang + 360.0 : ang > 180.0 ? ang - 360.0 : ang;
}

/*!
    \overload

    \param    degrees [In] - A <b>float</b> representing the angle in degrees to get the absolute of.

    \return    A float representing the absolute angle in degrees.
 */
inline const float ReduceAngle180Degrees(const float &degrees)
{
    float ang = ReduceAngleDegrees(degrees);
    return ang < -180.0f ? ang + 360.0f : ang > 180.0f ? ang - 360.0f : ang;
}

/*!
    \brief    Get the absolute angle [ \f$0 \le \theta \le 2\pi\f$ ].

    \param    radians [In] - A <b>double</b> representing the angle in radians to get the absolute of.

    \return    A double representing the absolute angle in radians.
 */
inline const double GetAbsoluteAngle(const double &radians)
{
    double ang = ReduceAngle(radians);
    return ang < 0.0 ? ang += KM_2PI : ang;
}

/*!
    \overload

    \param    radians [In] - A <b>float</b> representing the angle in radians to get the absolute of.

    \return    A float representing the absolute angle in radians.
 */
inline const float GetAbsoluteAngle(const float radians)
{
    float ang = ReduceAngle(radians);
    return ang < 0.0 ? ang += (float)KM_2PI : ang;
}

/*!
    \brief    Get the absolute angle [ \f$0^o \le \theta \le 360^o\f$ ].

    \param    degrees [In] - A <b>double</b> representing the angle in degrees to get the absolute of.

    \return    A double representing the absolute angle in degrees.
 */
inline const double GetAbsoluteAngleDegrees(const double &degrees)
{
    double ang = ReduceAngleDegrees(degrees);
    return ang < 0.0 ? ang += 360.00000000000000 : ang;
}

/*!
    \overload

    \param    degrees [In] - A <b>float</b> representing the angle in degrees to get the absolute of.

    \return    A float representing the absolute angle in degrees.
 */
inline const float GetAbsoluteAngleDegrees(const float &degrees)
{
    float ang = ReduceAngleDegrees(degrees);
    return ang < 0.0f ? ang += 360.00000f : ang;
}

/*!
    \brief    Convert an angle in degrees from a compass bearing (North = 0\f$^o\f$, angles are clockwise) to a mathematical angle.

    \param    bearingDegrees [In] - A constant <b>double</b> representing the compass bearing to convert.

    \return    A double representing the equivalent mathematical angle in degrees.
 */
inline const double CompassToMathDegrees(const double &bearingDegrees)
{
    return GetAbsoluteAngleDegrees(360.0 - (bearingDegrees - 90.0));
}

/*!
    \overload

    \param    bearingDegrees [In] - A constant <b>double</b> representing the compass bearing to convert.

    \return    A double representing the equivalent mathematical angle in degrees.
 */
inline const float CompassToMathDegrees(const float &bearingDegrees)
{
    return GetAbsoluteAngleDegrees(360.0f - (bearingDegrees - 90.0f));
}

/*!
    \brief    Convert an angle in radians from a compass bearing (North = 0, angles are clockwise) to a mathematical angle.

    \param    bearingRadians [In] - A constant <b>double</b> representing the compass bearing to convert.

    \return    A double representing the equivalent mathematical angle in radians.
 */
inline const double CompassToMathRadians(const double &bearingRadians)
{
    return GetAbsoluteAngle(KM_2PI - (bearingRadians - KM_PI_ON2));
}

/*!
    \overload

    \param    bearingRadians [In] - A constant <b>float</b> representing the compass bearing to convert.

    \return    A float representing the equivalent mathematical angle in radians.
 */
inline const float CompassToMathRadians(const float &bearingRadians)
{
    return GetAbsoluteAngle((float)KM_2PI - (bearingRadians - (float)KM_PI_ON2));
}

/*!
    \brief    Convert an angle in degrees from a mathematical angle to a compass bearing (North = 0\f$^o\f$, angles are clockwise).

    \param    angleDegrees [In] - A constant <b>double</b> representing the mathematical angle to convert.

    \return    A double representing the equivalent compass bearing in degrees.
 */
inline const double MathToCompassDegrees(const double &angleDegrees)
{
    return GetAbsoluteAngleDegrees((360.0 - angleDegrees) + 90.0);
}

/*!
    \overload

    \param    angleDegrees [In] - A constant <b>float</b> representing the mathematical angle to convert.

    \return    A float representing the equivalent compass bearing in degrees.
 */
inline const float MathToCompassDegrees(const float angleDegrees)
{
    return GetAbsoluteAngleDegrees((360.0f - angleDegrees) + 90.0f);
}

/*!
    \brief    Convert an angle in radians from a mathematical angle to a compass bearing (North = 0, angles are clockwise).

    \param    angleRadians [In] - A constant <b>double</b> representing the mathematical angle to convert.

    \return    A double representing the equivalent compass bearing in radians.
 */
inline const double MathToCompassRadians(const double &angleRadians)
{
    return GetAbsoluteAngle((KM_2PI - angleRadians) + KM_PI_ON2);
}

/*!
    \brief    Convert an angle in radians from a mathematical angle to a compass bearing (North = 0, angles are clockwise).

    \param    angleRadians [In] - A constant <b>float</b> representing the mathematical angle to convert.

    \return    A float representing the equivalent compass bearing in radians.
 */
inline const float MathToCompassRadians(const float angleRadians)
{
    return GetAbsoluteAngle(((float)KM_2PI - angleRadians) + (float)KM_PI_ON2);
}
//! @}

//=============================================================================
/*!
    \name Angle Comparison Functions
    @{
 */
/*!
    \brief Angle comparison enums
    Angles cannot always be simply compared with normal boolean comparisons.  As a result we use clockwise, and
    anti(counter) clockwise.  There are also 2 special cases: equal and opposite, as both of these cases are
    neither clockwise nor counter clockwise.
 */
enum KEAYS_MATH_EXPORTS_API eAngleComparisons
{
    ANGLE_EQUAL,        //!< angles are equal.
    ANGLE_OPPOSITE,        //!< angles are opposite.
    ANGLE_CW,            //!< angle is clockwise.
    ANGLE_CCW            //!< angle is counter-clockwise.
};

/*!
    \brief Test if an angle is clockwise, counterclockwise, equal or opposite of another.
    This is the extended funtion that can give one of 4 return values, it is frequently simpler to use one of the boolean functions.

    \param     refAngle [In]  - A constant reference to a <b>double</b> specifying the reference angle (in radians), this is tested against.
    \param    testAngle [In]  - A constant reference to a <b>double</b> specifying the test angle (in radians), this is the angle to test.
    \param    tolerance [In]  - A constant reference to a <b>double</b> representing the tolerance to use in the test [optional].
    \param    diffAngle [Out] - A pointer to a <b>double</b> to receive the angular difference between the reference and test angles (in radians) [optional].  WARNING: THIS IS NOT CURRENTLY IMPLEMENTED.

    \return One of (ANGLE_EQUAL | ANGLE_OPPOSITE | ANGLE_CW | ANGLE_CCW)
 */
KEAYS_MATH_EXPORTS_API const eAngleComparisons
AngleIsClockwiseEx(const double &refAngle, const double &testAngle,
                   const double &tolerance = keays::types::Float::TOLERANCE,
                   double *diffAngle = NULL);

/*!
    \brief Test if an angle is clockwise of another.

    \param     refAngle [In] - A constant reference to a <b>double</b> specifying the reference angle (in radians), this is tested against.
    \param    testAngle [In] - A constant reference to a <b>double</b> specifying the test angle (in radians), this is the angle to test.
    \param    tolerance [In] - A constant reference to a <b>double</b> representing the tolerance to use in the test [optional].

    \return True if the test angle is clockwise of the reference angle.

    \note    Equal and opposite angles are not considered clockwise for the purposes of this function, as a result it will return false for these cases.
 */
inline bool AngleIsClockwise(const double &refAngle, const double &testAngle,
                             const double &tolerance = keays::types::Float::TOLERANCE)
{
    return (AngleIsClockwiseEx(refAngle, testAngle, tolerance) == ANGLE_CW);
}

/*!
    \brief Test if an angle is counter clockwise of another.

    \param     refAngle [In] - A constant reference to a <b>double</b> specifying the reference angle (in radians), this is tested against.
    \param    testAngle [In] - A constant reference to a <b>double</b> specifying the test angle (in radians), this is the angle to test.
    \param    tolerance [In] - A constant reference to a <b>double</b> representing the tolerance to use in the test [optional].

    \return True if the test angle is counter clockwise of the reference angle.

    \note    Equal and opposite angles are not considered counter clockwise for the purposes of this function, as a result it will return false for these cases.
 */
inline bool AngleIsCounterClockwise(const double &refAngle, const double &testAngle,
                                    const double &tolerance = keays::types::Float::TOLERANCE)
{
    return (AngleIsClockwiseEx(refAngle, testAngle, tolerance) == ANGLE_CCW);
}

/*!
    \brief Calculate the absolute acute angular difference.

    \param     refAngle [In] - A constant reference to a <b>double</b> specifying the reference angle (in radians), this is tested against.
    \param    testAngle [In] - A constant reference to a <b>double</b> specifying the test angle (in radians), this is the angle to test.

    \return    A <b>double</b> representing the absolute (<\f$\pi\f$) angle between the reference and test angles.
 */
inline const double AngularDifference(const double &refAngle, const double &testAngle)
{
    double redRefAng = ReduceAngleHalfPi(refAngle);
    double redTestAng = ReduceAngleHalfPi(testAngle);
    double diff = ReduceAngleHalfPi(redRefAng - redTestAng);

    return abs(diff);
}

/*!
    \overload

    \param     refAngle [In] - A constant reference to a <b>float</b> specifying the reference angle (in radians), this is tested against.
    \param    testAngle [In] - A constant reference to a <b>float</b> specifying the test angle (in radians), this is the angle to test.

    \return A <b>float</b> representing the absolute (<\f$\pi\f$) angle between the reference and test angles.
 */
inline const float AngularDifference(const float &refAngle, const float &testAngle)
{
    float redRefAng = ReduceAngleHalfPi(refAngle);
    float redTestAng = ReduceAngleHalfPi(testAngle);
    float diff = ReduceAngleHalfPi(redRefAng - redTestAng);

    return abs(diff);
}

//! @}

//==============================  CONSTANTS  ==================================
/*!
    \name Generation Functions
    @{
 */
//! \brief Direction enum.
enum KEAYS_MATH_EXPORTS_API eAngleDirections
{
    CCW = 0,    //!< <b>C</b>ounter <b>C</b>lockwise <b>W</b>ise.
    CW = 1,        //!< <b>C</b>lockwise <b>W</b>ise
};

//! \brief Interval type specifications for polyline/curve generation.
enum KEAYS_MATH_EXPORTS_API eIntervalType
{
    INTERVAL_IS_ANGLE = 0,    //!< The interval is an angle in radians.
    INTERVAL_IS_ANGLE_DEG,    //!< The interval is an angle in degrees.
    INTERVAL_IS_LENGTH        //!< The interval is a chord length.
};

extern KEAYS_MATH_EXPORTS_API const double DEFAULT_INTERVAL_DEG;    //!< A default interval angle in degrees.
extern KEAYS_MATH_EXPORTS_API const double DEFAULT_INTERVAL_RAD;    //!< A default interval angle in radians.
extern KEAYS_MATH_EXPORTS_API const double DEFAULT_INTERVAL_LEN;    //!< A default interval chord length.

//============  GENERATING POINTS  ============================
/*!
    \brief Generate a series of points in an arc around a specifed point.
    Generates an arc of keays::types::VectorD3 pts around a specifed point, with a given radius.  Depending on combinations of
    values and arguments, the fuction will.
    <UL>
        <LI>append or replace points in the result, generate.</LI>
        <LI>generate a duplicate end point to close an arc.</LI>
        <LI>include extra angles that do not fall on the specified interval.</LI>
    </UL>

    \param              center [In]  - A constant reference to a keays::types::VectorD3 specifying the center position.
    \param              radius [In]  - A constant reference to a <b>double</b> specifying the radius.
    \param       startAngleRad [In]  - A constant reference to a <b>double</b> specifying the start angle.
    \param         endAngleRad [In]  - A constant reference to a <b>double</b> specifying the end angle.
    \param              result [I/O] - A reference to a  keays::types::Polyline3D.
    \param     intervalRadians [In]  - A constant reference to a <b>double</b> specifying the interval angle.
    \param         clearResult [In]  - A <b>bool</b> indicating if the result vector should be cleared.
    \param          doEndPoint [In]  - A <b>bool</b> indicating if the last point should also be generated.
    \param           direction [In]  - One of (CW | CCW) indicating the direction for generating the points.
    \param importantAnglesList [I/O] - A pointer to an array of <b>doubles</b> specifying the important angles to include.
    \param  numImportantAngles [In]  - An <b>int</b> indicating the number of important angles in the array.  This value is ignored if <I>importantAnglesList</I> is NULL.
    \param            isCircle [In]  - A <b>boolean</b> value indicating if we are generating an arc or a full circle.

    \return true if successful.
 */
KEAYS_MATH_EXPORTS_API bool
GenArcPointsRad(const keays::types::VectorD3 &center, const double &radius, const double &startAngleRad,
                const double &endAngleRad, keays::types::Polyline3D &result, const double &intervalRadians,
                const bool clearResult, const bool doEndPoint, const eAngleDirections direction,
                double *importantAnglesList = NULL, const int numImportantAngles = 0, bool isCircle = false);

/*!
    \brief Generate a series of points in an arc around a specifed point in a Clockwise direction.
    Generates an arc of keays::types::VectorD3 pts around a specifed point, with a given radius.  Depending on combinations of
    values and arguments, the fuction will.
    <UL>
        <LI>append or replace points in the result, generate.</LI>
        <LI>generate a duplicate end point to close an arc.</LI>
        <LI>include extra angles that do not fall on the specified interval.</LI>
    </UL>

    \param              center [In]  - A constant reference to a keays::types::VectorD3 specifying the center position.
    \param              radius [In]  - A constant reference to a <b>double</b> specifying the radius.
    \param circleStartAngleRad [In]  - A constant reference to a <b>double</b> specifying the start angle of the circle the arc is on in radians.
    \param         intervalRad [In]  - A constant reference to a <b>double</b> specifying the interval angle in radians.
    \param       startAngleRad [In]  - A constant reference to a <b>double</b> specifying the start angle of the arc in radians.
    \param         endAngleRad [In]  - A constant reference to a <b>double</b> specifying the end angle of the arc in radians, this is ignored if generating a circle.
    \param            isCircle [In]  - A <b>boolean</b> value indicating if we are generating an arc or a full circle.
    \param       genStartPoint [In]  - A <b>bool</b> indicating if the first point should be generated (allows for the arc to be tacked onto the end of an existing polyline).
    \param         genEndPoint [In]  - A <b>bool</b> indicating if the last point should also be generated.
    \param                 pts [I/O] - A reference to a  keays::types::Polyline3D for the generated points.
    \param importantAnglesList [I/O] - A pointer to an array of <b>doubles</b> specifying the important angles to include
    \param  numImportantAngles [In]  - An <b>int</b> indicating the number of important angles in the array.  This value is ignored if <I>importantAnglesList</I> is NULL.

    \return True if successful.
 */
KEAYS_MATH_EXPORTS_API bool
GenArcPointsRadCWEx(const keays::types::VectorD3 &center, const double &radius, const double &circleStartAngleRad,
                    const double &intervalRad, const double &arcStartAngleRad, const double &arcEndAngleRad,
                    const bool isCircle, const bool genStartPoint, const bool genEndPoint,
                    keays::types::Polyline3D &pts, double *importantAngles = NULL, const int numImportantAngles = 0);

/*!
    \brief Generate a series of points in an arc around a specifed point in a Counter Clockwise direction.
    Generates an arc of keays::types::VectorD3 pts around a specifed point, with a given radius.  Depending on combinations of
    values and arguments, the fuction will.
    <UL>
        <LI>append or replace points in the result, generate.</LI>
        <LI>generate a duplicate end point to close an arc.</LI>
        <LI>include extra angles that do not fall on the specified interval.</LI>
    </UL>

    \param              center [In]  - A constant reference to a keays::types::VectorD3 specifying the center position.
    \param              radius [In]  - A constant reference to a <b>double</b> specifying the radius.
    \param circleStartAngleRad [In]  - A constant reference to a <b>double</b> specifying the start angle of the circle the arc is on in radians.
    \param         intervalRad [In]  - A constant reference to a <b>double</b> specifying the interval angle in radians.
    \param       startAngleRad [In]  - A constant reference to a <b>double</b> specifying the start angle of the arc in radians.
    \param         endAngleRad [In]  - A constant reference to a <b>double</b> specifying the end angle of the arc in radians, this is ignored if generating a circle.
    \param            isCircle [In]  - A <b>boolean</b> value indicating if we are generating an arc or a full circle.
    \param       genStartPoint [In]  - A <b>bool</b> indicating if the first point should be generated (allows for the arc to be tacked onto the end of an existing polyline).
    \param         genEndPoint [In]  - A <b>bool</b> indicating if the last point should also be generated.
    \param                 pts [I/O] - A reference to a  keays::types::Polyline3D for the generated points.
    \param importantAnglesList [I/O] - A pointer to an array of <b>doubles</b> specifying the important angles to include.
    \param  numImportantAngles [In]  - An <b>int</b> indicating the number of important angles in the array.  This value is ignored if <I>importantAnglesList</I> is NULL.

    \return True if successful.
 */
KEAYS_MATH_EXPORTS_API bool
GenArcPointsRadCCWEx(const keays::types::VectorD3 &center, const double &radius, const double &circleStartAngleRad,
                     const double &intervalRad, const double &arcStartAngleRad, const double &arcEndAngleRad,
                     const bool isCircle, const bool genStartPoint, const bool genEndPoint,
                     keays::types::Polyline3D &pts, double *importantAngles = NULL, const int numImportantAngles = 0);

/*!
    \brief Generate a series of points in an arc around a specifed point.
    Generates an arc of keays::types::VectorD3 pts around a specifed point, with a given radius.  Depending on combinations of
    values and arguments, the fuction will.
    <UL>
        <LI>Append or replace points in the result, generate.</LI>
        <LI>Generate a duplicate end point to close an arc.</LI>
        <LI>Include extra angles that do not fall on the specified interval.</LI>
    </UL>

    \param              center [In]  - A constant reference to a keays::types::VectorD3 specifying the center position.
    \param              radius [In]  - A constant reference to a <b>double</b> specifying the radius.
    \param circleStartAngleRad [In]  - A constant reference to a <b>double</b> specifying the start angle.
    \param         intervalRad [In]  - A constant reference to a <b>double</b> specifying the interval angle.
    \param    arcStartAngleRad [In]  - A constant reference to a <b>double</b> specifying the start angle.
    \param      arcEndAngleRad [In]  - A constant reference to a <b>double</b> specifying the end angle.
    \param            isCircle [In]  - A constant boolean value indicating if the arc is a complete circle.
    \param       genStartPoint [In]  - A constant boolean value indicating if the first point of the arc should be generated.
    \param         genEndPoint [In]  - A constant boolean value indicating if the last point of the arc should be generated.
    \param                 dir [In]  - One of (CW | CCW) indicating the direction for generating the points.
    \param                 pts [I/O] - A reference to a  keays::types::Polyline3D.
    \param     importantAngles [I/O] - A pointer to an array of <b>tImportantAngle</b>elements specifying the important angles to include.
    \param  numImportantAngles [In]  - An <b>int</b> indicating the number of important angles in the array.  This value is ignored if <I>importantAnglesList</I> is NULL.

    \return True if successful.
 */
KEAYS_MATH_EXPORTS_API bool
GenArcPointsRadEx(const keays::types::VectorD3 &center, const double &radius, const double &circleStartAngleRad,
                  const double &intervalRad, const double &arcStartAngleRad, const double &arcEndAngleRad,
                  const bool isCircle, const bool genStartPoint, const bool genEndPoint,
                  const eAngleDirections dir, keays::types::Polyline3D &pts, double *importantAngles = NULL,
                  const int numImportantAngles = 0);

/*!
    \brief Generate a series of points in an circle around a specifed point.
    Generates an circle of keays::types::VectorD3 pts around a specifed point, with a given radius and direction.

    \param              center [In]  - A constant reference to a keays::types::VectorD3 specifying the center position.
    \param              radius [In]  - A constant reference to a <b>double</b> specifying the radius.
    \param              result [Out] - A reference to a  keays::types::Polyline3D for the resulting points.
    \param     intervalRadians [In]  - A constant reference to a <b>double</b> specifying the interval angle.
    \param           direction [In]  - One of (CW | CCW) indicating the direction for generating the points.
    \param          startAngle [In]  - A constant reference to a <b>double</b> specifying the start angle (in radians).
    \param importantAnglesList [I/O] - A pointer to an array of <b>doubles</b> specifying the important angles to include.
    \param  numImportantAngles [In]  - An <b>int</b> indicating the number of important angles in the array.  This value is ignored if <I>importantAnglesList</I> is NULL.

    \return True if successful.
 */
inline bool
GenCircle(const keays::types::VectorD3 &center, const double &radius, keays::types::Polyline3D &result,
          const double &intervalRadians = DEFAULT_INTERVAL_RAD, const eAngleDirections direction = CCW,
          const double &startAngle = 0, double *importantAnglesList = NULL, const int numImportantAngles = 0)
{
    return GenArcPointsRad(center, radius, startAngle, 0, result, intervalRadians,
                            true, true, direction, importantAnglesList, numImportantAngles, true);
}

/*!
    \brief Generate a series of points in an circle around a specifed point.
    Generates an circle of keays::types::VectorD3 pts around a specifed point, with a given radius and direction.

    \param              center [In]  - A constant reference to a keays::types::VectorD3 specifying the center position.
    \param              radius [In]  - A constant reference to a <b>double</b> specifying the radius.
    \param circleStartAngleRad [In]  - A constant reference to a <b>double</b> specifying the start angle.
    \param         intervalRad [In]  - A constant reference to a <b>double</b> specifying the interval angle.
    \param           direction [In]  - One of (CW | CCW) indicating the direction for generating the points.
    \param              result [Out] - A reference to a  keays::types::Polyline3D for the resulting points.
    \param       genStartPoint [In]  - A constant boolean value indicating if the first point of the arc should be generated.
    \param         genEndPoint [In]  - A constant boolean value indicating if the last point of the arc should be generated.
    \param     importantAngles [I/O] - A pointer to an array of <b>tImportantAngle</b>elements specifying the important angles to include.
    \param  numImportantAngles [In]  - An <b>int</b> indicating the number of important angles in the array.  This value is ignored if <I>importantAnglesList</I> is NULL.

    \return True if successful.
 */
inline bool GenCircleEx(const keays::types::VectorD3 &center, const double &radius, const double &circleStartAngleRad,
                        const double &intervalRad, const eAngleDirections direction, keays::types::Polyline3D &result,
                        const bool genStartPoint, const bool genEndPoint, double *importantAngles = NULL,
                        const int numImportantAngles = 0)
{
    return GenArcPointsRadEx(center, radius, circleStartAngleRad, intervalRad, 0, KM_2PI, true, genStartPoint,
                             genEndPoint, direction, result, importantAngles, numImportantAngles);
}

/*!
    \brief Generate height values for series of points.
    Generates height values for a series of keays::types::VectorD3 pts given a center point, a direction of fall, and a percentage gradient.

    \param    center [In]  - A constant reference to a keays::types::VectorD3 specifying a point on the plane.
    \param fallAngle [In]  - A constant reference to a <b>double</b> representing the direction of fall (in radians).
    \param   fallPct [In]  - A constant reference to a <b>double</b> representing the gradient of fall of the plane.
    \param       pts [I/O] - A reference to a  keays::types::Polyline3D points to generate heights for.

    \return True if successful.
 */
KEAYS_MATH_EXPORTS_API bool
GenPointsHeight(const keays::types::VectorD3 &center, const double &fallAngle,
                const double &fallPct, keays::types::Polyline3D &pts);

/*!
    \overload
    \brief Generate height values for series of points, accounting for cross grades.

    \param p_cGrades [In] - A constant pointer to a tGradeSegmentList object containing the extra grades to calc.
 */
KEAYS_MATH_EXPORTS_API bool
GenPointsHeight(const keays::types::VectorD3 &center, const double &fallAngle, const double &fallPct,  keays::types::Polyline3D &pts,
                 const tGradeSegmentList *p_cGrades);

/*!
    \brief Generate height values for a point.
    Generates height values for a keays::types::VectorD3 point given a center point, a direction of fall, and a percentage gradient

    \param    center [In]  - A constant reference to a keays::types::VectorD3 specifying the center of the fall.
    \param fallAngle [In]  - A constant reference to a <b>double</b> direction of fall (in radians).
    \param   fallPct [In]  - A constant reference to a <b>double</b> specifying gradient.
    \param        pt [I/O] - A reference to a keays::types::VectorD3 point to generate height for.

    \return True if successful.
 */
KEAYS_MATH_EXPORTS_API bool
GenPointHeight(const keays::types::VectorD3 &center, const double &fallAngle,
               const double &fallPct, keays::types::VectorD3 &pt);

/*!
    \overload
    \brief Generate height values for a point, compensating for a series of radial grades at set distances as well.

    \param p_cGrades [In]  - A constant pointer to a tGradeSegmentList object containing the extra grades to calc.
 */
KEAYS_MATH_EXPORTS_API bool
GenPointHeight(const keays::types::VectorD3 &center, const double &fallAngle,
               const double &fallPct, keays::types::VectorD3 &pt,
               const tGradeSegmentList *p_cGrades);

/*!
    \brief Generate more points on an existing polyline.
    Calculate more points in 3D space that fit along the given polyline. Polyline must contain at least two points and interval must be greater than 0.

    \param                polyline [In]  - A constant reference to a  keays::types::Polyline3D that represent the polyline to generate points on.
    \param                interval [In]  - A constant reference to a <b>double</b> representing the distance along the polyline that each point will occur at.
    \param            generatedPts [Out] - A reference to a  keays::types::Polyline3D that the generated points are pushed onto.
    \param        bIncludeOriginal [In]  - A constant boolean flag indicating if the original points from polyline will be included.
    \param minDistanceFromOriginal [In]  - A constant refernce to a double precision number specifying the minimum distance a generated point must be from an existing point to be included.

    \return true if successful.
 */
KEAYS_MATH_EXPORTS_API bool
GenDivisons(const keays::types::Polyline3D &polyline, const double &interval,
            keays::types::Polyline3D &generatedPts,
            const bool bIncludeOriginal = false,
            const double &minDistanceFromOriginal = 0.01);

/*!
    \brief Generate a parallel offset a given distance from an existing line.

    \param     pt1 [In] - A constant reference to a keays::types::VectorD2 specifying the start of the line.
    \param     pt2 [In] - A constant reference to a keays::types::VectorD2 specifying the end of the line.
    \param    dist [In] - A constant reference to a <b>double</b> specifying the distance to offset.
    \param    side [In] - One of SIDE_LEFT or SIDE_RIGHT to determine which side to offset. Side is determined by the direction of the source polyline.

    \return    False on failure, true on success.
 */
inline const Line Offset(const keays::types::VectorD2 &pt1, const keays::types::VectorD2 &pt2,
                         const double &dist, const eSideSelections side)
{
    return Line(pt1, pt2).CalcOffset(dist, side);
}

/*!
    \brief Generate a parallel offset a given distance from an existing line.

    \param   pt1 [In] - A constant reference to a keays::types::VectorD2 specifying the start of the line.
    \param   pt2 [In] - A constant reference to a keays::types::VectorD2 specifying the end of the line.
    \param  dist [In] - A constant reference to a <b>double</b> specifying the distance to offset.
    \param refPt [In] - A constant reference to a keays::types::VectorD2 as a reference point to determine the side to offset.

    \return    False on failure, true on success.
 */
inline const Line Offset(const keays::types::VectorD2 &pt1, const keays::types::VectorD2 &pt2, const double &dist, const keays::types::VectorD2 &refPt)
{
    return Line(pt1, pt2).CalcOffset(dist, refPt);
}

//-----------------------------------------------
/*!
    \brief Calculate a position in 2D space, given a bearing (in radians) and a distance.

    \param     base [In] - a constant reference to a keays::types::VectorD2 specifying the base point for the operation.
    \param distance [In] - a double specifying the distance for generation.
    \param  radians [In] - a double specifying a bearing in radians.

    \return    a keays::types::VectorD2 specifying the generated point.
 */
KEAYS_MATH_EXPORTS_API const keays::types::VectorD2
GenPolarPosRad(const keays::types::VectorD2 &base, const double &distance, const double &radians);

/*!
    \overload Calculate a position in 3D space on a 2D plane, given a bearing (in radians) and a distance.

    \param base [In] - a keays::types::VectorD2 specifying the base point for the operation.

    \return    a keays::types::VectorD3 specifying the generated point.
 */
KEAYS_MATH_EXPORTS_API const keays::types::VectorD3
GenPolarPosRad(const keays::types::VectorD3 &base, const double &distance, const double &radians);

/*!
    \brief Calculate a position in 3D space, given a bearing (radians), an zenith (radians) and a distance.

    \param     base [In] - a keays::types::VectorD2 specifying the base point for the operation.
    \param distance [In] - a double specifying the distance for generation.
    \param  bearing [In] - a double specifying a bearing in radians.
    \param   zenith [In] - a double specifying an zenith in radians.

    \return    a keays::types::VectorD2 specifying the generated point.
 */
KEAYS_MATH_EXPORTS_API const keays::types::VectorD3
GenPolarPosRad(const keays::types::VectorD3 &base, const double &distance, const double &bearing, const double &zenith);

//VectorD3 GetPolarPosRad(const keays::types::VectorD3 base, const double &distance, const double &radians, const double &strike, const double &dip)

/*! \brief Calculate a position in 2D space, given a bearing (in degrees) and a distance.

    \param     base [In] - a keays::types::VectorD2 specifying the base point for the operation.
    \param distance [In] - a double specifying the distance for generation.
    \param  degrees [In] - a double specifying a bearing in degrees.

    \return    a keays::types::VectorD2 specifying the generated point.
 */
KEAYS_MATH_EXPORTS_API const keays::types::VectorD2
GenPolarPos(const keays::types::VectorD2 &base, const double &distance, const double &degrees);

/*!
    \brief Generate vectors perpendicular to the specified polyline, in the XY plane.

    \param       polyline [In]  - a constant reference to a  keays::types::Polyline3D represengint the polyline to calculate from.
    \param   pPerpVectors [Out] - A pointer to a STL::vector of keays::types::VectorD3's to receive the resulting perpendicular vectors.
    \param averageVectors [In]  - a boolean flag indicating if the vectors should be averaged between points.
    \param       isClosed [In]  - a boolean flag indicating if the polyline is closed, and the start and end vectors should be averaged.

    \return a constant pointer to the STL::vector of keays::types::VectorD3's or NULL on failure.
 */
KEAYS_MATH_EXPORTS_API const keays::types::Polyline3D *
GeneratePerpendicularVectors(const keays::types::Polyline3D &polyline,  keays::types::Polyline3D *pPerpVectors,
                             bool averageVectors = true, bool isClosed = false);

/*!
    \brief Insert a new point at a given chainage from the start of the polyline.

    \param    pPolyline [I/O] - a pointer to a  keays::types::Polyline3D to have the point inserted.
    \param     chainage [In]  - a constant reference to a <b>double</b> specifying the distance from the begining of the string to insert.
    \param pInsertIndex [In]  - a pointer to a <b>size_t</b> to receive the index the point was inserted at. If a point already.
                                exists at that chainage it is the index of that point.

    \return    true if the point was successfully inserted, false otherwise.
 */
KEAYS_MATH_EXPORTS_API bool
InsertPoint(keays::types::Polyline3D *pPolyline, const double &chainage, size_t *pInsertIndex = NULL);

/*!
    \overload

    \param pPolyline [I/O] - a pointer to a Polyline2D to have the point inserted.
 */
KEAYS_MATH_EXPORTS_API bool
InsertPoint(keays::types::Polyline2D *pPolyline, const double &chainage, size_t *pInsertIndex = NULL);

/*!
    \brief Insert a points at a given interval from the start of the polyline.

    \param   srcPolyline [In]  - a constant reference to a  keays::types::Polyline3D as the source (reference) polyline.
    \param pDestPolyline [Out] - a pointer to a  keays::types::Polyline3D to have the points inserted.
    \param      chainage [In]  - a constant reference to a <b>double</b> specifying the interval to use.
    \param  pInsertIndex [In]  - a pointer to an <b>int</b> to receive the number of points added.

    \return    true if the point was successfully inserted, false otherwise.
 */
KEAYS_MATH_EXPORTS_API bool
InsertPoints(const keays::types::Polyline3D &srcPolyline,  keays::types::Polyline3D *pDestPolyline, const double &interval, int *pNumPtsAdded = NULL);

/*!
     \overload

     \param   srcPolyline [In]  - a constant reference to a Polyline2D as the source (reference) polyline.
     \param pDestPolyline [Out] - a pointer to a Polyline2D to have the points inserted.
 */
KEAYS_MATH_EXPORTS_API bool
InsertPoints(const keays::types::Polyline2D &srcPolyline, keays::types::Polyline2D *pDestPolyline,
             const double &interval, int *pNumPtsAdded = NULL);
//! @}

//==========================  UTILITY FUNCTIONS  ==============================
/*!
    \name Utility Functions
    @{
 */
/*!
    \brief Calculate the 2D Distance between 2 points.
    Calculate the 2D Distance between 2 2D points in the X/Y plane.

    \param start [in]  - a constant reference to a keays::types::VectorD2 point.
    \param   end [in]  - a constant reference to a keays::types::VectorD2 point.

    \return a <b>double</b> representing the distance between the points.
 */
inline double Dist2D(const keays::types::VectorD2 &start, const keays::types::VectorD2 &end)
{ return sqrt(((end.x-start.x)*(end.x-start.x)) + ((end.y-start.y)*(end.y-start.y))); }

/*!
    \overload Calculate the 2D Distance between 2 points.
    Calculate the 2D Distance between 2 3D points in the X/Y plane.

    \param start [in]  - a constant reference to a keays::types::VectorD3 point.
    \param   end [in]  - a constant reference to a keays::types::VectorD3 point.
 */
inline double Dist2D(const keays::types::VectorD3 &start, const keays::types::VectorD3 &end)
{ return sqrt(((end.x-start.x)*(end.x-start.x)) + ((end.y-start.y)*(end.y-start.y))); }

/*!
    \brief Calculate the 3D Distance between 2 points.
    Calculate the 3D Distance between 2 3D points in the X/Y plane.

    \param start [in]  - a constant reference to a keays::types::VectorD3 point.
    \param   end [in]  - a constant reference to a keays::types::VectorD3 point.

    \return a <b>double</b> representing the distance between the points.
 */
inline double Distance(const keays::types::VectorD3 &start, const keays::types::VectorD3 &end)
{
    return sqrt(((end.x-start.x)*(end.x-start.x)) +
                 ((end.y-start.y)*(end.y-start.y)) +
                 ((end.z-start.z)*(end.z-start.z)));
}

/*!
    \brief Calculate the Magnitude of a vector.

    \param vec [in]  - a constant reference to a keays::types::VectorD3 point.

    \return a <b>double</b> representing the distance between the points.
 */
inline double Magnitude(const keays::types::VectorD3 &vec)
{ return sqrt((vec.x*vec.x) + (vec.y*vec.y) + (vec.z*vec.z)    ); }

/*!
    \brief Calculate the 2D Direction between 2 points.
    Calculate the 2D Direction (Bearing) between 2 2D points in the X/Y plane.

    \param start [in]  - a constant reference to a keays::types::VectorD2 point.
    \param   end [in]  - a constant reference to a keays::types::VectorD2 point.

    \return a <b>double</b> representing the direction between the points (in radians).
 */
KEAYS_MATH_EXPORTS_API const double
Direction(const keays::types::VectorD2 &start, const keays::types::VectorD2 &end);

/*!
    \overload
    Calculate the 2D Direction (Bearing) between 2 3D points in the X/Y plane.

    \param start [in]  - a constant reference to a keays::types::VectorD3 point.
    \param   end [in]  - a constant reference to a keays::types::VectorD3 point.

    \return a <b>double</b> representing the direction between the points (in radians).
 */
KEAYS_MATH_EXPORTS_API const double
Direction(const keays::types::VectorD3 &start, const keays::types::VectorD3 &end);

/*!
    \brief Calculate the Sin of the Zenith between 2 points.
    Calculate the Sin of the Zenith (Ratio of height over length) between 2 3D points.

    \param start [in]  - a constant reference to a keays::types::VectorD3 point.
    \param   end [in]  - a constant reference to a keays::types::VectorD3 point.

    \return a <b>double</b> representing the sin of the zenith between the points (in radians).
 */
KEAYS_MATH_EXPORTS_API const double
ZenithSin(const keays::types::VectorD3 &start, const keays::types::VectorD3 &end);

/*!
    \brief Calculate the Zenith between 2 points.
    Calculate the Zenith (Angle of elevation) between 2 3D points.

    \param start [in]  - a constant reference to a keays::types::VectorD3 point.
    \param   end [in]  - a constant reference to a keays::types::VectorD3 point.

    \return a <b>double</b> representing the direction between the points (in radians).
 */
inline double Zenith(const keays::types::VectorD3 &start, const keays::types::VectorD3 &end)
{
    return asin(ZenithSin(start, end));
}

/*!
    \brief Calculate the Grade between 2 points.
    Calculate the Grade (gradient) between 2 3D points.

    \param start [in]  - a constant reference to a keays::types::VectorD3 point.
    \param   end [in]  - a constant reference to a keays::types::VectorD3 point.

    \return a <b>double</b> representing the gradient between the points.
 */
KEAYS_MATH_EXPORTS_API const double
Grade(const keays::types::VectorD3 &start, const keays::types::VectorD3 &end);

/*!
    \brief Calculate the 2D Length of a polyline.

    \param      pts [In]  - a constant reference to a  keays::types::Polyline3D points.
    \param    index [In]  - a constant <b>integer</b> giving the index to determine the length to.
    \param forwards [In]  - a constant <b>boolean</b> indicating if the length should be calculated from the begining or end of the vector.

    \return a <b>double</b> representing the length from the begining or end of the polyline.
 */
KEAYS_MATH_EXPORTS_API const double
Length2D(const keays::types::Polyline3D &pts, const size_t index, const bool forwards);

/*!
    \brief Calculate the 3D Length of a polyline.

    \param      pts [In]  - a constant reference to a  keays::types::Polyline3D points.
    \param    index [In]  - a constant <b>integer</b> giving the index to determine the length to.
    \param forwards [In]  - a constant <b>boolean</b> indicating if the length should be calculated from the begining or end of the vector.

    \return a <b>double</b> representing the length from the begining or end of the polyline.
 */
KEAYS_MATH_EXPORTS_API const double
Length3D(const keays::types::Polyline3D &pts, const size_t index, const bool forwards);

/*!
    \overload

    \param      pts [In]  - a constant reference to a Polyline2D points.
 */
KEAYS_MATH_EXPORTS_API const double
Length2D(const keays::types::Polyline2D &pts, const size_t index, const bool forwards);

/*!
    \brief Normalise the Vector given by the two points specified
    This is mearly a convenience wrapper function for class functions from keays::types::VectorD3.

    \param base [In] - A constant reference to a keays::types::VectorD3 specifying the base point of the vector to normalise.
    \param  tip [In] - A constant reference to a keays::types::VectorD3 specifying the point at the tip of the vector to normalise.

    \return A constant keays::types::VectorD3 representing the normalised vector from base to tip.
 */
inline const keays::types::VectorD3
Normalise(const keays::types::VectorD3 &base, const keays::types::VectorD3 &tip)
{
    return (tip - base).GetNormalised();
}

/*!
    \brief Calculate the cross product of 2 keays::types::VectorD3's

    \param A [In] - A constant reference to a keays::types::VectorD3 specifying the first Vector of the cross product.
    \param B [In] - A constant reference to a keays::types::VectorD3 specifying the second Vector of the cross product.

    \return A constant keays::types::VectorD3 representing the cross product of the two vectors.
 */
KEAYS_MATH_EXPORTS_API const keays::types::VectorD3
Cross(const keays::types::VectorD3 &A, const keays::types::VectorD3 &B);

/*!
    \brief Calculate the Dot product of 2 keays::types::VectorD2's.

    \param A [In] - A constant reference to a keays::types::VectorD2 specifying the first Vector of the dot product.
    \param B [In] - A constant reference to a keays::types::VectorD2 specifying the second Vector of the dot product.

    \return A <b>double</b> representing the dot product. [ Equivalent to \f$\|A\| \|B\|\cos(\theta)\f$ ].
 */
KEAYS_MATH_EXPORTS_API const double
Dot(const keays::types::VectorD2 &A, const keays::types::VectorD2 &B);

/*!
    \brief Calculate the Dot product of 2 keays::types::VectorD3's.

    \param A [In] - A constant reference to a keays::types::VectorD3 specifying the first Vector of the dot product.
    \param B [In] - A constant reference to a keays::types::VectorD3 specifying the second Vector of the dot product.

    \return A <b>double</b> representing the dot product. [ Equivalent to \f$\|A\| \|B\|\cos(\theta)\f$ ].
 */
KEAYS_MATH_EXPORTS_API const double
Dot(const keays::types::VectorD3 &A, const keays::types::VectorD3 &B);

/*!
    \brief Project a keays::types::VectorD3 onto another keays::types::VectorD3.

    \param subj [In]  - A constant reference to the keays::types::VectorD3 to project.
    \param  obj [In]  - A constant reference to the keays::types::VectorD3 to project ONTO.

    \return the projected keays::types::VectorD3.
 */
inline const keays::types::VectorD3 Project(const keays::types::VectorD3 &subj, const keays::types::VectorD3 &obj)
{
    return subj.GetNormalised() * (Dot(subj, obj) / Magnitude(obj));
}

/*!
    \brief Calculate the height of a point on a given plane.
    This will only fail if the plane is vertical.

    \param       normal [In]  - a keays::types::VectorD3 specifying the normal to the plane.
    \param pointOnPlane [In]  - a keays::types::VectorD3 poisiotn vector specifying a known point onm the plane.
    \param        point [In]  - a keays::types::VectorD2 specifying the 2D point to project up to the plane.
    \param       result [Out] - A reference to a double to receive the resulting height.

    \return true if successful.
 */
KEAYS_MATH_EXPORTS_API bool
PointHeightOnPlane(const keays::types::VectorD3 &normal, const keays::types::VectorD3 &pointOnPlane, const keays::types::VectorD2 &point, double &result);

/*!
    \brief Calculate the height of a point on a given plane.
    This will only fail if the plane is vertical.

    \param      a [In]  - a keays::types::VectorD3 specifying the position of the first point defining the plane.
    \param      b [In]  - a keays::types::VectorD3 specifying the position of the second point defining the plane.
    \param      c [In]  - a keays::types::VectorD3 specifying the position of the third point defining the plane.
    \param  point [In]  - a keays::types::VectorD2 specifying the 2D point to project up to the plane.
    \param result [Out] - A reference to a double to receive the resulting height.

    \return true if successful.
 */
inline bool
PointHeightOnPlaneTri(const keays::types::VectorD3 &a, const keays::types::VectorD3 &b, const keays::types::VectorD3 &c, const keays::types::VectorD2 &point,
                       double &result)
{
    // calc the normal and a vector on the plane
    keays::types::VectorD3 normal = Cross((a - c), (b - c));
    return PointHeightOnPlane(normal, c, point, result);
}

/*!
    \brief Calculate the height of a point on a given plane.
    This will only fail if the plane is vertical.

    \param    tri [In]  - an array of 3 keays::types::VectorD3's specifying the triangle defining the plane.
    \param  point [In]  - a keays::types::VectorD2 specifying the 2D point to project up to the plane.
    \param result [Out] - A reference to a double to receive the resulting height.

    \return true if successful.
 */
inline bool PointHeightOnPlaneTri(const keays::types::VectorD3 tri[3], keays::types::VectorD2 &point, double &result)
{
    return PointHeightOnPlaneTri(tri[0], tri[1], tri[2], point, result);
}

/*!
    \brief Calculate the vector from basePoint to point.
 */
inline const keays::types::VectorD3 VectorFrom(const keays::types::VectorD3 &basePoint, const keays::types::VectorD3 &point)
{
    return point - basePoint;
}

/*!
    \brief Generate a rotated point, representing the new position of origPt, base on the line points given
    This function generates a rotated position for <b>origPt</b>, calculated as if the line from <b>base</b> to
    <b>ref</b> had been rotated from its current position to be level (new line would be <b>base</b> to
    [Dist2D(base, ref), 0, 0].  This is a 2D operation, and as mentioned in the function name occurs in the X/Y plane.

    \param      base [In]  - a constant reference to a keays::types::VectorD2 representing the base point for the rotation.
    \param       ref [In]  - a constant reference to a keays::types::VectorD2 representing the refence point for the rotation, (this determines how much origPt is rotated).
    \param    origPt [In]  - a constant reference to a keays::types::VectorD2 representing the point to be rotated.

    \return a constant keays::types::VectorD2 representing the new, rotated point.
 */
KEAYS_MATH_EXPORTS_API const keays::types::VectorD2
RotatePointXY(const keays::types::VectorD2 &base, const keays::types::VectorD2 &ref, const keays::types::VectorD2 &origPt);

/*!
    \overload

    \param      base [In]  - a constant reference to a keays::types::VectorD3 representing the base point for the rotation.
    \param       ref [In]  - a constant reference to a keays::types::VectorD3 representing the reference point for the rotation, (this determines how much origPt is rotated).
    \param    origPt [In]  - a constant reference to a keays::types::VectorD3 representing the point to be rotated.
 */
inline const keays::types::VectorD2 RotatePointXY(const keays::types::VectorD3 &base, const keays::types::VectorD3 &ref, const keays::types::VectorD3 &origPt)
{
    return RotatePointXY(base.XY(), ref.XY(), origPt.XY());
}

/*!
    \brief Generate a rotated point, representing the un-rotated position of origPt, base on the line points given
    This function generates a un-rotated position for <b>origPt</b>, calculated as if the line from <b>base</b> to
    [Dist2D(base, ref), 0, 0] had been rotated to be a line from <b>base</b> to <b>ref</b>.  This is a 2D operation,
    and as mentioned in the function name occurs in the X/Y plane.

    \param      base [In]  - a constant reference to a keays::types::VectorD2 representing the base point for the rotation.
    \param       ref [In]  - a constant reference to a keays::types::VectorD2 representing the refence point for the rotation, (this determines how much origPt is rotated).
    \param    origPt [In]  - a constant reference to a keays::types::VectorD2 representing the point to be rotated.

    \return a constant keays::types::VectorD2 representing the new, un-rotated point.
 */
KEAYS_MATH_EXPORTS_API const keays::types::VectorD2
UnRotatePointXY(const keays::types::VectorD2 &base, const keays::types::VectorD2 &ref, const keays::types::VectorD2 &origPt);

/*!
    \overload

    \param      base [In]  - a constant reference to a keays::types::VectorD3 representing the base point for the rotation.
    \param       ref [In]  - a constant reference to a keays::types::VectorD3 representing the refence point for the rotation, (this determines how much origPt is rotated).
    \param    origPt [In]  - a constant reference to a keays::types::VectorD3 representing the point to be un-rotated.
 */
inline const keays::types::VectorD2
UnRotatePointXY(const keays::types::VectorD3 &base,
                const keays::types::VectorD3 &ref, const keays::types::VectorD3 &origPt)
{
    return UnRotatePointXY(keays::types::VectorD2(base), keays::types::VectorD2(ref), keays::types::VectorD2(origPt));
}

/*!
    \brief Indicate which side (if any) a point is to the a line (in the X/Y plane).

    \param     start [In]  - A constant reference to a keays::types::VectorD3 point specifying the start of the line.
    \param       end [In]  - A constant reference to a keays::types::VectorD3 point specifying the end of the line.
    \param        pt [In]  - A constant reference to a keays::types::VectorD3 point to test.
    \param tolerance [In]  - A constant reference to a <b>double</b> representing the tolerance value to use for the test [optional].
    \param pDistance [Out] - A pointer to a <b>double</b> to receive the distance of the point from the line [optional].

    \return One of (SIDE_LEFT | SIDE_NONE | SIDE_RIGHT) indicating the side the point is on.  NB: SIDE_NONE indicates the point is on the line.
 */
KEAYS_MATH_EXPORTS_API const eSideSelections
PointOnSide(const keays::types::VectorD3 &start, const keays::types::VectorD3 &end,
            const keays::types::VectorD3 &pt, const double &tolerance = keays::types::Float::TOLERANCE,
            double *pDistance = NULL);

/*!
    \overload

    \param     start [In]  - A constant reference to a keays::types::VectorD2 point specifying the start of the line.
    \param       end [In]  - A constant reference to a keays::types::VectorD2 point specifying the end of the line.
    \param        pt [In]  - A constant reference to a keays::types::VectorD2 point to test.
    \param tolerance [In]  - A constant reference to a <b>double</b> representing the tolerance value to use for the test [optional].
    \param pDistance [Out] - A pointer to a <b>double</b> to receive the distance of the point from the line [optional].
 */
KEAYS_MATH_EXPORTS_API const eSideSelections
PointOnSide(const keays::types::VectorD2 &start, const keays::types::VectorD2 &end,
            const keays::types::VectorD2 &pt, const double &tolerance = keays::types::Float::TOLERANCE,
            double *pDistance = NULL);

/*!
    \brief Sort an array of angles into order based on start angle and direction.
    Sorts an array of Angles into clockwise or counter clockwise order based on a starting angle.

    \param importantAngleList [I/O] - A pointer to an array of <b>doubles</b> representing the angles to be sorted.
    \param numImportantAngles [In]  - A constant <b>integer</b> representing the number of angles in the array.
    \param      startAngleRad [In]  - A constant reference to a <b>double</b> representing the starting angle for the array (in radians).
    \param          direction [In]  - A constant ubyte indicating which direction to sort the angles (CW | CCW)
 */
KEAYS_MATH_EXPORTS_API void
SortAngleList(double *importantAngleList, const int numImportantAngles, const double &startAngleRad = 0,
              const eAngleDirections direction = CCW);

/*
    \brief Sort an array of angles into order based on start angle and direction.
    Sorts an array of Angles into clockwise or counter clockwise order based on a starting angle.

    \param importantAngleList [I/O] - A pointer to an array of <b>tImportantAngle</b> elements representing the angles to be sorted.
    \param numImportantAngles [In]  - A constant <b>integer</b> representing the number of angles in the array.
    \param      startAngleRad [In]  - A constant <b>double</b> representing the starting angle for the array (in radians).
    \param          direction [In]  - A constant eAngleDirections indicating which direction to sort the angles (CW | CCW)
 */
/*  REMOVE ME
void SortAngleList(tImportantAngle *importantAngleList, const int numImportantAngles,
                   const double &startAngleRad = 0, const eAngleDirections direction = CCW);

/*!
    \brief Galculate an angular interval based on a chord length and a radius.

    \param      radius [In] - A constant reference to a <b>double</b> representing the radius to calculate for.
    \param    chordLen [In] - A constant reference to a <b>double</b> representing the desired interval.
    \param   direction [In] - A constant eAngleDirections value representing the direction of calculation.
    \param  startAngle [In] - A constant reference to a <b>double</b> representing the start angle of the arc to calculate for (in radians).
    \param    endAngle [In] - A constant reference to a <b>double</b> representing the end angle of the arc to calculate for (in radians).
    \param evenNumDivs [In] - A constant boolean flag indicating if the calculation should adjust to ensure that ALL intervals are of even size.

    \return A constant <b>double</b> specifying the angle in radians that provides the chord length specified for that radius.
 */
KEAYS_MATH_EXPORTS_API const double
AngleFromChordLength(const double &radius, const double &chordLen, const eAngleDirections direction = CCW,
                     const double &startAngle = 0, const double &endAngle = KM_2PI, const bool evenNumDivs = true);

/*! \brief Generate a parallel polyline offset a given distance from an existing polyline
    Form the parallel offset of the polyline specified by points, putting the results into result.

    /bug When generating lines for a smaller line (i.e inside a curve) and the parallel distance is too big compared to the size of the curve it will screw up.

    \param    points [In]  - A  keays::types::Polyline3D containing the 3D points for the existing polyline.
    \param    result [Out] - A  keays::types::Polyline3D containing the 3D points for the resulting polyline.
    \param  distance [In]  - A double specifying the distance to offset.
    \param      side [In]  - One of SIDE_LEFT or SIDE_RIGHT to determine which side to offset. Side is determined by the direction of the source polyline.
    \param zDistance [In]  - A double specifying the vertical (z) distance to offset.
    \param    closed [In]  - A boolean flag indicating that the polyline is closed and the offset should be averaged across the first and last segment as well.

    \return    False on failure, true on success.
 */
KEAYS_MATH_EXPORTS_API bool
ParallelPolylineOffset(const keays::types::Polyline3D &points,  keays::types::Polyline3D &result,
                       const double &distance, const eSideSelections side, const double &zDistance = 0.0, bool closed = false);

#if 0
/*! \brief Generate a parallel polyline offset a given distance from an existing polyline
    Form the parallel offset of the polyline specified by points, putting the results into result.

    /bug When generating lines for a smaller line (i.e inside a curve) and the parallel distance is too big compared to the size of the curve it will screw up.

    \param      points [In]  - A Polyline2D containing the 2D points for the existing polyline.
    \param      result [Out] - A Polyline2D containing the 2D points for the resulting polyline.
    \param    distance [In]  - A double specifying the distance to offset.
    \param        side [In]  - One of SIDE_LEFT or SIDE_RIGHT to determine which side to offset. Side is determined by the direction of the source polyline.

    \return    False on failure, true on success.
 */
KEAYS_MATH_EXPORTS_API bool
ParallelPolylineOffset(const Polyline2D & points, Polyline2D &result,
                       const double &distance, const eSideSelections side);
#endif

/*!
    \brief Calculate the perpendicular distance from an arbitary point to a line.

    \param    lnStart [In]  - A constant reference to a keays::types::VectorD2 specifying the start point of the line.
    \param      lnEnd [In]  - A constant reference to a keays::types::VectorD2 specifying the the end point for the line.
    \param         pt [In]  - A constant reference to a keays::types::VectorD2 specifying the point to calculate for.

    \return    A double representing the distance, -ve means the point is on the left of the line.
 */
KEAYS_MATH_EXPORTS_API const double
GetPerpendicularDist(const keays::types::VectorD2 &lnStart, const keays::types::VectorD2 &lnEnd, const keays::types::VectorD2 &pt);

/*!
    \brief Calculate the perpendicular distance from an arbitary point to a line.

    \param   lnStart [In]  - A constant reference to a keays::types::VectorD2 specifying the start point of the line.
    \param     lnEnd [In]  - A constant reference to a keays::types::VectorD2 specifying the the end point for the line.
    \param        pt [In]  - A constant reference to a keays::types::VectorD2 specifying the point to calculate for.
    \param    result [Out] - A reference to a keays::types::VectorD2 to receive the calculated point.
    \param pDistance [Out] - An optional pointer to a <b>double</b> to receive the perpedicular distance the point was from the line.

    \return    An int indicating success (S_INTERSECT) or an error code if the operation failed.
 */
KEAYS_MATH_EXPORTS_API const int
GetPerpendicularIntersect(const keays::types::VectorD2 &lnStart, const keays::types::VectorD2 &lnEnd,
                          const keays::types::VectorD2 &pt,
                          keays::types::VectorD2 &result, double *pDistance = NULL);

/*!
    \brief Calculate the perpendicular distance from an arbitary point to a line.

    \param  polyline [In]  - A constant reference to a Polyline2D specifying the data for the polyline.
    \param        pt [In]  - A constant reference to a keays::types::VectorD2 specifying the point to calculate for.
    \param    result [Out] - A reference to a keays::types::VectorD2 to receive the calculated point.
    \param pDistance [Out] - An optional pointer to a <b>double</b> to receive the perpedicular distance the point was from the line.
    \param pChainage [Out] - An optional pointer to a <b>double</b> to receive the chainage of the point from start of the polyline.

    \return    True if a point was found.
 */
KEAYS_MATH_EXPORTS_API const bool
GetPerpendicularIntersect(const keays::types::Polyline2D &polyline, const keays::types::VectorD2 &pt,
                          keays::types::VectorD2 &result, double *pDistance = NULL, double *pChainage = NULL);

/*!
    \brief Calculate the perpendicular distance from an arbitary point to a line.

    \param  polyline [In]  - A constant reference to a  keays::types::Polyline3D specifying the data for the polyline.
    \param        pt [In]  - A constant reference to a keays::types::VectorD2 specifying the point to calculate for.
    \param    result [Out] - A reference to a keays::types::VectorD2 to receive the calculated point.
    \param pDistance [Out] - An optional pointer to a <b>double</b> to receive the perpedicular distance the point was from the line.
    \param pChainage [Out] - An optional pointer to a <b>double</b> to receive the chainage of the point from start of the polyline.

    \return    True if a point was found.
 */
KEAYS_MATH_EXPORTS_API const bool
GetPerpendicularIntersect(const keays::types::Polyline3D &polyline, const keays::types::VectorD2 &pt, keays::types::VectorD3 &result, double *pDistance = NULL, double *pChainage = NULL);

//-----------------------------------------------------------------------------
/*!
    \brief A structure used for handling the data returned during nearest point tests.
 */
struct KEAYS_MATH_EXPORTS_API PerpTestData
{
    PerpTestData(const int segmentStartIndex, const keays::types::VectorD3 &intersectPoint,
                 const double &chainage, const double &distance, bool onSegment)
    :    m_segmentStartIndex(segmentStartIndex),
        m_intersectPoint(intersectPoint),
        m_chainage(chainage),
        m_distance(distance),
        m_onSegment(onSegment)
    {

    }

    bool operator<(const PerpTestData &rhs)
    {
        return keays::types::Float::Less(m_distance, rhs.m_distance, ms_tolerance);
    }

    bool operator>(const PerpTestData &rhs)
    {
        return keays::types::Float::Greater(m_distance, rhs.m_distance, ms_tolerance);
    }

    bool operator<=(const PerpTestData &rhs)
    {
        return keays::types::Float::LessOrEqual(m_distance, rhs.m_distance, ms_tolerance);
    }

    bool operator>=(const PerpTestData &rhs)
    {
        return keays::types::Float::GreaterOrEqual(m_distance, rhs.m_distance, ms_tolerance);
    }

    int m_segmentStartIndex;        //!< the index in the polyline that precedes the segment that this point lies upon.
    keays::types::VectorD3
            m_intersectPoint;        //!< the nearest point on this segment. (may be a vertex).
    double m_chainage;                //!< the chainage to m_intersectPoint from the begining of the polyline.
    double m_distance;                //!< the distance from the test point to m_intersectPoint.
    bool m_onSegment;                //!< indicates if the point occurs on a segment (perpedicular point) or is a vertex.

    static double ms_tolerance;        //!< a staic double precision value spcifying the tolerance to use for comparisions
};

/*!
    \brief Generate a polyline from the given polyline, and remove duplicate points.

    \param sourcePolyline [In] - A constant reference to a  keays::types::Polyline3D.
    \param      tolerance [In] - A constant reference to a double precision value specifying the tolerance value to use for double precision boolean comparisions.

    \return    A constant  keays::types::Polyline3D representing the source polyline with the duplicates removed
 */
KEAYS_MATH_EXPORTS_API const keays::types::Polyline3D
RemoveDuplicates(const keays::types::Polyline3D &sourcePolyline,
                 const double &tolerance = keays::types::Float::TOLERANCE);

/*!
    \brief Remove duplicate points from a given polyline.

    \param sourcePolyline [In] - A pointer to a  keays::types::Polyline3D to remove the duplicate points removed.
    \param      tolerance [In] - A constant reference to a double precision value specifying the tolerance value to use for double precision boolean comparisions.

    \return    A constant  keays::types::Polyline3D representing the source polyline with the duplicates removed
 */
KEAYS_MATH_EXPORTS_API const keays::types::Polyline3D *
RemoveDuplicates(keays::types::Polyline3D *sourcePolyline,
                 const double &tolerance = keays::types::Float::TOLERANCE);

/*!
    \brief Calculate the nearest point on a polyline.
    Calculates the nearest point on the given polyline and returning the nearest point on the polyline, this
    point may also be the perpendicular intersect from the test point, but the function also tests the
    distances to it's own vertices.  The boolean flag, interpolate at end, indicates the behaviour to use when
    testing the first and last points on the polyline.  A value of &quote;true&quote; will interpolate the
    first and last segments of the polyline, treating them as infinitly long lines extending, forwards and
    backwards from the first and last segments respectively.

    \warning This function assumes that the polyline it has been given has had duplicates removed from it already, it's behaviour is unspecified if it still has duplicate, consecutive vertices.

    \param     testPolyline [In]  - A constant reference to a  keays::types::Polyline3D for use as the reference polyline.
    \param        testPoint [In]  - A constant reference to a keays::types::VectorD2 specifying the test point.
    \param       pPerpPoint [Out] - A pointer to a keays::types::VectorD3 that will receive the calculated nearest point.
    \param        pChainage [Out] - A pointer to a douple precision variable to receive the chainage at which the nearest point occurs.
    \param        pPerpDist [Out] - A pointer to a double precision variable to receive the distance from the test point to the returned point.
    \param interpolateAtEnd [In]  - A boolean flag indicating the behaviour to use when testing past the first and last point of the reference polyline.
    \param        pTestData [Out] - A pointer to a STL list of PerpTestData structures representing all of the nearest points it calculated (the result data is taken from the first node in the list).
    \param        tolerance [In]  - A constant reference to a double precision value specifying the tolerance value to use for double precision boolean comparisions.

    \return    A boolean value indicating if the function succeeded.
 */
KEAYS_MATH_EXPORTS_API bool
GetNearestPoint(const keays::types::Polyline3D &testPolyline, const keays::types::VectorD2 &testPoint,
                keays::types::VectorD3 *pPerpPoint, double *pChainage = NULL,
                double *pPerpDist = NULL, bool interpolateAtEnd = true,
                std::list<PerpTestData> *pTestData = NULL,
                const double &tolerance = keays::types::Float::TOLERANCE);


/*!
    \brief Calculate the mid point on a line.

    \param    lnStart [In]  - A constant reference to a keays::types::VectorD3 specifying the start point of the line.
    \param      lnEnd [In]  - A constant reference to a keays::types::VectorD3 specifying the the end point for the line.
    \param        mid [Out] - A reference to a keays::types::VectorD3 to receive the calculated point.

    \return    An int indicating success (S_INTERSECT) or an error code if the operation failed.
 */
KEAYS_MATH_EXPORTS_API const int
GetMidPoint(const keays::types::VectorD3 &lnStart, const keays::types::VectorD3 &lnEnd, keays::types::VectorD3 &mid);

//! Vertical Curve return values.
enum KEAYS_MATH_EXPORTS_API eVCReturns
{
    S_VC_SUCCESS = 0,        //!< Success: The vertical curve generation was successful.
    E_VC_TOO_FEW_POINTS,    //!< Failure: At least 4 points are required so a VC can be generated.
    E_VC_CURVE_LENGTH,        //!< Failure: The requested curve length was to large for the polyline.
};

/*!
    \brief Modify point heights to make a vertical curve.
    \note This will always generate a double VC even if it looks like a singular curve.
    \param        pts [I/O] - A constant reference to a  keays::types::Polyline3D specifying the points to generate the vertical curve for, points will be added for the mid and turning points of each curve.
    \param     sChain [In]  - A constant reference to a <b>double</b> specifying the distance from the start of the polyline the curve should start. A value of 0.0 or less means the curve starts at the begining of the polyline.
    \param     length [In]  - A constant reference to a <b>double</b> representing the length of the desired vertical curve, a value of less than 0.0 indicates that the curve should extend the full length of the polyline.
    \param pArcCenter [In]  - A constant pointer to a keays::types::VectorD2 for use as the center point in a circular arc to calculate the position of the turning points in the XY plane.  The arc positioning is not used if this or pRadius is NULL.
    \param    pRadius [In]  - A constant pointer to a double specifying the radius of a circular arc to calculate the position of the turning points in the XY plane.  The arc positioning is not used if this or pArcCenter is NULL.

    \return An int indicating success (S_VC_SUCCESS) or an error code from the eVCReturns enum.
 */
KEAYS_MATH_EXPORTS_API const int
VerticalCurve(keays::types::Polyline3D &pts, const double &sChain = 0.0,
              const double &length = -1, const keays::types::VectorD2 *pArcCenter = NULL,
              const double *pRadius = NULL, keays::types::VectorD2 *pStartIndices = NULL,
              keays::types::VectorD3 *pIndices = NULL, double *pTotalLength = NULL);

/*!
    \overload

    \note This will always generate a double VC even if it looks like a singular curve.
    \note The gradients should indicate the gradient travelling in the direction of the curve.

    \param  sGrade [In]  - A constant reference to a <b>double</b> specifying the gradient at the start of the curve.
    \param sHeight [In]  - A constant reference to a <b>double</b> specifying the start height (z-value) of the curve.
    \param  eGrade [In]  - A constant reference to a <b>double</b> specifying the gradient at the end of the curve.
    \param eHeight [In]  - A constant reference to a <b>double</b> specifying the end height (z-value) of the curve.
 */
KEAYS_MATH_EXPORTS_API const int
VerticalCurve(keays::types::Polyline3D &pts, const double &sGrade, const double &sHeight,
              const double &eGrade, const double &eHeight,
              const keays::types::VectorD2 *pArcCenter = NULL,
              const double *pRadius = NULL, keays::types::VectorD3 *pIndices = NULL,
              double *pTotalLength = NULL);

/*!
    \overload
    \param        source [In]  - A constant reference to a  keays::types::Polyline3D for use as the original source data.
    \param         pDest [Out] - A pointer to a  keays::types::Polyline3D to receive the generated curve.  The destination polyline will be cleared.
    \param startChainage [In]  - A constant reference to a <b>double</b> specifying the desired start chainage for the curve
                                    domain. If this value is greater than the end chainage the curve domain will be from this
                                    value to the end of the polyline, if this value is less than 0 the curve domain will be
                                    from the start of the polyline, if this value is greater than the end chainage the function
                                    will fail and return false.
    \param   endChainage [In]  - A constant reference to a <b>double</b> specifying the desired end chainage for the curve domain. If
                                    this value is less than the start chainage the curve domain will be from the start to the end of the
                                    polyline, if this value is greater than the end chainage the curve domain also extends to the end.
    \param   curveLength [In]  - A constant reference to a <b>double</b> specifying the desired curve length.  If this value is
                                    greater than the curve domain, or this length does not fit in the available space for a single
                                    vertical curve the curve will be a double VC and extend over the entire curve domain length.
    \param         flags [In]  - A constant <b>unsigned int</b> specifying the flags to control tghe handling of curve lengths and styles.

    \todo Finish handling for flags
    \todo Add a return data option for information such as single/double vc, indicies and chainages for domain start/end, curve start/end, midpoint, and turning points.

    \return True if the curve was successfully generated, otherwise false
 */
KEAYS_MATH_EXPORTS_API bool
VerticalCurve(const keays::types::Polyline3D &source,  keays::types::Polyline3D *pDest,
              const double &startChainage, const double &endChainage,
              const double &curveLength, const unsigned int flags);

/*!
    \brief Remove redundant points from a polyline.
    Remove points from a polyline that are not needed to maintain its shape.
    As xyTol and altTol increase more points will be removed.

    \param   polyline [In]  - A vector of points that represent the polyline to simplify
    \param      xyTol [In]  - The percentage tolerance (0 -> 1.0) in the xy plane (bearing)
    \param     altTol [In]  - The percentage tolerance (0 -> 1.0) in the zx(zy) (altitude) plane
    \param simpleLine [Out] - The simplified polyline

    \return True if successful.
 */
KEAYS_MATH_EXPORTS_API bool
SimplifyPolyline(const keays::types::Polyline3D &polyline, const double &xyTol, const double &altTol,
                 const double &minimumChainage,  keays::types::Polyline3D &simpleLine);


/*
bool AdjustPolylineVCurve(const keays::types::Polyline3D &polyline, const int startIndex, const int endIndex,  keays::types::Polyline3D &result);

bool AdjustPolylineVCurve(const keays::types::Polyline3D &polyline, const double &startChainage, const double &endChainage,  keays::types::Polyline3D &result);
//*/

/*!
    \brief Calculate the area, it will be -ve if the polygon is wound CW.

    \param polygon [In] - A constant reference to a Polyline2D points describing a 2D polygon.

    \return The area in units squared, the answer will be -ve if the polygon is wound in a clockwise direction.
 */
KEAYS_MATH_EXPORTS_API const double
CalcPolygonArea(const keays::types::Polyline2D &polygon, bool needsClose = false);

/*!
    \overload

    \param polygon [In] - A constant reference to a std::list of keays::types::VectorD2 points describing a 2D polygon.
 */
KEAYS_MATH_EXPORTS_API const double
CalcPolygonArea(const std::list<keays::types::VectorD2> &polygon, bool needsClose = false);

/*!
    \overload

    \param polygon [In] - A constant pointer to an array of keays::types::VectorD2 points describing a 2D polygon.
 */
KEAYS_MATH_EXPORTS_API const double
CalcPolygonArea(const keays::types::VectorD2* polygon, const int numPoints, bool needsClose = false);

/*!
    \brief Calculate the area of an arbitary triangle, it will be -ve if the polygon is wound CW.

    \param     triPt1 [In] - A constant reference to a keays::types::VectorD2 representing the first point of a CCW wound triangle.
    \param     triPt2 [In] - A constant reference to a keays::types::VectorD2 representing the second point of a CCW wound triangle.
    \param     triPt3 [In] - A constant reference to a keays::types::VectorD2 representing the third point of a CCW wound triangle.

    \return The area in units squared, the answer will be -ve if the polygon is wound in a clockwise direction.
 */
KEAYS_MATH_EXPORTS_API const double
CalcTriangleArea(const keays::types::VectorD2 &triPt1,
                 const keays::types::VectorD2 &triPt2,
                 const keays::types::VectorD2 &triPt3);

/*!
    \overload

    \param triPts [In] - A constant pointer to the first element of an array of keays::types::VectorD2 points representing a CCW wound triangle.
 */
inline KEAYS_MATH_EXPORTS_API const double
CalcTriangleArea(const keays::types::VectorD2 *triPts)
{
    return CalcTriangleArea(triPts[0], triPts[1], triPts[2]);
}
//! @}

//=============================================================================
/*!
    \name Intersection Functions
    @{
 */
//-----------------------------------------------------------------------------
//! Intersection return values
enum KEAYS_MATH_EXPORTS_API eIntersectionReturns
{
    S_INTERSECT = 0,        //!< Success: Intersection point(s) found.
    E_SAME_POINT,            //!< Failure: Two or more points that are required to be different are duplicates.
    E_NO_INTERSECT,            //!< Failure: There is no intersection point.
    E_SAME_CIRCLE,            //!< Failure: For Circle/Circle Intersection - the 2 circles are the same.
    E_CIRC1_IN_CIRC2,        //!< Failure: For Circle/Circle Intersection - the first cicle is fully contained in the second.
    E_CIRC2_IN_CIRC1,        //!< Failure: For Circle/Circle Intersection - the second cicle is fully contained in the first.
    E_NO_LINE1,                //!< Failure: Line/Circle or Line/Line Intersection - the line does not exist (start == end).
    E_NO_LINE2,                //!< Failure: Line/Line Intersect - the 2nd line does not exist (start == end).
    E_LINES_PARALLEL,        //!< Failure: Line/Line Intersect - the 2 lines are parallel.
    E_RECT1_INSIDE,            //!< Failure: Rectangle 1 is contained inside rectangle 2.
    E_RECT2_INSIDE,            //!< Failure: Rectangle 2 is contained inside rectangle 1.
    E_SAME_RECT,            //!< Failure: Rectangles are the same.
    ES_LINES_IN_LINE,        //!< Semi-Failure: The lines are perfectly in line.
    E_NO_SEGMENT_INTERSECT,    //!< Failure: There may be an intersect, but not withing the limits of the segment.
    E_FAIL_OTHER,            //!< Failure: Unspecified reason.
    SS_L1P1_CONTACT,        //!< Semi-Success: The first point of the first line touches.
    SS_L1P2_CONTACT,        //!< Semi-Success: The second point of the second line touches.
    SS_L2P1_CONTACT,        //!< Semi-Success: The first point of the first line touches.
    SS_L2P2_CONTACT,        //!< Semi-Success: The second point of the second line touches.
};

//-----------------------------------------------------------------------------
/*!
    \brief Multiple Line intersection return values
    Pleas Note: two of the error values as indicated need to be tested with a mask of 0xffff, as the hi word is the
    error value from a called funtion.
 */
enum KEAYS_MATH_EXPORTS_API eLCErrorVals
{
    LC_SUCCESS = 0,                        //!< Success: An averaged intersection was found.
    LC_ERR_INVALID_RESULT_PTR,            //!< Failure: A NULL pointer was passed in for the return value.
    LC_ERR_TO_FEW_LINES,                //!< Failure: To few lines were passed in, there must be at least 2 for this to work.
    LC_ERR_NO_VALID_INTERSECT,            //!< Failure: No valid Intersect could be calculated for the lines. NOTE: the hi-word contains the error value from the line line intersect.
    LC_ERR_PROBLEM_AVERAGING_POINTS,    //!< Failure: An error occured while trying to average the points. NOTE: the hi-word contains the error value from the point averaging.
};

//-----------------------------------------------------------------------------
//! Point averaging return values
enum KEAYS_MATH_EXPORTS_API ePAveErrorVals
{
    PAV_SUCCESS = 0,                //!< Success: An averaged point was found.
    PAV_ERR_INVALID_RESULT_PTR,        //!< Failure: A NULL pointer was passed in for the return value.
    PAV_ERR_NO_POINTS,                //!< Failure: No points were passed in, need at least 1 point to average. Note: 1 point will average to itself, and 2 points will find the midpoint between the 2.
};

//-----------------------------------------------------------------------------
//! An array of strings representing error desriptions
extern KEAYS_MATH_EXPORTS_API const char *IRETURN_TEXT[];

//-----------------------------------------------------------------------------
/*!
    \brief A safe method for retrieving the error text based on a  return value
    \param    retVal [In]  - the return value to retrieve a string for
    \return The string (const char *) of the retrun value
 */
inline const char *GetIReturnText(int retVal)
{
    if ((retVal >= 0) && (retVal <= E_FAIL_OTHER))
        return IRETURN_TEXT[retVal];
    else
        return "UNKNOWN ERROR";
}

//-----------------------------------------------------------------------------
/*!
    \brief Find the point(s) of intersection of 2 circles if possible on the XY plane.

    \param  center1 [In]  - A constant reference to a keays::types::VectorD2 representing the XY position of the center of the first circle.
    \param  radius1 [In]  - A constant reference to a <b>double</b> specifying the radius of the first circle.
    \param  center2 [In]  - A constant reference to a keays::types::VectorD2 representing the XY position of the center of the second circle.
    \param  radius2 [In]  - A constant reference to a <b>double</b> specifying the radius of the second circle.
    \param refPoint [In]  - A constant reference to a keays::types::VectorD2 representing a reference point to use to determine which is the first and which is the second result.
    \param   result [Out] - A reference to a keays::types::VectorD2 to receive the first solution if any.
    \param  result2 [Out] - A pointer to a keays::types::VectorD2 to receive the second solution if any, pass NULL if not interested in the second solution.

    \return    An integer indicating if the operation succeeded or failed. This will be S_INTERSECT on success, on failure it will be one of E_SAME_POINT, E_NO_INTERSECT, E_CIRC1_IN_CIRC2, or E_CIRC2_IN_CIRC1.
 */
KEAYS_MATH_EXPORTS_API const int
CircleCircleIntersect(const keays::types::VectorD2 &center1,
                      const double &radius1, const keays::types::VectorD2 &center2,
                      const double &radius2, const keays::types::VectorD2 &refPoint,
                      keays::types::VectorD2 &result, keays::types::VectorD2 *result2);

//-----------------------------------------------------------------------------
/*!
    \brief Find the point(s) of intersection of a line and a circle if possible on the XY plane.

    \param        pt1 [In]  - A constant reference to a keays::types::VectorD2 representing the XY position of first point on the line.
    \param        pt2 [In]  - A constant reference to a keays::types::VectorD2 representing the XY position of second point on the line.
    \param     center [In]  - A constant reference to a keays::types::VectorD2 representing the XY position of the center of the circle.
    \param     radius [In]  - A constant reference to a <b>double</b> specifying the radius of the circle.
    \param   refPoint [In]  - A constant reference to a keays::types::VectorD2 representing a reference point to use to determine which is the first and which is the second result;
    \param     result [Out] - A reference to a keays::types::VectorD2 to receive the first solution if any.
    \param lineOffset [In]  - A constant double specifying an offset distance if required.
    \param    result2 [Out] - A pointer to a keays::types::VectorD2 to receive the second solution if any, pass NULL if not interested in the second solution.

    \return    An integer indicating if the operation succeeded or failed. This will be S_INTERSECT on success, on failure it will be one of E_SAME_POINT, E_NO_INTERSECT, E_CIRC1_IN_CIRC2, or E_CIRC2_IN_CIRC1.
 */
KEAYS_MATH_EXPORTS_API const int
LineCircleIntersect(const keays::types::VectorD2 &pt1, const keays::types::VectorD2 &pt2,
                    const keays::types::VectorD2 &center, const double &radius,
                    const keays::types::VectorD2 &refPoint, keays::types::VectorD2 &result,
                    const double &lineOffset = 0.0, keays::types::VectorD2 *result2 = NULL);

//-----------------------------------------------------------------------------
/*!
    \overload

    \param l1 [In] - A constant reference to a Line specifying the line to use for the intersect.
 */
inline const int
LineCircleIntersect(const Line &l1, const keays::types::VectorD2 &center, const double &radius,
                    const keays::types::VectorD2 &refPoint, keays::types::VectorD2 &result,
                    const double &lineOffset = 0.0, keays::types::VectorD2 *result2 = NULL)
{
    return LineCircleIntersect(l1.start.XY(), l1.end.XY(), center, radius, refPoint, result, lineOffset, result2);
}

//-----------------------------------------------------------------------------
/*!
    \brief Find the point of intersection of 2 lines if possible on the XY plane.

    \param   l1p1 [In]  - A constant reference to a keays::types::VectorD2 representing the XY position of first point on the first line.
    \param   l1p2 [In]  - A constant reference to a keays::types::VectorD2 representing the XY position of second point on the first line.
    \param   l2p1 [In]  - A constant reference to a keays::types::VectorD2 representing the XY position of first point on the second line.
    \param   l2p2 [In]  - A constant reference to a keays::types::VectorD2 representing the XY position of second point on the second line.
    \param result [Out] - A reference to a keays::types::VectorD2 to receive the first solution if any, interested in the second solution.

    \return    An integer indicating if the operation succeeded or failed. This will be S_INTERSECT on success, on failure it will be one of E_SAME_POINT, E_NO_INTERSECT, E_CIRC1_IN_CIRC2, or E_CIRC2_IN_CIRC1.
 */
KEAYS_MATH_EXPORTS_API const int
LineLineIntersect(const keays::types::VectorD2 &l1p1, const keays::types::VectorD2 &l1p2,
                  const keays::types::VectorD2 &l2p1, const keays::types::VectorD2 &l2p2,
                  keays::types::VectorD2 &result);

//-----------------------------------------------------------------------------
/*!
    \overload

    \param l1 [In] - A constant reference to a Line representing the first line to use.
    \param l2 [In] - A constant reference to a Line representing the second line to use.
 */
inline const int LineLineIntersect(const Line &l1, const Line &l2, keays::types::VectorD2 &result)
{
    return LineLineIntersect(l1.start.XY(), l1.end.XY(), l2.start.XY(), l2.end.XY(), result);
}

//-----------------------------------------------------------------------------
/*!
    \brief Find the point of intersection of 2 line segments.
    Find, if possible the intersection point of 2 line segments, only an actual intersection of the lines is
    considered a success, on failure it will be one of E_NO_LINE1, E_NO_LINE2, ES_LINES_IN_LINE, E_LINES_PARALLEL
    indicating failure to intersect, or SS_L1P1_CONTACT, SS_L1P2_CONTACT, SS_L2P1_CONTACT, SS_L2P2_CONTACT which
    is still an intersect of sorts (an end is touching, but not automatically considered a success, and will need
    to be checked for seperately.

    \param   l1p1 [In]  - A constant reference to a keays::types::VectorD2 representing the XY position of first point on the first line.
    \param   l1p2 [In]  - A constant reference to a keays::types::VectorD2 representing the XY position of second point on the first line.
    \param   l2p1 [In]  - A constant reference to a keays::types::VectorD2 representing the XY position of first point on the second line.
    \param   l2p2 [In]  - A constant reference to a keays::types::VectorD2 representing the XY position of second point on the second line.
    \param result [Out] - A reference to a keays::types::VectorD2 to receive the first solution if any, interested in the second solution.

    \return    An integer indicating if the operation succeeded or failed. This will be S_INTERSECT on success.
 */
KEAYS_MATH_EXPORTS_API const int
LineSegLineSegIntersect(const keays::types::VectorD2 &l1p1, const keays::types::VectorD2 &l1p2,
                        const keays::types::VectorD2 &l2p1, const keays::types::VectorD2 &l2p2,
                        keays::types::VectorD2 &result);

//-----------------------------------------------------------------------------
/*!
    \overload

    \param     l1 [In]  - A constant reference to a keays::types::VectorD2 representing the XY position of first point on the first line.
    \param     l2 [In]  - A constant reference to a keays::types::VectorD2 representing the XY position of first point on the second line.
    \param result [Out] - A reference to a keays::types::VectorD2 to receive the first solution if any, interested in the second solution.

    \return    An integer indicating if the operation succeeded or failed. This will be S_INTERSECT on success.
 */
inline const int
LineSegLineSegIntersect(const Line &l1, const Line &l2, keays::types::VectorD2 &result)
{
    return LineSegLineSegIntersect(l1.start.XY(), l1.end.XY(), l2.start.XY(), l2.end.XY(), result);
}

//-----------------------------------------------------------------------------
/*!
    \brief Find an averaged intersection point for a series of lines on the XY plane.
    This is not the optimal solution, merely a near enough answer for most cases.

    \warning <b>When testing for return values it is important to mask off the hi word (ie test against 0xffff) as the error
        code is stored in the low word, the high word may contain error codes from other function called during
        processing, see return values for LineLineIntersect (test for LC_ERR_NO_VALID_INTERSECT) and AveragePoints
        (test for LC_ERR_PROBLEM_AVERAGING_POINTS).</b>

    \param  lines [In]  - a constant reference to a std::vector of Lines to find the average intersect for
    \param result [Out] - A pointer to a keays::types::VectorD2 to receive the resultant point.

    \return    An integer indicating if the operation succeeded or failed. This will be LC_SUCCESS on success,
                on failure it will be one of LC_ERR_INVALID_RESULT_PTR, LC_ERR_TO_FEW_LINES, LC_ERR_NO_VALID_INTERSECT or LC_ERR_PROBLEM_AVERAGING_POINTS.
 */
KEAYS_MATH_EXPORTS_API const int
MultipleLineIntersectAverage(const std::vector<Line> &lines, keays::types::VectorD2 *result);

//-----------------------------------------------------------------------------
/*!
    \brief Find an average geometric centroid of a group of points on the XY plane.

    \param points [In]  - a constant reference to a stl::vector of keays::types::VectorD2 points to find the centroid of,..
    \note 1) It is rather pointless, although possible to find the average of 1 point.
    \note 2) If you use this on two points it will find the mid point between them.
    \param result [Out] - A pointer to a keays::types::VectorD2 to receive the resulting geometric centroid of the collection of points.

    \return    An integer indicating if the operation succeeded or failed. This will be PAV_SUCCESS on success,
                on failure it will be one of PAV_ERR_INVALID_RESULT_PTR or PAV_ERR_NO_POINTS.
 */
KEAYS_MATH_EXPORTS_API const int
AveragePoints(const keays::types::Polyline2D &points, keays::types::VectorD2 *result);

//-----------------------------------------------------------------------------
/*!
    \overload
    \brief Find an average geometric centroid of a group of 3D points.

    \param points [In]  - a constant reference to a stl::vector of keays::types::VectorD3 points to find the centroid of,..
    \param result [Out] - A pointer to a keays::types::VectorD3 to receive the resulting geometric centroid of the collection of points.
 */
KEAYS_MATH_EXPORTS_API const int
AveragePoints(const keays::types::Polyline3D &points, keays::types::VectorD3 *result);

//-----------------------------------------------------------------------------
/*!
    \brief Calculate the intersection points (if any) of the rectangles specified by topLeft1, botRight1
    and topLeft2, botRight2. These intersection points are placed in resTopLeft and resBotRight.

    <b>Note #1:</b> The coordinate system of the rectangles are <b>NOT</b> like that of a screen:
    The upper left hand corner is the smallest x & biggest y value and increases across(x)
    and decreases down(y).

    <b>Note #2:</b> Intersect doesn't include touching edges. I.e if two rectangles are touching then they
    are <b>NOT</b> intersecting

    \param    topLeft1  [In]  - a keays::types::VectorD2 specifying the upper left hand corner of the first rectangle
    \param    botRight1 [In]  - a keays::types::VectorD2 specifying the lower right hand corner of the first rectangle
    \param    topLeft2  [In]  - a keays::types::VectorD2 specifying the upper left hand corner of the second rectangle
    \param    botRight2 [In]  - a keays::types::VectorD2 specifying the lower right hand corner of the second rectangle
    \param  resTopLeft  [Out] - A keays::types::VectorD2 specifying the upper left hand corner of the intersection rectangle
    \param  resBotRight [Out] - A keays::types::VectorD2 specifying the lower right hand corner of the intersection rectangle

    \return    An integer indicating if the operation succeeded or failed. This will be S_INTERSECT on success.
    On failure it will be one of E_NO_INTERSECT, E_RECT1_INSIDE, E_RECT2_INSIDE, E_SAME_RECT or E_FAIL_OTHER.
    In any of the cases S_INTERSECT, E_RECT1_INSIDE, E_RECT2_INSIDE or E_SAME_RECT resTopLeft and resBotRight
    will be filled with the appropriate data:
               - S_INTERSECT: The points of intersection
               - E_RECT1_INSIDE: Rect1's topLeft and botRight points
               - E_RECT2_INSIDE: Rect2's topLeft and botRight points
               - E_SAME_RECT: Rect1's or Rect2's topLeft and botRight points

    For all other cases resTopLeft and resBotRight are undefined.
 */
KEAYS_MATH_EXPORTS_API const int
RectRectIntersect(const keays::types::VectorD2 & topLeft1, const keays::types::VectorD2 & botRight1,
                  const keays::types::VectorD2 & topLeft2, const keays::types::VectorD2 & botRight2,
                  keays::types::VectorD2 & resTopLeft, keays::types::VectorD2 & resBotRight);

//-----------------------------------------------------------------------------
/*!
    \brief Caculate the Y value for a given X position on a specified line

    \param       pt1 [In]  - a constant reference to a keays::types::VectorD2 specifying the first point on the line.
    \param       pt2 [In]  - a constant reference to a keays::types::VectorD2 specifying the second point on the line.
    \param         x [In]  - a constant reference to a double specifying the X position to calculate for.
    \param        pY [Out] - A pointer to a double for the calculated Y position.
    \param tolerance [In]  - a constant double representing the tolerance to use for testing.

    \return a pointer to the result if successful, NULL if it failed
 */
KEAYS_MATH_EXPORTS_API const double *
YfromXPt(const keays::types::VectorD2 &pt1, const keays::types::VectorD2 &pt2,
         const double &x, double *pY, const double &tolerance = keays::types::Float::TOLERANCE);

//-----------------------------------------------------------------------------
/*!
    \overload
    \param line [In]  - a constant reference to a Line specifying the line.
 */
inline const double *
YfromXPt(const Line &line, const double &x, double *pY,
         const double &tolerance = keays::types::Float::TOLERANCE)
{
    return YfromXPt(line.start.XY(), line.end.XY(), x, pY, tolerance);
}

//-----------------------------------------------------------------------------
/*
    \brief Caculate the X value for a given Y position on a specified line

    \param       pt1 [In]  - a constant reference to a keays::types::VectorD2 specifying the first point on the line.
    \param       pt2 [In]  - a constant reference to a keays::types::VectorD2 specifying the second point on the line.
    \param         y [In]  - a constant reference to a double specifying the Y position to calculate for.
    \param        pX [Out] - A pointer to a double for the calculated X position.
    \param tolerance [In]  - a constant double representing the tolerance to use for testing.

    \return a pointer to the result if successful, NULL if it failed
 */
KEAYS_MATH_EXPORTS_API const double *
XfromYPt(const keays::types::VectorD2 &pt1, const keays::types::VectorD2 &pt2, const double &y,
         double *pX, const double &tolerance = keays::types::Float::TOLERANCE);

//-----------------------------------------------------------------------------
/*!
    \overload
    \param line [In]  - a constant reference to a Line specifying the line.
 */
inline const double *
XfromYPt(const Line &line, const double &y, double *pX,
         const double &tolerance = keays::types::Float::TOLERANCE)
{
    return XfromYPt(line.start.XY(), line.end.XY(), y, pX, tolerance);
}

//-----------------------------------------------------------------------------
/*!
    \brief Line Crosses Rectangle return values
    Unlike most of the other functions in this library, this function has only 1 failure result, and multiple
    successes.
 */
enum KEAYS_MATH_EXPORTS_API eLCRVals
{
    E_FAILURE     = 0,    //!< The Line does NOT cross the rectangle or even touch it.
    S_LINE_CROSSES,        //!< The Line crosses the rectangle, (it passes through the regtangle, not just along it).
    S_LINE_TOUCHES,        //!< The Line touches the rectangle, (it passes along a side, or through a corner).
    S_LINE_CONTAINED,    //!< The Line Segment is contained fully in the regtangle.
    S_LINE_ENTERS,        //!< The End of the Line segment is contained, and the start was truncated.
    S_LINE_EXITS,        //!< The Start of the line segment is contained, and the end was truncated.
};

/*!
    \brief Get a string representation of the return value from the LineCrosses* and LineSegCrosses functions

    \param val [In]  - a constant integer with the return value to test
    \return a character string representing the text equivalent to the return value.
 */
KEAYS_MATH_EXPORTS_API const char *
LineTriReturnString(const int val);

#define LineRectReturnString(val) LineTriReturnString(val)

//-----------------------------------------------------------------------------
/*!
    \brief Calculate the intersection points of a line and a rectangle.

    \param       rect [In]  - a constant reference to the rectangle to test, must be valid or the function returns failure.
    \param       line [In]  - a constant reference to the line to test with, must have some 2D length or the function returns failure.
    \param pLineStart [Out] - A pointer to a keays::types::VectorD2 for the intersection point for the start of the line, MUST NOT BE NULL.
    \param   pLineEnd [Out] - A pointer to a keays::types::VectorD2 for the intersection point at the end of the line, MUST NOT BE NULL.
    \param  tolerance [In]  - a constant double representing the tolerance to use for testing.

    \return a constant integer representing the success of the operation. One of the following values from eLCRVals -
    E_FAILURE, S_LINE_CROSSES or S_LINE_TOUCHES.
 */
KEAYS_MATH_EXPORTS_API const int
LineCrossesRect(const RectD &rect, const Line &line, keays::types::VectorD2 *pLineStart,
                keays::types::VectorD2 *pLineEnd, const double &tolerance = keays::types::Float::TOLERANCE);

//-----------------------------------------------------------------------------
/*!
    \overload
    \param pLineStart [Out] - A pointer to a keays::types::VectorD3 for the intersection point for the start of the line, MUST NOT BE NULL.
    \param   pLineEnd [Out] - A pointer to a keays::types::VectorD3 for the intersection point at the end of the line, MUST NOT BE NULL.
 */
inline KEAYS_MATH_EXPORTS_API const int
LineCrossesRect(const RectD &rect, const Line &line, keays::types::VectorD3 *pLineStart,
                keays::types::VectorD3 *pLineEnd,
                const double &tolerance = keays::types::Float::TOLERANCE)
{
    keays::types::VectorD2 _start, _end;
    Line ln(*pLineStart, *pLineEnd);
    int result = LineCrossesRect(rect, line, &_start, &_end, tolerance);
    if (result != E_FAILURE)
    {
        double ht;
        (*pLineStart) = _start;
        if (ln.GetPointHeight(_start.VD3(), ht))
            pLineStart->z = ht;
        (*pLineEnd) = _end;
        if (ln.GetPointHeight(_end.VD3(), ht))
            pLineEnd->z = ht;
    }
    return result;
}

//-----------------------------------------------------------------------------
/*!
    \brief Calculate the intersection points of a line segment and a rectangle.

    \param       rect [In]  - a constant reference to the rectangle to test, must be valid or the function returns failure.
    \param       line [In]  - a constant reference to the line to test with, must have some 2D length or the function returns failure.
    \param pLineStart [Out] - A pointer to a keays::types::VectorD2 for the intersection point for the start of the line, MUST NOT BE NULL.
    \param   pLineEnd [Out] - A pointer to a keays::types::VectorD2 for the intersection point at the end of the line, MUST NOT BE NULL.
    \param  tolerance [In]  - a constant double representing the tolerance to use for testing.

    \return a constant integer representing the success of the operation. One of the following values from eLCRVals -
    E_FAILURE, S_LINE_CROSSES, S_LINE_TOUCHES, S_LINE_CONTAINED, S_LINE_ENTERS or S_LINE_EXITS.
 */
KEAYS_MATH_EXPORTS_API const int
LineSegCrossesRect(const RectD &rect, const Line &line, keays::types::VectorD2 *pLineStart,
                   keays::types::VectorD2 *pLineEnd,
                   const double &tolerance = keays::types::Float::TOLERANCE);

//-----------------------------------------------------------------------------
/*!
    \overload
    \param pLineStart [Out] - A pointer to a keays::types::VectorD3 for the intersection point for the start of the line, MUST NOT BE NULL.
    \param   pLineEnd [Out] - A pointer to a keays::types::VectorD3 for the intersection point at the end of the line, MUST NOT BE NULL.
 */
inline KEAYS_MATH_EXPORTS_API const int
LineSegCrossesRect(const RectD &rect, const Line &line, keays::types::VectorD3 *pLineStart,
                   keays::types::VectorD3 *pLineEnd,
                   const double &tolerance = keays::types::Float::TOLERANCE)
{
    keays::types::VectorD2 _start, _end;
    Line ln(*pLineStart, *pLineEnd);
    int result = LineSegCrossesRect(rect, line, &_start, &_end, tolerance);
    if (result != E_FAILURE)
    {
        double ht;
        (*pLineStart) = _start;
        if (ln.GetPointHeight(_start.VD3(), ht))
            pLineStart->z = ht;
        (*pLineEnd) = _end;
        if (ln.GetPointHeight(_end.VD3(), ht))
            pLineEnd->z = ht;
    }
    return result;
}

//-----------------------------------------------------------------------------
/*!
    \brief Test if a point is inside a triangle
    \param     triPt1 [In] - A constant reference to a keays::types::VectorD2 representing the first point of a CCW wound triangle.
    \param     triPt2 [In] - A constant reference to a keays::types::VectorD2 representing the second point of a CCW wound triangle.
    \param     triPt3 [In] - A constant reference to a keays::types::VectorD2 representing the third point of a CCW wound triangle.
    \param         pt [In] - A constant reference to a keays::types::VectorD2 representing the point to test.
    \param tolerance [In] - A constant reference to a <b>double</b> representing the tolerance to use for testing.

    \return true if the point is inside or touching the triangle the triangle
 */
KEAYS_MATH_EXPORTS_API bool
PointInTriangle(const keays::types::VectorD2 &triPt1, const keays::types::VectorD2 &triPt2,
                const keays::types::VectorD2 &triPt3, const keays::types::VectorD2 &pt,
                const double &tolerance = keays::types::Float::TOLERANCE);

//-----------------------------------------------------------------------------
/*!
    \brief Test to see if a specified point is on the segment.
    \param       pt1 [In] - A constant reference to a keays::types::VectorD2 representing the start point of the segment to test.
    \param       pt2 [In] - A constant reference to a keays::types::VectorD2 representing the end point of the segment to test.
    \param    testPt [In] - A constant reference to a keays::types::VectorD2 representing the point to test.
    \param tolerance [In] - A constant reference to a <b>double</b> representing the tolerance to use for testing.

    \return true if the point is on the segment
 */
KEAYS_MATH_EXPORTS_API bool
PointOnSegment(const keays::types::VectorD2 &pt1, const keays::types::VectorD2 &pt2,
               const keays::types::VectorD2 &testPt,
               const double &tolerance = keays::types::Float::TOLERANCE);

//-----------------------------------------------------------------------------
/*!
    \brief Test to see if a specified point is on the segment.
    \param       pt1 [In] - A constant reference to a keays::types::VectorD3 representing the start point of the segment to test.
    \param       pt2 [In] - A constant reference to a keays::types::VectorD3 representing the end point of the segment to test.
    \param    testPt [In] - A constant reference to a keays::types::VectorD3 representing the point to test.
    \param tolerance [In] - A constant reference to a <b>double</b> representing the tolerance to use for testing.

    \return true if the point is on the segment
 */
KEAYS_MATH_EXPORTS_API bool
PointOnSegment(const keays::types::VectorD3 &pt1, const keays::types::VectorD3 &pt2,
               const keays::types::VectorD3 &testPt,
               const double &tolerance = keays::types::Float::TOLERANCE);

//-----------------------------------------------------------------------------
/*!
    \brief Calculate the intersection points of a line and a triangle
    Calculates the intersection point(s0 of a line and a triangle, returns values indicating if the line didn't touch,
    just touched, or actually crossed the triangle.  If addresses are passed to the pStartEdge and pEndEdge pointers, the edge
    that each intersect occurs on is passed out.  For each edge, the start point is considered to be part of the edge.

    \param     triPt1 [In]  - a constant reference to a keays::types::VectorD3 representing the first point of a CCW wound triangle.
    \param     triPt2 [In]  - a constant reference to a keays::types::VectorD3 representing the second point of a CCW wound triangle.
    \param     triPt3 [In]  - a constant reference to a keays::types::VectorD3 representing the third point of a CCW wound triangle.
    \param       line [In]  - a constant reference to the line to test with, must have some 2D length or the function returns failure.
    \param pLineStart [Out] - A pointer to a keays::types::VectorD3 for the intersection point for the start of the line, MUST NOT BE NULL.
    \param   pLineEnd [Out] - A pointer to a keays::types::VectorD3 for the intersection point at the end of the line, MUST NOT BE NULL.
    \param pStartEdge [Out] - A pointer to an integer reepresenting the edge that the start point was found on, MUST NOT BE NULL.
    \param   pEndEdge [Out] - A pointer to an integer reepresenting the edge that the end point was found on, MUST NOT BE NULL.
    \param  tolerance [In]  - a constant double representing the tolerance to use for testing.

    \return a constant integer representing the success of the operation. One of the following values from eLCRVals -
    E_FAILURE, S_LINE_CROSSES or S_LINE_TOUCHES.
 */
KEAYS_MATH_EXPORTS_API const int
LineCrossesTriangle(const keays::types::VectorD3 &triPt1, const keays::types::VectorD3 &triPt2,
                    const keays::types::VectorD3 &triPt3, const Line &line, keays::types::VectorD3 *pLineStart,
                    keays::types::VectorD3 *pLineEnd, int *pStartEdge, int *pEndEdge,
                    const double &tolerance = keays::types::Float::TOLERANCE);

/*!
    \brief Calculate the intersect point, if any, of a line segment and a triangle in 3D space.
    \param triPt1 [In] - A constant reference to a keays::types::VectorD3 representing the first point of a triangle.
    \param triPt2 [In] - A constant reference to a keays::types::VectorD3 representing the second point of a triangle.
    \param triPt3 [In] - A constant reference to a keays::types::VectorD3 representing the third point of a triangle.
    \param line [In] - A constant reference to a Line representing the line segment to test.
    \param pIntersect [Out] - A pointer to a keays::types::VectorD3 to receive the intersect point if it is found.
    \param tolerance [In] - A constant reference to a <b>double</b> representing the tolerance value to use in testing.

    \return True is the intersection point was found.
 */
KEAYS_MATH_EXPORTS_API const bool
SegmentTriangleIntersect(const keays::types::VectorD3 &triPt1, const keays::types::VectorD3 &triPt2,
                         const keays::types::VectorD3 &triPt3, const Line &line,
                         keays::types::VectorD3 *pIntersect,
                         const double &tolerance = keays::types::Float::TOLERANCE);

/*!
    \brief Calculate the intersect point, if any, of a ray and a triangle in 3D space.
    \param triPt1 [In] - A constant reference to a keays::types::VectorD3 representing the first point of a triangle.
    \param triPt2 [In] - A constant reference to a keays::types::VectorD3 representing the second point of a triangle.
    \param triPt3 [In] - A constant reference to a keays::types::VectorD3 representing the third point of a triangle.
    \param line [In] - A constant reference to a Line representing the ray to test.
    \param pIntersect [Out] - A pointer to a keays::types::VectorD3 to receive the intersect point if it is found.
    \param tolerance [In] - A constant reference to a <b>double</b> representing the tolerance value to use in testing.

    \return True is the intersection point was found.
 */
KEAYS_MATH_EXPORTS_API const bool
RayTriangleIntersect(const keays::types::VectorD3 &triPt1, const keays::types::VectorD3 &triPt2,
                     const keays::types::VectorD3 &triPt3, const Line &line,
                     keays::types::VectorD3 *pIntersect,
                     const double &tolerance = keays::types::Float::TOLERANCE);

//-----------------------------------------------------------------------------
/*!
    \overload

    \param     triPts [In]  - a pointer to an array of 3 keays::types::VectorD2's representing the points of a CCW wound triangle.
 */
inline KEAYS_MATH_EXPORTS_API const int
LineCrossesTriangle(const keays::types::VectorD2 *triPts, const Line &line,
                    keays::types::VectorD3 *pLineStart, keays::types::VectorD3 *pLineEnd,
                    int *pStartEdge, int *pEndEdge,
                    const double &tolerance = keays::types::Float::TOLERANCE)
{
    return LineCrossesTriangle(triPts[0].VD3(), triPts[1].VD3(), triPts[2].VD3(), line, pLineStart, pLineEnd, pStartEdge, pEndEdge, tolerance);
}

//-----------------------------------------------------------------------------
/*!
    \overload

    \param     triPts [In]  - a pointer to an array of 3 keays::types::VectorD3's representing the points of a CCW wound triangle.
 */
inline KEAYS_MATH_EXPORTS_API const int
LineCrossesTriangle(const keays::types::VectorD3 *triPts, const Line &line,
                    keays::types::VectorD3 *pLineStart, keays::types::VectorD3 *pLineEnd,
                    int *pStartEdge, int *pEndEdge,
                    const double &tolerance = keays::types::Float::TOLERANCE)
{
    return LineCrossesTriangle(triPts[0], triPts[1], triPts[2], line, pLineStart, pLineEnd, pStartEdge, pEndEdge, tolerance);
}

//-----------------------------------------------------------------------------
/*!
    \brief Calculate the intersection points of a line segment and a triangle


    \param     triPt1 [In]  - a constant reference to a keays::types::VectorD3 representing the first point of a CCW wound triangle.
    \param     triPt2 [In]  - a constant reference to a keays::types::VectorD3 representing the second point of a CCW wound triangle.
    \param     triPt3 [In]  - a constant reference to a keays::types::VectorD3 representing the third point of a CCW wound triangle.
    \param       line [In]  - a constant reference to the line to test with, must have some 2D length or the function returns failure.
    \param pLineStart [Out] - A pointer to a keays::types::VectorD2 for the intersection point for the start of the line, MUST NOT BE NULL.
    \param   pLineEnd [Out] - A pointer to a keays::types::VectorD2 for the intersection point at the end of the line, MUST NOT BE NULL.
    \param pStartEdge [Out] - A pointer to an integer reepresenting the edge that the start point was found on, MUST NOT BE NULL.
    \param   pEndEdge [Out] - A pointer to an integer reepresenting the edge that the end point was found on, MUST NOT BE NULL.
    \param  tolerance [In]  - a constant double representing the tolerance to use for testing.

    \return a constant integer representing the success of the operation. One of the following values from eLCRVals -
    E_FAILURE, S_LINE_CROSSES, S_LINE_TOUCHES, S_LINE_CONTAINED, S_LINE_ENTERS or S_LINE_EXITS.
 */
KEAYS_MATH_EXPORTS_API const int
LineSegCrossesTriangle(const keays::types::VectorD3 &triPt1, const keays::types::VectorD3 &triPt2, const keays::types::VectorD3 &triPt3,
                        const Line &line, keays::types::VectorD3 *pLineStart, keays::types::VectorD3 *pLineEnd, int *pStartEdge, int *pEndEdge,
                        const double &tolerance = keays::types::Float::TOLERANCE);

//-----------------------------------------------------------------------------
/*!
    \overload

    \param     triPts [In]  - a pointer to an array of 3 keays::types::VectorD3's representing the points of a CCW wound triangle.
 */
inline KEAYS_MATH_EXPORTS_API const int
LineSegCrossesTriangle(const keays::types::VectorD3 *triPts, const Line &line,
                       keays::types::VectorD3 *pLineStart, keays::types::VectorD3 *pLineEnd,
                       int *pStartEdge, int *pEndEdge,
                       const double &tolerance = keays::types::Float::TOLERANCE)
{
    return LineSegCrossesTriangle(triPts[0], triPts[1], triPts[2], line, pLineStart, pLineEnd, pStartEdge, pEndEdge, tolerance);
}

/*!
    \overload

    \param     triPts [In]  - a pointer to an array of 3 keays::types::VectorD2's representing the points of a CCW wound triangle.
 */
inline KEAYS_MATH_EXPORTS_API const int
LineSegCrossesTriangle(const keays::types::VectorD2 *triPts, const Line &line,
                       keays::types::VectorD3 *pLineStart, keays::types::VectorD3 *pLineEnd,
                       int *pStartEdge, int *pEndEdge,
                       const double &tolerance = keays::types::Float::TOLERANCE)
{
    return LineSegCrossesTriangle(triPts[0].VD3(), triPts[1].VD3(), triPts[2].VD3(), line, pLineStart, pLineEnd, pStartEdge, pEndEdge, tolerance);
}
//! @}

typedef std::vector<keays::types::VectorD3> D3Vector;
typedef std::vector<keays::types::VectorD2> D2Vector;
typedef std::vector<Line> LineVector;
}    // namespace math
}    // namespace keays

// eof
