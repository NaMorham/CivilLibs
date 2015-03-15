#include "../include/ksr.h"

#include "../include/leakwatcher.h"

#ifdef _DO_MEMORY_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace KSR;

float collisionTolerance = 0.0001f;
float collisionPrecision = 0.0001f;

const Vector3 Math::I(1, 0, 0);
const Vector3 Math::J(0, 1, 0);
const Vector3 Math::K(0, 0, 1);
const Vector3 Math::I_NEG(-1,  0,  0);
const Vector3 Math::J_NEG(0, -1,  0);
const Vector3 Math::K_NEG(0,  0, -1);

//-----------------------------------------------------------------------
    void SetCollisionTolerance(float tolerance)
    //-------------------------------------------------------------------
    {
        collisionTolerance = tolerance;
    }


//-----------------------------------------------------------------------
    void SetCollisionPrecision(float precision)
    //-------------------------------------------------------------------
    {
        collisionPrecision = precision;
    }


//-----------------------------------------------------------------------
    Math::Sphere::Sphere()
    //-------------------------------------------------------------------
    {
        center = Vector3(0, 0, 0);
        radius = 0;
    }


//-----------------------------------------------------------------------
    Math::Sphere::Sphere(const Vector3 &sphereCenter, float sphereRadius)
    //-------------------------------------------------------------------
    {
        center = sphereCenter;
        radius = sphereRadius;
    }


//-----------------------------------------------------------------------
    Math::AABB::AABB()
    //-------------------------------------------------------------------
    {
        min = max = Vector3(0, 0, 0);
    }


//-----------------------------------------------------------------------
    Math::AABB::AABB(const Vector3 &boxMin, const Vector3 &boxMax)
    //-------------------------------------------------------------------
    {
        min = boxMin;
        max = boxMax;
    }


//-----------------------------------------------------------------------
    void Math::AABB::IncludePoint(const Vector3 &point)
    //-------------------------------------------------------------------
    {
        if (point.x < min.x)
            min.x = point.x;
        if (point.x > max.x)
            max.x  = point.x;

        if (point.y < min.y)
            min.y = point.y;
        if (point.y > max.y)
            max.y  = point.y;

        if (point.z < min.z)
            min.z = point.z;
        if (point.z > max.z)
            max.z  = point.z;
    }


//-----------------------------------------------------------------------
    void Math::AABB::MakeInvalid()
    //-------------------------------------------------------------------
    {
        min = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
        max = Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    }


//-----------------------------------------------------------------------
    bool Math::AABB::PointInside(const Vector3 &point)
    //-------------------------------------------------------------------
    {
        return point.x >= min.x && point.x < max.x &&
               point.y >= min.y && point.y < max.y &&
               point.z >= min.z && point.z < max.z;
    }


//-----------------------------------------------------------------------
    Math::AABBPlanes::AABBPoly::AABBPoly()
    //-------------------------------------------------------------------
    {
        ZeroMemory(this, sizeof(AABBPlanes::AABBPoly));
    }

//-----------------------------------------------------------------------
    void Math::AABBPlanes::AABBPoly::Set(const Vector3 &a, const Vector3 &b, const Vector3 &c, const Vector3 &d)
    //-------------------------------------------------------------------
    {
        m_verts[0] = a; m_verts[1] = b; m_verts[2] = c; m_verts[3] = d;
    }

//-----------------------------------------------------------------------
    void Math::AABBPlanes::AABBPoly::Get(Vector3 *pA, Vector3 *pB, Vector3 *pC, Vector3 *pD)
    //-------------------------------------------------------------------
    {
        if (pA)
            *pA = m_verts[0];
        if (pB)
            *pB = m_verts[1];
        if (pC)
            *pC = m_verts[2];
        if (pD)
            *pD = m_verts[3];
    }

//-----------------------------------------------------------------------
    Math::AABBPlanes::AABBPlanes(const AABB &src)
    //-------------------------------------------------------------------
    :    m_AABB(src)
    //-------------------------------------------------------------------
    {
        D3DXPlaneFromPointNormal(m_planes, &src.max, &I);
        D3DXPlaneFromPointNormal(m_planes + 1, &src.max, &J);
        D3DXPlaneFromPointNormal(m_planes + 2, &src.max, &K);
        D3DXPlaneFromPointNormal(m_planes + 3, &src.min, &I_NEG);
        D3DXPlaneFromPointNormal(m_planes + 4, &src.min, &J_NEG);
        D3DXPlaneFromPointNormal(m_planes + 5, &src.min, &K_NEG);

        m_polygons[0].Set(src.max, Vector3(src.max.x, src.min.y, src.max.z),
                          Vector3(src.max.x, src.min.y, src.min.z), Vector3(src.max.x, src.max.y, src.min.z));
        m_polygons[1].Set(src.max, Vector3(src.max.x, src.max.y, src.min.z),
                          Vector3(src.min.x, src.max.y, src.min.z), Vector3(src.min.x, src.max.y, src.max.z));
        m_polygons[2].Set(src.max, Vector3(src.min.x, src.max.y, src.max.z),
                          Vector3(src.min.x, src.min.y, src.max.z), Vector3(src.max.x, src.min.y, src.max.z));
        m_polygons[3].Set(src.min, Vector3(src.min.x, src.min.y, src.max.z),
                          Vector3(src.min.x, src.max.y, src.max.z), Vector3(src.min.x, src.max.y, src.min.z));
        m_polygons[4].Set(src.min, Vector3(src.max.x, src.min.y, src.min.z),
                          Vector3(src.max.x, src.min.y, src.max.z), Vector3(src.min.x, src.min.y, src.max.z));
        m_polygons[5].Set(src.min, Vector3(src.min.x, src.max.y, src.min.z),
                          Vector3(src.max.x, src.max.y, src.min.z), Vector3(src.max.x, src.min.y, src.min.z));
    }

//    D3DXPLANE m_planes[6];
//    AABBPoly m_polygons[6];

//-----------------------------------------------------------------------
    Math::OBB::OBB()
    //-------------------------------------------------------------------
    {
        min = max = orientation = Vector3(0, 0, 0);
    }


//-----------------------------------------------------------------------
    Math::OBB::OBB(const Vector3 &boxMin, const Vector3 &boxMax, const Vector3 &boxOrientation)
    //-------------------------------------------------------------------
    {
        min = boxMin;
        max = boxMax;
        orientation = boxOrientation;
    }


//-----------------------------------------------------------------------
    Math::Polygon::Polygon()
    //-------------------------------------------------------------------
    {
        normal = Vector3(0, 0, 0);
        vertices = NULL;
        numVerts = 0;
    }


//-----------------------------------------------------------------------
    Math::Polygon::Polygon(const Vector3 &n, Vector3 *verts, int nVerts)
    //-------------------------------------------------------------------
    {
        normal = n;
        vertices = verts;
        numVerts = nVerts;
    }


//-----------------------------------------------------------------------
    const float Math::Random(const float min, const float max)
    //-------------------------------------------------------------------
    {
        return (max - min) * (rand()/RAND_MAX) + min;
    }


//-----------------------------------------------------------------------
    const Vector3 Math::Random(const Vector3 &min, const Vector3 &max)
    //-------------------------------------------------------------------
    {
        return Vector3(Math::Random(min.x, max.x),
                       Math::Random(min.y, max.y),
                       Math::Random(min.z, max.z));
    }


//-----------------------------------------------------------------------
    void Math::SortMinMax(const float &sourceMin, const float &sourceMax, float &destMin, float &destMax)
    //-------------------------------------------------------------------
    {
        if (sourceMin < destMin)
            destMin = sourceMin;

        if (sourceMax > destMax)
            destMax = sourceMax;
    }


//-------------------------------------------------------------------
    void Math::SortMinMax(const Vector3 &sourceMin, const Vector3 &sourceMax, Vector3 &destMin, Vector3 &destMax)
    //---------------------------------------------------------------
    {
        if (sourceMin.x < destMin.x)
            destMin.x = sourceMin.x;
        if (sourceMin.y < destMin.y)
            destMin.y = sourceMin.y;
        if (sourceMin.z < destMin.z)
            destMin.z = sourceMin.z;

        if (sourceMax.x > destMax.x)
            destMax.x = sourceMax.x;
        if (sourceMax.y > destMax.y)
            destMax.y = sourceMax.y;
        if (sourceMax.z > destMax.z)
            destMax.z = sourceMax.z;
    }


//-----------------------------------------------------------------------
    DWORD Math::FloatToDW(FLOAT f)
    //-------------------------------------------------------------------
    {
        return *((DWORD*)&f);
    }


//-----------------------------------------------------------------------
    int Math::Round(float f)
    //-------------------------------------------------------------------
    {
        float t = f;

        if (t < 0)
            t = -t;

        float r = t - (int)t;

        if (r >= 0.5f)
            return (int)ceil(f);

        return (int)floor(f);
    }


//-----------------------------------------------------------------------
    int Math::Equals(float x, float y)
    //-------------------------------------------------------------------
    {
        return (((y - collisionPrecision) < x) && (x < (y + collisionPrecision)));
    }


//-----------------------------------------------------------------------
    int Math::Greater(float x, float y)
    //-------------------------------------------------------------------
    {
        return ((y - collisionPrecision) < x);
    }


//-----------------------------------------------------------------------
    int Math::GreaterEquals(float x, float y)
    //-------------------------------------------------------------------
    {
        return Math::Greater(x, y) || Math::Equals(x, y);
    }


//-----------------------------------------------------------------------
    int Math::Less(float x, float y)
    //-------------------------------------------------------------------
    {
        return ((x - collisionPrecision) < y);
    }


//-----------------------------------------------------------------------
    int Math::LessEquals(float x, float y)
    //-------------------------------------------------------------------
    {
        return Math::Less(x, y) || Math::Equals(x, y);
    }


//-----------------------------------------------------------------------
    Vector3 Math::GetXAxis(Matrix &m)
    //-------------------------------------------------------------------
    {
        return *(Vector3 *)&m._11;
    }


//-----------------------------------------------------------------------
    Vector3 Math::GetYAxis(Matrix &m)
    //-------------------------------------------------------------------
    {
        return *(Vector3 *)&m._21;
    }


//-----------------------------------------------------------------------
    Vector3 Math::GetZAxis(Matrix &m)
    //-------------------------------------------------------------------
    {
        return *(Vector3 *)&m._31;
    }


//-----------------------------------------------------------------------
    void Math::PerspectiveOffCenterFovLH(Matrix &out, float l, float r, float b, float t, float zn,
                                         float zf, float fov, float aspect)
    //-------------------------------------------------------------------
    {
        float h = float(cos(fov/2) / sin(fov/2));
        float w = h / aspect;

        out._11 = 2*zn/((r-l)/w);    out._12 = 0;                out._13 = 0;                out._14 = 0;
        out._21 = 0;                out._22 = 2*zn/((t-b)/h);    out._23 = 0;                out._24 = 0;
        out._31 = (l+r)/(l-r);        out._32 = (t+b)/(b-t);        out._33 = zf/(zf-zn);        out._34 = 1;
        out._41 = 0;                out._42 = 0;                out._43 = zn*zf/(zn-zf);    out._44 = 0;
    }


//-------------------------------------------------------------------
    Vector3 Math::Cross(const Vector3 &a, const Vector3 &b)
    //---------------------------------------------------------------
    {
        Vector3 result;
        D3DXVec3Cross(&result, &a, &b);
        return result;
    }


//-------------------------------------------------------------------
    Vector3 Math::Cross(const Vector3 *pA, const Vector3 *pB)
    //---------------------------------------------------------------
    {
        Vector3 result;
        D3DXVec3Cross(&result, pA, pB);
        return result;
    }


//-------------------------------------------------------------------
    Vector2 Math::Normalise(const Vector2 &v)
    //---------------------------------------------------------------
    {
        float mag = Math::Magnitude(v);

        if (mag == 0)
            return Vector2(0, 1);

        return Vector2(v.x/mag, v.y/mag);
    }


//-------------------------------------------------------------------
    Vector2 *Math::Normalise(Vector2 *pOut, const Vector2 *pV)
    //---------------------------------------------------------------
    {
        float mag = Math::Magnitude(pV);

        if (mag == 0)
            return NULL;

        pOut->x = pV->x / mag;
        pOut->y = pV->y / mag;

        return pOut;
    }


