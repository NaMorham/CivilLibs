#pragma once

#include "utils.h"

/*
#include <LeakWatcher.h>

#ifdef _DO_MEMORY_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
//*/

namespace KSR
{
namespace utils
{
extern float TEX_TO_WIDTH_RATIO;
class keays::math::Line;

struct TexValues1D 
{
	float viewWidth;
	float lastTexVal;

	TexValues1D(float vw = 0.0f, float ltv = 0.0f) { viewWidth = vw; lastTexVal = ltv; }
};
typedef TexValues1D TexValues;

inline HRESULT InitKSRGeometry(KSR::PGEOMETRY pGeom)
{
	if (!pGeom)
		return E_FAIL;

	return pGeom->Init(sizeof(VERTEX_DIFFUSE_TEX1), FVF_DIFFUSE_TEX1, IT_32);
}

HRESULT AddKSRPoint(KSR::PGEOMETRY pGeom, const keays::types::VectorD3 pos, int &totalPts,
					const DWORD colour, const int effectID, const double size,
					const int type, const int dpBias);

HRESULT AddKSRCross(KSR::PGEOMETRY pGeom, const keays::types::VectorD3 pos, 
					int &totalPts, const DWORD colour, const int effectID, 
					const double size, const int dpBias);

HRESULT AddKSRDiamond(KSR::PGEOMETRY pGeom, keays::types::VectorD3 pos, 
					  int &totalPts, const DWORD colour, const int effectID, 
					  const double size, const int dpBias);

/*!
	\brief Draw an arrow using KSR
	NOTE: if the length is negative the position refers to the tip of the arrow, otherwise 
	it refers to the base.
 */
HRESULT AddKSRArrowHead(KSR::PGEOMETRY pGeom, keays::types::VectorD3 pos, int &totalPts, const DWORD colour, 
						const int effectID, const double directionRad, 
						const double length, const double width, const bool closed, 
						const bool solid, const int dpBias);

HRESULT AddKSRHalfArrowHead(KSR::PGEOMETRY pGeom, keays::types::VectorD3 pos, int &totalPts, 
							const DWORD colour, int effectID, const double directionRad, 
							const keays::math::eSideSelections side, const double length, 
							const double width, const bool closed, bool solid, const int dpBias);

HRESULT AddKSRFallIndicator(KSR::PGEOMETRY pGeom, const keays::types::VectorD3 center, 
							int &totalPts, const DWORD colour, const int effectID, 
							const double bearing, const double radius, 
							const double length, const double width, const int dpBias = 0);

HRESULT AddKSRLine(KSR::PGEOMETRY pGeom, const keays::types::VectorD3 p1, 
				   const keays::types::VectorD3 p2, int &totalPts, const DWORD colour, 
				   const int effectID, const int lineStyle, 
				   TexValues *texVals = NULL, const int dpBias = 0);

HRESULT AddKSRPolyLine(KSR::PGEOMETRY pGeom, const std::vector<keays::types::VectorD3> &pts, 
					   int &totalPts, const DWORD colour, const int effectID, 
					   const int lineStyle, TexValues *texVals = NULL, const int dpBias = 0);

HRESULT AddKSRCircle(KSR::PGEOMETRY pGeom, const keays::types::VectorD3 cPos, 
					 const double radius, const double intervalRad, int &totalPts, 
					 const DWORD colour, const int effectID, const keays::math::eAngleDirections dir, 
					 const int lineStyle, TexValues *texVals = NULL, const int dpBias = 0);

HRESULT AddKSRArc(KSR::PGEOMETRY pGeom, const keays::types::VectorD3 cPos, const double radius, 
				  const double startAngleRad, const double endAngleRad, 
				  const keays::math::eAngleDirections dir, const double intervalRad, 
				  const bool doEndPoint, int &totalPts, const DWORD colour, const int effectID, 
				  const int lineStyle, TexValues *texVals = NULL, const int dpBias = 0);

const Vector2 GetWorldCoords(const POINT &pt, const PVIEWPORT vp);

const Vector2 GetWorldSize(const POINT &pt, const PVIEWPORT vp);

const Vector2 GetWorldCoords(const POINT &pt, const keays::math::RectD &vpRect, const RECT &rect);

const POINT GetScreenCoords(const keays::types::VectorD3 &pt, const PVIEWPORT vp, const RECT &rect);

const POINT GetScreenCoords(const keays::types::VectorD3 &pt, const keays::math::RectD &vpRect, const RECT &rect);

#ifdef _DEBUG
HRESULT DebugAxisGrid(KSR::PGEOMETRY pGeom, int &totalPts);
#endif
}
}