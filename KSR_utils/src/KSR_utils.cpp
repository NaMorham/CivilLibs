#include "../include/KSR_utils.h"

#include <LeakWatcher.h>

#ifdef _DO_MEMORY_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace keays::types;
using namespace keays::math;

namespace KSR
{
namespace utils
{
float TEX_TO_WIDTH_RATIO = 16.0f/300.0f * 30.0f * 2.0f;

HRESULT AddKSRPoint(KSR::PGEOMETRY pGeom, const keays::types::VectorD3 pos,
                     int &totalPts, const DWORD colour, const int effectID,
                     const double size, const int type, const int dpBias)
{
    if (type == POINT_CROSS)
        return AddKSRCross(pGeom, pos, totalPts, colour, effectID, size, dpBias);
    else
        return AddKSRDiamond(pGeom, pos, totalPts, colour, effectID, size, dpBias);
}

HRESULT AddKSRDiamond(KSR::PGEOMETRY pGeom, const keays::types::VectorD3 pos,
                       int &totalPts, const DWORD colour, const int effectID,
                       const double size, const int dpBias)
{
    KSR::Chunk chunk;
    float rSize = float(size * ROOT_2_ON2);

    chunk.idTexture0    = -1;
    chunk.idTexture1    = -1;
    chunk.numVerts        = 4;
    chunk.depthBias        = dpBias;
    chunk.startIndex    = totalPts;
    chunk.type            = CT_TRIANGLESTRIP;
    chunk.effect        = -1;

    VERTEX_DIFFUSE_TEX1 verticies[4];

    for (int nm = 0; nm < 4; ++nm)
    {
        verticies[nm].color = colour;
        verticies[nm].uv = Vector2(0, 0);
    }

    verticies[0].position = Vector3((float)(pos.x-rSize), (float)pos.y, 0);
    verticies[1].position = Vector3((float)pos.x, (float)(pos.y-rSize), 0);
    verticies[2].position = Vector3((float)pos.x, (float)(pos.y+rSize), 0);
    verticies[3].position = Vector3((float)(pos.x+rSize), (float)pos.y, 0);

    totalPts += 4;

    if (FAILED(pGeom->Insert(1, chunk.numVerts, &chunk, verticies, 0)))
    {
        KSR::Logf(LL_LOWEST, "ERROR: %s:%d Insert Geometry Failed!", __FILE__, __LINE__);
        return E_FAIL;
    }

    return S_OK;
}

HRESULT AddKSRCross(KSR::PGEOMETRY pGeom, const keays::types::VectorD3 pos,
                     int &totalPts, const DWORD colour, const int effectID,
                     const double size, const int dpBias)
{
    if (!pGeom)
        return E_FAIL;

    KSR::Chunk chunk;

    chunk.idTexture0    = -1;
    chunk.idTexture1    = -1;
    chunk.numVerts        = 4;
    chunk.depthBias        = dpBias;
    chunk.startIndex    = totalPts;
    chunk.type            = CT_LINELIST;
    chunk.effect        = effectID;

    VERTEX_DIFFUSE_TEX1 verticies[4];

    for (int nm = 0; nm < 4; ++nm)
    {
        verticies[nm].color = colour;
        verticies[nm].uv = Vector2(0, 0);
    }

    verticies[0].position = Vector3((float)(pos.x+size), (float)(pos.y+size), 0);
    verticies[1].position = Vector3((float)(pos.x-size), (float)(pos.y-size), 0);
    verticies[2].position = Vector3((float)(pos.x-size), (float)(pos.y+size), 0);
    verticies[3].position = Vector3((float)(pos.x+size), (float)(pos.y-size), 0);

    totalPts += 4;

    if (FAILED(pGeom->Insert(1, chunk.numVerts, &chunk, verticies, 0)))
    {
        KSR::Logf(LL_LOWEST, "ERROR: %s:%d Insert Geometry Failed!", __FILE__, __LINE__);
        return E_FAIL;
    }

    return S_OK;
}

/*!
    \brief Draw an arrowhead
    NOTE: if the length is negative the position reers to the tip of the arrow, otherwise it refers to the base
 */
HRESULT AddKSRArrowHead(KSR::PGEOMETRY pGeom, const keays::types::VectorD3 pos, int &totalPts,
                         const DWORD colour, const int effectID, const double directionRad,
                         const double length, const double width,
                         bool closed, const bool solid,
                         const int dpBias)
{
    if (!pGeom)
        return E_FAIL;

    // calc the position of the end points
    double bearingL, bearingR;
    keays::types::VectorD3 l, r, end, basePt;
    KSR::Chunk chunk;

    chunk.idTexture0    = -1;
    chunk.idTexture1    = -1;
    chunk.numVerts        = 3;
    chunk.depthBias        = dpBias;
    chunk.startIndex    = totalPts;
    chunk.type            = (solid ? CT_TRIANGLESTRIP : CT_LINESTRIP);

    VERTEX_DIFFUSE_TEX1 verticies[4];

    bearingL = directionRad + KM_PI_ON2;
    bearingR = directionRad - KM_PI_ON2;

    if (length < 0.0f)
    {
        end = pos;
        basePt = keays::math::GenPolarPosRad(pos, length, directionRad + KM_PI);
    } else
    {
        basePt = pos;
        end = keays::math::GenPolarPosRad(pos, length, directionRad);
    }

    double halfWidth = width/2.0;
    l = keays::math::GenPolarPosRad(basePt, halfWidth, bearingL, 0);
    r = keays::math::GenPolarPosRad(basePt, halfWidth, bearingR, 0);

    for (int nm = 0; nm < 4; ++nm)
    {
        verticies[nm].color = colour;
        verticies[nm].uv = Vector2(0, 0);
    }

    verticies[0].position = VD3toV3(r);
    verticies[1].position = VD3toV3(end);
    verticies[2].position = VD3toV3(l);
    if (closed  && !solid)
    {
        verticies[3].position = VD3toV3(r);
        chunk.numVerts = 4;
    }
    totalPts += chunk.numVerts;

    if (FAILED(pGeom->Insert(1, chunk.numVerts, &chunk, verticies, 0)))
    {
        KSR::Logf(LL_LOWEST, "ERROR: %s:%d Insert Geometry Failed!", __FILE__, __LINE__);
        return E_FAIL;
    }

    return S_OK;
}

HRESULT AddKSRHalfArrowHead(KSR::PGEOMETRY pGeom, keays::types::VectorD3 pos, int &totalPts,
                             const DWORD colour, int effectID, const double directionRad,
                             const keays::math::eSideSelections side, const double length,
                             const double width, const bool closed, bool solid, const int dpBias)
{
    if (!pGeom)
        return E_FAIL;

    // calc the position of the end points
    double bearingL, bearingR;
    keays::types::VectorD3 corner, end, basePt;
    KSR::Chunk chunk;

    chunk.idTexture0    = -1;
    chunk.idTexture1    = -1;
    chunk.numVerts        = 3;
    chunk.depthBias        = dpBias;
    chunk.startIndex    = totalPts;
    chunk.type            = (solid ? CT_TRIANGLESTRIP : CT_LINESTRIP);

    VERTEX_DIFFUSE_TEX1 verticies[4];

    bearingL = directionRad + KM_PI_ON2;
    bearingR = directionRad - KM_PI_ON2;

    if (length < 0.0f)
    {
        end = pos;
        basePt = keays::math::GenPolarPosRad(pos, length, directionRad + KM_PI);
    } else
    {
        basePt = pos;
        end = keays::math::GenPolarPosRad(pos, length, directionRad);
    }

    if (side == keays::math::SIDE_RIGHT)
        corner = keays::math::GenPolarPosRad(basePt, width, bearingR, basePt.z);
    else
        corner = keays::math::GenPolarPosRad(basePt, width, bearingL, basePt.z);

    for (int nm = 0; nm < 4; ++nm)
    {
        verticies[nm].color = colour;
        verticies[nm].uv = Vector2(0, 0);
    }

    // if on the right AND solid we need to ensure a CCW winding
        verticies[0].position = VD3toV3(basePt);
    if ((side == keays::math::SIDE_RIGHT) && solid)
    {
        verticies[1].position = VD3toV3(corner);
        verticies[2].position = VD3toV3(end);
    } else
    {
        verticies[1].position = VD3toV3(end);
        verticies[2].position = VD3toV3(corner);
        if (closed)
        {
            verticies[3].position = VD3toV3(basePt);
            chunk.numVerts = 4;
        }
    }

    totalPts += chunk.numVerts;

    if (FAILED(pGeom->Insert(1, chunk.numVerts, &chunk, verticies, 0)))
    {
        KSR::Logf(LL_LOWEST, "ERROR: %s:%d Insert Geometry Failed!", __FILE__, __LINE__);
        return E_FAIL;
    }

    return S_OK;
}

HRESULT AddKSRFallIndicator(KSR::PGEOMETRY pGeom, const keays::types::VectorD3 center,
                             int &totalPts, const DWORD colour, const int effectID,
                             const double bearing, const double radius, const double length,
                             const double width, const int dpBias /*= 0*/)
{
    if (!pGeom)
        return E_FAIL;

    double bearingL, bearingR;
    keays::types::VectorD3 lIn, lOut, rIn, rOut, end, peak;
    KSR::Chunk chunk;

    chunk.idTexture0    = -1;
    chunk.idTexture1    = -1;
    chunk.numVerts        = 8;
    chunk.depthBias        = dpBias;
    chunk.startIndex    = totalPts;
    chunk.type            = CT_LINELIST;
    chunk.effect        = effectID;

    if (center == keays::types::BAD_POINT)
        return E_FAIL;

    VERTEX_DIFFUSE_TEX1 *verticies = new VERTEX_DIFFUSE_TEX1[8];

    // calc angles
    bearingL = bearing + KM_PI_ON2;
    bearingR = bearing - KM_PI_ON2;

    // calc points
    end  = keays::math::GenPolarPosRad(center, radius, bearing);
    peak = keays::math::GenPolarPosRad(center, length, bearing);

    float halfWidth = float(width/2.0);
    lIn  = keays::math::GenPolarPosRad(center, halfWidth, bearingL, 0);
    lOut = keays::math::GenPolarPosRad(center, radius,    bearingL, 0);
    rIn  = keays::math::GenPolarPosRad(center, halfWidth, bearingR, 0);
    rOut = keays::math::GenPolarPosRad(center, radius,    bearingR, 0);

    // init vertices
    for (int nm = 0; nm < 8; ++nm)
    {
        verticies[nm].color = colour;
        verticies[nm].uv = Vector2(0, 0);
    }

    verticies[0].position = VD3toV3(lOut);
    verticies[1].position = VD3toV3(rOut);
    verticies[2].position = VD3toV3(lIn);
    verticies[3].position = VD3toV3(peak);
    verticies[4].position = VD3toV3(peak);
    verticies[5].position = VD3toV3(end);
    verticies[6].position = VD3toV3(peak);
    verticies[7].position = VD3toV3(rIn);

    totalPts += chunk.numVerts;

    if (FAILED(pGeom->Insert(1, chunk.numVerts, &chunk, verticies, 0)))
    {
        KSR::Logf(LL_LOWEST, "ERROR: %s:%d Insert Geometry Failed!", __FILE__, __LINE__);
        delete[] verticies;
        return E_FAIL;
    }

    delete[] verticies;
    return S_OK;
}

HRESULT AddKSRLine(KSR::PGEOMETRY pGeom, const keays::types::VectorD3 p1,
                    const keays::types::VectorD3 p2, int &totalPts, const DWORD colour,
                    const int effectID, const int lineStyle, TexValues *texVals /*= NULL*/,
                    const int dpBias /*= 0*/)
{
    if (!pGeom)
        return E_FAIL;

    KSR::Chunk chunk;
    float xTex, xLen;
    float dist;

//*
    if (!texVals)
    {
        xTex = (lineStyle < 0 ? 0.0f : 1.0f);
        xLen = 0;
    } else
    {
        dist = (float)keays::math::Dist2D(p1.XY(), p2.XY());
        xLen = dist / TEX_TO_WIDTH_RATIO;
        xLen = 5;
        xTex = texVals->lastTexVal;
    }
/*
    if (!texVals)
    {
        xTex     = (lineStyle < 0 ? 0 : 1);
    } else
    {
        dist     = float(keays::math::Dist2D(p1, p2));
        lRatio = texVals->viewWidth / dist;
        xTex     = TEX_TO_WIDTH_RATIO * lRatio;
        xTex    += texVals->lastTexVal;
    }
//*/

    chunk.idTexture0    = lineStyle;
    chunk.idTexture1    = -1;
    chunk.numVerts        = 2;
    chunk.depthBias        = dpBias;
    chunk.startIndex    = totalPts;
    chunk.type            = CT_LINELIST;
    chunk.effect        = effectID;

    VERTEX_DIFFUSE_TEX1 *verticies = new VERTEX_DIFFUSE_TEX1[2];

    verticies[0].position = VD3toV3(p1);
    verticies[0].color     = colour;
    verticies[1].uv         = Vector2(xTex, 0);

    verticies[1].position = VD3toV3(p2);
    verticies[1].color     = colour;
    verticies[1].uv         = Vector2(xTex + xLen, 0);

    totalPts += 2;

    if (FAILED(pGeom->Insert(1, chunk.numVerts, &chunk, verticies, 0)))
    {
        KSR::Logf(LL_LOWEST, "ERROR: %s:%d Insert Geometry Failed!", __FILE__, __LINE__);
        delete[] verticies;
        return E_FAIL;
    }

    delete[] verticies;
    return S_OK;
}

HRESULT AddKSRPolyLine(KSR::PGEOMETRY pGeom,
                        const std::vector<keays::types::VectorD3> &pts, int &totalPts,
                        const DWORD colour, const int effectID, const int lineStyle,
                        TexValues *texVals /*= NULL*/, const int dpBias /*= 0*/)
{
    if (!pGeom)
        return E_FAIL;

    KSR::Chunk chunk;
    int numPts = (int)pts.size();

    if (numPts < 2)
        return E_FAIL;

    chunk.idTexture0    = lineStyle;
    chunk.idTexture1    = -1;
    chunk.numVerts        = numPts;
    chunk.depthBias        = dpBias;
    chunk.startIndex    = totalPts;
    chunk.type            = CT_LINESTRIP;
    chunk.effect        = effectID;

    float dist, xLen, xTex;

    VERTEX_DIFFUSE_TEX1 *vertices = new VERTEX_DIFFUSE_TEX1[pts.size()];

    if (!texVals)
    {
        xTex = (lineStyle < 0 ? 0.0f : 1.0f);
        xLen = 0;
    } else
    {
        xTex = texVals->lastTexVal;
    }
    vertices[0].position        = VD3toV3(pts[0]);
    vertices[0].uv                = Vector2(0,0);
    vertices[0].color            = colour;
    for (int i = 1; i < numPts; ++i)
    {
        if (texVals)
        {
            dist = (float)keays::math::Dist2D(pts[i-1], pts[i]);
            xLen = dist / TEX_TO_WIDTH_RATIO;
        }
        vertices[i].position        = VD3toV3(pts[i]);
        vertices[i].uv                = Vector2(xTex += xLen, 0);
        vertices[i].color            = colour;
    }

    totalPts += numPts;

    if (FAILED(pGeom->Insert(1, chunk.numVerts, &chunk, vertices, 0)))
    {
        KSR::Logf(LL_LOWEST, "ERROR: %s:%d Insert Geometry Failed!", __FILE__, __LINE__);
        delete[] vertices;
        return E_FAIL;
    }

    delete[] vertices;
    return S_OK;
}

HRESULT AddKSRCircle(KSR::PGEOMETRY pGeom, const keays::types::VectorD3 cPos,
                      const double radius, const double intervalRad, int &totalPts,
                      const DWORD colour, const int effectID,
                      const keays::math::eAngleDirections dir, const int lineStyle,
                      TexValues *texVals /*= NULL*/, const int dpBias /*= 0*/)
{
    float xTex, xLen;
    float dist;
    float lRatio;
    KSR::Chunk chunk;
    vector<keays::types::VectorD3> pts;
    VERTEX_DIFFUSE_TEX1 *verticies = NULL;

    chunk.idTexture0    = lineStyle;
    chunk.idTexture1    = -1;
    chunk.numVerts        = 4;
    chunk.depthBias        = dpBias;
    chunk.startIndex    = totalPts;
    chunk.type            = CT_LINESTRIP;
    chunk.effect        = effectID;

//*
    if (!texVals)
    {
        xTex = (lineStyle < 0 ? 0.0f : 1.0f);
        xLen = 0;
    } else
    {
        dist        = float(radius * intervalRad);
        lRatio    = TEX_TO_WIDTH_RATIO;// * texVals->viewWidth;
        xLen        = dist / lRatio;
        xTex        = texVals->lastTexVal;
    }
/*/
    xTex = 1;
    xLen = 0;
//*/
    if (keays::math::GenCircle(cPos, radius, pts, intervalRad, dir))
    {
        verticies = new VERTEX_DIFFUSE_TEX1[pts.size()];

        //scene
        for (unsigned int i = 0; i < pts.size(); ++i)
        {
            verticies[i].color        = colour;
            verticies[i].position    = VD3toV3(pts[i]);
            verticies[i].uv            = Vector2(xTex += xLen, 0);
        }
        totalPts += (int)pts.size();

        chunk.numVerts = (unsigned int)pts.size();

        if (FAILED(pGeom->Insert(1, chunk.numVerts, &chunk, verticies, 0)))
        {
            KSR::Logf(LL_LOWEST, "ERROR: %s:%d Insert Geometry Failed!", __FILE__, __LINE__);
            delete[] verticies;
            return E_FAIL;
        }

        delete[] verticies;

        return S_OK;
    }

    return E_FAIL;
}

HRESULT AddKSRArc(KSR::PGEOMETRY pGeom, const keays::types::VectorD3 cPos,
                   const double radius, const double startAngleRad, const double endAngleRad,
                   const keays::math::eAngleDirections dir, const double intervalRad, const bool doEndPoint,
                   int &totalPts, const DWORD colour, const int effectID, const int lineStyle,
                   TexValues *texVals /*= NULL*/, const int dpBias /*= 0*/)
{
    if (!pGeom)
        return E_FAIL;

    KSR::Chunk chunk;
    vector<keays::types::VectorD3> pts;
    VERTEX_DIFFUSE_TEX1 *verticies = NULL;

    chunk.idTexture0    = lineStyle;
    chunk.idTexture1    = -1;
    chunk.numVerts        = 4;
    chunk.depthBias        = dpBias;
    chunk.startIndex    = totalPts;
    chunk.type            = CT_LINESTRIP;
    chunk.effect        = effectID;

    float xLen, xTex, dist;
    if (!texVals)
    {
        xTex = (lineStyle < 0 ? 0.0f : 1.0f);
        xLen = 0;
    } else
    {
        dist = float(radius * intervalRad);
        xLen = dist / TEX_TO_WIDTH_RATIO;
        xTex = texVals->lastTexVal;
    }

    if (keays::math::GenArcPointsRad(cPos, radius, startAngleRad, endAngleRad, pts, intervalRad, true, doEndPoint, dir))
    {
        verticies = new VERTEX_DIFFUSE_TEX1[pts.size()];

        //scene
        for (unsigned int i = 0; i < pts.size(); ++i)
        {
            verticies[i].color        = colour;
            verticies[i].position    = VD3toV3(pts[i]);
            verticies[i].uv            = (lineStyle < 0 ? Vector2(0, 0) : Vector2(xTex += xLen, 0));
        }
        totalPts += (int)pts.size();

        chunk.numVerts = (unsigned int)pts.size();

        if (FAILED(pGeom->Insert(1, chunk.numVerts, &chunk, verticies, 0)))
        {
            KSR::Logf(LL_LOWEST, "ERROR: %s:%d Insert Geometry Failed!", __FILE__, __LINE__);
            delete[] verticies;
            return E_FAIL;
        }

        delete[] verticies;

        return S_OK;
    }

    return E_FAIL;
}

const Vector2 GetWorldCoords(const POINT &pt, const PVIEWPORT vp)
{
    Vector2 size = GetWorldSize(pt, vp);

    Vector3 cameraTarget = vp->GetCameraPosition();

    ViewportSettings vps;
    vp->GetSettings(&vps);

    float halfWidth = vps.viewWidth * 0.5f;
    float halfHeight = vps.viewHeight * 0.5f;

    return Vector2(cameraTarget.x - halfWidth + size.x, cameraTarget.y - halfHeight + size.y);
}

const Vector2 GetWorldSize(const POINT &pt, const PVIEWPORT vp)
{
    float xRatio = 0.0f,
          yRatio = 0.0f,
          x, y;
    POINT center;
    KSR::ViewportSettings vps;
    Vector3 cameraTarget;

    vp->GetSettings(&vps);
    center.x = vps.width/2;
    center.y = vps.height/2;

    // this is the % of distance from center to each edge
    if (center.x)
        xRatio = float(pt.x) / float(vps.width);
    if (center.y)
        yRatio = float(pt.y) / float(vps.height);

    // world co-ords
    x = xRatio * vps.viewWidth;
    y = -yRatio * vps.viewHeight;

    return Vector2(x, y);
}

const POINT GetScreenCoords(const keays::types::VectorD3 &pt, const PVIEWPORT vp,
                             const RECT &rect)
{
    float xRatio, yRatio;
    POINT scPt, center;
    KSR::ViewportSettings vps;
    Vector3 cameraTarget;

    center.x = (rect.right - rect.left)/2;
    center.y = (rect.bottom - rect.top)/2;

    // view and camera settings
    vp->GetSettings(&vps);
    cameraTarget = vp->GetCameraPosition();

    // this is the % of distance from center to each edge
    xRatio = (float(pt.x) - cameraTarget.x)/(vps.viewWidth / 2);
    yRatio = (float(pt.y) - cameraTarget.y)/(vps.viewHeight / 2);

    // world co-ords
    scPt.x = long(xRatio * ((rect.right - rect.left)/2) + center.x);
    scPt.y = long(-yRatio * ((rect.bottom - rect.top)/2) + center.y);

    return scPt;
}
//*
const Vector2 GetWorldCoords(const POINT &pt, const keays::math::RectD &vpRect, const RECT &rect)
{
    float xRatio, yRatio, x, y, viewWidth, viewHeight;
    POINT center;
    Vector3 cameraTarget;

    center.x = (rect.right - rect.left)/2;
    center.y = (rect.bottom - rect.top)/2;

    // this is the % of distance from center to each edge
    xRatio = float(pt.x - center.x)/float(center.x);
    yRatio = float(pt.y - center.y)/float(center.y);

    // view and camera settings
    viewWidth = (float)vpRect.GetWidth();
    viewHeight = (float)vpRect.GetHeight();
    cameraTarget = Vector3(float((viewWidth / 2) + vpRect.GetBottom()),
                            float((viewHeight / 2) + vpRect.GetLeft()), 0);

    // world co-ords
    x = xRatio * (viewWidth / 2) + cameraTarget.x;
    y = -yRatio * (viewHeight / 2) + cameraTarget.y;

    return Vector2(x, y);
}

const POINT GetScreenCoords(const keays::types::VectorD3 &pt, const keays::math::RectD &vpRect,
                             const RECT &rect)
{
    float xRatio, yRatio, viewWidth, viewHeight;
    POINT scPt, center;
    Vector3 cameraTarget;

    center.x = (rect.right - rect.left)/2;
    center.y = (rect.bottom - rect.top)/2;

    // view and camera settings
    viewWidth = float(vpRect.GetWidth());
    viewHeight = float(vpRect.GetHeight());
    cameraTarget = Vector3(float((viewWidth / 2) + vpRect.GetBottom()),
                            float((viewHeight / 2) + vpRect.GetLeft()), 0);

    // this is the % of distance from center to each edge
    xRatio = (float(pt.x) - cameraTarget.x)/(viewWidth / 2);
    yRatio = (float(pt.y) - cameraTarget.y)/(viewHeight / 2);

    // world co-ords
    scPt.x = long(xRatio * ((rect.right - rect.left)/2) + center.x);
    scPt.y = long(-yRatio * ((rect.bottom - rect.top)/2) + center.y);

    return scPt;
}
//*/


#ifdef _DEBUG
HRESULT DebugAxisGrid(KSR::PGEOMETRY pGeom, int &totalPts)
{
    KSR::Chunk chunk;
    VERTEX_DIFFUSE_TEX1 vertices[90];

    chunk.idTexture0    = -1;
    chunk.idTexture1    = -1;
    chunk.numVerts        = 90;
    chunk.depthBias        = 0;
    chunk.startIndex    = totalPts;
    chunk.type            = CT_LINELIST;
    chunk.effect        = -1;

    float dists[] = { -1000, -500, -100, -50, -10, -5, -1, 0, 1, 5, 10, 50, 100, 500, 1000 };

    int idx = 0;
    for (int i = 0; i < 15; i++)
    {
        // xy plane
        vertices[idx].color        = (i <= 6 ? 0xffff0000 : 0xffff2f2f);
        vertices[idx].position    = Vector3(dists[i], 1000, 0);
        vertices[idx].uv        = Vector2(0, 0);
        // xz plane
        vertices[idx+30].color        = (i <= 6 ? 0xff00ff00 : 0xff2fff2f);
        vertices[idx+30].position    = Vector3(dists[i], 0, 1000);
        vertices[idx+30].uv            = Vector2(0, 0);
        // yz plane
        vertices[idx+60].color        = (i <= 6 ? 0xff0000ff : 0xff2f2fff);
        vertices[idx+60].position    = Vector3(0, dists[i], 1000);
        vertices[idx+60].uv            = Vector2(0, 0);
        idx++;
        // xy plane
        vertices[idx].color        = (i <= 6 ? 0xffff0000 : 0xffff2f2f);
        vertices[idx].position    = Vector3(dists[i], -1000, 0);
        vertices[idx].uv        = Vector2(0, 0);
        // xz plane
        vertices[idx+30].color        = (i <= 6 ? 0xff00ff00 : 0xff2fff2f);
        vertices[idx+30].position    = Vector3(dists[i], 0, -1000);
        vertices[idx+30].uv            = Vector2(0, 0);
        // yz plane
        vertices[idx+60].color        = (i <= 6 ? 0xff0000ff : 0xff2f2fff);
        vertices[idx+60].position    = Vector3(0, dists[i], -1000);
        vertices[idx+60].uv            = Vector2(0, 0);
        idx++;
    }

    for (int j = 0; j < 90; j++)
        KSR::Logf(LL_LOWEST, "pt[%2d/%2d] = 0x%8.8X, (%7.3f, %7.3f, %7.3f)",
                   j, 90, vertices[j].color, vertices[j].position.x, vertices[j].position.y, vertices[j].position.z);

    totalPts += 90;

    if (FAILED(pGeom->Insert(1, chunk.numVerts, &chunk, vertices, 0)))
    {
        KSR::Logf(LL_LOWEST, "ERROR: %s:%d Insert Geometry Failed!", __FILE__, __LINE__);
        return E_FAIL;
    }

    return S_OK;
}
#endif
}
}
// -- EOF