//-------------------------------------------------------------------
    Vector2 *Math::Normalise(Vector2 *pV)
    //---------------------------------------------------------------
    {
        float mag = Math::Magnitude(pV);

        if (mag == 0)
            return NULL;

        pV->x = pV->x / mag;
        pV->y = pV->y / mag;

        return pV;
    }


//-------------------------------------------------------------------
    Vector3 Math::Normalise(const Vector3 &v)
    //---------------------------------------------------------------
    {
        float mag = Math::Magnitude(v);

        if (mag == 0)
            return Vector3(0, 1, 0);

        return Vector3(v.x/mag, v.y/mag, v.z/mag);
    }


//-------------------------------------------------------------------
    Vector3 *Math::Normalise(Vector3 *pOut, const Vector3 *pV)
    //---------------------------------------------------------------
    {
        float mag = Math::Magnitude(pV);

        if (mag == 0)
            return NULL;

        pOut->x = pV->x / mag;
        pOut->y = pV->y / mag;
        pOut->z = pV->z / mag;

        return pOut;
    }


//-------------------------------------------------------------------
    Vector3 *Math::Normalise(Vector3 *pV)
    //---------------------------------------------------------------
    {
        float mag = Math::Magnitude(pV);

        if (mag == 0)
            return NULL;

        pV->x = pV->x / mag;
        pV->y = pV->y / mag;
        pV->z = pV->z / mag;

        return pV;
    }


//-------------------------------------------------------------------
    float Math::Magnitude(const Vector2 &v)
    //---------------------------------------------------------------
    {
        return sqrt(v.x * v.x + v.y * v.y);
    }


//-------------------------------------------------------------------
    float Math::Magnitude(const Vector2 *pV)
    //---------------------------------------------------------------
    {
        return sqrt(pV->x * pV->x + pV->y * pV->y);
    }


//-------------------------------------------------------------------
    float Math::Magnitude(const Vector3 &v)
    //---------------------------------------------------------------
    {
        return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    }


//-------------------------------------------------------------------
    float Math::Magnitude(const Vector3 *pV)
    //---------------------------------------------------------------
    {
        return sqrt(pV->x * pV->x + pV->y * pV->y + pV->z * pV->z);
    }


//-------------------------------------------------------------------
    float Math::Length(const Vector2 &v)
    //---------------------------------------------------------------
    {
        return D3DXVec2Length(&v);
    }


//-------------------------------------------------------------------
    float Math::Length(const Vector3 &v)
    //---------------------------------------------------------------
    {
        return D3DXVec3Length(&v);
    }


//-------------------------------------------------------------------
    float Math::Length(const Vector4 &v)
    //---------------------------------------------------------------
    {
        return D3DXVec4Length(&v);
    }


//-------------------------------------------------------------------
    float Math::Dot(const Vector3 &a, const Vector3 &b)
    //---------------------------------------------------------------
    {
        return D3DXVec3Dot(&a, &b);
    }


//-------------------------------------------------------------------
    float Math::Dot(const Vector3 *pA, const Vector3 *pB)
    //---------------------------------------------------------------
    {
        return D3DXVec3Dot(pA, pB);
    }


//-------------------------------------------------------------------
    float Math::Dot(const Vector2 &a, const Vector2 &b)
    //---------------------------------------------------------------
    {
        return D3DXVec2Dot(&a, &b);
    }


//-------------------------------------------------------------------
    float Math::Dot(const Vector2 *pA, const Vector2 *pB)
    //---------------------------------------------------------------
    {
        return D3DXVec2Dot(pA, pB);
    }


//-------------------------------------------------------------------
    const Vector2 &Math::TransformCoord(Vector2 *pOut, const Vector2 &in, const Matrix &transform)
    //---------------------------------------------------------------
    {
        if (!pOut)
            return in;

        D3DXVec2TransformCoord(pOut, &in, &transform);

        return *pOut;
    }


//-------------------------------------------------------------------
    const Vector3 &Math::TransformCoord(Vector3 *pOut, const Vector3 &in, const Matrix &transform)
    //---------------------------------------------------------------
    {
        if (!pOut)
            return in;

        D3DXVec3TransformCoord(pOut, &in, &transform);

        return *pOut;
    }


//-------------------------------------------------------------------
    const Vector2 &Math::TransformNormal(Vector2 *pOut, const Vector2 &in, const Matrix &transform)
    //---------------------------------------------------------------
    {
        if (!pOut)
            return in;

        D3DXVec2TransformNormal(pOut, &in, &transform);

        return *pOut;
    }


//-------------------------------------------------------------------
    const Vector3 &Math::TransformNormal(Vector3 *pOut, const Vector3 &in, const Matrix &transform)
    //---------------------------------------------------------------
    {
        if (!pOut)
            return in;

        D3DXVec3TransformNormal(pOut, &in, &transform);

        return *pOut;
    }


//-----------------------------------------------------------------------
    const Matrix &Math::Invert(Matrix *pOut, const Matrix &transform)
    //-------------------------------------------------------------------
    {
        if (!pOut)
            return transform;

        D3DXMatrixInverse(pOut, NULL, &transform);

        return *pOut;
    }


//-----------------------------------------------------------------------
    const Matrix &Math::Transpose(Matrix *pOut, const Matrix &transform)
    //-------------------------------------------------------------------
    {
        if (!pOut)
            return transform;

        D3DXMatrixTranspose(pOut, &transform);

        return *pOut;
    }


//-----------------------------------------------------------------------
    const Matrix &Math::InvertAndTranspose(Matrix *pOut, const Matrix &transform)
    //-------------------------------------------------------------------
    {
        if (!pOut)
            return transform;

        D3DXMatrixInverse(pOut, NULL, &transform);
        D3DXMatrixTranspose(pOut, pOut);

        return *pOut;
    }


//-----------------------------------------------------------------------
    Vector3 Math::Unproject(const Vector2 &v, const Matrix &view, float width, float height, float fov,
                            float aspect, float zNear, DWORD coordSystemMode)
    //-------------------------------------------------------------------
    {
        Matrix invView;
        D3DXMatrixInverse(&invView, NULL, &view);

        float halfFov = fov * 0.5f;

        float vH = zNear * tanf(halfFov);
        float vW = vH * aspect;

        Vector2 uv1 = Vector2(v.x / width, v.y / height);
        Vector2 uv2 = Vector2(2.0f * uv1.x - 1.0f, 2.0f * uv1.y - 1.0f);
        Vector2 uvOffset = Vector2(uv2.x * vW, uv2.y * vH);

        Vector3 uvScreen = Vector3(uvOffset.x, uvOffset.y, zNear);

        if (coordSystemMode == VS_RIGHTHANDED)
            uvScreen.z *= -1;

        Vector3 vRay;
        D3DXVec3TransformNormal(&vRay, &uvScreen, &invView);

        return Normalise(vRay);
    }


//-----------------------------------------------------------------------
    Vector2 Math::Project(const Vector3 &v, const Matrix &view, const Matrix &projection, float width, float height, float zNear, float zFar)
    //-------------------------------------------------------------------
    {
        Vector3 vProj3;

        D3DVIEWPORT9 D3DViewport;
        D3DViewport.X = 0;
        D3DViewport.Y = 0;
        D3DViewport.Width = (DWORD)width;
        D3DViewport.Height = (DWORD)height;
        D3DViewport.MinZ = zNear;
        D3DViewport.MaxZ = zFar;

        Matrix world;
        D3DXMatrixIdentity(&world);

        D3DXVec3Project(&vProj3, &v, &D3DViewport, &projection, &view, &world);

        return Vector2(vProj3.x, vProj3.y);
    }


//-------------------------------------------------------------------
    float Math::PlaneDistance(const Vector3 &n, const Vector3 &p)
    //---------------------------------------------------------------
    {
        return (-((n.x * p.x) + (n.y * p.y) + (n.z * p.z)));
    }


//-------------------------------------------------------------------
    bool Math::PointInsidePolygon(const Vector3 &point, const Vector3 *pVerts, int numVerts)
    //---------------------------------------------------------------
    {
        float angle = 0;

        for (int n = 0; n < numVerts; n++)
        {
            const Vector3 &vertex = *(pVerts + n);
            const Vector3 &nextVertex = *(pVerts + ((n + 1) % numVerts));

            Vector3 a = Math::Normalise(vertex - point);
            Vector3 b = Math::Normalise(nextVertex - point);

            float c = D3DXVec3Dot(&a, &b);
            float e = acos(c);

            angle += e;
        }

        return Math::Equals(angle, TWO_PI) != 0;
    }


// TODO: Fix this function.
//-------------------------------------------------------------------
    Vector3 Math::ClosestPointOnLine(const Vector3 &a, const Vector3 &b, const Vector3 &position)
    //---------------------------------------------------------------
    {
        Vector3 aMinusB(a - b);
        Vector3 bMinusA(b - a);

        Vector3 v1 = position - a;
        Vector3 v2 = Math::Normalise(bMinusA);

        float d = D3DXVec3Length(&aMinusB);
        float t = D3DXVec3Dot(&v2, &v1);

        if (t <= 0)
            return a;

        if (t >= d)
            return b;

        Vector3 v3 = v2 * t;
        Vector3 cp = a + v3;

        return cp;
    }


//-------------------------------------------------------------------
    bool Math::QuadraticFormula(float a, float b, float c, float *pT0, float *pT1)
    //---------------------------------------------------------------
    {
        float q = b * b - 4.0f * a * c;

        if (q >= 0)
        {
            float sq = (float)sqrt(q);
            float d = 1.0f / (2.0f * a);

            if (pT0)
                *pT0 = (-b + sq) * d;

            if (pT1)
                *pT1 = (-b - sq) * d;

            return true;
        }

        return false;
    }


//-------------------------------------------------------------------
    bool Math::SphereEdgeCollision(const Math::Sphere &sphere, const Math::Polygon &polygon)
    //---------------------------------------------------------------
    {
        if (!polygon.vertices)
            return false;

        for (int n = 0; n < polygon.numVerts; n++)
        {
            Vector3 point = Math::ClosestPointOnLine(polygon.vertices[n], polygon.vertices[(n + 1) % polygon.numVerts], sphere.center);

            float distance = Math::Length(polygon.vertices[0] - sphere.center);

            //if (distance < sphere.radius)
            if (Math::Less(distance, sphere.radius))
                return true;
        }

        return false;
    }


//-------------------------------------------------------------------
    bool Math::SweepSphereStaticPolygon(const Math::Sphere &sphere1, const Math::Sphere &sphere2, const Math::Polygon &polygon,
                                        float *pTime, Vector3 *pPosition, Vector3 *pPoint)
    //---------------------------------------------------------------
    {
        if (sphere1.center != sphere2.center)
        {
            float r = sphere1.radius;
            float d0, d1;

            float planeDistance = PlaneDistance(polygon.normal, polygon.vertices[0]);
            d0 = (polygon.normal.x * sphere1.center.x + polygon.normal.y * sphere1.center.y + polygon.normal.z * sphere1.center.z
                 + planeDistance);
            d1 = (polygon.normal.x * sphere2.center.x + polygon.normal.y * sphere2.center.y + polygon.normal.z * sphere2.center.z
                 + planeDistance);

            if (Math::GreaterEquals(d0, r) && Math::LessEquals(d1, r))
            {
                float ti = (d0 - r) / (d0 - d1);

                Vector3 position = (1 - ti) * sphere1.center + ti * sphere2.center;

                float di = d0 + (ti * (d1 - d0));
                Vector3 point = position - (polygon.normal * di);

                if (Math::PointInsidePolygon(point, polygon.vertices, polygon.numVerts) ||
                    Math::SphereEdgeCollision(Math::Sphere(point, r), polygon))
                {
                    if (pPosition)
                        *pPosition = position;

                    if (pTime)
                        *pTime = ti;

                    if (pPoint)
                        *pPoint = point;

                    return true;
                }

                return false;
            }

            if (Math::PolygonSphereCollision(polygon, sphere2, pPoint))
            {
                if (pTime)
                    *pTime = 1;

                if (pPosition)
                    *pPosition = sphere2.center;

                return true;
            }
        }
        else if (Math::PolygonSphereCollision(polygon, sphere1, pPoint))
        {
            if (pTime)
                *pTime = 0;

            if (pPosition)
                *pPosition = sphere1.center;

            return true;
        }

        return false;
    }


