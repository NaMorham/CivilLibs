/*
 * Filename: keays_types.h
 * Date: May 2004
 * Authors: Andrew Hickey, Keays Software
 *			Karl Janke, Keays Software
 *
 * Common KEAYS types.
 *
 * Part of the keays::types namespace
 */

#ifndef _KEAYS_TYPES_H_
#define _KEAYS_TYPES_H_

#pragma once

#ifdef KEAYS_TYPES_EXPORTS
	#define KEAYS_TYPES_EXPORTS_API __declspec(dllexport)
#else
	#define KEAYS_TYPES_EXPORTS_API __declspec(dllimport)
#endif

#include <vector>
#include <tchar.h>

/*!
 * Typedefs for basic types. Placed outside the namespace for simplicity
 * when using
 */
typedef unsigned int	uInt;
typedef unsigned long	uLong;

/*!
	\brief General Keays Software Namespace
	Used to prevent conflicts with existing functions/ variables
 */
namespace keays
{
/*!
	\brief Maths & Geometry routines for use in Keays Software applications
 */
namespace types
{

/*!
 * \brief Function typedef for use in progress displays
 * this has been moved to here from keays triangle since it is used in
 * more than just triangle files now.
 *
 * \todo Attach this to saving functions as well (when they are done)
 */
typedef KEAYS_TYPES_EXPORTS_API unsigned int (*pFnProgressUpdate)(const float, const TCHAR *, void *);

//-----------------------------------------------------------------------------
/*!
	\brief Floating Point routines for comparisons/conversions with tolerances
 */
namespace Float
{
	/*!
		\brief a default tolerance value to use in comparisons.
		We need TOLERANCE because casting between doubles & floats can create a small margin of
		error that screws with comparisons. These functions compenstate for this error.
	 */
	extern KEAYS_TYPES_EXPORTS_API double TOLERANCE;

	/*!
		\brief Less than comparison (lhs < rhs)
		\param lhs [In] - a constant reference to a <b>double</b> representing the lhs of the comparison.
		\param rhs [In] - a constant reference to a <b>double</b> representing the rhs of the comparison.
		\param tol [In] - a constant reference to a <b>double</b> representing the tolerance for the comparison [optional].
	 */
	KEAYS_TYPES_EXPORTS_API bool Less(const double &lhs, const double &rhs, const double &tol = TOLERANCE);

	/*!
		\brief Less than or equal to comparison (lhs <= rhs)
		\param lhs [In] - a constant reference to a <b>double</b> representing the lhs of the comparison.
		\param rhs [In] - a constant reference to a <b>double</b> representing the rhs of the comparison.
		\param tol [In] - a constant reference to a <b>double</b> representing the tolerance for the comparison [optional].
	 */
	KEAYS_TYPES_EXPORTS_API bool LessOrEqual(const double &lhs, const double &rhs, const double &tol = TOLERANCE);

	/*!
		\brief Equal to comparison (lhs == rhs)
		\param lhs [In] - a constant reference to a <b>double</b> representing the lhs of the comparison.
		\param rhs [In] - a constant reference to a <b>double</b> representing the rhs of the comparison.
		\param tol [In] - a constant reference to a <b>double</b> representing the tolerance for the comparison [optional].
	 */
	KEAYS_TYPES_EXPORTS_API bool EqualTo(const double &lhs, const double &rhs, const double &tol = TOLERANCE);

	/*!
		\brief Greater than comparison (lhs > rhs)
		\param lhs [In] - a constant reference to a <b>double</b> representing the lhs of the comparison.
		\param rhs [In] - a constant reference to a <b>double</b> representing the rhs of the comparison.
		\param tol [In] - a constant reference to a <b>double</b> representing the tolerance for the comparison [optional].
	 */
	KEAYS_TYPES_EXPORTS_API bool Greater(const double &lhs, const double &rhs, const double &tol = TOLERANCE);

