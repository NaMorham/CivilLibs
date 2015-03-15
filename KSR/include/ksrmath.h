/*! \file */
/*-----------------------------------------------------------------------
    ksrmath.h

    Description: Math routines
    Author:         Julian McKinlay
    ---------------------------------------------------------------------*/

#pragma once

// Math initialisation
void SetCollisionTolerance(float tolerance);
void SetCollisionPrecision(float precision);

//! \namespace Math
namespace Math
{
const float TWO_PI = D3DX_PI * 2.0f;
const float PI = D3DX_PI;
const float PI_ON_2 = D3DX_PI * 0.5f;
const float PI_ON_4 = D3DX_PI * 0.25f;
const float PI_ON_8 = D3DX_PI * 0.125f;
const float PI_ON_3 = D3DX_PI / 3.0f;
const float PI_ON_6 = D3DX_PI / 6.0f;

extern const Vector3 I;
extern const Vector3 J;
extern const Vector3 K;
extern const Vector3 I_NEG;
extern const Vector3 J_NEG;
extern const Vector3 K_NEG;

enum AXISPLANES
{
    AP_XY = 0,
    AP_XZ,
    AP_YZ,
};

enum INTERSECTIONSTATE
{
    IS_NONE                    = 0, // No intersection is found.
    IS_CONTACT                = 1, // The volumes do not intersect.
    IS_PENETRATION            = 3, // The volume of one partially intersects the volume of another. IS_PENETRATION is a form of contact and so has IS_CONTACT set.
    IS_A_CONTAINS_B            = 7, // Object A entirely encloses Object B. IS_A_CONTAINS_B is a form of contact and penetration and so has IS_CONTACT and IS_PENETRATION set.
    IS_RECT_CONTAINS_TRI    = 7,
    IS_AABB_CONTAINS_TRI    = 7,
    IS_B_CONTAINS_A            = 11, // Object B entirely encloses Object A. IS_B_CONTAINS_A is a form of contact and penetration and so has IS_CONTACT and IS_PENETRATION set.
    IS_TRI_CONTAINS_RECT    = 11,
    IS_IDENTICAL            = 19, // The volumes of objects A and B are perfectly. IS_IDENTICAL is a form of contact and penetration and so has IS_CONTACT and IS_PENETRATION set.
};

class Rectangle2D;
class Triangle2D;
class Triangle;

// Geometry structures
struct Sphere
{
    Sphere();
    Sphere(const Vector3 &sphereCenter, float sphereRadius);

    Vector3 center;
    float radius;
};

struct AABB
{
    AABB();
    AABB(const Vector3 &boxMin, const Vector3 &boxMax);

    float GetDeltaX() const { return max.x - min.x; }
    float GetDeltaY() const { return max.y - min.y; }
    float GetDeltaZ() const { return max.z - min.z; }
    const Vector3 GetCentre() const { return min + (max - min) * 0.5f; }

    void IncludePoint(const Vector3 &point);
    void MakeInvalid();

    bool PointInside(const Vector3 &point);

    bool IsValid() const { return min.x <= max.x && min.y <= max.y && min.z <= max.z; }

    Vector3 min, max;
};

struct AABBPlanes
{
    struct AABBPoly
    {
    private:
        friend struct AABBPlanes;

        AABBPoly();

    public:
        void Set(const Vector3 &a, const Vector3 &b, const Vector3 &c, const Vector3 &d);

        void Get(Vector3 *pA, Vector3 *pB, Vector3 *pC, Vector3 *pD);
        const Vector3 *Get() const { return m_verts; }
        Vector3 *Get() { return m_verts; }

        const Vector3 &operator[](const unsigned int index) const { return m_verts[index]; }
        Vector3 &operator[](const unsigned int index) { return m_verts[index]; }

        const Vector3 Vert(const unsigned int index) const { return m_verts[index]; }
        Vector3 Vert(const unsigned int index) { return m_verts[index]; }

        Vector3 m_verts[4];
    };

    AABBPlanes(const AABB &src);