//-------------------------------------------------------------------
    bool Math::SweepSphereSphere(const Math::Sphere &sphereA1, const Math::Sphere &sphereA2, const Math::Sphere &sphereB1, const Math::Sphere &sphereB2,
                                 float *pTime, Vector3 *pPosition1, Vector3 *pPosition2, Vector3 *pPoint)
    //---------------------------------------------------------------
    {
        Vector3 va = sphereA2.center - sphereA1.center;
        Vector3 vb = sphereB2.center - sphereB1.center;
        Vector3 ab = sphereB1.center - sphereA1.center;
        Vector3 vab = vb - va;

        float rab = sphereA1.radius + sphereB1.radius + collisionTolerance;
        float cab = Dot(ab, ab);
        //float rabsq = rab * rab + collisionTolerance;
        float rabsq = rab * rab;

        float a = Dot(vab, vab);
        float b = 2.0f * Dot(vab, ab);
        float c = cab - rabsq;

        //if (!a || !b || !c)
        //    return false;

        //if (!a)
            //return false;

        float r1, r2;

        if (sphereA1.center == sphereA2.center && sphereB1.center == sphereB2.center)
        {
            if (Math::SphereSphereCollision(sphereA1, sphereB1))
            {
                if (pTime)
                    *pTime = 0;

                if (pPosition1)
                    *pPosition1 = sphereA1.center;

                if (pPosition2)
                    *pPosition2 = sphereB1.center;

                if (pPoint)
                    *pPoint = sphereA1.center + (sphereB1.center - sphereA1.center) / (rab / sphereA1.radius);

                return true;
            }
            else
                return false;
        }

        if (!QuadraticFormula(a, b, c, &r1, &r2))
            return false;

        if (r1 > r2)
        {
            float rt = r1;
            r1 = r2;
            r2 = rt;
        }

        if (r1 < 0 || r1 > 1)
            return false;

        Vector3 pos1 = sphereA1.center + va * r1;
        Vector3 pos2 = sphereB1.center + vb * r1;

        if (pTime)
            *pTime = r1;

        if (pPosition1)
            *pPosition1 = pos1;

        if (pPosition2)
            *pPosition2 = pos2;

        if (pPoint)
            *pPoint = pos1 + (pos2 - pos1) / (rab / sphereA1.radius);

        return true;
    }


//-------------------------------------------------------------------
    bool Math::SweepAABBAABB(const Math::AABB &AABBA1, const Math::AABB &AABBA2, const Math::AABB &AABBB1, const Math::AABB &AABBB2,
                       float *pTime, Vector3 *pPosition1, Vector3 *pPosition2)
    //---------------------------------------------------------------
    {
        Vector3 pA1 = AABBA1.min + (AABBA1.max - AABBA1.min) * 0.5f;
        Vector3 pA2 = AABBA2.min + (AABBA2.max - AABBA2.min) * 0.5f;
        Vector3 pB1 = AABBB1.min + (AABBB1.max - AABBB1.min) * 0.5f;
        Vector3 pB2 = AABBB2.min + (AABBB2.max - AABBB2.min) * 0.5f;

        Vector3 va = pA2 - pA1;
        Vector3 vb = pB2 - pB1;
        Vector3 vab = vb - va;
        Vector3 r0 = Vector3(0, 0, 0),
                r1 = Vector3(1, 1, 1);

        for (int i = 0; i < 3; i++)
        {
            if (AABBA1.max[i] < AABBB1.min[i] && vab[i] < 0)
                r0[i] = (AABBA1.max[i] - AABBB1.min[i]) / vab[i];
            else if (AABBB1.max[i] < AABBA1.min[i] && vab[i] > 0)
                r0[i] = (AABBA1.min[i] - AABBB1.max[i]) / vab[i];

            if (AABBB1.max[i] < AABBA1.min[i] && vab[i] < 0)
                r1[i] = (AABBA1.min[i] - AABBB1.max[i]) / vab[i];
            else if (AABBA1.max[i] < AABBB1.min[i] && vab[i] > 0)
                r1[i] = (AABBA1.max[i] - AABBB1.min[i]) / vab[i];
        }

        float u0 = r0.x;
        float u1 = r1.x;

        if (r0.y > u0)
            u0 = r0.y;

        if (r0.z > u0)
            u0 = r0.z;

        if (r1.y < u1)
            u1 = r1.y;

        if (r1.z < u1)
            u1 = r1.z;

        if (u0 <= u1)
        {
            if (pTime)
                *pTime = u0;

            if (pPosition1)
                *pPosition1 = pA1 + va * u0;

            if (pPosition2)
                *pPosition2 = pB1 + vb * u0;

            return true;
        }

        return false;
    }


//-------------------------------------------------------------------
    bool Math::SweepOBBStaticPolygon(const Math::OBB &OBB1, const Math::OBB &OBB2, const Math::Polygon &polygon, float *pTime, Vector3 *pPoint)
    //---------------------------------------------------------------
    {

        return false;
    }


//-------------------------------------------------------------------
    bool Math::SweepOBBOBB(const Math::OBB &OBBA1, const Math::OBB &OBBA2, const Math::OBB &OBBB1, const Math::OBB &OBBB2, float *pTime)
    //---------------------------------------------------------------
    {

        return false;
    }


//-------------------------------------------------------------------
    bool Math::SphereSphereCollision(const Math::Sphere &sphere1, const Math::Sphere &sphere2)
    //---------------------------------------------------------------
    {
        float length = D3DXVec3Length(&Vector3(sphere2.center - sphere1.center));

        //if (length <= sphere1.radius + sphere2.radius)
        if (Math::LessEquals(length, sphere1.radius + sphere2.radius))
            return true;

        return false;
    }


//-------------------------------------------------------------------
    bool Math::SpherePlaneCollision(const Math::Sphere &sphere, const Plane &plane, float *pDistance)
    //---------------------------------------------------------------
    {
        Vector3 n = Vector3(plane.a, plane.b, plane.c);
        Vector3 p = n * (plane.d / (FLOAT)sqrt(plane.a * plane.a + plane.b * plane.b + plane.c * plane.c));

        float planeDistance = PlaneDistance(n, p);
        float distance = (n.x * sphere.center.x + n.y * sphere.center.y + n.z * sphere.center.z + planeDistance);

        if (pDistance)
            *pDistance = distance;

        //if (fabs(distance) <= sphere.radius)
        if (Math::LessEquals((float)fabs(distance), sphere.radius))
            return true;

        return false;
    }


//-------------------------------------------------------------------
    bool Math::SpherePointCollision(const Math::Sphere &sphere, const Vector3 &point)
    //---------------------------------------------------------------
    {
        float length = D3DXVec3Length(&Vector3(sphere.center - point));

        //if (length <= sphere.radius)
        if (Math::LessEquals(length, sphere.radius))
            return true;

        return false;
    }


//-------------------------------------------------------------------
    bool Math::PolygonSphereCollision(const Math::Polygon &polygon, const Math::Sphere &sphere, Vector3 *pPoint)
    //---------------------------------------------------------------
    {
        if (!polygon.vertices || !polygon.numVerts || polygon.numVerts < 3)
            return false;

        float distance = 0;
        Plane plane;

        D3DXPlaneFromPointNormal(&plane, &polygon.vertices[0], &polygon.normal);
        if (Math::SpherePlaneCollision(sphere, plane, &distance))
        {
            Vector3 point = sphere.center - (polygon.normal * distance);

            if (pPoint)
                *pPoint = point;

            if (PointInsidePolygon(point, polygon.vertices, polygon.numVerts))
                return true;
            else if (SphereEdgeCollision(sphere, polygon))
                return true;
        }

        return false;
    }


// Unfinished
//-------------------------------------------------------------------
    bool Math::AABBPolygonCollision(const Math::AABB &box, const Math::Polygon &polygon)
    //---------------------------------------------------------------
    {
        Vector3 min = box.min;
        Vector3 max = box.max;
        Vector3 mid = min + max * 0.5f;

        D3DXPLANE planes[6];
        Vector3 p0(min.x, mid.y, mid.z);
        Vector3 p1(mid.x, mid.y, max.z);
        Vector3 p2(max.x, mid.y, mid.z);
        Vector3 p3(mid.x, mid.y, min.z);
        Vector3 p4(mid.x, min.y, mid.z);
        Vector3 p5(mid.x, max.y, mid.z);

        D3DXPlaneFromPointNormal(planes, &p0, &I);
        D3DXPlaneFromPointNormal(planes + 1, &p1, &K_NEG);
        D3DXPlaneFromPointNormal(planes + 2, &p2, &I_NEG);
        D3DXPlaneFromPointNormal(planes + 3, &p3, &K);
        D3DXPlaneFromPointNormal(planes + 4, &p4, &J);
        D3DXPlaneFromPointNormal(planes + 5, &p5, &J_NEG);

        for (int p = 0; p < 6; p++)
        {
            int inCount = 0;

            for (int v = 0; v < polygon.numVerts; v++)
            {
                Vector3 &point = polygon.vertices[v];

                if (AABBPointCollision(box, point))
                    return true;

                float d = D3DXPlaneDotCoord(&planes[p], &point);

                if (d >= 0)
                    inCount++;
            }

            if (inCount == 0)
                return false;
        }

        // check that it isn't passing by.

        return true;
    }