	/*!
		\brief Greater than or equal to comparison (lhs >= rhs)
		\param lhs [In] - a constant reference to a <b>double</b> representing the lhs of the comparison.
		\param rhs [In] - a constant reference to a <b>double</b> representing the rhs of the comparison.
		\param tol [In] - a constant reference to a <b>double</b> representing the tolerance for the comparison [optional].
	 */
	KEAYS_TYPES_EXPORTS_API bool GreaterOrEqual(const double &lhs, const double &rhs, const double &tol = TOLERANCE);

	/*!
		\brief Not equal to comparison (lhs != rhs)
		\param lhs [In] - a constant reference to a <b>double</b> representing the lhs of the comparison.
		\param rhs [In] - a constant reference to a <b>double</b> representing the rhs of the comparison.
		\param tol [In] - a constant reference to a <b>double</b> representing the tolerance for the comparison [optional].
	 */
	KEAYS_TYPES_EXPORTS_API bool NotEqual(const double &lhs, const double &rhs, const double &tol = TOLERANCE);

	/*!
		\brief Safe conversion from a float to a double
		\param  fl [In] - a constant reference to a <b>float</b> representing the floating point value to convert.
		\param tol [In] - a constant reference to a <b>double</b> representing the tolerance for the conversion [optional].
	 */
	KEAYS_TYPES_EXPORTS_API const double SafeFloatToDouble(float fl, const double &tol = TOLERANCE);

	/*!
		\brief Limit the Precion of a double with rounding.
		\param       val [In] - a constant reference to a <b>double</b> representing the value to limit.
		\param tolerance [In] - a constant reference to a <b>double</b> representing the tolerance value to use for the limit.
	 */
	const double LimitPrecisionRound(const double &val, const double &tolerance);

	/*!
		\brief Convert a double to an integer with rounding.
		\param val [In] - a constant reference to a <b>double</b> representing the value to cast to an integer.
	 */
	const int ToInt(const double &val);

	/*!
		\brief A VERY large negative number to use when an invalid double value is required.
		This is really evil. Its not even 3vil, but _evil_. kj.
		However we can get away with it if its big (or small)enough because the planet isn't that big, it will still only be
		valid in certain contexts - ah.
		Effectively it represents -999,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000
	 */
	extern KEAYS_TYPES_EXPORTS_API const double INVALID_DOUBLE;
}	// namespace Float


//-----------------------------------------------------------------------------
// predeclare classes for use in external, yet dependant functions
class KEAYS_TYPES_EXPORTS_API VectorD3;
class KEAYS_TYPES_EXPORTS_API VectorD2;

/*!
	\brief A constructive operator to allow a double to be multiplied by a vector.
	\note Functionally equivalent to the VectorD2::operator* member function.

	\param  scale [In] - a constant reference to a <b>double</b> representing the amount to scale the vector by.
	\param vector [In] - a constant reference to a keays::types::VectorD2 representing the vector to scale.
	\return A constant keays::types::VectorD2 representing the scaled vector.
 */
KEAYS_TYPES_EXPORTS_API const VectorD2 operator*(const double &scale, const VectorD2 &vector);

/*!
	\brief A constructive operator to allow a double to be multiplied by a vector.
	\note Functionally equivalent to the VectorD3::operator* member function.

	\param  scale [In] - a constant reference to a <b>double</b> representing the amount to scale the vector by.
	\param vector [In] - a constant reference to a keays::types::VectorD3 representing the vector to scale.
	\return A constant keays::types::VectorD3 representing the scaled vector.
 */
KEAYS_TYPES_EXPORTS_API const VectorD3 operator*(const double &scale, const VectorD3 &vector);

/*!
	\brief A constructive operator to allow a double to be multiplied by a vector.
	\note Functionally equivalent to the VectorD2::operator/ member function.

	\param  scale [In] - a constant reference to a <b>double</b> representing the amount to scale the vector by.
	\param vector [In] - a constant reference to a keays::types::VectorD2 representing the vector to scale.
	\return A constant keays::types::VectorD2 representing the scaled vector.
 */
KEAYS_TYPES_EXPORTS_API const VectorD2 operator/(const double &scale, const VectorD2 &vector);

/*!
	\brief A constructive operator to allow a double to be multiplied by a vector.
	\note Functionally equivalent to the VectorD3::operator/ member function.

	\param  scale [In] - a constant reference to a <b>double</b> representing the amount to scale the vector by.
	\param vector [In] - a constant reference to a keays::types::VectorD3 representing the vector to scale.
	\return A constant keays::types::VectorD3 representing the scaled vector.
 */
KEAYS_TYPES_EXPORTS_API const VectorD3 operator/(const double &scale, const VectorD3 &vector);

//-----------------------------------------------------------------------------
/*!
	\brief A 3D Point class using double precision.
	The VectorD3 class provides a representation of a 3 points in space.
	They are stored as 'D'oubles for increased precesion. Many convience member functions are available.
 */
class KEAYS_TYPES_EXPORTS_API VectorD3
{
public:
	/*!
		\brief Constructor (3 doubles)
		Construct a VectorD3 at the position specified.

		\param xPos [In] - a constant reference to a <b>double</b> representing the X component. Defaults to 0.0
		\param yPos [In] - a constant reference to a <b>double</b> representing the Y component. Defaults to 0.0
		\param zPos [In] - a constant reference to a <b>double</b> representing the Z component. Defaults to 0.0
	 */
	VectorD3(const double &xPos = 0, const double &yPos = 0, const double &zPos = 0): x(xPos), y(yPos), z(zPos){}