    const AABB &m_AABB;
    D3DXPLANE m_planes[6];
    AABBPoly m_polygons[6];
};

struct TriangleIntersectionData
{
    Vector2 centroid;
    Vector2 vertices[3];
};

struct LineSegmentIntersectionData
{
    Vector2 points[2];
};

struct RectangleIntersectionData
{
    Vector2 centroid;
    Vector2 vertices[4];
    Vector2 relativeVertices[4];
};

struct OBB
{
    OBB();
    OBB(const Vector3 &boxMin, const Vector3 &boxMax, const Vector3 &boxOrientation);

    Vector3 min, max;
    Vector3 orientation;
};

struct Polygon
{
    Polygon();
    Polygon(const Vector3 &n, Vector3 *pVerts, int nVerts);

    Vector3 normal;
    Vector3 *vertices;

    int numVerts;
};

// Colour conversion
inline const Vector4 ARGBToV4RGBA(const DWORD col)
{
    D3DXCOLOR c(col);

    return Vector4(c.r, c.g, c.b, c.a);
}

inline const DWORD V4RGBAtoARGB(const Vector4 &col)
{
    D3DXCOLOR c(col.x, col.y, col.z, col.w);

    return (DWORD)c;
}

// Pseudorandom math
//! \brief Calculates a pseudorandom float
/*! \param min Minimum value of result
    \param max Minimum value of result */
const float    Random(const float min, const float max);
//! \brief Calculates a pseudorandom vector
/*! \param min Minimum value of result
    \param max Minimum value of result */
const Vector3 Random(const Vector3 &min, const Vector3 &max);

// Sorting math
//! \brief Sorts minimum and maximum floats
/*! \param sourceMin minimum value to test
    \param sourceMax maximum value to test
    \param destMin   current minimum value
    \param destMax   current maximum value */
void SortMinMax(const float &sourceMin, const float &sourceMax, float &destMin, float &destMax);
//! \brief Sorts minimum and maximum vectors
/*! \param sourceMin minimum value to test
    \param sourceMax maximum value to test
    \param destMin   current minimum value
    \param destMax   current maximum value */
void SortMinMax(const Vector3 &sourceMin, const Vector3 &sourceMax, Vector3 &destMin, Vector3 &destMax);

// Floating-point math
DWORD FloatToDW(FLOAT f);
int Round(float f);
int Equals(float x, float y);
int Greater(float x, float y);
int GreaterEquals(float x, float y);
int Less(float x, float y);
int LessEquals(float x, float y);

// Matrix math
//! \brief Retrieves X Axis from a matrix
/*! \param m matrix containing axis */
Vector3 GetXAxis(Matrix &m);
//! \brief Retrieves Y Axis from a matrix
/*! \param m matrix containing axis */
Vector3 GetYAxis(Matrix &m);
//! \brief Retrieves Z Axis from a matrix
/*! \param m matrix containing axis */
Vector3 GetZAxis(Matrix &m);
//! \brief Retrieves Z Axis from a matrix
/*! \param m matrix containing axis */
void PerspectiveOffCenterFovLH(Matrix &out, float l, float r, float b, float t, float zn,
                                     float zf, float fov, float aspect);
// Vector math
//! \brief Calculates the cross product of two vectors
/*! \param a first vector
    \param b second vector */
Vector3 Cross(const Vector3 &a, const Vector3 &b);
Vector3 Cross(const Vector3 *pA, const Vector3 *pB);
//! \brief Calculates the normal of a 3D vector
/*! \param v vector to normalise */
Vector3 Normalise(const Vector3 &v);
Vector3 *Normalise(Vector3 *pOut, const Vector3 *pV);
Vector3 *Normalise(Vector3 *pV);
//! \brief Calculates the normal of a 2D vector
/*! \param v vector to normalise */
Vector2 Normalise(const Vector2 &v);
Vector2 *Normalise(Vector2 *pOut, const Vector2 *pV);
Vector2 *Normalise(Vector2 *pV);
//! \brief Projects a 3D Vector to 2D Screen Space
/*! \param v vector to calculate */
Vector2 Project(const Vector3 &v, const Matrix &view, const Matrix &projection, float width, float height, float zNear, float zFar);
//! \brief Unprojects a 2D Vector in Screen Space to 3D
/*! \param v vector to calculate */
Vector3 Unproject(const Vector2 &v, const Matrix &view, float width, float height, float fov, float aspect, float zNear, DWORD coordSystemMode);
//! \brief Calculates the magnitude of a 3D vector
/*! \param v vector to calculate */
float   Magnitude(const Vector3 &v);
float   Magnitude(const Vector3 *pV);
//! \brief Calculates the magnitude of a 2D vector
/*! \param v vector to calculate */
float   Magnitude(const Vector2 &v);
float   Magnitude(const Vector2 *pV);
//! \brief Calculates the length of a vector
/*! \param v vector to calculate */
float   Length(const Vector2 &v);
//! \brief Calculates the length of a vector
/*! \param v vector to calculate */
float   Length(const Vector3 &v);
//! \brief Calculates the length of a vector
/*! \param v vector to calculate */
float   Length(const Vector4 &v);
//! \brief Calculates the dot product of two 3D vectors
/*! \param a first vector
    \param b second vector */
float    Dot(const Vector3 &a, const Vector3 &b);
float    Dot(const Vector3 *pA, const Vector3 *pB);
//! \brief Calculates the dot product of two 2D vectors
/*! \param a first vector
    \param b second vector */
float    Dot(const Vector2 &a, const Vector2 &b);
float    Dot(const Vector2 *pA, const Vector2 *pB);

const Vector2 &TransformCoord(Vector2 *pOut, const Vector2 &in, const Matrix &transform);
const Vector3 &TransformCoord(Vector3 *pOut, const Vector3 &in, const Matrix &transform);

const Vector2 &TransformNormal(Vector2 *pOut, const Vector2 &in, const Matrix &transform);
const Vector3 &TransformNormal(Vector3 *pOut, const Vector3 &in, const Matrix &transform);

const Matrix &Invert(Matrix *pOut, const Matrix &transform);
const Matrix &Transpose(Matrix *pOut, const Matrix &transform);
const Matrix &InvertAndTranspose(Matrix *pOut, const Matrix &transform);

//! \brief Calculates the distance from a plane to the origin
/*! \param n normal of the plane
    \param p any point on the plane */
float   PlaneDistance(const Vector3 &n, const Vector3 &p);

//! \brief Finds if a point lies within a coplanarpolygon
/*! \param point Point to test
    \param vertices [in] Pointer to an array of vectors, representing the vertices of the polygon
    \param numVerts number of vertices in the polygon */
bool    PointInsidePolygon(const Vector3 &point, const Vector3 *pVerts, int numVerts);

//! \brief Calculates the nearest point of a line to a vector
/*! \param a first line vector
    \param b second line vector
    \param position point to test from */
Vector3 ClosestPointOnLine(const Vector3 &a, const Vector3 &b, const Vector3 &position);

// Quadratic Math
bool QuadraticFormula(float a, float b, float c, float *pT0, float *pT1);

// Collision Detection
bool SweepSphereStaticPolygon(const Sphere &sphere1, const Sphere &sphere2, const Polygon &polygon,
                              float *pTime, Vector3 *pPosition, Vector3 *pPoint);
bool SweepSphereSphere(const Sphere &sphereA1, const Sphere &sphereA2, const Sphere &sphereB1, const Sphere &sphereB2,
                       float *pTime, Vector3 *pPosition1, Vector3 *pPosition2, Vector3 *pPoint);
bool SweepAABBAABB(const AABB &AABBA1, const AABB &AABBA2, const AABB &AABBB1, const AABB &AABBB2,
                   float *pTime, Vector3 *pPosition1, Vector3 *pPosition2);
bool SweepOBBSphere(const OBB &OBB1, const OBB &OBB2, const Sphere &sphere1, const Sphere &sphere2);
bool SweepOBBStaticPolygon(const OBB &OBB1, const OBB &OBB2, const Polygon &polygon, float *pTime, Vector3 *pPoint);
bool SweepOBBOBB(const OBB &OBBA1, const OBB &OBBA2, const OBB &OBBB1, const OBB &OBBB2, float *pTime);

bool SphereSphereCollision(const Sphere &sphere1, const Sphere &sphere2);
bool SpherePlaneCollision(const Sphere &sphere, const Plane &plane, float *pDistance);
bool SpherePointCollision(const Sphere &sphere, const Vector3 &point);
bool SphereEdgeCollision(const Sphere &sphere, const Polygon &polygon);

bool PolygonSphereCollision(const Polygon &polygon, const Sphere &sphere, Vector3 *pPoint);
bool PolygonRayCollision(const Polygon &polygon, const Vector3 &position, const Vector3 &direction, Vector3 *pPoint);

bool PointFrustumCollision(const Vector3 &point, const Plane *frustumPlanes);

bool AABBSphereCollision(const AABB &box, const Sphere &sphere, Vector3 *pPoint);
bool AABBPolygonCollision(const AABB &box, const Polygon &polygon);
bool AABBRayCollision(const AABB &box, const Vector3 &position, const Vector3 &direction);
bool AABBAABBCollision(const AABB &box1, const AABB &box2);
bool AABBPointCollision(const AABB &box, const Vector3 &point);
bool AABBFrustumCollision(const AABB &box, const Plane *frustumPlanes);

/*inline bool AABBTriangleCollision(const AABB &box, const Vector3 *pVerts)
{
    return AABBTriangleCollision(AABBPlanes(box), pVerts);
}*/

bool OBBSphereCollision(const OBB &OBB, const Sphere &sphere, Vector3 *pPoint);
bool OBBPointCollision(const OBB &OBB, const Vector3 &point);
bool OBBRayCollision(const OBB &OBB, const Vector3 &min, const Vector3 &max, const Vector3 &position, const Vector3 &direction, Vector3 *pPoint);
bool OBBOBBCollision(const OBB &OBB1, const OBB &OBB2, Vector3 *pPoint);
bool OBBPolygonCollision(const OBB &OBB, const Polygon &polygon, Vector3 *pPoint);
bool OBBAABBCollision(const OBB &OBB, const AABB &box, Vector3 *pPoint);

// Intersection Testing
INTERSECTIONSTATE IntersectionOnAxisRectangleTriangle2D(const RectangleIntersectionData &riData, const TriangleIntersectionData &tiData,
                                                        const Vector2 *pTriangleEdgeNormal, Vector2 *pTriResults, Vector2 *pRectResults, bool contactTesting = false);
INTERSECTIONSTATE IntersectionOnAxisRectangleLineSegment2D(const RectangleIntersectionData &riData, const LineSegmentIntersectionData &tiData,
                                                           Vector2 *pLineSegmentResults, Vector2 *pRectResults, bool contactTesting = false);
INTERSECTIONSTATE RectangleTriangleIntersection2D(const Triangle2D &triangle, const RectangleIntersectionData &riData,
                                                  const TriangleIntersectionData &tiData, Vector2 *pTriResults,
                                                  Vector2 *pRectResults, bool contactTesting = false);

INTERSECTIONSTATE RectangleLineSegmentIntersection2D(const Triangle2D &triangle, const RectangleIntersectionData &riData,
                                                     const TriangleIntersectionData &tiData, Vector2 *pTriResults,
                                                     Vector2 *pRectResults, bool contactTesting = false);
INTERSECTIONSTATE AABBTriangleIntersection(const AABB &box, const Vector3 *pVerts,
                                           Vector2 *pAABBResults = NULL, Vector2 *pTriangleResults = NULL, bool contactTesting = false);
INTERSECTIONSTATE AABBLineSegmentIntersection(const AABB &box, const Vector3 &v1, const Vector3 &v2,
                                              Vector2 *pAABBResults = NULL, Vector2 *pLineSegmentResults = NULL, bool contactTesting = false);

bool SimpleRectangleTriangleIntersection2D(const Triangle2D &triangle, const RectangleIntersectionData &riData,
                                                        const TriangleIntersectionData &tiData);
bool SimpleIntersectionOnAxis(const RectangleIntersectionData &riData, const TriangleIntersectionData &tiData, const Vector2 *pTriangleEdgeNormal);
bool SimpleAABBTriangleIntersection(const AABB &box, const Vector3 *pVerts);
bool SimpleAABBTriangleIntersectionXY(const AABB &box, const Vector3 *pVerts);
bool SimpleAABBTriangleIntersectionXZ(const AABB &box, const Vector3 *pVerts);
bool SimpleAABBTriangleIntersectionYZ(const AABB &box, const Vector3 *pVerts);

bool AABBContainsTriangle(const AABB &box, const Vector3 *pVerts);

class Rectangle2D
{
public:
    Rectangle2D(float l, float r, float t, float b)
        :    left(l), right(r), top(t), bottom(b) {}