//-------------------------------------------------------------------
    Math::INTERSECTIONSTATE Math::IntersectionOnAxisRectangleTriangle2D(const Math::RectangleIntersectionData &riData,
                            const Math::TriangleIntersectionData &tiData, const Vector2 *pTriangleEdgeNormal,
                            Vector2 *pTriResults, Vector2 *pRectResults, bool contactTesting)
    //---------------------------------------------------------------
    {
        const Vector2 &nEdgeNormal = *pTriangleEdgeNormal;

        Vector2 triResults[4];
        float triLengths[4];
        bool triForwards[3];

        triResults[0] = tiData.centroid;
        triResults[1] = nEdgeNormal * Math::Dot(tiData.vertices[0], nEdgeNormal);    // Partial solution for the results.
        triResults[2] = nEdgeNormal * Math::Dot(tiData.vertices[1], nEdgeNormal);    // These are the projected vectors from the centroid
        triResults[3] = nEdgeNormal * Math::Dot(tiData.vertices[2], nEdgeNormal);    // to each of the vertices of the triangle.

        triLengths[0] = 0;
        triLengths[1] = Math::Magnitude(triResults[1]);
        triLengths[2] = Math::Magnitude(triResults[2]);
        triLengths[3] = Math::Magnitude(triResults[3]);

        triForwards[0] = Math::Dot(triResults+1, pTriangleEdgeNormal) > 0;
        triForwards[1] = Math::Dot(triResults+2, pTriangleEdgeNormal) > 0;
        triForwards[2] = Math::Dot(triResults+3, pTriangleEdgeNormal) > 0;

        triResults[1] += tiData.centroid;
        triResults[2] += tiData.centroid;
        triResults[3] += tiData.centroid;

        int tForwardResult = 0,
            tBackwardResult = 0;

        int n = 0;
        for (n = 0; n < 3; ++n)
        {
            if (triForwards[n])
            {
                if (triLengths[n+1] > triLengths[tForwardResult])
                    tForwardResult = n+1;
            }
            else
            {
                if (triLengths[n+1] > triLengths[tBackwardResult])
                    tBackwardResult = n+1;
            }
        }

        if (pTriResults)
        {
            pTriResults[0] = triResults[tForwardResult];
            pTriResults[1] = triResults[tBackwardResult];
        }

        Vector2 rectResults[5];
        float rectLengths[5];
        bool rectForwards[4];

        rectResults[0] = nEdgeNormal * Math::Dot(riData.centroid - tiData.centroid, nEdgeNormal) + tiData.centroid;
        rectResults[1] = nEdgeNormal * Math::Dot(riData.vertices[0] - rectResults[0], nEdgeNormal);
        rectResults[2] = nEdgeNormal * Math::Dot(riData.vertices[1] - rectResults[0], nEdgeNormal);
        rectResults[3] = nEdgeNormal * Math::Dot(riData.vertices[2] - rectResults[0], nEdgeNormal);
        rectResults[4] = nEdgeNormal * Math::Dot(riData.vertices[3] - rectResults[0], nEdgeNormal);

        rectLengths[0] = 0;
        rectLengths[1] = Math::Magnitude(rectResults[1]);
        rectLengths[2] = Math::Magnitude(rectResults[2]);
        rectLengths[3] = Math::Magnitude(rectResults[3]);
        rectLengths[4] = Math::Magnitude(rectResults[4]);

        rectForwards[0] = Math::Dot(rectResults+1, pTriangleEdgeNormal) > 0;
        rectForwards[1] = Math::Dot(rectResults+2, pTriangleEdgeNormal) > 0;
        rectForwards[2] = Math::Dot(rectResults+3, pTriangleEdgeNormal) > 0;
        rectForwards[3] = Math::Dot(rectResults+4, pTriangleEdgeNormal) > 0;

        rectResults[1] += rectResults[0];
        rectResults[2] += rectResults[0];
        rectResults[3] += rectResults[0];
        rectResults[4] += rectResults[0];

        int rForwardResult = 0,
            rBackwardResult = 0;

        for (n = 0; n < 4; ++n)
        {
            if (rectForwards[n])
            {
                if (rectLengths[n+1] > rectLengths[rForwardResult])
                    rForwardResult = n+1;
            }
            else
            {
                if (rectLengths[n+1] > rectLengths[rBackwardResult])
                    rBackwardResult = n+1;
            }
        }

        if (pRectResults)
        {
            pRectResults[0] = rectResults[rForwardResult];
            pRectResults[1] = rectResults[rBackwardResult];
        }

        Vector2 triCentre((triResults[tForwardResult].x + triResults[tBackwardResult].x) * 0.5f,
                            (triResults[tForwardResult].y + triResults[tBackwardResult].y) * 0.5f);

        Vector2 rectCentre((rectResults[rForwardResult].x + rectResults[rBackwardResult].x) * 0.5f,
                            (rectResults[rForwardResult].y + rectResults[rBackwardResult].y) * 0.5f);

        float doubleDistance = Math::Magnitude(triCentre - rectCentre) * 2.0f;
        float triDiameter = triLengths[tForwardResult] + triLengths[tBackwardResult];
        float rectDiameter = rectLengths[rForwardResult] + rectLengths[rBackwardResult];

        if (doubleDistance <= triDiameter + rectDiameter)
        {
            if (contactTesting)
            {
                if (doubleDistance + triDiameter < rectDiameter)
                {
                    return IS_RECT_CONTAINS_TRI;
                }
                else if (doubleDistance + rectDiameter < triDiameter)
                {
                    return IS_TRI_CONTAINS_RECT;
                }
                else if (doubleDistance - triDiameter == rectDiameter)
                {
                    return IS_CONTACT;
                }
                else if (doubleDistance + triDiameter == rectDiameter)
                {
                    return IS_IDENTICAL;
                }
                else
                {
                    return IS_PENETRATION;
                }
            }
            else
            {
                if (doubleDistance + triDiameter < rectDiameter)
                {
                    return IS_RECT_CONTAINS_TRI;
                }
                else if (doubleDistance + rectDiameter < triDiameter)
                {
                    return IS_TRI_CONTAINS_RECT;
                }
                else
                {
                    return IS_PENETRATION;
                }
            }
        }

        return IS_NONE;
    }


//-------------------------------------------------------------------
    Math::INTERSECTIONSTATE Math::RectangleTriangleIntersection2D(const Math::Triangle2D &triangle, const Math::RectangleIntersectionData &riData,
                            const Math::TriangleIntersectionData &tiData, Vector2 *pTriResults, Vector2 *pRectResults, bool contactTesting)
    //---------------------------------------------------------------
    {
        INTERSECTIONSTATE s1, s2, s3;

        if (pTriResults || pRectResults)
        {
            s1 = Math::IntersectionOnAxisRectangleTriangle2D(riData, tiData, triangle.GetEdgeNormal(0), pTriResults, pRectResults, contactTesting);
            s2 = Math::IntersectionOnAxisRectangleTriangle2D(riData, tiData, triangle.GetEdgeNormal(1), pTriResults + 2, pRectResults + 2, contactTesting);
            s3 = Math::IntersectionOnAxisRectangleTriangle2D(riData, tiData, triangle.GetEdgeNormal(2), pTriResults + 4, pRectResults + 4, contactTesting);

            if (s1 == IS_NONE || s2 == IS_NONE || s3 == IS_NONE)
            {
                return IS_NONE;
            }
        }
        else
        {
            s1 = Math::IntersectionOnAxisRectangleTriangle2D(riData, tiData, triangle.GetEdgeNormal(0), NULL, NULL, contactTesting);

            if (s1 == IS_NONE)
            {
                return IS_NONE;
            }

            s2 = Math::IntersectionOnAxisRectangleTriangle2D(riData, tiData, triangle.GetEdgeNormal(1), NULL, NULL, contactTesting);

            if (s2 == IS_NONE)
            {
                return IS_NONE;
            }

            s3 = Math::IntersectionOnAxisRectangleTriangle2D(riData, tiData, triangle.GetEdgeNormal(2), NULL, NULL, contactTesting);

            if (s3 == IS_NONE)
            {
                return IS_NONE;
            }
        }

        if (contactTesting)
        {
            if (s1 == s2 && s1 == s3)
            {
                // If they're all [A/B]_CONTAINS_[B/A], or all IS_PENETRATION or all IS_CONTACT
                return s1;
            }
            else if (s1 == IS_CONTACT || s2 == IS_CONTACT || s3 == IS_CONTACT)
            {
                // It's not possible to get only one, but where there's one, there must be another.
                return IS_CONTACT;
            }
            else if ((s1 == IS_RECT_CONTAINS_TRI && s2 == IS_RECT_CONTAINS_TRI) ||
                        (s1 == IS_RECT_CONTAINS_TRI && s3 == IS_RECT_CONTAINS_TRI) ||
                        (s2 == IS_RECT_CONTAINS_TRI && s3 == IS_RECT_CONTAINS_TRI))
            {
                return IS_RECT_CONTAINS_TRI;
            }
            else if ((s1 == IS_TRI_CONTAINS_RECT && s2 == IS_TRI_CONTAINS_RECT) ||
                        (s1 == IS_TRI_CONTAINS_RECT && s3 == IS_TRI_CONTAINS_RECT) ||
                        (s2 == IS_TRI_CONTAINS_RECT && s3 == IS_TRI_CONTAINS_RECT))
            {
                return IS_TRI_CONTAINS_RECT;
            }

            return IS_PENETRATION;
        }
        else
        {
            if (s1 == IS_PENETRATION || s2 == IS_PENETRATION || s3 == IS_PENETRATION)
            {
                return IS_PENETRATION;
            }

            // At this point, s1 == s2 == s3, and they are either IS_RECT_CONTAINS_TRI or IS_TRI_CONTAINS_RECT.
            return s1;
        }
    }


//-------------------------------------------------------------------
    Math::INTERSECTIONSTATE Math::AABBTriangleIntersection(const Math::AABB &box, const Vector3 *pVerts, Vector2 *pAABBResults,
                            Vector2 *pTriangleResults, bool contactTesting)
    //---------------------------------------------------------------
    {
        INTERSECTIONSTATE s1, s2, s3;

        RectangleIntersectionData riData;
        TriangleIntersectionData tiData;

        Triangle triangle(pVerts, true);

        // XY
        Rectangle2D rectXY(box, AP_XY);
        GetRectangleIntersectionData(rectXY, &riData);

        Triangle2D triXY(triangle.ProjectToAxisPlane(AP_XY));
        triXY.CalculateEdgeNormals();
        GetTriangleIntersectionData(triXY, &tiData);

        if (pAABBResults || pTriangleResults)
        {
            s1 = Math::RectangleTriangleIntersection2D(triXY, riData, tiData, pAABBResults, pTriangleResults);
        }
        else
        {
            s1 = Math::RectangleTriangleIntersection2D(triXY, riData, tiData, NULL, NULL);

            if (s1 == IS_NONE)
            {
                return IS_NONE;
            }
        }

        // XZ
        Rectangle2D rectXZ(box, AP_XZ);
        GetRectangleIntersectionData(rectXZ, &riData);

        Triangle2D triXZ(triangle.ProjectToAxisPlane(AP_XZ));
        triXZ.CalculateEdgeNormals();
        GetTriangleIntersectionData(triXZ, &tiData);

        if (pAABBResults || pTriangleResults)
        {
            s2 = Math::RectangleTriangleIntersection2D(triXZ, riData, tiData, pAABBResults + 6, pTriangleResults + 6);
        }
        else
        {
            s2 = Math::RectangleTriangleIntersection2D(triXZ, riData, tiData, NULL, NULL);

            if (s2 == IS_NONE)
            {
                return IS_NONE;
            }
        }

        // YZ
        Rectangle2D rectYZ(box, AP_YZ);
        GetRectangleIntersectionData(rectYZ, &riData);

        Triangle2D triYZ(triangle.ProjectToAxisPlane(AP_YZ));
        triYZ.CalculateEdgeNormals();
        GetTriangleIntersectionData(triYZ, &tiData);

        if (pAABBResults || pTriangleResults)
        {
            s3 = Math::RectangleTriangleIntersection2D(triYZ, riData, tiData, pAABBResults + 12, pTriangleResults + 12);
        }
        else
        {
            s3 = Math::RectangleTriangleIntersection2D(triYZ, riData, tiData, NULL, NULL);

            if (s3 == IS_NONE)
            {
                return IS_NONE;
            }
        }

        if (pAABBResults || pTriangleResults)
        {
            if (s1 == IS_NONE || s2 == IS_NONE || s3 == IS_NONE)
            {
                return IS_NONE;
            }
        }

        if (contactTesting)
        {
            if (s1 == s2 && s1 == s3)
            {
                // If they're all [A/B]_CONTAINS_[B/A], or all IS_PENETRATION or all IS_CONTACT
                return s1;
            }
            else if (s1 == IS_CONTACT || s2 == IS_CONTACT || s3 == IS_CONTACT)
            {
                // It's not possible to get only one, but where there's one, there must be another.
                return IS_CONTACT;
            }
            else if ((s1 == IS_RECT_CONTAINS_TRI && s2 == IS_RECT_CONTAINS_TRI) ||
                        (s1 == IS_RECT_CONTAINS_TRI && s3 == IS_RECT_CONTAINS_TRI) ||
                        (s2 == IS_RECT_CONTAINS_TRI && s3 == IS_RECT_CONTAINS_TRI))
            {
                return IS_AABB_CONTAINS_TRI;
            }

            return IS_PENETRATION;
        }
        else
        {
            if (s1 == IS_PENETRATION || s2 == IS_PENETRATION || s3 == IS_PENETRATION)
            {
                return IS_PENETRATION;
            }

            // At this point, s1 == s2 == s3, and must be IS_AABB_CONTAINS_TRI.
            return IS_AABB_CONTAINS_TRI;
        }
    }


//-------------------------------------------------------------------
    bool Math::AABBSphereCollision(const Math::AABB &box, const Sphere &sphere, Vector3 *pPoint)
    //---------------------------------------------------------------
    {
        /*
        Vector3 min = AABB.min;
        Vector3 max = AABB.max;

        if (AABBPointCollision(AABB, sphere.center))
            return true;

        D3DXVECTOR3 mid = min + max * 0.5f;
        D3DXVECTOR3 p0 = D3DXVECTOR3(min.x, mid.y, mid.z);
        D3DXVECTOR3 p1 = D3DXVECTOR3(mid.x, mid.y, max.z);
        D3DXVECTOR3 p2 = D3DXVECTOR3(max.x, mid.y, mid.z);
        D3DXVECTOR3 p3 = D3DXVECTOR3(mid.x, mid.y, min.z);
        D3DXVECTOR3 p4 = D3DXVECTOR3(mid.x, min.y, mid.z);
        D3DXVECTOR3 p5 = D3DXVECTOR3(mid.x, max.y, mid.z);

        Plane planes[6];
        D3DXPlaneFromPointNormal(&planes[0], &p0, &D3DXVECTOR3(1, 0, 0));
        D3DXPlaneFromPointNormal(&planes[1], &p1, &D3DXVECTOR3(0, 0, -1));
        D3DXPlaneFromPointNormal(&planes[2], &p2, &D3DXVECTOR3(-1, 0, 0));
        D3DXPlaneFromPointNormal(&planes[3], &p3, &D3DXVECTOR3(0, 0, 1));
        D3DXPlaneFromPointNormal(&planes[4], &p4, &D3DXVECTOR3(0, 1, 0));
        D3DXPlaneFromPointNormal(&planes[5], &p5, &D3DXVECTOR3(0, -1, 0));

        for (int p = 0; p < 6; p++)
        {
            float d = D3DXVec3Dot(&Vector3(planes[p].a, planes[p].b, planes[p].c), &sphere.center) + planes[p].d;

            if (d < -sphere.radius)
                return false;

            if (fabs(d) < sphere.radius)
                return true;
        }

        return true;*/

        float s = 0,
              d = 0;

        for (int i = 0; i < 3; i++)
        {
            if (sphere.center[i] < box.min[i])
            {
                s = sphere.center[i] - box.min[i];
                d += s * s;
            }
            else if (sphere.center[i] > box.max[i])
            {
                s = sphere.center[i] - box.max[i];
                d += s * s;
            }
        }

        if (d <= sphere.radius * sphere.radius)
        {
            //if (pPoint)
            //    *pPoint = sphere.centersqrt(d);
            // TODO: Calculate collision point.

            return true;
        }

        return false;
    }