	/*!
		\brief Copy Constructor. Create a duplicate of an existing VectorD3.

		\param vd3 [In] - a constant reference to a keays::types::VectorD3 to duplicate.
	 */
	VectorD3(const VectorD3 &vd3): x(vd3.x), y(vd3.y), z(vd3.z){}

	/*!
		\brief Copy Constructor. Create a duplicate of an existing VectorD2, with a specified height.

		\param  vd2 [In] - a constant reference to a keays::types::VectorD2 to duplicate.
		\param zPos [In] - a constant reference to <b>double</b> representing the Z component. Defaults to 0.0
	 */
	VectorD3(const VectorD2 &vd2, const double &zPos = 0);

	/*!
		\brief Operator=, copy an existing VectorD3.

		\param rhs [In] - a constant reference to a keays::types::VectorD3 to copy.
		\return a constant reference to the current object.
	 */
	const VectorD3 &operator=(const VectorD3 &rhs);

	/*!
		\brief Operator=, copy an existing VectorD2.

		\param rhs [In] - a constant reference to a keays::types::VectorD2 to copy.
		\return a constant reference to the current object.
	 */
	const VectorD3 &operator=(const VectorD2 &rhs);

	/*!
		\brief Comparison operator==, Test for equivalence (with tolerances).

		\param rhs [In] - a constant reference to a keays::types::VectorD3 to compare.
		\return true if the specified keays::types::VectorD3 is equivalent.
	 */
	bool operator==(const VectorD3 &rhs)const;
	/*!
		\overload
		\param rhs [In] - a constant reference to a keays::types::VectorD2 to compare.
	 */
	bool operator==(const VectorD2 &rhs)const;

	/*!
		\brief Comparison operator!=, Test for inequivalence (with tolerances).

		\param rhs [In] - a constant reference to a keays::types::VectorD3 to compare.
		\return true if the specified keays::types::VectorD3 is not equivalent.
	 */
	bool operator!=(const VectorD3 &rhs)const;
	/*!
		\overload
		\param rhs [In] - a constant reference to a keays::types::VectorD2 to compare.
	 */
	bool operator!=(const VectorD2 &rhs)const;

	/*!
		\brief Addition operator+. Add another keays::types::VectorD3 to this one (Translation)

		\param rhs [In] - a constant reference to a keays::types::VectorD3 to add.
		\return the resulting, translated keays::types::VectorD3.
	 */
	const VectorD3 operator+(const VectorD3 &rhs)const;

	/*!
		\overload

		\param rhs [In] - a constant reference to a keays::types::VectorD2 to add.
		\return the resulting, translated keays::types::VectorD3.
	 */
	const VectorD3 operator+(const VectorD2 &rhs)const;