    Rectangle2D(const Rectangle2D &rect)
        :    left(rect.left), right(rect.right), top(rect.top), bottom(rect.bottom) {}

    Rectangle2D(const Math::AABB &box, AXISPLANES plane)
    {
        if (plane == AP_XY)
        {
            left = box.min.x;
            right = box.max.x;
            top = box.max.y;
            bottom = box.min.y;
        }
        else if (plane == AP_XZ)
        {
            left = box.min.x;
            right = box.max.x;
            top = box.max.z;
            bottom = box.min.z;
        }
        else
        {
            left = box.min.y;
            right = box.max.y;
            top = box.max.z;
            bottom = box.min.z;
        }
    }

    const float GetWidth() const { return right - left; }
    const float GetHeight() const { return top - bottom; }

    float left, right, top, bottom;
};

class Triangle2D
{
public:
    Triangle2D(const Vector2 &pt1, const Vector2 &pt2, const Vector2 &pt3)
    {
        ZeroMemory(m_pEdgeNormals, sizeof(Vector2 *) * 3);

        m_pts[0] = pt1;
        m_pts[1] = pt2;
        m_pts[2] = pt3;
    }

    Triangle2D(const Triangle2D &tri)
    {
        ZeroMemory(m_pEdgeNormals, sizeof(Vector2 *) * 3);

        m_pts[0] = *tri[0];
        m_pts[1] = *tri[1];
        m_pts[2] = *tri[2];
    }