// Unfinished
//-------------------------------------------------------------------
    bool Math::AABBAABBCollision(const Math::AABB &box1, const Math::AABB &box2)
    //---------------------------------------------------------------
    {
        /*
        Vector3 min1 = AABB1.min;
        Vector3 max1 = AABB1.max;

        Vector3 points[8];
        points[0] = Vector3(min1.x, min1.y, min1.z);
        points[1] = Vector3(max1.x, min1.y, min1.z);
        points[2] = Vector3(min1.x, min1.y, max1.z);
        points[3] = Vector3(max1.x, min1.y, max1.z);
        points[4] = Vector3(min1.x, max1.y, min1.z);
        points[5] = Vector3(max1.x, max1.y, min1.z);
        points[6] = Vector3(min1.x, max1.y, max1.z);
        points[7] = Vector3(max1.x, max1.y, max1.z);

        for (int i = 0; i < 8; i++)
        {
            if (AABBPointCollision(AABB2, points[i]))
                return true;
        }*/

        Vector3 e1 = box1.max - box1.min;
        Vector3 e2 = box2.max - box2.min;
        Vector3 e = e1 + e2;

        Vector3 p1 = box1.min + e1 * 0.5f;
        Vector3 p2 = box2.min + e2 * 0.5f;
        Vector3 axis = p2 - p1;

        if (fabs(axis.x) <= e.x &&
            fabs(axis.y) <= e.y &&
            fabs(axis.z) <= e.z)
        {
            /*if (pPoint)
            {
                (*pPoint).x = e1.x + e.x - (float)fabs(axis.x);
                (*pPoint).y = e1.y + e.y - (float)fabs(axis.y);
                (*pPoint).z = e1.z + e.z - (float)fabs(axis.z);
            }*/

            return true;
        }

        return false;
    }


// Unfinished
//-------------------------------------------------------------------
    bool Math::AABBRayCollision(const AABB &box, const Vector3 &position, const Vector3 &direction)
    //---------------------------------------------------------------
    {
        if (AABBPointCollision(box, position))
            return true;

        Vector3 mid = box.min + (box.max - box.min) * 0.5f;

        Plane planes[6];
        D3DXPlaneFromPointNormal(planes,     &box.min, &Vector3(-1,  0,  0));
        D3DXPlaneFromPointNormal(planes + 1, &box.min, &Vector3(0, -1,  0));
        D3DXPlaneFromPointNormal(planes + 2, &box.min, &Vector3(0,  0, -1));
        D3DXPlaneFromPointNormal(planes + 3, &box.max, &Vector3(1,  0,  0));
        D3DXPlaneFromPointNormal(planes + 4, &box.max, &Vector3(0,  1,  0));
        D3DXPlaneFromPointNormal(planes + 5, &box.max, &Vector3(0,  0,  1));

        Vector3 lineEnd = position + direction;

        Math::AABB boxes[6] =
        {
            Math::AABB(box.min + Vector3(-1, 0,  0), Vector3(box.min.x + 1, box.max.y, box.max.z)),
            Math::AABB(box.min + Vector3(0, -1,  0), Vector3(box.max.x, box.min.y + 1, box.max.z)),
            Math::AABB(box.min + Vector3(0,  0, -1), Vector3(box.max.x, box.max.y, box.min.z + 1)),
            Math::AABB(Vector3(box.max.x - 1, box.min.y, box.min.z), box.max + Vector3(1, 0, 0)),
            Math::AABB(Vector3(box.min.x, box.max.y - 1, box.min.z), box.max + Vector3(0, 1, 0)),
            Math::AABB(Vector3(box.min.x, box.min.y, box.max.z - 1), box.max + Vector3(0, 0, 1))
        };

        for (int p = 0; p < 6; p++)
        {
            Vector3 intersectionPoint(0, 0, 0);

            if (D3DXPlaneIntersectLine(&intersectionPoint, planes + p, &position, &lineEnd))
            {
                if (AABBPointCollision(boxes[p], intersectionPoint))
                {
                    Vector3 posToMid(Normalise(mid - position));
                    if (Dot(posToMid, direction) > 0.0f)
                        return true;
                }
            }
        }

        return false;
    }


//-------------------------------------------------------------------
    bool Math::AABBPointCollision(const Math::AABB &box, const Vector3 &point)
    //---------------------------------------------------------------
    {
        if (point.x >= box.min.x && point.x <= box.max.x &&
            point.y >= box.min.y && point.y <= box.max.y &&
            point.z >= box.min.z && point.z <= box.max.z)
            return true;

        return false;
    }


//-------------------------------------------------------------------
    bool Math::PointFrustumCollision(const Vector3 &point, const Plane *frustumPlanes)
    //---------------------------------------------------------------
    {
        int inCount = 0;

        for (int p = 0; p < 6; p++)
        {
            float t = frustumPlanes[p].a * point.x + frustumPlanes[p].b * point.y +
                      frustumPlanes[p].c * point.z + frustumPlanes[p].d;

            if (t >= 0)
                inCount++;
        }

        return inCount == 6;
    }


//-------------------------------------------------------------------
    bool Math::AABBFrustumCollision(const Math::AABB &box, const Plane *frustumPlanes)
    //---------------------------------------------------------------
    {
        Vector3 points[8];

        points[0] = Vector3(box.min.x, box.min.y, box.max.z);
        points[1] = Vector3(box.min.x, box.max.y, box.max.z);
        points[2] = Vector3(box.max.x, box.max.y, box.max.z);
        points[3] = Vector3(box.max.x, box.min.y, box.max.z);
        points[4] = Vector3(box.min.x, box.min.y, box.min.z);
        points[5] = Vector3(box.min.x, box.max.y, box.min.z);
        points[6] = Vector3(box.max.x, box.max.y, box.min.z);
        points[7] = Vector3(box.max.x, box.min.y, box.min.z);

        for (int p = 0; p < 6; p++)
        {
            int inCount = 0;

            for (int v = 0; v < 8; v++)
            {
                /*float a = frustumPlanes[p].normal.x;
                float b = frustumPlanes[p].normal.y;
                float c = frustumPlanes[p].normal.z;
                float d = frustumPlanes[p].normal.x;*/

                float t = frustumPlanes[p].a * points[v].x +
                          frustumPlanes[p].b * points[v].y +
                          frustumPlanes[p].c * points[v].z + frustumPlanes[p].d;

                // if on front side of plane
                if (t >= 0)
                    inCount++;
            }

            if (inCount == 0)
                return false;
        }

        return true;
    }


//-------------------------------------------------------------------
    bool Math::OBBSphereCollision(const Math::OBB &OBB, const Math::Sphere &sphere, Vector3 *pPoint)
    //---------------------------------------------------------------
    {
        /*
        Vector3 min = OBB.min;
        Vector3 max = OBB.max;

        D3DXVECTOR3 mid = min + max * 0.5f;
        D3DXVECTOR3 p0 = D3DXVECTOR3(min.x, mid.y, mid.z);
        D3DXVECTOR3 p1 = D3DXVECTOR3(mid.x, mid.y, max.z);
        D3DXVECTOR3 p2 = D3DXVECTOR3(max.x, mid.y, mid.z);
        D3DXVECTOR3 p3 = D3DXVECTOR3(mid.x, mid.y, min.z);
        D3DXVECTOR3 p4 = D3DXVECTOR3(mid.x, min.y, mid.z);
        D3DXVECTOR3 p5 = D3DXVECTOR3(mid.x, max.y, mid.z);

        Plane planes[6];
        D3DXPlaneFromPointNormal(&planes[0], &p0, &D3DXVECTOR3(1, 0, 0));
        D3DXPlaneFromPointNormal(&planes[1], &p1, &D3DXVECTOR3(0, 0, -1));
        D3DXPlaneFromPointNormal(&planes[2], &p2, &D3DXVECTOR3(-1, 0, 0));
        D3DXPlaneFromPointNormal(&planes[3], &p3, &D3DXVECTOR3(0, 0, 1));
        D3DXPlaneFromPointNormal(&planes[4], &p4, &D3DXVECTOR3(0, 1, 0));
        D3DXPlaneFromPointNormal(&planes[5], &p5, &D3DXVECTOR3(0, -1, 0));

        D3DXPlaneTransform(&planes[0], &planes[0], &OBB.orientation);
        D3DXPlaneTransform(&planes[1], &planes[1], &OBB.orientation);
        D3DXPlaneTransform(&planes[2], &planes[2], &OBB.orientation);
        D3DXPlaneTransform(&planes[3], &planes[3], &OBB.orientation);
        D3DXPlaneTransform(&planes[4], &planes[4], &OBB.orientation);
        D3DXPlaneTransform(&planes[5], &planes[5], &OBB.orientation);

        for (int p = 0; p < 6; p++)
        {
            float d = D3DXVec3Dot(&Vector3(planes[p].a, planes[p].b, planes[p].c), &sphere.center) + planes[p].d;

            if (d < -sphere.radius)
                return false;

            if (fabs(d) < sphere.radius)
            {
                if (pPoint)
                    *pPoint = sphere.center;

                return true;
            }
        }

        if (pPoint)
            *pPoint = sphere.center;

        return true;*/

        Math::AABB localBox(OBB.min, OBB.max);
        Math::Sphere localSphere(sphere.center, sphere.radius);

        return Math::AABBSphereCollision(localBox, localSphere, pPoint);
    }


//-------------------------------------------------------------------
    bool Math::OBBOBBCollision(const Math::OBB &OBB1, const Math::OBB &OBB2, Vector3 *pPoint)
    //---------------------------------------------------------------
    {
        Vector3 a = (OBB1.min + OBB1.max/2);
        Vector3 b = (OBB2.min + OBB2.max/2);
        Vector3 s;

        Matrix ao, bo;
        D3DXMatrixRotationYawPitchRoll(&ao, D3DXToRadian(OBB1.orientation.y), D3DXToRadian(OBB1.orientation.x), D3DXToRadian(OBB1.orientation.z));
        D3DXMatrixRotationYawPitchRoll(&bo, D3DXToRadian(OBB2.orientation.y), D3DXToRadian(OBB2.orientation.x), D3DXToRadian(OBB2.orientation.z));

        Vector3 axisA[3];
        Vector3 axisB[3];

        D3DXVec3TransformCoord(&axisA[0], &Vector3(1, 0, 0), &ao);
        D3DXVec3TransformCoord(&axisA[1], &Vector3(0, 1, 0), &ao);
        D3DXVec3TransformCoord(&axisA[2], &Vector3(0, 0, 1), &ao);
        D3DXVec3TransformCoord(&axisB[0], &Vector3(1, 0, 0), &bo);
        D3DXVec3TransformCoord(&axisB[1], &Vector3(0, 1, 0), &bo);
        D3DXVec3TransformCoord(&axisB[2], &Vector3(0, 0, 1), &bo);

        Vector3 vab = a + b;
        vab = Vector3(Dot(vab, axisA[0]),
                      Dot(vab, axisA[1]),
                      Dot(vab, axisA[2]));

        // 6 Principle Axes
            for (int i = 0; i < 3; i++)
            {
                float ra = (OBB1.max[i] - OBB1.min[i]) * 0.5f;
                float rb = b[0] * bo(i, 0) +
                           b[1] * bo(i, 1) +
                           b[2] * bo(i, 2);

                if (fabs(vab[i]) > ra + rb)
                    return false;
            }

            for (i = 0; i < 3; i++)
            {
                float ra = 0;
                float rb = (OBB2.max[i] - OBB2.min[i]) * 0.5f;

                //if (fabs(vab[i]) > ra + rb)
                //    return false;
            }

        // 9 Cross Axes

        return true;
    }