	/*!
		\brief Addition operator+=. Add another VectorD3 to this one and keep the result (Self Translation).

		\param rhs [In] - a constant reference to a keays::types::VectorD3 to add.
		\return a constant reference to the current object, after translation.
	 */
	const VectorD3 &operator+=(const VectorD3 &rhs);

	/*!
		\overload

		\param rhs [In] - a constant reference to a keays::types::VectorD2 to add.
		\return a constant reference to the current object, after translation.
	 */
	const VectorD3 &operator+=(const VectorD2 &rhs);

	/*!
		\brief Subtraction operator-. Subtract another VectorD3 from this one (Translation).

		\param rhs [In] - a constant reference to a keays::types::VectorD3 to subtract.
		\return the resulting, translated keays::types::VectorD3.
	 */
	const VectorD3 operator-(const VectorD3 &rhs)const;

	/*!
		\brief Unary operator -. Provides an inverse operator.

		\return The inverse of the current object.
	 */
	const VectorD3 operator-()const { return *this * -1; }

	/*!
		\overload

		\param rhs [In] - a constant reference to a keays::types::VectorD2 to subtract.
		\return the resulting, translated keays::types::VectorD3.
	 */
	const VectorD3 operator-(const VectorD2 &rhs)const;

	/*!
		\brief Subtraction operator-=. Subtract another VectorD3 from this one and keep the result (Self Translation).

		\param rhs [In] - a constant reference to a keays::types::VectorD3 to subtract.
		\return a constant reference to the current object, after translation.
	 */
	const VectorD3 &operator-=(const VectorD3 &rhs);

	/*!
		\overload

		\param rhs [In] - a constant reference to a keays::types::VectorD2 to subtract.
		\return a constant reference to the current object, after translation.
	 */
	const VectorD3 &operator-=(const VectorD2 &rhs);

	/*!
		\brief Multiplication operator*. Multiply the current VectorD3 by a given value (Scaling).

		\param scale [In] - a constant reference to a <b>double</b> specifying the amout to scale by.
		\return the resultant, scaled keays::types::VectorD3.
	 */
	const VectorD3 operator*(const double &scale)const;
	/*!
		\brief Multiplication operator*=. Multiply the current VectorD3 by a given value and keep the result (Self Scaling).

		\param scale [In] - a constant reference to a <b>double</b> specifying the amout to scale by.
		\return a constant reference to the current object, after scaling.
	 */
	const VectorD3 &operator*=(const double &scale);

	/*!
		\brief Division operator*. Divide the current VectorD3 by a given value (Scaling).

		\param scale [In] - a constant reference to a <b>double</b> specifying the amout to scale by.
		\return the resultant, scaled keays::types::VectorD3.
	 */
	const VectorD3 operator/(const double &scale)const;
	/*!
		\brief Division operator*=. Divide the current VectorD3 by a given value and keep the result (Self Scaling).

		\param scale [In] - a constant reference to a <b>double</b> specifying the amout to scale by.
		\return a constant reference to the current object, after scaling.
	 */
	const VectorD3 &operator/=(const double &scale);

	/*!
		\brief Casting operator VectorD2().
		\return a keays::types::VectorD2 representing the x and y components of the keays::types::VectorD3.
	 */
	operator VectorD2()const;

	/*!
		\brief Cast to a constant double pointer.
		\warning This is not a safe operator as it cannot perform/provide bounds testing.
		\return the address of the first element of an array of 3 points.
	 */
	operator const double*()const { return m_loc; }

	/*!
		\brief Calculate the magnitude of the keays::types::VectorD3.
	 */
	const double Magnitude()const;

	/*!
		\brief Calculate the 2D magnitude of the keays::types::VectorD3.
	 */
	const double Magnitude2D()const;

	/*!
		\brief Calculate the grade of the vector
	 */
	const double Grade()const;

	/*!
		\brief Normalise the VectorD3.
		\return a constant reference to the current object, after normalising.
	 */
	const VectorD3 &Normalise();

	/*!
		\brief Get a normalised copy of the VectorD3.
		\return a normalised copy of the current object.
	 */
	const VectorD3 GetNormalised()const;