    ~Triangle2D()
    {
        delete m_pEdgeNormals[0];
        delete m_pEdgeNormals[1];
        delete m_pEdgeNormals[2];
    }

    const Vector2 *operator[](const unsigned int index) const
    {
        return m_pts + index;
    }

    bool IsDegenerate() const
    {
        return CalculateArea() == 0.0f;
    }

    const float CalculateArea() const
    {
        Vector2 u = m_pts[1] - m_pts[0];
        Vector2 v = m_pts[2] - m_pts[1];

        return Magnitude(u) * Magnitude(v) * 0.5f;
    }

    const Vector2 *CalculateEdgeNormal(const unsigned int index) const
    {
        if (!m_pEdgeNormals[index])
        {
            Vector2 v(m_pts[(index + 1) % 3] - m_pts[index]);
            m_pEdgeNormals[index] = new Vector2(v.y, -v.x);
            Normalise(m_pEdgeNormals[index]);

            /*DebugPrintf("Calculating normal for (%.2f, %.2f), (%.2f, %.2f): (%.2f, %.2f)\n",
                        m_pts[(index + 1) % 3].x, m_pts[(index + 1) % 3].y, m_pts[index].x, m_pts[index].y,
                        m_pEdgeNormals[index]->x, m_pEdgeNormals[index]->y);*/
        }

        return m_pEdgeNormals[index];
    }