//-------------------------------------------------------------------
    bool Math::OBBPolygonCollision(const Math::OBB &OBB, const Math::Polygon &polygon, Vector3 *pPoint)
    //---------------------------------------------------------------
    {
        Vector3 min = OBB.min;
        Vector3 max = OBB.max;

        Matrix orientation;
        D3DXMatrixRotationYawPitchRoll(&orientation, OBB.orientation.y, OBB.orientation.x, OBB.orientation.z);

        D3DXVECTOR3 mid = min + max * 0.5f;

        Plane planes[6];
        D3DXVECTOR3 p0 = D3DXVECTOR3(min.x, mid.y, mid.z);
        D3DXVECTOR3 p1 = D3DXVECTOR3(mid.x, mid.y, max.z);
        D3DXVECTOR3 p2 = D3DXVECTOR3(max.x, mid.y, mid.z);
        D3DXVECTOR3 p3 = D3DXVECTOR3(mid.x, mid.y, min.z);
        D3DXVECTOR3 p4 = D3DXVECTOR3(mid.x, min.y, mid.z);
        D3DXVECTOR3 p5 = D3DXVECTOR3(mid.x, max.y, mid.z);

        D3DXPlaneFromPointNormal(&planes[0], &p0, &D3DXVECTOR3(1, 0, 0));
        D3DXPlaneFromPointNormal(&planes[1], &p1, &D3DXVECTOR3(0, 0, -1));
        D3DXPlaneFromPointNormal(&planes[2], &p2, &D3DXVECTOR3(-1, 0, 0));
        D3DXPlaneFromPointNormal(&planes[3], &p3, &D3DXVECTOR3(0, 0, 1));
        D3DXPlaneFromPointNormal(&planes[4], &p4, &D3DXVECTOR3(0, 1, 0));
        D3DXPlaneFromPointNormal(&planes[5], &p5, &D3DXVECTOR3(0, -1, 0));

        D3DXPlaneTransform(&planes[0], &planes[0], &orientation);
        D3DXPlaneTransform(&planes[1], &planes[1], &orientation);
        D3DXPlaneTransform(&planes[2], &planes[2], &orientation);
        D3DXPlaneTransform(&planes[3], &planes[3], &orientation);
        D3DXPlaneTransform(&planes[4], &planes[4], &orientation);
        D3DXPlaneTransform(&planes[5], &planes[5], &orientation);

        for (int p = 0; p < 6; p++)
        {
            int inCount = 0;

            for (int v = 0; v < polygon.numVerts; v++)
            {
                float d = D3DXPlaneDotCoord(&planes[p], &polygon.vertices[v]);

                if (d >= 0)
                    inCount++;
            }

            if (inCount == 0)
                return false;
        }

        if (pPoint)
            *pPoint = mid;

        return true;
    }


/*
//-------------------------------------------------------------------
    bool SphereSphereCollision(Vector3 center1, Vector3 center2, float radius1, float radius2)
    //---------------------------------------------------------------
    {
        float length = D3DXVec3Length(&Vector3(center1 - center2));

        if (length <= radius1 + radius2)
            return true;

        return false;
    }


//-------------------------------------------------------------------
    bool SpherePlaneCollision(Vector3 center, Vector3 normal, Vector3 point, float radius, float *pDistance)
    //---------------------------------------------------------------
    {
        float planeDistance = PlaneDistance(normal, point);
        float distance = (normal.x * center.x + normal.y * center.y + normal.z * center.z + planeDistance);

        if (pDistance)
            *pDistance = distance;

        if (fabs(distance) < radius)
            return true;

        return false;
    }


//-------------------------------------------------------------------
    bool SpherePointCollision(Vector3 center, Vector3 point, float radius)
    //---------------------------------------------------------------
    {
        float length = D3DXVec3Length(&Vector3(center - point));

        if (length <= radius)
            return true;

        return false;
    }


//-------------------------------------------------------------------
    bool PolygonSphereCollision(Vector3 center, float radius, Vector3 *vertices, Vector3 normal, int numVerts, Vector3 *pPoint)
    //---------------------------------------------------------------
    {
        if (!vertices || !numVerts || numVerts < 3)
            return false;

        float distance = 0;
        if (SpherePlaneCollision(center, normal, vertices[0], radius, &distance))
        {
            Vector3 point = center - (normal * distance);

            if (pPoint)
                *pPoint = point;

            if (PointInsidePolygon(point, vertices, numVerts))
                return true;
            else if (SphereEdgeCollision(center, vertices, numVerts, radius))
                return true;
        }

        return false;
    }


//-------------------------------------------------------------------
    bool AABBPolygonCollision(Vector3 min, Vector3 max, Vector3 *vertices, int numVerts, Vector3 *pPoint)
    //---------------------------------------------------------------
    {
        D3DXVECTOR3 mid = min + max * 0.5f;

        Plane planes[6];
        D3DXVECTOR3 p0 = D3DXVECTOR3(min.x, mid.y, mid.z);
        D3DXVECTOR3 p1 = D3DXVECTOR3(mid.x, mid.y, max.z);
        D3DXVECTOR3 p2 = D3DXVECTOR3(max.x, mid.y, mid.z);
        D3DXVECTOR3 p3 = D3DXVECTOR3(mid.x, mid.y, min.z);
        D3DXVECTOR3 p4 = D3DXVECTOR3(mid.x, min.y, mid.z);
        D3DXVECTOR3 p5 = D3DXVECTOR3(mid.x, max.y, mid.z);

        D3DXPlaneFromPointNormal(&planes[0], &p0, &D3DXVECTOR3(1, 0, 0));
        D3DXPlaneFromPointNormal(&planes[1], &p1, &D3DXVECTOR3(0, 0, -1));
        D3DXPlaneFromPointNormal(&planes[2], &p2, &D3DXVECTOR3(-1, 0, 0));
        D3DXPlaneFromPointNormal(&planes[3], &p3, &D3DXVECTOR3(0, 0, 1));
        D3DXPlaneFromPointNormal(&planes[4], &p4, &D3DXVECTOR3(0, 1, 0));
        D3DXPlaneFromPointNormal(&planes[5], &p5, &D3DXVECTOR3(0, -1, 0));

        for (int p = 0; p < 6; p++)
        {
            int inCount = 0;

            for (int v = 0; v < numVerts; v++)
            {
                float d = D3DXPlaneDotCoord(&planes[p], &vertices[v]);

                if (d >= 0)
                    inCount++;
            }

            if (inCount == 0)
                return false;
        }

        return true;
    }


//-------------------------------------------------------------------
    bool AABBSphereCollision(Vector3 min, Vector3 max, Vector3 center, float radius, Vector3 *pPoint)
    //---------------------------------------------------------------
    {
        if (AABBPointCollision(min, max, center))
            return true;

        D3DXVECTOR3 mid = min + max * 0.5f;
        D3DXVECTOR3 p0 = D3DXVECTOR3(min.x, mid.y, mid.z);
        D3DXVECTOR3 p1 = D3DXVECTOR3(mid.x, mid.y, max.z);
        D3DXVECTOR3 p2 = D3DXVECTOR3(max.x, mid.y, mid.z);
        D3DXVECTOR3 p3 = D3DXVECTOR3(mid.x, mid.y, min.z);
        D3DXVECTOR3 p4 = D3DXVECTOR3(mid.x, min.y, mid.z);
        D3DXVECTOR3 p5 = D3DXVECTOR3(mid.x, max.y, mid.z);

        Plane planes[6];
        D3DXPlaneFromPointNormal(&planes[0], &p0, &D3DXVECTOR3(1, 0, 0));
        D3DXPlaneFromPointNormal(&planes[1], &p1, &D3DXVECTOR3(0, 0, -1));
        D3DXPlaneFromPointNormal(&planes[2], &p2, &D3DXVECTOR3(-1, 0, 0));
        D3DXPlaneFromPointNormal(&planes[3], &p3, &D3DXVECTOR3(0, 0, 1));
        D3DXPlaneFromPointNormal(&planes[4], &p4, &D3DXVECTOR3(0, 1, 0));
        D3DXPlaneFromPointNormal(&planes[5], &p5, &D3DXVECTOR3(0, -1, 0));

        for (int p = 0; p < 6; p++)
        {
            float d = D3DXVec3Dot(&Vector3(planes[p].a, planes[p].b, planes[p].c), &center) + planes[p].d;

            if (d < -radius)
                return false;

            if (fabs(d) < radius)
                return true;
        }

        return true;

        Vector3 points[8];
        points[0] = Vector3(min.x, min.y, min.z);
        points[1] = Vector3(max.x, min.y, min.z);
        points[2] = Vector3(min.x, min.y, max.z);
        points[3] = Vector3(max.x, min.y, max.z);
        points[4] = Vector3(min.x, max.y, min.z);
        points[5] = Vector3(max.x, max.y, min.z);
        points[6] = Vector3(min.x, max.y, max.z);
        points[7] = Vector3(max.x, max.y, max.z);

        for (int i = 0; i < 8; i++)
        {
            if (SpherePointCollision(center, points[i], radius))
                return true;
        }

        // Sphere/Face testing below can be heavily optimized. w/planes?
        for (i = 0; i < 6; i++)
        {
            Vector3 faceVerts[4];

            switch (i)
            {
            case 0: faceVerts[0] = points[0];
                    faceVerts[1] = points[1];
                    faceVerts[2] = points[2];
                    faceVerts[3] = points[3];
                    break;
            case 1: faceVerts[0] = points[4];
                    faceVerts[1] = points[5];
                    faceVerts[2] = points[6];
                    faceVerts[3] = points[7];
                    break;
            case 2: faceVerts[0] = points[0];
                    faceVerts[1] = points[1];
                    faceVerts[2] = points[4];
                    faceVerts[3] = points[5];
                    break;
            case 3: faceVerts[0] = points[2];
                    faceVerts[1] = points[3];
                    faceVerts[2] = points[6];
                    faceVerts[3] = points[7];
                    break;
            case 4: faceVerts[0] = points[0];
                    faceVerts[1] = points[2];
                    faceVerts[2] = points[4];
                    faceVerts[3] = points[5];
                    break;
            case 5: faceVerts[0] = points[1];
                    faceVerts[1] = points[3];
                    faceVerts[2] = points[4];
                    faceVerts[3] = points[7];
                    break;
            default: continue;
            }

            Vector3 normal = Normalise(Cross(faceVerts[1] - faceVerts[0], faceVerts[2] - faceVerts[0]));

            if (PolygonSphereCollision(center, radius, faceVerts, normal, 4, pPoint))
                return true;
        }

        return false;
    }


//-------------------------------------------------------------------
    bool AABBAABBCollision(Vector3 min1, Vector3 max1, Vector3 min2, Vector3 max2, Vector3 *pPoint)
    //---------------------------------------------------------------
    {
        Vector3 points[8];
        points[0] = Vector3(min1.x, min1.y, min1.z);
        points[1] = Vector3(max1.x, min1.y, min1.z);
        points[2] = Vector3(min1.x, min1.y, max1.z);
        points[3] = Vector3(max1.x, min1.y, max1.z);
        points[4] = Vector3(min1.x, max1.y, min1.z);
        points[5] = Vector3(max1.x, max1.y, min1.z);
        points[6] = Vector3(min1.x, max1.y, max1.z);
        points[7] = Vector3(max1.x, max1.y, max1.z);

        for (int i = 0; i < 8; i++)
        {
            if (AABBPointCollision(min2, max2, points[i]))
                return true;
        }

        return false;
    }


//-------------------------------------------------------------------
    bool AABBPointCollision(Vector3 min, Vector3 max, Vector3 point)
    //---------------------------------------------------------------
    {
        if (point.x >= min.x && point.x <= max.x &&
            point.y >= min.y && point.y <= max.y &&
            point.z >= min.z && point.z <= max.z)
            return true;

        return false;
    }


//-------------------------------------------------------------------
    bool OBBSphereCollision(Vector3 min, Vector3 max, Vector3 center, float radius, Matrix orientation, Vector3 *pPoint)
    //---------------------------------------------------------------
    {
        D3DXVECTOR3 mid = min + max * 0.5f;
        D3DXVECTOR3 p0 = D3DXVECTOR3(min.x, mid.y, mid.z);
        D3DXVECTOR3 p1 = D3DXVECTOR3(mid.x, mid.y, max.z);
        D3DXVECTOR3 p2 = D3DXVECTOR3(max.x, mid.y, mid.z);
        D3DXVECTOR3 p3 = D3DXVECTOR3(mid.x, mid.y, min.z);
        D3DXVECTOR3 p4 = D3DXVECTOR3(mid.x, min.y, mid.z);
        D3DXVECTOR3 p5 = D3DXVECTOR3(mid.x, max.y, mid.z);

        Plane planes[6];
        D3DXPlaneFromPointNormal(&planes[0], &p0, &D3DXVECTOR3(1, 0, 0));
        D3DXPlaneFromPointNormal(&planes[1], &p1, &D3DXVECTOR3(0, 0, -1));
        D3DXPlaneFromPointNormal(&planes[2], &p2, &D3DXVECTOR3(-1, 0, 0));
        D3DXPlaneFromPointNormal(&planes[3], &p3, &D3DXVECTOR3(0, 0, 1));
        D3DXPlaneFromPointNormal(&planes[4], &p4, &D3DXVECTOR3(0, 1, 0));
        D3DXPlaneFromPointNormal(&planes[5], &p5, &D3DXVECTOR3(0, -1, 0));

        D3DXPlaneTransform(&planes[0], &planes[0], &orientation);
        D3DXPlaneTransform(&planes[1], &planes[1], &orientation);
        D3DXPlaneTransform(&planes[2], &planes[2], &orientation);
        D3DXPlaneTransform(&planes[3], &planes[3], &orientation);
        D3DXPlaneTransform(&planes[4], &planes[4], &orientation);
        D3DXPlaneTransform(&planes[5], &planes[5], &orientation);

        for (int p = 0; p < 6; p++)
        {
            float d = D3DXVec3Dot(&Vector3(planes[p].a, planes[p].b, planes[p].c), &center) + planes[p].d;

            if (d < -radius)
                return false;

            if (fabs(d) < radius)
            {
                if (pPoint)
                    *pPoint = center;

                return true;
            }
        }

        if (pPoint)
            *pPoint = center;

        return true;
    }


//-------------------------------------------------------------------
    bool OBBPolygonCollision(Vector3 min, Vector3 max, Vector3 *vertices, Vector3 normal, int numVerts,
                             Matrix orientation, Vector3 *pPoint)
    //---------------------------------------------------------------
    {
        D3DXVECTOR3 mid = min + max * 0.5f;

        Plane planes[6];
        D3DXVECTOR3 p0 = D3DXVECTOR3(min.x, mid.y, mid.z);
        D3DXVECTOR3 p1 = D3DXVECTOR3(mid.x, mid.y, max.z);
        D3DXVECTOR3 p2 = D3DXVECTOR3(max.x, mid.y, mid.z);
        D3DXVECTOR3 p3 = D3DXVECTOR3(mid.x, mid.y, min.z);
        D3DXVECTOR3 p4 = D3DXVECTOR3(mid.x, min.y, mid.z);
        D3DXVECTOR3 p5 = D3DXVECTOR3(mid.x, max.y, mid.z);

        D3DXPlaneFromPointNormal(&planes[0], &p0, &D3DXVECTOR3(1, 0, 0));
        D3DXPlaneFromPointNormal(&planes[1], &p1, &D3DXVECTOR3(0, 0, -1));
        D3DXPlaneFromPointNormal(&planes[2], &p2, &D3DXVECTOR3(-1, 0, 0));
        D3DXPlaneFromPointNormal(&planes[3], &p3, &D3DXVECTOR3(0, 0, 1));
        D3DXPlaneFromPointNormal(&planes[4], &p4, &D3DXVECTOR3(0, 1, 0));
        D3DXPlaneFromPointNormal(&planes[5], &p5, &D3DXVECTOR3(0, -1, 0));

        D3DXPlaneTransform(&planes[0], &planes[0], &orientation);
        D3DXPlaneTransform(&planes[1], &planes[1], &orientation);
        D3DXPlaneTransform(&planes[2], &planes[2], &orientation);
        D3DXPlaneTransform(&planes[3], &planes[3], &orientation);
        D3DXPlaneTransform(&planes[4], &planes[4], &orientation);
        D3DXPlaneTransform(&planes[5], &planes[5], &orientation);

        for (int p = 0; p < 6; p++)
        {
            int inCount = 0;

            for (int v = 0; v < numVerts; v++)
            {
                float d = D3DXPlaneDotCoord(&planes[p], &vertices[v]);

                if (d >= 0)
                    inCount++;
            }

            if (inCount == 0)
                return false;
        }

        if (pPoint)
            *pPoint = mid;

        return true;
    }
*/