	/*!
		\brief Get a normalised VectorD3 specified by the values provided.
		\param xSize [In] - a constant reference to a double representing the x component of the vector to normalise.
		\param ySize [In] - a constant reference to a double representing the y component of the vector to normalise.
		\param zSize [In] - a constant reference to a double representing the z component of the vector to normalise.
		\return a normalised version of the Vector defined by the xSize, ySize and zSize values.
	 */
	static const VectorD3 GetNormalised(const double &xSize, const double &ySize, const double &zSize);

	/*!
		\brief Calculate the crossproduct of the current Vector with the specified vector.
		\param vec [In] - a const reference to a keays::types::VectorD3 representing the second vector to use in the
						  cross product with the current object.
		\return The keays::types::VectorD3 representing the cross product of the current object with vec.
	 */
	const VectorD3 Cross(const VectorD3 &vec)const;

	/*!
		\brief Calculate the dot product of the current object with another VectorD3.
		\note This operation requires both the current object and passed in VectorD3 to be normalised to obtain the cosine
			  of the angle between them.
		\param vec [In] - a constant reference to a keays::types::VectorD2 specifying the second vector to calculate with.
		\return a constant double specifying the dot product of the current object with vec.
	 */
	const double Dot(const VectorD3 &vec)const;

	/*!
		\brief Calculate the angle between the current object and another VectorD3.
		\param vec [In] - a constant reference to a keays::types::VectorD3 specifying the second vector to calculate with.
		\return a constant double specifying the angle between the current object with vec, in radians.
	 */
	const double AngleBetween(const VectorD3 &vec)const;

	/*!
		\brief Export a VectorD2 representing the X/Y component of the VectorD3.
		\return a keays::types::VectorD2 representing the x and y components of the VectorD3.
	 */
	const VectorD2 XY()const;

	/*!
		\brief Modify the X/Y components of the VectorD3.

		\param xPos [In] - a constant reference to a <b>double</b> representing the X component.
		\param yPos [In] - a constant reference to a <b>double</b> representing the Y component.
		\return a constant reference to the current object, after modification.
	 */
	inline const VectorD3 &XY(const double &xPos, const double &yPos){ x = xPos; y = yPos; return *this; }

	/*!
		\brief Export a VectorD2 representing the X/Z component of the VectorD3.
		\return a keays::types::VectorD2 representing the x and z components of the keays::types::VectorD3.
	 */
	const VectorD2 XZ()const;
	/*!
		\brief Modify the X/Z components of the VectorD3.

		\param xPos [In] - a constant reference to a <b>double</b> representing the X component.
		\param zPos [In] - a constant reference to a <b>double</b> representing the Z component.
		\return a constant reference to the current object, after modification.
	 */
	inline const VectorD3 &XZ(const double &xPos, const double &zPos){ x = xPos; z = zPos; return *this; }

	/*!
		\brief Export a VectorD2 representing the Y/Z component of the VectorD3.
		\return a VectorD2 representing the y and z components of the keays::types::VectorD3.
	 */
	const VectorD2 YZ()const;
	/*!
		\brief Modify the Y/Z components of the VectorD3.

		\param yPos [In] - a constant reference to a <b>double</b> representing the Y component.
		\param zPos [In] - a constant reference to a <b>double</b> representing the Z component.
		\return a constant reference to the current object, after modification.
	 */
	inline const VectorD3 &YZ(const double &yPos, const double &zPos){ y = yPos; z = zPos; return *this; }

	/*!
		\brief Modify the components of the VectorD3.

		\param xPos [In] - a constant reference to a <b>double</b> representing the X component.
		\param yPos [In] - a constant reference to a <b>double</b> representing the Y component.
		\param zPos [In] - a constant reference to a <b>double</b> representing the Z component.
		\return a constant reference to the current object, after modification.
	 */
	inline const VectorD3 &Set(const double &xPos, const double &yPos, const double &zPos){ x = xPos; y = yPos; z = zPos; return *this; }

	union
	{
		struct
		{
			double	x,	//!< the X component of the VectorD3.
					y,	//!< the Y component of the VectorD3.
					z;	//!< the Z component of the VectorD3.
		};
		double m_loc[3];
	};

