/*! \file */
/*-----------------------------------------------------------------------
	ksrxmath.h

	Description: Keays Simulation & Rendering Extensions API math header
	Author:		 Julian McKinlay
	---------------------------------------------------------------------*/

#pragma once

/*-----------------------------------------------------------------------
	Function Declarations
	---------------------------------------------------------------------*/
	//! \brief Projects a 3D Vector from world space to screen space.
	/*! Returns the resulting Vector2.
		\param pViewport [in] Pointer to a valid KSRViewport, storing the projection used for calculation.
		\param v [out] 3D Vector to unproject. */
	KSR::Vector3 Unproject(KSR::PVIEWPORT pViewport, const KSR::Vector2 &v);

	//! \brief Unprojects a 2D Vector from screen space to world space.
	/*! Returns the resulting Vector2.
		\param pViewport [in] Pointer to a valid KSRViewport, storing the projection used for calculation.
		\param v [out] 3D Vector to unproject. */
	KSR::Vector2 Project(KSR::PVIEWPORT pViewport, const KSR::Vector3 &v);

	const DWORD DivideARGB(const DWORD colour, const unsigned char divisor, bool modifyAlpha = false);
	const DWORD MultiplyARGB(const DWORD colour, const unsigned char scalar, bool modifyAlpha = false);
// EOF