    void CalculateEdgeNormals() const
    {
        CalculateEdgeNormal(0);
        CalculateEdgeNormal(1);
        CalculateEdgeNormal(2);
    }

    const Vector2 *GetEdgeNormal(const unsigned int index) const
    {
        if (m_pEdgeNormals[index])
        {
            return m_pEdgeNormals[index];
        }

        return CalculateEdgeNormal(index);
    }

    const Vector2 *GetVerts() const { return m_pts; }

private:
    Vector2 m_pts[3];
    mutable Vector2 *m_pEdgeNormals[3]; // 1-2, 2-3, 3-1
};

class Triangle
{
public:
    Triangle(const Vector3 &pt1, const Vector3 &pt2, const Vector3 &pt3, bool calculateNormal = true)
        : m_p1(pt1), m_p2(pt2), m_p3(pt3), m_pRawNormal(NULL), m_pNormal(NULL)

    {
        if (calculateNormal)
        {
            CalculateNormal();
        }
    }

    Triangle(const Vector3 *pts, bool calculateNormal = true)
        : m_p1(pts[0]), m_p2(pts[1]), m_p3(pts[2]), m_pRawNormal(NULL), m_pNormal(NULL)
    {
        if (calculateNormal)
        {
            CalculateNormal();
        }
    }