	// general information for the class
	friend KEAYS_TYPES_EXPORTS_API const VectorD3 operator*(const double &scale, const VectorD3 &vector);	//!< allow the external operator* to access private data
	friend KEAYS_TYPES_EXPORTS_API const VectorD3 operator/(const double &scale, const VectorD3 &vector);	//!< allow the external operator/ to access private data

	static double ms_tolerance;
};

extern KEAYS_TYPES_EXPORTS_API const VectorD3 BAD_POINT;
extern KEAYS_TYPES_EXPORTS_API const double INVALID_ANGLE;

//-----------------------------------------------------------------------------
/*!
	\brief A 2D Point class using double precision.
	The VectorD2 class provides a representation for 2D points in space.
	They are stored as 'D'oubles for increased precesion. Many convience member functions are available.
 */
class KEAYS_TYPES_EXPORTS_API VectorD2
{
public:
	/*!
		\brief Constructor. Construct a VectorD2 at the position specified.

		\param xPos [In] - a constant reference to a <b>double</b> representing the X component.
		\param yPos [In] - a constant reference to a <b>double</b> representing the Y component.
	 */
	VectorD2(const double &xPos = 0, const double &yPos = 0): x(xPos), y(yPos){}
	/*!
		\brief Copy Constructor. Create a duplicate of an existing VectorD3.

		\param vd3 [In] - a constant reference to a keays::types::VectorD3 to duplicate.
	 */
	VectorD2(const VectorD3 &vd3): x(vd3.x), y(vd3.y){}
	/*!
		\brief Copy Constructor. Create a duplicate of an existing VectorD2.

		\param  vd2 [In] - a constant reference to a keays::types::VectorD3 to duplicate.
	 */
	VectorD2(const VectorD2 &vd2): x(vd2.x), y(vd2.y){}

	/*!
		\brief Operator=, copy an existing VectorD3.

		\param rhs [In] - a constant reference to a keays::types::VectorD3 to copy.
		\return a constant reference to the current object.
	 */
	const VectorD2 &operator=(const VectorD3 &rhs);
	/*!
		\brief Operator=, copy an existing VectorD2.

		\param rhs [In] - a constant reference to a keays::types::VectorD2 to copy.
		\return a constant reference to the current object.
	 */
	const VectorD2 &operator=(const VectorD2 &rhs);

	/*!
		\brief Comparison operator==, Test for equivalence (with tolerances).

		\param rhs [In] - a constant reference to a keays::types::VectorD3 to compare.
		\return true if the specified keays::types::VectorD3 is equivalent.
	 */
	bool operator==(const VectorD3 &rhs)const;
	/*!
		\overload
		\param rhs [In] - a constant reference to a keays::types::VectorD2 to compare.
	 */
	bool operator==(const VectorD2 &rhs)const;

	/*!
		\brief Comparison operator!=, Test for inequivalence (with tolerances).

		\param rhs [In] - a constant reference to a keays::types::VectorD3 to compare.
		\return true if the specified keays::types::VectorD3 is not equivalent.
	 */
	bool operator!=(const VectorD3 &rhs)const { return !(*this == rhs); }
	/*!
		\overload
		\param rhs [In] - a constant reference to a keays::types::VectorD2 to compare.
	 */
	bool operator!=(const VectorD2 &rhs)const { return !(*this == rhs); }

	/*!
		\brief Addition operator+. Add another VectorD2 to this one (Translation).

		\param rhs [In] - a constant reference to a keays::types::VectorD2 to add.
		\return the resulting, translated keays::types::VectorD2.
	 */
	const VectorD2 operator+(const VectorD2 &rhs)const;

	/*!
		\brief Addition operator+=. Add another VectorD2 to this one and keep the result (Self Translation).

		\param rhs [In] - a constant reference to a keays::types::VectorD2 to add.
		\return a constant reference to the current object, after translation.
	 */
	const VectorD2 &operator+=(const VectorD2 &rhs);

