// disable annoying STL name warning
#pragma warning (disable: 4786)

#include "..\include\triangle.h"
#include <assert.h>
#include <stdio.h>

// STL Includes
#include <vector>    //std::vector
#include <set>        //std::set

#include <keays_types.h>    // keays::types
#include <keays_math.h>        // keays::math
#ifdef _DEBUG
//#include <string>
#include <cstdio>
#include <cstdarg>
//#include <varargs.h> // this is needed for non Unix V compatibility
#endif

#include <LeakWatcher.h>

#ifdef _DO_MEMORY_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace kt = keays::types;
namespace km = keays::math;

#pragma warning(disable : 4786) // ignore the long name warning associated with stl stuff

//---------------------------------------------------------------------------
namespace keays
{
namespace triangle
{
//#region Debug Logging Functions
#ifdef _DEBUG
FILE **g_pLogFile = NULL;
void KEAYS_TRIANGLE_API AttachLogFile(FILE **pLogFile)
{
    if (pLogFile && *pLogFile)
    {
        FPrintf(*pLogFile, "--- Attaching keays::triangle to log file ---\n");
        fflush(*pLogFile);
    }
    g_pLogFile = pLogFile;
}

void KEAYS_TRIANGLE_API DetachLogFile()
{
    if (g_pLogFile && *g_pLogFile)
    {
        FPrintf(*g_pLogFile, "--- Detaching keays::triangle from log file ---\n");
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
void KEAYS_TRIANGLE_API AttachLogFile(FILE **pLogFile) {}
void KEAYS_TRIANGLE_API DetachLogFile() {}
void WriteDebugLog(LPCTSTR fmt, ...) {}
#endif
//#endregion


//#region -- TriLayerInfo --
TriLayerInfo::TriLayerInfo()
{
    m_Layer = 0;
    m_Size = 0;
    m_pStartIndex = new std::vector<int>;
    m_pLength = new std::vector<int>;
}

const TriLayerInfo & TriLayerInfo::operator = (const TriLayerInfo & rhs)
{
    m_Layer = rhs.m_Layer;
    m_Size = rhs.m_Size;

    if (!m_pStartIndex)
        m_pStartIndex = new std::vector<int>;

    if (!m_pLength)
        m_pLength = new std::vector<int>;

    m_pStartIndex->clear();
    m_pStartIndex->assign(rhs.m_pStartIndex->begin(), rhs.m_pStartIndex->end());

    m_pLength->clear();
    m_pLength->assign(rhs.m_pLength->begin(), rhs.m_pLength->end());

    return *this;
}

TriLayerInfo::TriLayerInfo(const TriLayerInfo & rhs)
{
    m_Layer = rhs.m_Layer;
    m_Size = rhs.m_Size;
    m_pStartIndex = new std::vector<int>;
    (*m_pStartIndex) = (*rhs.m_pStartIndex);
    m_pLength = new std::vector<int>;
    (*m_pLength) = (*rhs.m_pLength);
}

TriLayerInfo::~TriLayerInfo()
{
    m_pStartIndex->clear();
    delete m_pStartIndex;
    m_pLength->clear();
    delete m_pLength;
}
//#endregion

//#region -- CutSectionNode --
CutSectionNode::CutSectionNode()
{
    x = y = z    = 0.0;
    dist            = 0.0;
    triangleID    = -1;
    layerNum        = 0;
    triFlags        = eUT_TF_NONE;
}

CutSectionNode::CutSectionNode(const CutSectionNode &orig)
{
    x                = orig.x;
    y                = orig.y;
    z                = orig.z;
    dist            = orig.dist;
    triangleID    = orig.triangleID;
    layerNum        = orig.layerNum;
    triFlags        = orig.triFlags;
}

const CutSectionNode &CutSectionNode::operator=(const CutSectionNode &rhs)
{
    x                = rhs.x;
    y                = rhs.y;
    z                = rhs.z;
    dist            = rhs.dist;
    triangleID    = rhs.triangleID;
    layerNum        = rhs.layerNum;
    triFlags        = rhs.triFlags;

    return *this;
}

const CutSectionNode &CutSectionNode::operator=(const UTPoint &pt)
{
    x    = pt.x;
    y    = pt.y;
    z    = pt.z;
    return *this;
}
//#endregion

//#region -- CutSectionList --
/*
bool CutSectionList::CalcBatterPoint(const double &startHeight, const double &maxWidth,
                                      const double &cutGrade, const double &fillGrade,
                                      VectorD3 *pResult) const
{
    //#region
    using namespace std;
    using namespace keays::math;

    if (!pResult)
        return false;

    double height;
    double width = 0.0;
    double lastWidth = 0.0;
    double lastHeight = 0.0;
    double fGrade = fillGrade;
    double cGrade = cutGrade;

    bool fill = false;

    height = startHeight;

    if (size() < 2)
        return false;

//    if ((fillGrade == 0.0) || (cutGrade == 0.0)) // cannot have 0.0 grades
//        return false;

    const CutSectionNode &startNode = (*begin());
    if (height > startNode.z)
        fill = false; // we are in fill

    // need to ensure that the fill grade is -ve, and the cut grade is +ve
    if (fillGrade > 0.0)
        fGrade = -fGrade;
    if (cutGrade < 0.0)
        cGrade = -cGrade;

    const_iterator itr;
    const_iterator nextItr;

    itr = begin();
    nextItr = itr;
    nextItr++;
    for (; nextItr != end(); itr++, nextItr++)
    {
        const CutSectionNode &node = (*itr);
        const CutSectionNode &nextNode = (*nextItr);

        double endHeight;
        double dist = (nextNode.dist - node.dist);

        // calc the height at the distance given the direction
        if (fill)
        {
            endHeight = (dist * fGrade) + height;
        } else
        {
            endHeight = (dist * cGrade) + height;
        }
        if ((fill && (endHeight < nextNode.z)) ||
            (!fill && (endHeight > nextNode.z)))
        {
            // its the end point
            // calc the crossing point
            Line rotLine(VectorD2(0, node.z), VectorD2(dist, nextNode.z));
            VectorD2 rPoint1 = RotatePointXY(rotLine.start.XY(), rotLine.end.XY(), VectorD2(0, height));
            VectorD2 rPoint2 = RotatePointXY(rotLine.start.XY(), rotLine.end.XY(), VectorD2(dist, endHeight));
            double intersectDist = 0.0;

            XfromYPt(rPoint1, rPoint2, node.z, &intersectDist, 0.00001);

            VectorD2 intPt(intersectDist, node.z);
            VectorD2 urPoint = UnRotatePointXY(rotLine.start.XY(), rotLine.end.XY(), intPt);
            VectorD3 endPoint = GenPolarPosRad(node.XY(), urPoint.x, Direction(node, nextNode)).VD3(urPoint.y);
            (*pResult) = endPoint;
            return true;
        }
        height = endHeight;
    }

    return false;
    //#endregion
}
/*/
bool KEAYS_TRIANGLE_API
CalcBatterPoint(const CutSectionList &csList, const double &startHeight, const double &maxWidth,
                const double &cutGrade, const double &fillGrade, VectorD3 *pResult, VectorD2 *pProfile /*= NULL*/)
{
    //#region
    using namespace std;
    using namespace keays::math;

    if (!pResult)
        return false;

    double height;
    double width = 0.0;
    double lastWidth = 0.0;
    double lastHeight = 0.0;
    double fGrade = fillGrade;
    double cGrade = cutGrade;

    bool fill = false;

    height = startHeight;

    if (csList.size() < 2)
        return false;

//    if ((fillGrade == 0.0) || (cutGrade == 0.0)) // cannot have 0.0 grades
//        return false;

    const CutSectionNode &startNode = (*(csList.begin()));
    CutSectionList::const_iterator lastItr = csList.end();
    lastItr--;
    const CutSectionNode &endNode = (*(lastItr));

    if (height > startNode.z)
        fill = true; // we are in fill

    // need to ensure that the fill grade is -ve, and the cut grade is +ve
    if (fillGrade > 0.0)
    {
        fGrade = -fGrade;
    }
    if (cutGrade < 0.0)
    {
        cGrade = -cGrade;
    }
    if (fill)
    {
        if (pProfile)
            (*pProfile) = VectorD2(maxWidth, maxWidth*fGrade);
        (*pResult) = VectorD3(endNode.x, endNode.y, startHeight+maxWidth*fGrade);
    } else
    {
        if (pProfile)
            (*pProfile) = VectorD2(maxWidth, maxWidth*cGrade);
        (*pResult) = VectorD3(endNode.x, endNode.y, startHeight+maxWidth*cGrade);
    }


    CutSectionList::const_iterator itr;
    CutSectionList::const_iterator nextItr;

    itr = csList.begin();
    nextItr = itr;
    nextItr++;
    for (; nextItr != csList.end(); itr++, nextItr++)
    {
        const CutSectionNode &node = (*itr);
        const CutSectionNode &nextNode = (*nextItr);

        double endHeight;
        double dist = (nextNode.dist - node.dist);

        if (node.dist > maxWidth)
        {
            return true;
        }

        // calc the height at the distance given the direction
        if (fill)
        {
            endHeight = (dist * fGrade) + height;
        } else
        {
            endHeight = (dist * cGrade) + height;
        }
        if ((fill && (endHeight < nextNode.z)) ||
            (!fill && (endHeight > nextNode.z)))
        {
            // TODO: determine if this would be better as vector or trig maths
            //*
            Line nodeSeg(VectorD2(node.dist, node.z), VectorD2(nextNode.dist, nextNode.z));
            Line battSeg(VectorD2(node.dist, height), VectorD2(nextNode.dist, endHeight));
            VectorD2 result;
            LineSegLineSegIntersect(nodeSeg, battSeg, result);
            if (pProfile)
//                (*pProfile) = result;//VectorD2(Dist2D((*csList.begin()), endPoint), endPoint.z);
                (*pProfile) = VectorD2(result.x, result.y-startHeight);
            double endDist = result.x - node.dist;
            Line endLine((VectorD3)node, (VectorD3)nextNode);
            (*pResult) = GenPolarPosRad(((VectorD3)node).XY(), endDist, endLine.GetBearing()).VD3(result.y);
            return true;
            /*/
            // its the end point
            // calc the crossing point
            Line rotLine(VectorD2(0, node.z), VectorD2(dist, nextNode.z));
            VectorD2 rPoint1 = RotatePointXY(rotLine.start.XY(), rotLine.end.XY(), VectorD2(0, height));
            VectorD2 rPoint2 = RotatePointXY(rotLine.start.XY(), rotLine.end.XY(), VectorD2(dist, endHeight));
            double intersectDist = 0.0;

            XfromYPt(rPoint1, rPoint2, node.z, &intersectDist, 0.00001);

            VectorD2 intPt(intersectDist, node.z);
            VectorD2 urPoint = UnRotatePointXY(rotLine.start.XY(), rotLine.end.XY(), intPt);
            VectorD3 endPoint = GenPolarPosRad(node.XY(), urPoint.x, Direction(node, nextNode)).VD3(urPoint.y + height);
            (*pResult) = endPoint;
            if (pProfile)
                (*pProfile) = VectorD2(Dist2D((*csList.begin()), endPoint), endPoint.z);
            return true;
            //*/
        }
        height = endHeight;
    }

    return true;
    //#endregion
}
//*/
//#endregion

//#region -- CBatterFLags --
CBatterFlags::CBatterFlags(const keays::math::eSideSelections side /*= keays::math::SIDE_LEFT*/,
                            const double &maxWidth /*= 0.0*/, const double &cutGrade /*= 1.0*/,
                            const double &fillGrade /*= -1.0*/, const bool drape /*= true*/,
                            const bool close /*= true*/, const bool capStart /*= true*/,
                            const bool capEnd /*= true*/ )
{
    m_side = side;
    m_maxWidth = (maxWidth < 0.0 ? 0.0 : maxWidth);
    m_cutGrade = (cutGrade < 0.0 ? -cutGrade : cutGrade);
    m_fillGrade = (fillGrade > 0.0 ? -fillGrade : fillGrade);
    m_drape = drape;
    m_capStart = capStart;
    m_capEnd = capEnd;
    m_close = close;
}

CBatterFlags::CBatterFlags(const CBatterFlags &orig)
{
    *this = orig;
}

const CBatterFlags &CBatterFlags::operator=(const CBatterFlags &rhs)
{
    Side(rhs.Side());
    MaxWidth(rhs.MaxWidth());
    CutGrade(rhs.CutGrade());
    FillGrade(rhs.FillGrade());
    Drape(rhs.Drape());
    CapStart(rhs.CapStart());
    CapEnd(rhs.CapEnd());
    Close(rhs.Close());

    return *this;
}

const keays::math::eSideSelections &CBatterFlags::Side(keays::math::eSideSelections side)
{
    if ((side == keays::math::SIDE_LEFT) || (side == keays::math::SIDE_RIGHT))
        m_side = side;
    return m_side;
}

const double &CBatterFlags::MaxWidth(const double &maxWidth)
{
    return m_maxWidth = (maxWidth < 0.0 ? 0.0 : maxWidth);
}

const double &CBatterFlags::CutGrade(    const double &cutGrade)
{
    return m_cutGrade = (cutGrade < 0.0 ? -cutGrade : cutGrade);
}

const double &CBatterFlags::FillGrade(const double &fillGrade)
{
    return m_fillGrade = (fillGrade > 0.0 ? -fillGrade : fillGrade);
}

const bool CBatterFlags::Drape(bool drape)
{
    return m_drape = drape;
}

void CBatterFlags::Cap(bool cap)
{
    m_capStart = cap;
    m_capEnd = cap;
}

const bool CBatterFlags::CapStart(bool cap)
{
    return m_capStart = cap;
}

const bool CBatterFlags::CapEnd(bool cap)
{
    return m_capEnd = cap;
}

const bool CBatterFlags::Close(bool close)
{
    return m_close = close;
}
//#endregion

//-----------------------------------------------------------------------------
//#region -- Triangles class --
Triangles::Triangles()
{
    m_seedTriangleIndex = -1; // set this to an invalid value
    m_pTriangles = NULL;
    m_pPoints = NULL;
    m_pVertexNormals = NULL;
    m_extents.MakeInvalid();
}

Triangles::~Triangles()
{
    m_seedTriangleIndex = -1; // set this to an invalid value
    if (m_pTriangles)
    {
        delete [] m_pTriangles;
        m_pTriangles = NULL;
    }
    if (m_pPoints)
    {
        delete [] m_pPoints;
        m_pPoints = NULL;
    }
    if (m_pVertexNormals)
    {
        delete [] m_pVertexNormals;
        m_pVertexNormals = NULL;
    }

    m_extents.MakeInvalid();
}

const UTPoint Triangles::CalculateAverageNormal(unsigned int triIndex, UTPoint * pThePoint,
                                                 UTPoint *pFaceNormals,
                                                 std::set<int> * pVisitedTris)
{
    UTPoint p1, p2, p3, *p = NULL;
    UTPoint retVal;
    int linkIndex;
    int tflags;

    assert(triIndex >= 0);
    assert(triIndex < m_NumberTriangles);

    pVisitedTris->insert(triIndex);

    p1 = m_pPoints[ m_pTriangles[triIndex].vertices[0] ];
    p2 = m_pPoints[ m_pTriangles[triIndex].vertices[1] ];
    p3 = m_pPoints[ m_pTriangles[triIndex].vertices[2] ];

    if (*pThePoint == p1) p = &p1;
    if (*pThePoint == p2) p = &p2;
    if (*pThePoint == p3) p = &p3;

    if (NULL == p)
        return retVal;

    retVal = pFaceNormals[triIndex];

    for (int i = 0; i < 3; i++)
    {
        linkIndex = m_pTriangles[triIndex].links[i];
        tflags      = m_pTriangles[triIndex].tflags;

        // some sanity checking
        if (pVisitedTris->find(linkIndex) == pVisitedTris->end() // haven't alread visisted here
             && linkIndex >= 0    // its a sane link value
             && (tflags & eUT_TF_ACTIVE))    // its a visible triangle
        {
            retVal = retVal + CalculateAverageNormal(linkIndex, p, pFaceNormals, pVisitedTris);
        }
    }

    return retVal;
}

void Triangles::CalculateVertexNormals(pFnProgressUpdate pfnProgressUpdate /*= NULL*/,
                                        int numUpdates /*= 20*/, void *pProgressPayload /* = NULL*/)
{
    UTPoint* pFaceNormals = new UTPoint[m_NumberTriangles];
    UTPoint fNormal;
    unsigned int triIndex = 0;
    int pointIndex;
    std::set<int> visited;
    std::set<int> pointsDone;

    if (m_pVertexNormals)
        delete [] m_pVertexNormals;
    m_pVertexNormals = new UTPoint[m_NumberPoints];

    int progressDivisor = 0;
    float percentage = 0.0f;

    if (pfnProgressUpdate)
    {
        progressDivisor = (m_NumberTriangles / numUpdates);
        if (progressDivisor == 0)
            progressDivisor = 1;

        pfnProgressUpdate(0.0f, "Calculating Face Normals", pProgressPayload);
    }

    // Calculate face normals
    for (triIndex = 0; triIndex < m_NumberTriangles; triIndex++)
    {
        UTPoint normal(0, 0, 1);
        UTPoint c;
        UTPoint p1, p2, p3;

        p1 = m_pPoints[ m_pTriangles[triIndex].vertices[0] ];
        p2 = m_pPoints[ m_pTriangles[triIndex].vertices[1] ];
        p3 = m_pPoints[ m_pTriangles[triIndex].vertices[2] ];

        kt::VectorD3 u(p1 - p2);//p1.x-p2.x, p1.y-p2.y, p1.z-p2.z);
        kt::VectorD3 v(p1 - p3);//p1.x-p3.x, p1.y-p3.y, p1.z-p3.z);

        c = u.Cross(v);//km::Cross(u, v);
        normal = c.GetNormalised();//km::NormaliseD(c);

        pFaceNormals[triIndex] = normal;

        if (pfnProgressUpdate && (triIndex % progressDivisor == 0))
        {
            percentage = triIndex * 1.0f * 0.1f/ m_NumberTriangles;
//            _snprintf(progressText, 255, "Calculating Face Normals");        // removed due to performance issues
            pfnProgressUpdate(percentage, "Calculating Face Normals", pProgressPayload);    // progressText);
        }
    }

    // calculate the vertex normals
    for (triIndex = 0; triIndex < m_NumberTriangles; triIndex++)
    {
        for (int n = 0; n < 3; n++)
        {
            pointIndex =  m_pTriangles[triIndex].vertices[n];

            if (pointsDone.find(pointIndex) == pointsDone.end())
            {
                fNormal = CalculateAverageNormal(triIndex, &(m_pPoints[pointIndex]), pFaceNormals, &visited);
                m_pVertexNormals[pointIndex] = fNormal.GetNormalised();//km::NormaliseD(fNormal);
                visited.clear();
                pointsDone.insert(pointIndex);
            }
        }

        if (pfnProgressUpdate && (triIndex % progressDivisor == 0))
        {
            percentage = 0.1f + (triIndex * 1.0f * 0.9f/ m_NumberTriangles);
//            _snprintf(progressText, 255, "Calculating Vertex Normals");    // removed due to performance issues
            pfnProgressUpdate(percentage, "Calculating Vertex Normals", pProgressPayload);    // progressText);
        }
    }

    if (pfnProgressUpdate)
    {
        pfnProgressUpdate(1.0f, "Finished Calculating Vertex Normals", pProgressPayload);
    }

    delete [] pFaceNormals;
}

const keays::math::Cube &Triangles::CalcExtents()
{
    // recalc, so we invalidate the cube and start again
    m_extents.MakeInvalid();
    m_visExtents.MakeInvalid();

    if (NULL == m_pPoints || 0 == m_NumberPoints)
    {
        return m_extents;
    }

    /*
    for (unsigned int i = 0; i < m_NumberPoints; i++)
        m_extents.IncludePoint(m_pPoints[i]);
    /*/
    for (unsigned int triIdx = 0; triIdx < m_NumberTriangles; triIdx++)
    {
        m_extents.IncludePoint(m_pPoints[ m_pTriangles[triIdx].vertices[0] ]);
        m_extents.IncludePoint(m_pPoints[ m_pTriangles[triIdx].vertices[1] ]);
        m_extents.IncludePoint(m_pPoints[ m_pTriangles[triIdx].vertices[2] ]);

        if (m_pTriangles[triIdx].IsActive())
        {
            m_visExtents.IncludePoint(m_pPoints[ m_pTriangles[triIdx].vertices[0] ]);
            m_visExtents.IncludePoint(m_pPoints[ m_pTriangles[triIdx].vertices[1] ]);
            m_visExtents.IncludePoint(m_pPoints[ m_pTriangles[triIdx].vertices[2] ]);
        }
    }
    //*/
    return m_extents;
}

void Triangles::Translate(double x, double y, double z)
{
    // this also recals the extents of the model
    m_extents.MakeInvalid();
    if (NULL == m_pPoints || 0 == m_NumberPoints)
        return;

    for (unsigned int i = 0; i < m_NumberPoints; i++)
    {
        m_pPoints[i].x -= x;
        m_pPoints[i].y -= y;
        m_pPoints[i].z -= z;
        m_extents.IncludePoint(m_pPoints[i]);
    }
}

/*
 * AreaP() returns the plan area of the parallelogram defined by the vectors
 * AB and BC.  If the parallelogram is defined anti-clockwise then the area
 * will be positive, otherwise the area will be negative.
double Triangles::AreaParallelogram(UTPoint *pA, UTPoint *pB, UTPoint *pC)
{
    return (pB->x - pA->x) * (pC->y - pA->y) - (pC->x - pA->x) * (pB->y - pA->y);
}
 */

bool Triangles::Locate(const VectorD2 &pt, int &seedTriangleIndex, bool allowInactive /*= false*/, const int startSeed /*= -1*/) const
{
    UTPoint p(pt);
    return Locate(&p, seedTriangleIndex, allowInactive, startSeed);
}

bool Triangles::Locate(const double &x, const double &y, int &seedTriangleIndex, bool allowInactive /*= false*/, const int startSeed /*= -1*/) const
{
    UTPoint p(x, y);
    return Locate(&p, seedTriangleIndex, allowInactive, startSeed);
}

//#define Point        UTPoint
//#define Triangle    UTTriangle
#define TriangleID    int

static double
AreaP(const UTPoint *ap, const UTPoint *bp, const UTPoint *cp)
{
    return (bp->x - ap->x) * (cp->y - ap->y) - (cp->x - ap->x) * (bp->y - ap->y);
}

bool Triangles::Locate(const UTPoint *pPoint, int &seedTriangleIndex, bool allowInactive /*= false*/, const int startSeed /*= -1*/) const
{
    UTTriangle    *triangles    = m_pTriangles;
    UTPoint        *points        = m_pPoints;

    UTTriangle    *tp;
    UTPoint        *sp;
    int i = 0;
    TriangleID t;
    if ((startSeed >= 0) && ((unsigned int)startSeed < m_NumberTriangles))
        t = startSeed;
    else
        t = m_seedTriangleIndex; // the internal one

    tp = &triangles[t];

    double a;

doagain:
    sp = &points[tp->vertices[i]];
    // scan anti-clockwise
    while (Float::Less(AreaP(pPoint, &points[tp->vertices[(i+2) % 3]], sp), 0.0, 0.00001))
    {
        i = (i+1) % 3;
        t = tp->links[i];
        if (t < 0)
        {
            seedTriangleIndex = -1;
            return false;
        }
        i = (tp->back[i]+1) % 3;
        tp = &triangles[t];
    }
    // scan clockwise
    while (Float::Less(AreaP(pPoint, sp, &points[tp->vertices[(i+1) % 3]]), 0.0, 0.00001))
    {
        i = (i+2) % 3;
        t = tp->links[i];
        if (t < 0)
        {
            seedTriangleIndex = -1;
            return false;
        }
        i = (tp->back[i]+2) % 3;
        tp = &triangles[t];
    }
    while (Float::Less(AreaP(pPoint, &points[tp->vertices[(i+1) % 3]], &points[tp->vertices[(i+2) % 3]]), 0.0, 0.00001))
    {
        t = tp->links[i];
        if (t < 0)
        {
            seedTriangleIndex = -1;
            return false;
        }
        i = tp->back[i];
        tp = &triangles[t];
        a = AreaP(pPoint, &points[tp->vertices[i]], sp);
        if (Float::EqualTo(a, 0.0, 0.00001))
            goto doagain; // AAAAUGH! A GOTO! KILL IT! KILL IT! ;)
        if (Float::Greater(a, 0.0, 0.00001))
        {
            // point is left of line
            i = (i+1) % 3;
        } else
        {
            // point is right of line
            i = (i+2) % 3;
        }
    }

    seedTriangleIndex = m_seedTriangleIndex = t;
    // check visibility
    if (!allowInactive)
    {
        if (!((triangles[m_seedTriangleIndex]).tflags & eUT_TF_ACTIVE))
        {
            return false;
        }
    }

    return true;
}

void Triangles::SetNumberTriangles(long numTris)
{
    delete [] m_pTriangles;

    m_seedTriangleIndex = 0;
    m_pTriangles = new UTTriangle[numTris];
    memset(m_pTriangles, 0, sizeof(UTTriangle) * numTris);
    m_NumberTriangles = numTris;
}

void Triangles::SetNumberPoints(long numPoints)
{
    delete [] m_pPoints;

    m_pPoints = new UTPoint[numPoints];
    memset(m_pPoints, 0, sizeof(UTPoint) * numPoints);
    m_NumberPoints = numPoints;
}

void Triangles::SetTriangles(UTTriangle * pT, long numTris)
{    // why does this not duplicate the memory
    delete [] m_pTriangles;

    m_seedTriangleIndex = 0;
    m_pTriangles = pT;
    m_NumberTriangles = numTris;
}

void Triangles::SetPoints(UTPoint *pPt, long numPoints)
{    // why does this not duplicate the memory
    delete [] m_pPoints;
    m_pPoints = pPt;
    m_NumberPoints = numPoints;
}

const UTPoint *Triangles::GetPoint(unsigned long pointNo) const
{
    assert(pointNo < m_NumberPoints);
    return &(m_pPoints[pointNo]);
}

const UTTriangle *Triangles::GetTriangle(unsigned long triNo) const
{
    assert(triNo < m_NumberTriangles);
    return &(m_pTriangles[triNo]);
}

const UTPoint Triangles::CalcFaceNormal(const unsigned long triangleID) const
{
    if (!m_pPoints || !m_pTriangles || (m_NumberPoints < 1) || (m_NumberTriangles < 1))
        return keays::types::BAD_POINT;

    if ((m_pTriangles[triangleID].vertices[0] >= (int)m_NumberPoints) ||
        (m_pTriangles[triangleID].vertices[1] >= (int)m_NumberPoints) ||
        (m_pTriangles[triangleID].vertices[2] >= (int)m_NumberPoints))
        return keays::types::BAD_POINT;

    return CalcFaceNormalNC(triangleID);
}

const double Triangles::Grade(const unsigned long triangleID) const
{
    if (!m_pPoints || !m_pTriangles || (m_NumberPoints < 1) || (m_NumberTriangles < 1))
        return keays::types::INVALID_ANGLE;

    if ((m_pTriangles[triangleID].vertices[0] >= (int)m_NumberPoints) ||
        (m_pTriangles[triangleID].vertices[1] >= (int)m_NumberPoints) ||
        (m_pTriangles[triangleID].vertices[2] >= (int)m_NumberPoints))
        return keays::types::INVALID_ANGLE;

    return GradeNC(triangleID);
}

const UTPoint Triangles::GeometricCentroid(const unsigned long triangleID) const
{
    if (!m_pPoints || !m_pTriangles || (m_NumberPoints < 1) || (m_NumberTriangles < 1))
        return keays::types::BAD_POINT;

    if ((m_pTriangles[triangleID].vertices[0] >= (int)m_NumberPoints) ||
        (m_pTriangles[triangleID].vertices[1] >= (int)m_NumberPoints) ||
        (m_pTriangles[triangleID].vertices[2] >= (int)m_NumberPoints))
        return keays::types::BAD_POINT;

    return GeometricCentroidNC(triangleID);
}

bool Triangles::HeightAtPoint(const keays::types::VectorD2 &pt, double *pHeight, int *pTriIndex /*= NULL*/, bool allowInactive /*= false*/) const
{
    int seed = -1;
    double result = 0.0;

    if (!pHeight)
        return false;

    if (m_pTriangles && m_pPoints)
    {
        if (Locate(pt, seed, allowInactive) && (seed > 0))
        {
            if (pTriIndex)
                *pTriIndex = seed;

            if (allowInactive && !(m_pTriangles[seed].tflags & eUT_TF_ACTIVE))
            {
                *pHeight = m_extents.GetBase();
                return true;
            }

            const keays::types::VectorD3 &a = m_pPoints[m_pTriangles[seed].vertices[0]];
            const keays::types::VectorD3 &b = m_pPoints[m_pTriangles[seed].vertices[1]];
            const keays::types::VectorD3 &c = m_pPoints[m_pTriangles[seed].vertices[2]];

            if (keays::math::PointHeightOnPlaneTri(a, b, c, pt, result))
            {
                *pHeight = result;
                return true;
            }

            return false;

        } else
        {
            return false;
        }
    } else
    {
        return false;
    }
}

const CutSectionList *Triangles::Section(const UTPoint &pt0, const UTPoint &pt1,
                                          CutSectionList *pCutList, double *pStartDistance,
                                          bool genEndPoint /*= true*/, bool breaklinesOnly /*= false*/,
                                          int *pNumCutRet /*= NULL*/, int *pMaxCut /*= NULL*/) const
{
    using namespace keays::types;
    using namespace keays::math;
    if (!pCutList || !pStartDistance)
        return NULL;

    CutSectionNode cutNode;

    WriteDebugLog(_T("TRIANGLE: %5d: Generate Section from [%.3f, %.3f] to [%.3f, %.3f]\n"), __LINE__, pt0.x, pt0.y, pt1.x, pt1.y);

    // first step is trim our line off at the extents, this should be guaranteed to be contained in the bounding triangle
    VectorD3 start, end, lineEnd;
    int startEdge, endEdge;
    int result = LineSegCrossesTriangle(m_pPoints[0], m_pPoints[1], m_pPoints[2], Line(pt0, pt1),
                                 &start, &lineEnd, &startEdge, &endEdge);
    if (result == E_FAILURE)
    {
        WriteDebugLog(_T("TRIANGLE: %5d: Line does not cross bounding triangle\n\n"), __LINE__);
        return NULL;
    }
    WriteDebugLog(_T("TRIANGLE: %5d: Generate Section from [%.3f, %.3f] to [%.3f, %.3f]\n"), __LINE__, start.x, start.y, lineEnd.x, lineEnd.y);

    end = lineEnd;

    double height;
    int triIndex = 0;
    size_t triCount = 0;
    double distance = 0.0;
    double finalDist = Dist2D(start, end);

    bool lastPoint = false;

#ifdef _DEBUG
    std::vector<int> visited;
    UTPoint dbgStart = start, dbgEnd = end;
#endif

    // locate the start
    if (!HeightAtPoint(start.XY(), &height, &triIndex, true))
    {
        WriteDebugLog(_T("TRIANGLE: %5d: Triangles::Section(...): Could not locate start point\n\n"), __LINE__);
        return NULL;
    }

    // add the start point
    cutNode.x = start.x;
    cutNode.y = start.y;
    cutNode.z = height;
    cutNode.dist = distance + (*pStartDistance);
    cutNode.layerNum = m_pTriangles[triIndex].layer;
    cutNode.triangleID = triIndex;
    cutNode.triFlags = m_pTriangles[triIndex].tflags;

    pCutList->push_back(cutNode);
    WriteDebugLog(_T("\t\tStart Position = [%.3f, %.3f, %.3f];\n"
                     "\t\t\ttriIdx = %d [%.3f, %.3f, %.3f], [%.3f, %.3f, %.3f], [%.3f, %.3f, %.3f];\n"),
                     start.x, start.y, height,
                     triIndex, m_pPoints[m_pTriangles[triIndex].vertices[0]],
                     m_pPoints[m_pTriangles[triIndex].vertices[1]], m_pPoints[m_pTriangles[triIndex].vertices[2]]);

    while (Float::Less(distance, finalDist))
    {
        if (triCount++ > GetNumberTriangles())
            break;
        // locate its intersect with its triangle, and calc the distance
        const UTPoint &a = m_pPoints[m_pTriangles[triIndex].vertices[0]];
        const UTPoint &b = m_pPoints[m_pTriangles[triIndex].vertices[1]];
        const UTPoint &c = m_pPoints[m_pTriangles[triIndex].vertices[2]];

        WriteDebugLog(_T("\t\tLine [%.3f, %.3f]->[%.3f, %.3f] crosses triangle\n"
                         "\t\t\ttri %d -> [%.3f, %.3f, %.3f], [%.3f, %.3f, %.3f], [%.3f, %.3f, %.3f]\n"),
                         start.x, start.y, end.x, end.y,
                         triIndex, a.x, a.y, a.z, b.x, b.y, b.z, c.x, c.y, c.z);

        int result = LineSegCrossesTriangle(a, b, c, Line(start, end), &start, &end, &startEdge, &endEdge);
        if (result == E_FAILURE)
        {
            WriteDebugLog(_T("\tFAILED, returning NULL\n"));
            return NULL;
        }
        WriteDebugLog(_T("TRIANGLE: %5d: Result = %s, start = [%.3f, %.3f]; end = [%.3f, %.3f]\n"),
                      __LINE__, LineTriReturnString(result), start.x, start.y, end.x, end.y);

        double tmpDist = Dist2D(start, end);
        distance += tmpDist;

#ifdef _DEBUG
        if (Float::EqualTo(tmpDist, 0.0) && Float::Less(distance, finalDist))
        {
            WriteDebugLog(_T("TRIANGLE: %5d: Dist between start and end is 0\n"), __LINE__);
            WriteDebugLog(_T("\t\t\tSegment from [%.3f, %.3f] to [%.3f, %.3f]\n\n"), dbgStart.x, dbgStart.y, dbgEnd.x, dbgEnd.y);
        }
#endif
        lastPoint = Float::GreaterOrEqual(distance, finalDist);

#ifdef _DEBUG
        if (lastPoint)
        {
            WriteDebugLog(_T("TRIANGLE: %5d: Doing Last Point %.3f >= %.3f\n\n"), __LINE__, distance, finalDist);
        }
#endif

        if ((result == S_LINE_TOUCHES) && (distance < finalDist))
        {
            // if it is only a touch, and we haven't finished
            std::set<int> visited;
            do
            {
                end = lineEnd;
                visited.insert(triIndex);
                if (endEdge == -1)
                    return NULL;
                triIndex = m_pTriangles[triIndex].links[endEdge];
                const UTPoint &a2 = m_pPoints[m_pTriangles[triIndex].vertices[0]];
                const UTPoint &b2 = m_pPoints[m_pTriangles[triIndex].vertices[1]];
                const UTPoint &c2 = m_pPoints[m_pTriangles[triIndex].vertices[2]];

                result = LineSegCrossesTriangle(a2, b2, c2, Line(start, end), &start, &end, &startEdge, &endEdge);
                if ((result != S_LINE_TOUCHES) && (result != E_FAILURE))
                {
                    distance += Dist2D(start, end);
                    break;
                }
                if (triCount++ > GetNumberTriangles())
                    break;
            } while (triIndex != -1);//(result != S_LINE_CROSSES) && (result != S_LINE_ENTERS));
        }

        if (!lastPoint || genEndPoint)
        {
            if (!breaklinesOnly ||
                (m_pTriangles[triIndex].eflags[endEdge] & (eUT_EF_BREAKLINE | eUT_EF_BOUNDARY | eUT_EF_INTERNAL | eUT_EF_XBREAKLINE)))
            cutNode.x = end.x;
            cutNode.y = end.y;
            cutNode.z = end.z;
            cutNode.dist = distance + (*pStartDistance);
            cutNode.layerNum = m_pTriangles[triIndex].layer;
            cutNode.triangleID = triIndex;
            cutNode.triFlags = m_pTriangles[triIndex].tflags;

            pCutList->push_back(cutNode);
        }

        if ((result == S_LINE_CONTAINED) || (result == S_LINE_ENTERS))
            break;
        if (endEdge == -1)
            return NULL;
        triIndex = m_pTriangles[triIndex].links[endEdge];

        start = end;
        end = lineEnd;
    }

    // do not forget to update the total dist
    (*pStartDistance) = finalDist + (*pStartDistance);

    WriteDebugLog(_T("TRIANGLE: %5d: Leaving Section...\n\n"), __LINE__);

    return pCutList;
}

const CutSectionList *Triangles::Section(const std::vector< UTPoint > &pts,  CutSectionList *pCutList,
                                          bool breaklinesOnly /*= false*/,
                                          int *pNumCutRet /*= NULL*/, int *pMaxCut /*= NULL*/) const
{
    using namespace keays::types;
    using namespace keays::math;

    if (pts.size() < 2)
    {
        WriteDebugLog(_T("TRIANGLE: %5d: Too few points to perform section\n\n"), __LINE__);
        return NULL;
    }

    if (!pCutList)
    {
        WriteDebugLog(_T("TRIANGLE: %5d: NULL pointer passed for output section\n\n"), __LINE__);
        return NULL;
    }

    CutSectionNode cutNode;
    pCutList->clear();

    int count = 0;
    std::vector< UTPoint >::const_iterator i = pts.begin();
    std::vector< UTPoint >::const_iterator j = i;
    j++;
    double totalDistance = 0.0;

    WriteDebugLog(_T("\n\nTRIANGLE: %d: Generate polyline section %d points:\n"), __LINE__, pts.size());
    for (; j != pts.end(); i++, j++)
    {
        const UTPoint &pt0 = (*i);
        const UTPoint &pt1 = (*j);

        WriteDebugLog(_T("\t\tGen Section for segment [%.3f, %.3f] to [%.3f, %.3f]\n"), pt0.x, pt0.y, pt1.x, pt1.y);
        if (!Section(pt0, pt1, pCutList, &totalDistance, false, breaklinesOnly, pNumCutRet, pMaxCut))
        {
            WriteDebugLog(_T("TRIANGLE: %5d: Section[%d](pt, pt, ...) returned NULL\n\n"), __LINE__, count);
            return NULL;
        }
        count++;
    }
    WriteDebugLog(_T("TRIANGLE: %5d: Generate polyline section %d points - all except end\n"), __LINE__, pts.size());

    // add end point
    // i should be the 2nd last position, so add it
    // locate the start
    double height;
    int triIndex = 0;
    const UTPoint &endPt = (*i);
    if (!HeightAtPoint(endPt.XY(), &height, &triIndex, true))
    {
        WriteDebugLog(_T("TRIANGLE: %5d: Triangles::Section(...): Could not locate start point [%.3f, %.3f]\n\n"), __LINE__, endPt.x, endPt.y);
        return NULL;
    }

    // add the end point
    cutNode.x = endPt.x;
    cutNode.y = endPt.y;
    cutNode.z = height;
    cutNode.dist = 0.0;
    cutNode.layerNum = m_pTriangles[triIndex].layer;
    cutNode.triangleID = triIndex;
    cutNode.triFlags = m_pTriangles[triIndex].tflags;

    pCutList->push_back(cutNode);

    WriteDebugLog(_T("TRIANGLE: %5d: Generate polyline section %d points - COMPLETE\n"), __LINE__, pts.size());
    return count > 0 ? pCutList : NULL;
}

/*
    S_OK = 0,            //!< Batter was successfully generated.
    E_TOO_FEW_POINTS,    //!< The source polyline does not have enough points.
    E_INVALID_DISTANCE,    //!< The max distance is too small.
    E_GEN_PERPS_FAILED,    //!< Generating the perpendicular vectors failed.
    E_PERP_INEQUALITY,    //!< There is an inequality between the number of perpendicular vectors and source points.
    E_PERP_SECTION,        //!< The process failed when generating a perpendicular section
    E_CALC_BATTER,        //!< The process failed when calculating the batter line
    E_FAILED,            //!< Batter generation failed.
 */
/*!
    \brief Generate a batter string on the side of the polyline indicated
 */
int Triangles::GenerateBatterString(const keays::types::Polyline3D &polyline,
                                     keays::types::Polyline3D *pResult,
                                     const CBatterFlags &flags) const
{
    using namespace keays::types;
    using namespace keays::math;

    assert(pResult != NULL);
    if (polyline.size() < 2)
        return E_TOO_FEW_POINTS;
    if (flags.MaxWidth() <= 0.0)
        return E_INVALID_DISTANCE;

    // calculate the perpendicular vectors
    std::vector<VectorD3> perps;
    if (!GeneratePerpendicularVectors(polyline, &perps, true, flags.Close()))
    {
        WriteDebugLog(_T("TRIANGLE: %5d: GeneratePerpendicularVectors(...) FAILED\n"), __LINE__, E_GEN_PERPS_FAILED);
        return E_GEN_PERPS_FAILED;
    }

    if (perps.size() != polyline.size())
    {
        WriteDebugLog(_T("TRIANGLE: %5d: (%d) perpdicular vector count inequality FAILED\n"), __LINE__, E_PERP_INEQUALITY);
        return E_PERP_INEQUALITY;
    }

    // iterate through the perps, generate sections and batter
    double start;
    CutSectionList csList;

    Polyline3D batter;
    Polyline3D::const_iterator ptItr, perpsItr;
    int ptCount = 0;
    for (ptItr = polyline.begin(), perpsItr = perps.begin(); (ptItr != polyline.end() && perpsItr != perps.end()); ptItr++, perpsItr++, ptCount++)
    {
        // generate sections
        const VectorD3 &pt = (*ptItr);
        const VectorD3 &vec = (*perpsItr);
        const VectorD3 &sVec = vec * flags.MaxWidth() * (flags.Side() == SIDE_LEFT ? -1 : 1);
        const VectorD3 exPt(pt + sVec);

        csList.clear();
        start = 0;
        VectorD3 bPoint;

        if (!Section(pt, exPt, &csList, &start, true, NULL, NULL))
        {
            WriteDebugLog(_T("TRIANGLE: %5d: (%d) Perpendicular Section(...) FAILED. [E_PERP_SECTION]\n"), __LINE__, ptCount);
            return E_PERP_SECTION;
        }

        if (!CalcBatterPoint(csList, pt.z, flags.MaxWidth(), flags.CutGrade(), flags.FillGrade(), &bPoint, NULL))
        {
            WriteDebugLog(_T("TRIANGLE: %5d: (%d) CalcBatterPoint(...) FAILED. [E_CALC_BATTER]\n"), __LINE__, ptCount);
            return E_CALC_BATTER;
        }
        batter.push_back(bPoint);
    }

    //pResult->clear();   <--- We do not clear this, it is the resposibility of the calling function
    if (flags.CapStart())
        pResult->push_back(*polyline.begin());

    if (flags.Drape())
    {
        csList.clear();
        Polyline3D resultBatter;
        if (Section(batter, &csList, NULL, NULL))
        {
            CutSectionList::const_iterator cItr;
            for (cItr = csList.begin(); cItr != csList.end(); cItr++)
            {
                const CutSectionNode &node = (*cItr);
                pResult->push_back((VectorD3)node);
            }
        } else
        {
            Polyline3D::const_iterator itr;
            for (itr = batter.begin(); itr != batter.end(); itr++)
            {
                pResult->push_back(*itr);
            }
        }
    } else
    {
        Polyline3D::const_iterator itr;
        for (itr = batter.begin(); itr != batter.end(); itr++)
        {
            pResult->push_back(*itr);
        }
    }

    if (flags.CapEnd())
        pResult->push_back(*polyline.rbegin());

    return S_SUCCESS;
}
//#endregion

};
};