//-------------------------------------------------------------------
    bool Math::SimpleIntersectionOnAxis(const Math::RectangleIntersectionData &riData, const Math::TriangleIntersectionData &tiData,
                                        const Vector2 *pTriangleEdgeNormal)
    //---------------------------------------------------------------
    {
        const Vector2 &nEdgeNormal = *pTriangleEdgeNormal;

        Vector2 triResults[4];
        float triLengths[4];
        bool triForwards[3];

        triResults[0] = tiData.centroid;
        triResults[1] = nEdgeNormal * Math::Dot(tiData.vertices[0], nEdgeNormal);    // Partial solution for the results.
        triResults[2] = nEdgeNormal * Math::Dot(tiData.vertices[1], nEdgeNormal);    // These are the projected vectors from the centroid
        triResults[3] = nEdgeNormal * Math::Dot(tiData.vertices[2], nEdgeNormal);    // to each of the vertices of the triangle.

        triLengths[0] = 0;
        triLengths[1] = Math::Magnitude(triResults[1]);
        triLengths[2] = Math::Magnitude(triResults[2]);
        triLengths[3] = Math::Magnitude(triResults[3]);

        triForwards[0] = Math::Dot(triResults+1, pTriangleEdgeNormal) > 0;
        triForwards[1] = Math::Dot(triResults+2, pTriangleEdgeNormal) > 0;
        triForwards[2] = Math::Dot(triResults+3, pTriangleEdgeNormal) > 0;

        triResults[1] += tiData.centroid;
        triResults[2] += tiData.centroid;
        triResults[3] += tiData.centroid;

        int tForwardResult = 0,
            tBackwardResult = 0;

        int n = 0;
        for (n = 0; n < 3; ++n)
        {
            if (triForwards[n])
            {
                if (triLengths[n+1] > triLengths[tForwardResult])
                    tForwardResult = n+1;
            }
            else
            {
                if (triLengths[n+1] > triLengths[tBackwardResult])
                    tBackwardResult = n+1;
            }
        }

        Vector2 rectResults[5];
        float rectLengths[5];
        bool rectForwards[4];

        rectResults[0] = nEdgeNormal * Math::Dot(riData.centroid - tiData.centroid, nEdgeNormal) + tiData.centroid;
        rectResults[1] = nEdgeNormal * Math::Dot(riData.vertices[0] - rectResults[0], nEdgeNormal);
        rectResults[2] = nEdgeNormal * Math::Dot(riData.vertices[1] - rectResults[0], nEdgeNormal);
        rectResults[3] = nEdgeNormal * Math::Dot(riData.vertices[2] - rectResults[0], nEdgeNormal);
        rectResults[4] = nEdgeNormal * Math::Dot(riData.vertices[3] - rectResults[0], nEdgeNormal);

        rectLengths[0] = 0;
        rectLengths[1] = Math::Magnitude(rectResults[1]);
        rectLengths[2] = Math::Magnitude(rectResults[2]);
        rectLengths[3] = Math::Magnitude(rectResults[3]);
        rectLengths[4] = Math::Magnitude(rectResults[4]);

        rectForwards[0] = Math::Dot(rectResults+1, pTriangleEdgeNormal) > 0;
        rectForwards[1] = Math::Dot(rectResults+2, pTriangleEdgeNormal) > 0;
        rectForwards[2] = Math::Dot(rectResults+3, pTriangleEdgeNormal) > 0;
        rectForwards[3] = Math::Dot(rectResults+4, pTriangleEdgeNormal) > 0;

        rectResults[1] += rectResults[0];
        rectResults[2] += rectResults[0];
        rectResults[3] += rectResults[0];
        rectResults[4] += rectResults[0];

        int rForwardResult = 0,
            rBackwardResult = 0;

        for (n = 0; n < 4; ++n)
        {
            if (rectForwards[n])
            {
                if (rectLengths[n+1] > rectLengths[rForwardResult])
                    rForwardResult = n+1;
            }
            else
            {
                if (rectLengths[n+1] > rectLengths[rBackwardResult])
                    rBackwardResult = n+1;
            }
        }

        //*
        Vector2 triCentre((triResults[tForwardResult].x + triResults[tBackwardResult].x) * 0.5f,
                            (triResults[tForwardResult].y + triResults[tBackwardResult].y) * 0.5f);

        Vector2 rectCentre((rectResults[rForwardResult].x + rectResults[rBackwardResult].x) * 0.5f,
                            (rectResults[rForwardResult].y + rectResults[rBackwardResult].y) * 0.5f);

        float doubleDistance = Math::Magnitude(triCentre - rectCentre) * 2.0f;
        float triDiameter = triLengths[tForwardResult] + triLengths[tBackwardResult];
        float rectDiameter = rectLengths[rForwardResult] + rectLengths[rBackwardResult];

        if (doubleDistance <= triDiameter + rectDiameter)
        {
            return true;
        }
        /*/

        // This is less efficient, but more intuitive. Remains here for illustration purposes only.

        Vector3 a = Math::Normalise(triResults[tForwardResult] - rectResults[rForwardResult]);
        Vector3 b = Math::Normalise(triResults[tForwardResult] - rectResults[rBackwardResult]);

        if (!Math::Equals(Dot(a, b), 1.0f))
        {
            return true;
        }

        Vector3 c = Math::Normalise(triResults[tBackwardResult] - rectResults[rForwardResult]);

        if (!Math::Equals(Dot(a, c), 1.0f))
        {
            return true;
        }

        Vector3 d = Math::Normalise(triResults[tBackwardResult] - rectResults[rBackwardResult]);

        if (!Math::Equals(Dot(c, d), 1.0f))
        {
            return true;
        }
        //*/

        return false;
    }


//-------------------------------------------------------------------
    bool Math::SimpleRectangleTriangleIntersection2D(const Math::Triangle2D &triangle,
                            const Math::RectangleIntersectionData &riData, const Math::TriangleIntersectionData &tiData)
    //---------------------------------------------------------------
    {
        if (!Math::SimpleIntersectionOnAxis(riData, tiData, triangle.GetEdgeNormal(0)))
        {
            return false;
        }

        if (!Math::SimpleIntersectionOnAxis(riData, tiData, triangle.GetEdgeNormal(1)))
        {
            return false;
        }

        if (!Math::SimpleIntersectionOnAxis(riData, tiData, triangle.GetEdgeNormal(2)))
        {
            return false;
        }

        return true;
    }