	/*!
		\brief Subtraction operator-. Subtract another VectorD2 from this one (Translation).

		\param rhs [In] - a constant reference to a keays::types::VectorD2 to subtract.
		\return the resulting, translated keays::types::VectorD2.
	 */
	const VectorD2 operator-(const VectorD2 &rhs)const;

	/*!
		\brief Unary operator -. Provides an inverse operator.

		\return The inverse of the current object.
	 */
	const VectorD2 operator-()const { return *this * -1; }

	/*!
		\brief Subtraction operator-=. Subtract another VectorD2 from this one and keep the result (Self Translation).

		\param rhs [In] - a constant reference to a keays::types::VectorD2 to subtract.
		\return a constant reference to the current object, after translation.
	 */
	const VectorD2 &operator-=(const VectorD2 &rhs);

	/*!
		\brief Multiplication operator*. Multiply the current VectorD2 by a given value (Scaling).

		\param scale [In] - a constant reference to a <b>double</b> specifying the amout to scale by.
		\return the resultant, scaled keays::types::VectorD2.
	 */
	const VectorD2 operator*(const double &scale)const;
	/*!
		\brief Multiplication operator*=. Multiply the current VectorD2 by a given value and keep the result (Self Scaling).

		\param scale [In] - a constant reference to a <b>double</b> specifying the amout to scale by.
		\return a constant reference to the current object, after scaling.
	 */
	const VectorD2 &operator*=(const double &scale);

	/*!
		\brief Division operator*. Divide the current VectorD2 by a given value (Scaling).

		\param scale [In] - a constant reference to a <b>double</b> specifying the amout to scale by.
		\return the resultant, scaled keays::types::VectorD2.
	 */
	const VectorD2 operator/(const double &scale)const;
	/*!
		\brief Division operator*=. Divide the current VectorD2 by a given value and keep the result (Self Scaling).

		\param scale [In] - a constant reference to a <b>double</b> specifying the amout to scale by.
		\return a constant reference to the current object, after scaling.
	 */
	const VectorD2 &operator/=(const double &scale);

	/*!
		\brief Casting operator VectorD3()
		\return a VectorD3 representing the x and y components of the keays::types::VectorD2, with a z of 0.
	 */
	operator VectorD3();

	/*!
		\brief Calculate the dot product of the current object with another VectorD2.
		\note This operation requires both the current object and passed in VectorD2 to be normalised to obtain the cosine
			  of the angle between them.
		\param vec [In] - a constant reference to a keays::types::VectorD2 specifying the second vector to calculate with.
		\return a constant double specifying the dot product of the current object with vec. (The Cos of the angle between them)
	 */
	const double Dot(const VectorD2 &vec)const;

	/*!
		\brief Calculate the angle between the current object and another VectorD2.
		\param vec [In] - a constant reference to a keays::types::VectorD2 specifying the second vector to calculate with.
		\return a constant double specifying the angle between the current object with vec, in radians.
	 */
	const double AngleBetween(const VectorD2 &vec)const;

	/*!
		\brief Cast to a constant double pointer
		\warning This is not a safe operator as it performs no bounds testing
		\return the address of the first element of an array of 2 points.
	 */
	operator const double*()const { return m_loc; }

	/*!
		\brief Normalise the VectorD2.
		\return a constant reference to the current object, after normalising.
	 */
	const VectorD2 &Normalise();

	/*!
		\brief Get a normalised copy of the VectorD3.
		\return a normalised copy of the current object.
	 */
	const VectorD2 GetNormalised()const;

	/*!
		\brief Get a normalised VectorD2 specified by the values provided.
		\param xSize [In] - a constant reference to a double representing the x component of the vector to normalise
		\param ySize [In] - a constant reference to a double representing the y component of the vector to normalise
		\return a normalised version of the Vector defined by the xSize and ySize values.
	 */
	static const VectorD2 GetNormalised(const double &xSize, const double &ySize);

	/*!
		\brief Casting operator VectorD3()

		\param zPos [In] - a constant reference to a <b>double</b> specifying the z value for the keays::types::VectorD3.
		\return a VectorD3 representing the x and y components of the keays::types::VectorD3, with a z as specified.
	 */
	const VectorD3 VD3(const double &zPos = 0.0)const { return VectorD3(x, y, zPos); }