    Triangle(const Triangle2D &tri, const float offset, AXISPLANES plane, bool calculateNormal = true)
        : m_pRawNormal(NULL), m_pNormal(NULL)
    {
        if (plane == AP_XY)
        {
            m_p1 = Vector3(tri[0]->x, tri[0]->y, offset);
            m_p2 = Vector3(tri[1]->x, tri[1]->y, offset);
            m_p3 = Vector3(tri[2]->x, tri[2]->y, offset);
        }
        else if (plane == AP_XZ)
        {
            m_p1 = Vector3(tri[0]->x, offset, tri[0]->y);
            m_p2 = Vector3(tri[1]->x, offset, tri[1]->y);
            m_p3 = Vector3(tri[2]->x, offset, tri[2]->y);
        }
        else
        {
            m_p1 = Vector3(offset, tri[0]->x, tri[0]->y);
            m_p2 = Vector3(offset, tri[1]->x, tri[1]->y);
            m_p3 = Vector3(offset, tri[2]->x, tri[2]->y);
        }

        if (calculateNormal)
        {
            CalculateNormal();
        }
    }

    Triangle(const Triangle &tri, bool calculateNormal = true)
        : m_p1(*tri[0]), m_p2(*tri[1]), m_p3(*tri[2]), m_pRawNormal(NULL), m_pNormal(NULL)
    {
        if (calculateNormal)
        {
            CalculateNormal();
        }
    }

    ~Triangle()
    {
        Clear();
    }

    Triangle2D ProjectToAxisPlane(AXISPLANES plane) const
    {
        if (plane == AP_XY)
        {
            return Triangle2D(Vector2(m_p1.x, m_p1.y), Vector2(m_p2.x, m_p2.y), Vector2(m_p3.x, m_p3.y));
        }
        else if (plane == AP_XZ)
        {
            return Triangle2D(Vector2(m_p1.x, m_p1.z), Vector2(m_p2.x, m_p2.z), Vector2(m_p3.x, m_p3.z));
        }
        else
        {
            return Triangle2D(Vector2(m_p1.y, m_p1.z), Vector2(m_p2.y, m_p2.z), Vector2(m_p3.y, m_p3.z));
        }
    }

    Triangle2D ProjectToPlane(const Plane &plane) const
    {
        return Triangle(*this).TransformToPlane(plane).ProjectToAxisPlane(AP_XY);
    }

    const Triangle GetTransformedToPlane(const Plane &plane) const    // return a transformed triangle to the plane
    {
        return Triangle(*this).TransformToPlane(plane);
    }

    const Triangle &TransformToPlane(const Plane &plane)    // transform the current triangle to the plane
    {
        // This transforms this triangle

        return *this;
    }

    void SetPt1(const Vector3 &pt1)
    {
        m_p1 = pt1;

        Clear();
    }

    void SetPt2(const Vector3 &pt2)
    {
        m_p2 = pt2;

        Clear();
    }

    void SetPt3(const Vector3 &pt3)
    {
        m_p3 = pt3;

        Clear();
    }