//-------------------------------------------------------------------
    bool Math::SimpleAABBTriangleIntersection(const AABB &box, const Vector3 *pVerts)
    //---------------------------------------------------------------
    {
        // Triangle's min x
            float minTriX = pVerts[0].x;

            if (pVerts[1].x < minTriX)
            {
                minTriX = pVerts[1].x;
            }

            if (pVerts[2].x < minTriX)
            {
                minTriX = pVerts[2].x;
            }

        // Triangle's max x
            float maxTriX = pVerts[0].x;

            if (pVerts[1].x > maxTriX)
            {
                maxTriX = pVerts[1].x;
            }

            if (pVerts[2].x > maxTriX)
            {
                maxTriX = pVerts[2].x;
            }

        if (minTriX < box.min.x)
        {
            if (maxTriX < box.min.x)
            {
                return false;
            }
        }
        else if (maxTriX > box.max.x)
        {
            if (minTriX > box.max.x)
            {
                return false;
            }
        }

        // Triangle's min y
            float minTriY = pVerts[0].y;

            if (pVerts[1].y < minTriY)
            {
                minTriY = pVerts[1].y;
            }

            if (pVerts[2].y < minTriY)
            {
                minTriY = pVerts[2].y;
            }

        // Triangle's max y
            float maxTriY = pVerts[0].y;

            if (pVerts[1].y > maxTriY)
            {
                maxTriY = pVerts[1].y;
            }

            if (pVerts[2].y > maxTriY)
            {
                maxTriY = pVerts[2].y;
            }

        if (minTriY < box.min.y)
        {
            if (maxTriY < box.min.y)
            {
                return false;
            }
        }
        else if (maxTriY > box.max.y)
        {
            if (minTriY > box.max.y)
            {
                return false;
            }
        }

        // Triangle's min z
            float minTriZ = pVerts[0].z;

            if (pVerts[1].z < minTriZ)
            {
                minTriZ = pVerts[1].z;
            }

            if (pVerts[2].z < minTriZ)
            {
                minTriZ = pVerts[2].z;
            }

        // Triangle's max z
            float maxTriZ = pVerts[0].z;

            if (pVerts[1].z > maxTriZ)
            {
                maxTriZ = pVerts[1].z;
            }

            if (pVerts[2].z > maxTriZ)
            {
                maxTriZ = pVerts[2].z;
            }

        if (minTriZ < box.min.z)
        {
            if (maxTriZ < box.min.z)
            {
                return false;
            }
        }
        else if (maxTriZ > box.max.z)
        {
            if (minTriZ > box.max.z)
            {
                return false;
            }
        }

        RectangleIntersectionData riData;
        TriangleIntersectionData tiData;

        Triangle triangle(pVerts, true);

        // XY
        Rectangle2D rectXY(box, AP_XY);
        GetRectangleIntersectionData(rectXY, &riData);

        Triangle2D triXY(triangle.ProjectToAxisPlane(AP_XY));
        triXY.CalculateEdgeNormals();
        GetTriangleIntersectionData(triXY, &tiData);

        if (!Math::SimpleRectangleTriangleIntersection2D(triXY, riData, tiData))
        {
            return false;
        }

        // XZ
        Rectangle2D rectXZ(box, AP_XZ);
        GetRectangleIntersectionData(rectXZ, &riData);

        Triangle2D triXZ(triangle.ProjectToAxisPlane(AP_XZ));
        triXZ.CalculateEdgeNormals();
        GetTriangleIntersectionData(triXZ, &tiData);

        if (!Math::SimpleRectangleTriangleIntersection2D(triXZ, riData, tiData))
        {
            return false;
        }

        // YZ
        Rectangle2D rectYZ(box, AP_YZ);
        GetRectangleIntersectionData(rectYZ, &riData);

        Triangle2D triYZ(triangle.ProjectToAxisPlane(AP_YZ));
        triYZ.CalculateEdgeNormals();
        GetTriangleIntersectionData(triYZ, &tiData);

        if (!Math::SimpleRectangleTriangleIntersection2D(triYZ, riData, tiData))
        {
            return false;
        }

        return true;
    }


//-------------------------------------------------------------------
    bool Math::SimpleAABBTriangleIntersectionXY(const AABB &box, const Vector3 *pVerts)
    //---------------------------------------------------------------
    {
        // Triangle's min x
            float minTriX = pVerts[0].x;

            if (pVerts[1].x < minTriX)
            {
                minTriX = pVerts[1].x;
            }

            if (pVerts[2].x < minTriX)
            {
                minTriX = pVerts[2].x;
            }

        // Triangle's max x
            float maxTriX = pVerts[0].x;

            if (pVerts[1].x > maxTriX)
            {
                maxTriX = pVerts[1].x;
            }

            if (pVerts[2].x > maxTriX)
            {
                maxTriX = pVerts[2].x;
            }

        if (minTriX < box.min.x)
        {
            if (maxTriX < box.min.x)
            {
                return false;
            }
        }
        else if (maxTriX > box.max.x)
        {
            if (minTriX > box.max.x)
            {
                return false;
            }
        }

        // Triangle's min y
            float minTriY = pVerts[0].y;

            if (pVerts[1].y < minTriY)
            {
                minTriY = pVerts[1].y;
            }

            if (pVerts[2].y < minTriY)
            {
                minTriY = pVerts[2].y;
            }

        // Triangle's max y
            float maxTriY = pVerts[0].y;

            if (pVerts[1].y > maxTriY)
            {
                maxTriY = pVerts[1].y;
            }

            if (pVerts[2].y > maxTriY)
            {
                maxTriY = pVerts[2].y;
            }

        if (minTriY < box.min.y)
        {
            if (maxTriY < box.min.y)
            {
                return false;
            }
        }
        else if (maxTriY > box.max.y)
        {
            if (minTriY > box.max.y)
            {
                return false;
            }
        }

        RectangleIntersectionData riData;
        TriangleIntersectionData tiData;

        Triangle triangle(pVerts, true);

        // XY
        Rectangle2D rectXY(box, AP_XY);
        GetRectangleIntersectionData(rectXY, &riData);

        Triangle2D triXY(triangle.ProjectToAxisPlane(AP_XY));
        triXY.CalculateEdgeNormals();
        GetTriangleIntersectionData(triXY, &tiData);

        if (!Math::SimpleRectangleTriangleIntersection2D(triXY, riData, tiData))
        {
            return false;
        }

        return true;
    }


//-------------------------------------------------------------------
    bool Math::SimpleAABBTriangleIntersectionXZ(const AABB &box, const Vector3 *pVerts)
    //---------------------------------------------------------------
    {
        // Triangle's min x
            float minTriX = pVerts[0].x;

            if (pVerts[1].x < minTriX)
            {
                minTriX = pVerts[1].x;
            }

            if (pVerts[2].x < minTriX)
            {
                minTriX = pVerts[2].x;
            }

        // Triangle's max x
            float maxTriX = pVerts[0].x;

            if (pVerts[1].x > maxTriX)
            {
                maxTriX = pVerts[1].x;
            }

            if (pVerts[2].x > maxTriX)
            {
                maxTriX = pVerts[2].x;
            }

        if (minTriX < box.min.x)
        {
            if (maxTriX < box.min.x)
            {
                return false;
            }
        }
        else if (maxTriX > box.max.x)
        {
            if (minTriX > box.max.x)
            {
                return false;
            }
        }

        // Triangle's min z
            float minTriZ = pVerts[0].z;

            if (pVerts[1].z < minTriZ)
            {
                minTriZ = pVerts[1].z;
            }

            if (pVerts[2].z < minTriZ)
            {
                minTriZ = pVerts[2].z;
            }

        // Triangle's max z
            float maxTriZ = pVerts[0].z;

            if (pVerts[1].z > maxTriZ)
            {
                maxTriZ = pVerts[1].z;
            }

            if (pVerts[2].z > maxTriZ)
            {
                maxTriZ = pVerts[2].z;
            }

        if (minTriZ < box.min.z)
        {
            if (maxTriZ < box.min.z)
            {
                return false;
            }
        }
        else if (maxTriZ > box.max.z)
        {
            if (minTriZ > box.max.z)
            {
                return false;
            }
        }

        RectangleIntersectionData riData;
        TriangleIntersectionData tiData;

        Triangle triangle(pVerts, true);

        // XZ
        Rectangle2D rectXZ(box, AP_XZ);
        GetRectangleIntersectionData(rectXZ, &riData);

        Triangle2D triXZ(triangle.ProjectToAxisPlane(AP_XZ));
        triXZ.CalculateEdgeNormals();
        GetTriangleIntersectionData(triXZ, &tiData);

        if (!Math::SimpleRectangleTriangleIntersection2D(triXZ, riData, tiData))
        {
            return false;
        }

        return true;
    }


//-------------------------------------------------------------------
    bool Math::SimpleAABBTriangleIntersectionYZ(const AABB &box, const Vector3 *pVerts)
    //---------------------------------------------------------------
    {
        // Triangle's min y
            float minTriY = pVerts[0].y;

            if (pVerts[1].y < minTriY)
            {
                minTriY = pVerts[1].y;
            }

            if (pVerts[2].y < minTriY)
            {
                minTriY = pVerts[2].y;
            }

        // Triangle's max y
            float maxTriY = pVerts[0].y;

            if (pVerts[1].y > maxTriY)
            {
                maxTriY = pVerts[1].y;
            }

            if (pVerts[2].y > maxTriY)
            {
                maxTriY = pVerts[2].y;
            }

        if (minTriY < box.min.y)
        {
            if (maxTriY < box.min.y)
            {
                return false;
            }
        }
        else if (maxTriY > box.max.y)
        {
            if (minTriY > box.max.y)
            {
                return false;
            }
        }

        // Triangle's min z
            float minTriZ = pVerts[0].z;

            if (pVerts[1].z < minTriZ)
            {
                minTriZ = pVerts[1].z;
            }

            if (pVerts[2].z < minTriZ)
            {
                minTriZ = pVerts[2].z;
            }

        // Triangle's max z
            float maxTriZ = pVerts[0].z;

            if (pVerts[1].z > maxTriZ)
            {
                maxTriZ = pVerts[1].z;
            }

            if (pVerts[2].z > maxTriZ)
            {
                maxTriZ = pVerts[2].z;
            }

        if (minTriZ < box.min.z)
        {
            if (maxTriZ < box.min.z)
            {
                return false;
            }
        }
        else if (maxTriZ > box.max.z)
        {
            if (minTriZ > box.max.z)
            {
                return false;
            }
        }

        RectangleIntersectionData riData;
        TriangleIntersectionData tiData;

        Triangle triangle(pVerts, true);

        // YZ
        Rectangle2D rectYZ(box, AP_YZ);
        GetRectangleIntersectionData(rectYZ, &riData);

        Triangle2D triYZ(triangle.ProjectToAxisPlane(AP_YZ));
        triYZ.CalculateEdgeNormals();
        GetTriangleIntersectionData(triYZ, &tiData);

        if (!Math::SimpleRectangleTriangleIntersection2D(triYZ, riData, tiData))
        {
            return false;
        }

        return true;
    }


#if 0
//-------------------------------------------------------------------
    bool Math::SimpleAABBTriangleIntersection(const AABB &box, const Vector3 *pVerts)
    //---------------------------------------------------------------
    {
        RectangleIntersectionData riData;
        TriangleIntersectionData tiData;

        Triangle triangle(pVerts, true);

        // XY
        Rectangle2D rectXY(box, AP_XY);
        GetRectangleIntersectionData(rectXY, &riData);

        Triangle2D triXY(triangle.ProjectToAxisPlane(AP_XY));
        triXY.CalculateEdgeNormals();
        GetTriangleIntersectionData(triXY, &tiData);

        if (!Math::RectangleTriangleIntersection2D(triXY, riData, tiData, NULL, NULL))
        {
            return false;
        }

        // XZ
        Rectangle2D rectXZ(box, AP_XZ);
        GetRectangleIntersectionData(rectXZ, &riData);

        Triangle2D triXZ(triangle.ProjectToAxisPlane(AP_XZ));
        triXZ.CalculateEdgeNormals();
        GetTriangleIntersectionData(triXZ, &tiData);

        if (!Math::RectangleTriangleIntersection2D(triXZ, riData, tiData, NULL, NULL))
        {
            return false;
        }

        // YZ
        Rectangle2D rectYZ(box, AP_YZ);
        GetRectangleIntersectionData(rectYZ, &riData);

        Triangle2D triYZ(triangle.ProjectToAxisPlane(AP_YZ));
        triYZ.CalculateEdgeNormals();
        GetTriangleIntersectionData(triYZ, &tiData);

        if (!Math::RectangleTriangleIntersection2D(triYZ, riData, tiData, NULL, NULL))
        {
            return false;
        }

        return true;
    }
#endif

//-------------------------------------------------------------------
    bool Math::AABBContainsTriangle(const AABB &box, const Vector3 *pVerts)
    //---------------------------------------------------------------
    {
        if (!Math::AABBPointCollision(box, pVerts[0]))
        {
            return false;
        }

        if (!Math::AABBPointCollision(box, pVerts[1]))
        {
            return false;
        }

        if (!Math::AABBPointCollision(box, pVerts[2]))
        {
            return false;
        }

        return true;
    }

// EOF