	/*!
		\brief Modify the components of the VectorD2.

		\param xPos [In] - a constant reference to a <b>double</b> representing the X component.
		\param yPos [In] - a constant reference to a <b>double</b> representing the Y component.
		\return a constant reference to the current object, after modification.
	 */
	inline const VectorD2 &Set(const double &xPos, const double &yPos){ x = xPos; y = yPos; return *this; }

	const VectorD2 GetPerpendicular(bool rightSide = true)const;

	union
	{
		struct
		{
			double	x,	//!< the X component of the VectorD2.
					y;	//!< the Y component of the VectorD2.
		};
		double m_loc[2];
	};

	static double ms_tolerance;

	friend KEAYS_TYPES_EXPORTS_API const VectorD2 operator*(const double &scale, const VectorD2 &vector);
	friend KEAYS_TYPES_EXPORTS_API const VectorD2 operator/(const double &scale, const VectorD2 &vector);
};

//-----------------------------------------------------------------------------
typedef std::vector<VectorD2> Polyline2D;
typedef std::vector<VectorD3> Polyline3D;

//-----------------------------------------------------------------------------
// Local Axes Declaration
/*
	\brief A simple axes type
 */
class KEAYS_TYPES_EXPORTS_API LocalAxes
{
public:
	LocalAxes();
	LocalAxes(const VectorD3 &i, const VectorD3 &j, const VectorD3 &k);

	// accessors
	const VectorD3 &I()const { return m_i; }
	const VectorD3 &J()const { return m_j; }
	const VectorD3 &K()const { return m_k; }

	// mutators
	const VectorD3 &I(const VectorD3 &i);
	const VectorD3 &J(const VectorD3 &j);
	const VectorD3 &K(const VectorD3 &k);

	const VectorD3 &CalcI();
	const VectorD3 &CalcJ();
	const VectorD3 &CalcK();

	const LocalAxes &operator=(const LocalAxes &rhs);
//	size_t WriteFile(FILE *file)const;

private:
	VectorD3 m_i, m_j, m_k;
};

}	// namespace keays
}	// namespace types

//-----------------------------------------------------------------------------
/*
	\brief Basic 32 bit bitvector
	A very simple bitvector, for use when the STL bitset is to heavyweight
 */
class KEAYS_TYPES_EXPORTS_API BitVector
{
public:
	BitVector(const long bits = 0, const long mask = 0xFFFFFFFF): m_bits(bits), m_mask(mask){}
	BitVector(const BitVector &original){ *this = original; }
	~BitVector(){ m_bits = 0; m_mask = 0xFFFFFFFF; }

	const BitVector &operator=(const BitVector &rhs){ m_bits = rhs.Bits(); m_mask = rhs.Mask(); return *this; }

	const long Mask()const								{ return m_mask; }
	const long Mask(const long mask){ return m_mask = mask; }

	const long Set(const long bits){ return m_bits |= (m_mask & bits); }
	const long Toggle(const long bits){ return m_bits = m_bits ^ (m_mask & bits); } // something wrong here, not sure what
	const long Clear(const long bits){ return m_bits = m_bits & m_mask & (~bits); }
	const bool IsSet(const long bits)const			{ return (m_bits & (m_mask & bits))!= 0; }

	/*
		#define IS_SET(flag,bit)((flag)& (bit))
		__inline bool IsSet(bitvector_t *bvt, bitvector_t bit){ return (*(bvt)& bit)!= 0; }

		#define SET_BIT(var,bit)((var)|= (bit))
		#define REMOVE_BIT(var,bit)((var)&= ~(bit))
		#define TOGGLE_BIT(var,bit)((var)= (var)^ (bit))
	*/

	void SetAll(){ m_bits = 0xFFFFFFFF; }
	void ClearAll(){ m_bits = 0; }

	const long Bits()const						{ return m_bits; }

	static const long MakeBitValue(const unsigned char bit){ return (bit < 32 && bit >= 0 ? 1 << bit : 0); }
private:
	long m_bits;
	long m_mask;
};

#endif //_KEAYS_TYPES_H_