    const Vector3 *operator[](const unsigned int index) const
    {
        return (&m_p1)+(index % 3);
    }

    const Vector3 *GetVerts() const { return &m_p1; }

    bool IsDegenerate() const
    {
        return CalculateArea() != 0.0f;
    }

    const bool ContainsPoint(const Vector3 &pt) const
    {
        if (!m_pNormal)
        {
            CalculateNormal();
        }

        Vector3 u(pt - m_p1);

        //return abs(Math::Dot(m_pNormal, &u)) == 1.0f;

        /* TODO:
        if (point is on plane)
        {
            if the sum of the angles is 360
                return true
        }
        */

        return true;
    }

    void CalculateNormal() const
    {
        if (!m_pRawNormal)
        {
            CalculateRawNormal();
        }

        if (!m_pNormal)
        {
            m_pNormal = new Vector3(Math::Normalise(*m_pRawNormal));
        }
        else
        {
            *m_pNormal = Math::Normalise(*m_pRawNormal);
        }
    }

    void CalculateRawNormal() const
    {
        Vector3 u(m_p1 - m_p2);
        Vector3 v(m_p1 - m_p3);

        if (!m_pRawNormal)
        {
            m_pRawNormal = new Vector3(Math::Cross(u, v));
        }
        else
        {
            *m_pRawNormal = Math::Cross(u, v);
        }
    }

    const float CalculateArea() const
    {
        if (!m_pRawNormal)
        {
            CalculateRawNormal();
        }

        return Math::Magnitude(m_pRawNormal) * 0.5f;
    }

    const Vector3 *GetNormal() const { return m_pNormal; }
    const Vector3 *GetRawNormal() const { return m_pRawNormal; }

private:
    void Clear()
    {
        if (m_pRawNormal)
        {
            delete m_pRawNormal;
            m_pRawNormal = NULL;
        }

        if (m_pNormal)
        {
            delete m_pNormal;
            m_pNormal = NULL;
        }
    }

    mutable Vector3 *m_pRawNormal;    // Non-normalised surface normal
    mutable Vector3 *m_pNormal;        // Normalised surface normal (face normal)

    Vector3 m_p1, m_p2, m_p3;
};

inline bool GetTriangleIntersectionData(const Triangle2D &tri, TriangleIntersectionData *pOut)
{
    if (!pOut)// || tri.IsDegenerate())
        return false;

    const Vector2 *triVerts = tri.GetVerts();

    pOut->centroid = Vector2((triVerts[0] + triVerts[1] + triVerts[2]) * 0.3333333333333333f);

    pOut->vertices[0] = triVerts[0] - pOut->centroid;
    pOut->vertices[1] = triVerts[1] - pOut->centroid;
    pOut->vertices[2] = triVerts[2] - pOut->centroid;

    return true;
}

inline bool GetRectangleIntersectionData(const Rectangle2D &rect, RectangleIntersectionData *pOut)
{
    if (!pOut)// || rect.GetHeight() <= 0 || rect.GetWidth() <= 0)
    {
        return false;
    }

    pOut->centroid = Vector2((rect.left + rect.right) * 0.5f, (rect.top + rect.bottom) * 0.5f);

    pOut->vertices[0] = Vector2(rect.left, rect.top);
    pOut->vertices[1] = Vector2(rect.left, rect.bottom);
    pOut->vertices[2] = Vector2(rect.right, rect.bottom);
    pOut->vertices[3] = Vector2(rect.right, rect.top);

    pOut->relativeVertices[0] = Vector2(rect.left - pOut->centroid.x, rect.top - pOut->centroid.y);
    pOut->relativeVertices[1] = Vector2(rect.left - pOut->centroid.x, rect.bottom - pOut->centroid.y);
    pOut->relativeVertices[2] = Vector2(rect.right - pOut->centroid.x, rect.bottom - pOut->centroid.y);
    pOut->relativeVertices[3] = Vector2(rect.right - pOut->centroid.x, rect.top - pOut->centroid.y);

    return true;
}
};

// EOF