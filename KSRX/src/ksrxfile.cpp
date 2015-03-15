#include <ksr.h>
#include "..\include\ksrx.h"

#include <FCNTL.H>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>

using namespace KSR;

//-----------------------------------------------------------------------
    HRESULT KSRX::LoadUTFromFile(LPCTSTR filename, const KSR::Chunk &baseChunk, bool centerOnOrigin,
                                 DWORD flags, PGEOMETRY pGeometry, const Vector3 &offset)
    //-------------------------------------------------------------------
    {
        if (!filename || !pGeometry)
            return E_FAIL;

        int pointsFile,
            trianglesFile;

        HRESULT result = S_OK;

        String pointsFilename(filename);
        String triangleFilename(filename);
        pointsFilename[strlen(filename) - 2] = _T('p');

        pointsFile = open(pointsFilename.c_str(), O_RDONLY | O_BINARY);
        trianglesFile = open(triangleFilename.c_str(), O_RDONLY | O_BINARY);

        UTPoint *points = NULL;
        UTTriangle *triangles = NULL;
        std::vector<UTTriangle> visibleTriangles;

        int numPoints = 0,
            numTriangles = 0;

        numPoints = filelength(pointsFile) / sizeof(UTPoint);
        numTriangles = filelength(trianglesFile) / sizeof(UTTriangle);

        points = new UTPoint[numPoints];
        triangles = new UTTriangle[numTriangles];

        read(pointsFile, (PVOID)points, filelength(pointsFile));
        read(trianglesFile, (PVOID)triangles, filelength(trianglesFile));

        close(pointsFile);
        close(trianglesFile);

        // Create triangles
            for (int i = 0; i < numTriangles; i++)
            {
                if (triangles[i].tflags & UT_TF_ACTIVE)
                    visibleTriangles.push_back(triangles[i]);
            }

        // Center points around origin to compensate for flat earth coords
            UTPoint min = points[3];
            UTPoint max = points[3];

            for (int n = 3; n < numPoints; n++)
            {
                if (points[n].x < min.x)
                    min.x = points[n].x;
                if (points[n].y < min.y)
                    min.y = points[n].y;
                if (points[n].z < min.z)
                    min.z = points[n].z;

                if (points[n].x > max.x)
                    max.x = points[n].x;
                if (points[n].y > max.y)
                    max.y = points[n].y;
                if (points[n].z > max.z)
                    max.z = points[n].z;
            }

            double xDist = max.x - min.x;
            double yDist = max.y - min.y;
            double zDist = max.z - min.z;

            if (centerOnOrigin)
            {
                for (n = 3; n < numPoints; n++)
                {
                    points[n].x -= min.x + xDist/2;
                    points[n].y -= min.y + yDist/2;
                    points[n].z -= min.z + zDist/2;
                }
            }
            else
            {
                for (n = 3; n < numPoints; n++)
                {
                    points[n].x -= offset.x;
                    points[n].y -= offset.y;
                    points[n].z -= offset.z;
                }
                min.x -= offset.x;
                min.y -= offset.y;
                min.z -= offset.z;
                max.x -= offset.x;
                max.y -= offset.y;
                max.z -= offset.z;
            }

        // Create verts
            VERTEX_NORMAL_TEX1 *verts = new VERTEX_NORMAL_TEX1[numPoints];
            int *indices = new int[numTriangles * 3];

            ZeroMemory(verts, sizeof(VERTEX_NORMAL_TEX1) * numPoints);

        // Create indices
            int v = 0;
            for (i = 0; i < (int)visibleTriangles.size(); i++, v += 3)
            {
                indices[v    ] = visibleTriangles[i].vertices[0] - 3;
                indices[v + 1] = visibleTriangles[i].vertices[1] - 3;
                indices[v + 2] = visibleTriangles[i].vertices[2] - 3;

                Vector3 v0 = points[visibleTriangles[i].vertices[0]].V3();
                Vector3 v1 = points[visibleTriangles[i].vertices[1]].V3();
                Vector3 v2 = points[visibleTriangles[i].vertices[2]].V3();
                Vector3 n = Math::Cross(v1 - v0, v2 - v0);

                verts[indices[v]].normal += n;
                verts[indices[v + 1]].normal += n;
                verts[indices[v + 2]].normal += n;
            }
/*
            for (i = 0; i < numTriangles * 3; i++)
                indices[i] += pGeometry->GetNumVertexIndices();
*/
        // Fill verts
            for (i = 3, v = 0; i < numPoints; i++, v++)
            {
                Vector3 p = points[i].V3();

                verts[v].position = p;
                verts[v].normal = Math::Normalise(verts[v].normal);
                verts[v].uv = Vector2(p.x / (float)xDist, p.y / (float)yDist);
            }

        Chunk *faces = new Chunk[visibleTriangles.size()];
        for (i = 0, v = 0; i < (int)visibleTriangles.size(); i++, v += 3)
        {
            faces[i] = baseChunk;
            faces[i].startIndex = pGeometry->GetNumVertexIndices() + i * 3;
            faces[i].numVerts = 3;
            faces[i].type = CT_POLYGON;
        }

        Chunk chunk(baseChunk);
        chunk.startIndex = pGeometry->GetNumVertexIndices();
        chunk.numVerts = (int)visibleTriangles.size() * 3;
        chunk.type = CT_TRIANGLELIST;

        if (FAILED(pGeometry->Init(sizeof(VERTEX_NORMAL_TEX1), FVF_NORMAL_TEX1, IT_32)))
        {
            result = E_FAIL;
            goto cleanup;
        }

        int chunkIndex = pGeometry->GetNumChunkIndices();

        if (FAILED(pGeometry->Insert(1, numPoints - 3, 1, (int)visibleTriangles.size() * 3,
                                     &chunk, verts, &chunkIndex, indices, 0)))
            result = E_FAIL;

cleanup:
        delete[] faces;

        delete[] verts;
        delete[] indices;

        delete[] points;
        delete[] triangles;

        return result;
    }

//-----------------------------------------------------------------------
    HRESULT KSRX::LoadLayeredUTFromFile(LPCTSTR filename, const KSR::Chunk &baseChunk, const int *pTextureIDs, const int *pTextureScales,
                                        int numTextureIDs, const DWORD *pMaterialColours, int numMaterialColours,
                                        bool centerOnOrigin, DWORD flags, PGEOMETRY pGeometry,
                                        PINTERFACE pKSR, const Vector3 &offset, const unsigned char ambientDivisor,
                                        pProgressUpdateFn ProgressCallback, void *pProgressPayload, const unsigned int numProgrssUpdates,
                                        const unsigned char specularDivisor, const unsigned char emmisiveDivisor)
    //-------------------------------------------------------------------
    {
        if (!filename || !pGeometry || !pKSR || !flags)
            return E_FAIL;

        // for use with the callback function
        float percentageRead = 0.0f;
        TCHAR buf[256];
        int progressDivisor = 1;

        memset(buf, 0, sizeof(TCHAR)*256);

        int pointsFile,
            trianglesFile;

        const DWORD *_pMaterialColours = pMaterialColours ? pMaterialColours : UTPalette;
        int numAvailableColours = pMaterialColours ? numMaterialColours : 256;

        HRESULT result = S_OK;

        String pointsFilename(filename);
        String triangleFilename(filename);
        pointsFilename[strlen(filename) - 2] = _T('p');

        pointsFile = open(pointsFilename.c_str(), O_RDONLY | O_BINARY);
        trianglesFile = open(triangleFilename.c_str(), O_RDONLY | O_BINARY);

        UTPoint *points = NULL;
        UTTriangle *triangles = NULL;
        std::vector<UTTriangle> visibleTriangles;

        int numPoints = 0,
            numTriangles = 0;

        numPoints = filelength(pointsFile) / sizeof(UTPoint);
        numTriangles = filelength(trianglesFile) / sizeof(UTTriangle);

        points = new UTPoint[numPoints];
        triangles = new UTTriangle[numTriangles];

        read(pointsFile, (PVOID)points, filelength(pointsFile));
        read(trianglesFile, (PVOID)triangles, filelength(trianglesFile));

        close(pointsFile);
        close(trianglesFile);

        std::map< unsigned char, std::vector< int > > layerMap;

        // todo: determine the weighting for the progress percentage of this section
        // todo: implement the progress callback for this section
        // Create triangles
            for (int i = 0; i < numTriangles; i++)
            {
                if (triangles[i].tflags & UT_TF_ACTIVE)
                {
                    visibleTriangles.push_back(triangles[i]);

                    layerMap[triangles[i].layer].push_back(visibleTriangles.size() - 1);
                }
            }

        // todo: determine the weighting for the progress percentage of this section
        // todo: implement the progress callback for this section
        // Create materials
            int materialIds[256];
            memset(materialIds, -1, sizeof(int) * 256);

            std::map< unsigned char, std::vector< int > >::iterator layerItr = layerMap.begin();
            for (; layerItr != layerMap.end(); layerItr++)
            {
                DWORD diffuse = _pMaterialColours[layerItr->first % numAvailableColours];
                DWORD ambient = KSRX::DivideARGB(diffuse, ambientDivisor);
                DWORD specular = KSRX::DivideARGB(diffuse, specularDivisor);
                DWORD emmisive = KSRX::DivideARGB(diffuse, emmisiveDivisor);
                pKSR->CreateMaterial(diffuse, ambient, specular, emmisive, 0.0f, materialIds + layerItr->first);
            }

        // todo: determine the weighting for the progress percentage of this section
        // todo: implement the progress callback for this section
        // Center points around origin to compensate for flat earth coords
            UTPoint min = points[3];
            UTPoint max = points[3];

            for (int n = 3; n < numPoints; n++)
            {
                if (points[n].x < min.x)
                    min.x = points[n].x;
                if (points[n].y < min.y)
                    min.y = points[n].y;
                if (points[n].z < min.z)
                    min.z = points[n].z;

                if (points[n].x > max.x)
                    max.x = points[n].x;
                if (points[n].y > max.y)
                    max.y = points[n].y;
                if (points[n].z > max.z)
                    max.z = points[n].z;
            }

            double xDist = max.x - min.x;
            double yDist = max.y - min.y;
            double zDist = max.z - min.z;

            if (centerOnOrigin)
            {
                for (n = 3; n < numPoints; n++)
                {
                    points[n].x -= min.x + xDist/2;
                    points[n].y -= min.y + yDist/2;
                    points[n].z -= min.z + zDist/2;
                }
            }
            else
            {
                for (n = 3; n < numPoints; n++)
                {
                    points[n].x -= offset.x;
                    points[n].y -= offset.y;
                    points[n].z -= offset.z;
                }

                min.x -= offset.x;
                min.y -= offset.y;
                min.z -= offset.z;
                max.x -= offset.x;
                max.y -= offset.y;
                max.z -= offset.z;
            }

        // Create verts
            VERTEX_NORMAL_TEX1 *verts = new VERTEX_NORMAL_TEX1[numPoints];
            ZeroMemory(verts, sizeof(VERTEX_NORMAL_TEX1) * numPoints);

            std::vector<int> indices;

        // todo: determine the weighting for the progress percentage of this section
        // todo: implement the progress callback for this section
        // Create indices
            int v = 0;
            for (i = 0; i < (int)visibleTriangles.size(); i++, v += 3)
            {
                Vector3 v0 = points[visibleTriangles[i].vertices[0]].V3();
                Vector3 v1 = points[visibleTriangles[i].vertices[1]].V3();
                Vector3 v2 = points[visibleTriangles[i].vertices[2]].V3();
                Vector3 n = Math::Cross(v1 - v0, v2 - v0);

                verts[visibleTriangles[i].vertices[0]-3].normal += n;
                verts[visibleTriangles[i].vertices[1]-3].normal += n;
                verts[visibleTriangles[i].vertices[2]-3].normal += n;
            }

        // todo: determine the weighting for the progress percentage of this section
        // todo: implement the progress callback for this section
        // Fill verts
            for (i = 3, v = 0; i < numPoints; i++, v++)
            {
                Vector3 p = points[i].V3();

                verts[v].position = p;
                verts[v].normal = Math::Normalise(verts[v].normal);
                verts[v].uv = Vector2(p.x / (float)xDist, p.y / (float)yDist);
            }

        Chunk *faces = new Chunk[layerMap.size()];

        int start = pGeometry->GetNumVertexIndices();

        // todo: determine the weighting for the progress percentage of this section
        // todo: implement the progress callback for this section
        layerItr = layerMap.begin();
        unsigned int indexCount;
        for (i = 0; i < (int)layerMap.size() && layerItr != layerMap.end(); i++, layerItr++)
        {
            indexCount = 0;
            std::vector<int>::iterator idxItr = layerItr->second.begin();
            for(; idxItr != layerItr->second.end(); idxItr++)
            {
                indices.push_back(visibleTriangles[*idxItr].vertices[0] - 3);
                indices.push_back(visibleTriangles[*idxItr].vertices[1] - 3);
                indices.push_back(visibleTriangles[*idxItr].vertices[2] - 3);
                indexCount += 3;
            }

            faces[i] = baseChunk;
            faces[i].startIndex = start;
            faces[i].numIndices = indexCount;
            faces[i].material = materialIds[layerItr->first];

            if (pTextureScales && layerItr->first <= numTextureIDs)
                faces[i].idTexture0 = (int)layerItr->first;
            else
                faces[i].idTexture0 = -1;

            faces[i].type = CT_TRIANGLELIST;

            start += faces[i].numIndices;
        }

        if (FAILED(pGeometry->Init(sizeof(VERTEX_NORMAL_TEX1), FVF_NORMAL_TEX1, IT_32)))
        {
            result = E_FAIL;
            goto cleanup;
        }

        int chunkIndex = pGeometry->GetNumChunkIndices();

        if (FAILED(pGeometry->Insert((int)layerMap.size(), numPoints - 3, 1, indices.size(),
                                     faces, verts, &chunkIndex, (int*)&(*indices.begin()), 0)))
            result = E_FAIL;

cleanup:
        // todo: implement the progress callback for this section - 100%

        delete[] faces;

        delete[] verts;

        delete[] points;
        delete[] triangles;

        return result;
    }

/*
    // process UTTriangles to form utLayer objects
 */
struct UTLayer
{
    std::set<u_int>        m_vertIds;
    std::vector<u_int>     m_triIds;
};

//-----------------------------------------------------------------------
    HRESULT KSRX::LoadLayeredUTFromFile(LPCTSTR filename, const KSR::Chunk &baseChunk, bool centerOnOrigin, DWORD flags,
                                        PGEOMETRY pGeometry, const unsigned int materialIDs[256], const Vector3 &offset)
    //-------------------------------------------------------------------
    {
        if (!filename || !pGeometry || !flags)
            return E_FAIL;

        int pointsFile,
            trianglesFile;

        HRESULT result(S_OK);

        String pointsFilename(filename);
        String triangleFilename(filename);
        pointsFilename[strlen(filename) - 2] = _T('p');

        pointsFile = open(pointsFilename.c_str(), O_RDONLY | O_BINARY);
        trianglesFile = open(triangleFilename.c_str(), O_RDONLY | O_BINARY);

        UTPoint *points = NULL;
        UTTriangle *triangles = NULL;
        std::vector<UTTriangle> visibleTriangles;

        int numPoints = 0,
            numTriangles = 0;

        numPoints = filelength(pointsFile) / sizeof(UTPoint);
        numTriangles = filelength(trianglesFile) / sizeof(UTTriangle);

        points = new UTPoint[numPoints];
        triangles = new UTTriangle[numTriangles];

        read(pointsFile, (PVOID)points, filelength(pointsFile));
        read(trianglesFile, (PVOID)triangles, filelength(trianglesFile));

        close(pointsFile);
        close(trianglesFile);

        std::map< unsigned char, std::vector< int > > layerMap;

        std::map< unsigned char, UTLayer * > layers;

        // Create triangles
            for (int i = 0; i < numTriangles; i++)
            {
                if (triangles[i].tflags & UT_TF_ACTIVE)
                {
                    visibleTriangles.push_back(triangles[i]);

                    layerMap[triangles[i].layer].push_back(visibleTriangles.size() - 1);

                    // temp
                    if( layers.find(triangles[i].layer) == layers.end() )
                        layers[triangles[i].layer] = new UTLayer;

                    layers[triangles[i].layer]->m_triIds.push_back(i);
                    layers[triangles[i].layer]->m_vertIds.insert(triangles[i].vertices[0]);
                    layers[triangles[i].layer]->m_vertIds.insert(triangles[i].vertices[1]);
                    layers[triangles[i].layer]->m_vertIds.insert(triangles[i].vertices[2]);
                }
            }

        // Center points around origin to compensate for flat earth coords
            UTPoint min = points[3];
            UTPoint max = points[3];

            for (int n = 3; n < numPoints; n++)
            {
                if (points[n].x < min.x)
                    min.x = points[n].x;
                if (points[n].y < min.y)
                    min.y = points[n].y;
                if (points[n].z < min.z)
                    min.z = points[n].z;

                if (points[n].x > max.x)
                    max.x = points[n].x;
                if (points[n].y > max.y)
                    max.y = points[n].y;
                if (points[n].z > max.z)
                    max.z = points[n].z;
            }

            double xDist = max.x - min.x;
            double yDist = max.y - min.y;
            double zDist = max.z - min.z;

            if (centerOnOrigin)
            {
                for (n = 3; n < numPoints; n++)
                {
                    points[n].x -= min.x + xDist/2;
                    points[n].y -= min.y + yDist/2;
                    points[n].z -= min.z + zDist/2;
                }
            }
            else
            {
                for (n = 3; n < numPoints; n++)
                {
                    points[n].x -= offset.x;
                    points[n].y -= offset.y;
                    points[n].z -= offset.z;
                }

                min.x -= offset.x;
                min.y -= offset.y;
                min.z -= offset.z;
                max.x -= offset.x;
                max.y -= offset.y;
                max.z -= offset.z;
            }

        /*
        // Create verts
            VERTEX_NORMAL_TEX1 *verts = new VERTEX_NORMAL_TEX1[numPoints];
            ZeroMemory(verts, sizeof(VERTEX_NORMAL_TEX1) * numPoints);

            std::vector<int> indices;

        // Create indices
            int v = 0;
            for (i = 0; i < (int)visibleTriangles.size(); i++, v += 3)
            {
                Vector3 v0 = points[visibleTriangles[i].vertices[0]].V3();
                Vector3 v1 = points[visibleTriangles[i].vertices[1]].V3();
                Vector3 v2 = points[visibleTriangles[i].vertices[2]].V3();
                Vector3 n = Math::Cross(v1 - v0, v2 - v0);

                verts[visibleTriangles[i].vertices[0]-3].normal += n;
                verts[visibleTriangles[i].vertices[1]-3].normal += n;
                verts[visibleTriangles[i].vertices[2]-3].normal += n;
            }

        // Fill verts
            for (i = 3, v = 0; i < numPoints; i++, v++)
            {
                Vector3 p = points[i].V3();

                verts[v].position = p;
                verts[v].normal = Math::Normalise(verts[v].normal);
                verts[v].uv = Vector2(p.x / (float)xDist, p.y / (float)yDist);
            }

        Chunk *faces = new Chunk[layerMap.size()];

        int start = pGeometry->GetNumVertexIndices();

        std::map< unsigned char, std::vector< int > >::iterator layerItr = layerMap.begin();
        for (i = 0; i < (int)layerMap.size(); i++)
        {
            std::vector<int>::iterator idxItr = layerItr->second.begin();
            for(; idxItr != layerItr->second.end(); idxItr++)
            {
                indices.push_back(*idxItr);
            }

            faces[i].startIndex = start;
            faces[i].numIndices = layerItr->second.size();
            faces[i].depthBias = 0;
            faces[i].material = -1;
            faces[i].effect = effectID;

            faces[i].idTexture0 = -1;
            faces[i].idTexture1 = -1;
            faces[i].type = CT_TRIANGLELIST;

            start += faces[i].numIndices;
        }
        */

        if (FAILED(pGeometry->Init(sizeof(VERTEX_NORMAL_TEX1), FVF_NORMAL_TEX1, IT_32)))
        {
            result = E_FAIL;
            goto cleanup;
        }

        int chunkIndex = pGeometry->GetNumChunkIndices();

//        if (FAILED(pGeometry->Insert((int)layerMap.size(), numPoints - 3, 1, indices.size(),
//                                     faces, verts, &chunkIndex, (int*)&(*indices.begin()), 0)))
//            result = E_FAIL;

cleanup:
//        delete[] faces;

//        delete[] verts;

        delete[] points;
        delete[] triangles;

        return result;
    }

#if 0
//-----------------------------------------------------------------------
    HRESULT KSRX::LoadLayeredUTFromFile(LPCTSTR filename, int *pTextureIDs, int *pTextureScales, int numTextureIDs,
                                        bool centerOnOrigin, DWORD flags, PGEOMETRY pGeometry,
                                        const Vector3 &offset)
    //-------------------------------------------------------------------
    {
        if (!filename || !pGeometry || !flags)
            return E_FAIL;

        // Load geometry from UT
        int pointsFile,
            trianglesFile;

        HRESULT result(S_OK);

        String pointsFilename(filename);
        String triangleFilename(filename);
        pointsFilename[strlen(filename) - 2] = _T('p');

        pointsFile = open(pointsFilename.c_str(), O_RDONLY | O_BINARY);
        trianglesFile = open(triangleFilename.c_str(), O_RDONLY | O_BINARY);

        UTPoint *points = NULL;
        UTTriangle *triangles = NULL;
        std::vector<UTTriangle> visibleTriangles;

        int numPoints = 0,
            numTriangles = 0,
            numVisibleTriangles = 0;

        numPoints = filelength(pointsFile) / sizeof(UTPoint);
        numTriangles = filelength(trianglesFile) / sizeof(UTTriangle);

        points = new UTPoint[numPoints];
        triangles = new UTTriangle[numTriangles];

        read(pointsFile, (PVOID)points, filelength(pointsFile));
        read(trianglesFile, (PVOID)triangles, filelength(trianglesFile));

        close(pointsFile);
        close(trianglesFile);

        // Center points around origin to compensate for flat earth coords
        UTPoint min = points[3];
        UTPoint max = points[3];

        for (int n = 3; n < numPoints; n++)
        {
            if (points[n].x < min.x)
                min.x = points[n].x;
            if (points[n].y < min.y)
                min.y = points[n].y;
            if (points[n].z < min.z)
                min.z = points[n].z;

            if (points[n].x > max.x)
                max.x = points[n].x;
            if (points[n].y > max.y)
                max.y = points[n].y;
            if (points[n].z > max.z)
                max.z = points[n].z;
        }

        double xDist = max.x - min.x;
        double yDist = max.y - min.y;
        double zDist = max.z - min.z;

        if (centerOnOrigin)
        {
            for (n = 3; n < numPoints; n++)
            {
                points[n].x -= min.x + xDist/2;
                points[n].y -= min.y + yDist/2;
                points[n].z -= min.z + zDist/2;
            }
        }
        else
        {
            for (n = 3; n < numPoints; n++)
            {
                points[n].x -= offset.x;
                points[n].y -= offset.y;
                points[n].z -= offset.z;
            }
            min.x -= offset.x;
            min.y -= offset.y;
            min.z -= offset.z;
            max.x -= offset.x;
            max.y -= offset.y;
            max.z -= offset.z;
        }

        // Create geometry
        LPVOID verts = NULL;
        std::vector<UTTriangle> visibleTriangleList;

        for (int i = 0; i < numTriangles; i++)
        {
            if (triangles[i].tflags & UT_TF_ACTIVE)
                visibleTriangleList.push_back(triangles[i]);
        }

        numVisibleTriangles = visibleTriangleList.size();
        visibleTriangles = new UTTriangle[numVisibleTriangles];

        for (int i = 0; i < numVisibleTriangles; i++)
            visibleTriangles[i] = visibleTriangleList[i];

        if (flags & UT_TRIANGLES)
            verts = new VERTEX_NORMAL_DIFFUSE_TEX1[numVisibleTriangles * 3];
        else if (flags & UT_POINTS)
            verts = new VERTEX_DIFFUSE_TEX1[numPoints];
/*
        std::vector<Vector3> faceNormals;
        faceNormals.resize(numVisibleTriangles);

        std::vector<Vector3> vertexNormals;
        vertexNormals.resize(numPoints - 3);

        for (int i = 0; i < numVisibleTriangles; i++)
        {
            UTPoint p0 = points[visibleTriangles[i].vertices[0] - 3];
            UTPoint p1 = points[visibleTriangles[i].vertices[1] - 3];
            UTPoint p2 = points[visibleTriangles[i].vertices[2] - 3];

            Vector3 v0 = Vector3(p0.x, p0.z, p0.y);
            Vector3 v1 = Vector3(p1.x, p1.z, p1.y);
            Vector3 v2 = Vector3(p2.x, p2.z, p2.y);

            Vector3 a = Math::Normalise(v0 - v1);
            Vector3 b = Math::Normalise(v0 - v2);
            Vector3 n = Math::Normalise(Math::Cross(a, b));

            faceNormals[i] = n;
        }

        std::vector< std::vector<int> > adjacentTriangles;
        adjacentTriangles.resize(numPoints - 3);

        for (int v = 0; v < numPoints - 3; v++)
        {
            for (i = 0; i < numVisibleTriangles; i++)
            {
                for (int n = 0; n < 3; n++)
                {
                    if (visibleTriangles[i].vertices[n] - 3 == v)
                        adjacentTriangles[v].push_back(i);
                }
            }

            Vector3 n;

            std::vector<int>::iterator a = adjacentTriangles[i].begin();
            for (; a != adjacentTriangles[i].end(); a++)
                n += faceNormals[*a];

            Vector3 vertexNormal = Math::Normalise(n);

            vertexNormals.push_back(vertexNormal);
        }
*/
        if (flags & UT_TRIANGLES)
        {
            for (int i = 0; i < numVisibleTriangles; i++)
            {
                float uvScale = (float)pTextureScales[visibleTriangles[i].layer];

                for (int v = 0; v < 3; v++)
                {
                    const UTPoint &p = points[visibleTriangles[i].vertices[v]];

                    Vector3 pV3 = p.V3();

                    ((VERTEX_NORMAL_DIFFUSE_TEX1*)verts)[i * 3 + v].position = Vector3(pV3.x, pV3.z, pV3.y);
                    //((VERTEX_NORMAL_DIFFUSE_TEX1*)verts)[i * 3 + v].normal = vertexNormals[i * 3 + v];
                    ((VERTEX_NORMAL_DIFFUSE_TEX1*)verts)[i * 3 + v].normal = Vector3(0, 1, 0);
                    ((VERTEX_NORMAL_DIFFUSE_TEX1*)verts)[i * 3 + v].color = UTPalette[visibleTriangles[i].layer];
                    ((VERTEX_NORMAL_DIFFUSE_TEX1*)verts)[i * 3 + v].uv = Vector2((pV3.x / (float)xDist + 0.5f) * uvScale,
                                                                         (pV3.y / (float)yDist + 0.5f) * uvScale);
                }
            }
        }
        else if (flags & UT_POINTS)
        {
            for (int i = 3, v = 0; i < numPoints; i++, v++)
            {
                const UTPoint &p = points[i];

                Vector3 pV3 = p.V3();

                ((VERTEX_DIFFUSE_TEX1*)verts)[v].position = Vector3(pV3.x, pV3.z, pV3.y);
                ((VERTEX_DIFFUSE_TEX1*)verts)[v].color = 0xffffffff;
                ((VERTEX_DIFFUSE_TEX1*)verts)[v].uv = Vector2((pV3.x / (float)xDist) * pTextureScales[18],
                                                             (pV3.y / (float)yDist) * pTextureScales[18]);
            }
        }

        int *indices = new int[numVisibleTriangles * 3];

        for (i = 0; i < numVisibleTriangles * 3; i++)
        {
            indices[i] = i;
        }

        for (i = 0; i < numVisibleTriangles * 3; i++)
            indices[i] += pGeometry->GetNumVertexIndices();

        Chunk *faces = NULL;

        if (flags & UT_TRIANGLES)
        {
            faces = new Chunk[numVisibleTriangles];

            for (int v = 0, i = 0; i < numVisibleTriangles; i++, v += 3)
            {
                faces[i].startIndex = pGeometry->GetNumVertexIndices() + i * 3;
                faces[i].numVerts = 3;
                faces[i].depthBias = 0;
                faces[i].material = 0;
                faces[i].effect = effectID;
                faces[i].idTexture0 = pTextureIDs[visibleTriangles[i].layer];
                faces[i].idTexture1 = -1;
                faces[i].type = CT_POLYGON;
            }

            if (FAILED(pGeometry->Init(sizeof(VERTEX_NORMAL_DIFFUSE_TEX1), FVF_NORMAL_DIFFUSE_TEX1, IT_32)))
            {
                result = E_FAIL;
                goto cleanup;
            }

            if (FAILED(pGeometry->Insert(numVisibleTriangles, numVisibleTriangles * 3, 0, 0,
                                         faces, verts, NULL, NULL, 0)))
            {
                result = E_FAIL;
                goto cleanup;
            }
        }
        else if (flags & UT_POINTS)
        {
            faces = new Chunk[numVisibleTriangles];

            for (int v = 0, i = 0; i < numVisibleTriangles; i++, v += 3)
            {
                faces[i].startIndex = pGeometry->GetNumVertexIndices() + i * 3;
                faces[i].numVerts = 3;
                faces[i].depthBias = 0;
                faces[i].material = 0;
                faces[i].effect = effectID;
                faces[i].idTexture0 = pTextureIDs[visibleTriangles[i].layer];
                faces[i].idTexture1 = -1;
                faces[i].type = CT_POINTLIST;
            }

            if (FAILED(pGeometry->Init(sizeof(VERTEX_DIFFUSE_TEX1), FVF_DIFFUSE_TEX1, IT_32)))
            {
                result = E_FAIL;
                goto cleanup;
            }

            if (FAILED(pGeometry->Insert(numVisibleTriangles, numPoints - 3, 0, numVisibleTriangles * 3,
                                         faces, verts, NULL, indices, 0)))
            {
                result = E_FAIL;
                goto cleanup;
            }
        }

cleanup:
        delete[] faces;

        delete[] verts;
        delete[] indices;

        delete[] points;
        delete[] triangles;

        return result;
    }
#endif

//-----------------------------------------------------------------------
    HRESULT KSRX::LoadXMeshFromFile(LPCTSTR filename, int effectID, PGEOMETRY pGeometry, PINTERFACE pKSRInterface)
    //-------------------------------------------------------------------
    {
        if (!filename || !pKSRInterface || !pGeometry)
            return E_FAIL;

        LPDIRECT3DDEVICE9 pDevice = pKSRInterface->GetRenderer()->GetD3DDevice();

        DWORD         numMaterials = 0,
                     attributeTableSize = 0;
        LPD3DXMESH     mesh = NULL;
        LPD3DXBUFFER materialBuf = NULL,
                     adjacencyBuf = NULL;

        int *textures = NULL;

        if (FAILED(D3DXLoadMeshFromX(filename,
                                     D3DXMESH_32BIT,
                                     pDevice,
                                     &adjacencyBuf,
                                     &materialBuf,
                                     NULL,
                                     &numMaterials,
                                     &mesh)))
            return E_FAIL;

        // Optimise & Generate attribute table
            if (FAILED(mesh->GenerateAdjacency(FLT_MIN, (DWORD*)adjacencyBuf->GetBufferPointer())))
                return E_FAIL;

            if (FAILED(mesh->OptimizeInplace(D3DXMESHOPT_ATTRSORT, (DWORD*)adjacencyBuf->GetBufferPointer(), NULL, NULL, NULL)))
                return E_FAIL;

            void *vertices = NULL;
            void *indices = NULL;

            D3DXATTRIBUTERANGE *attributeTable = NULL;

            mesh->GetAttributeTable(NULL, &attributeTableSize);

            attributeTable = new D3DXATTRIBUTERANGE[attributeTableSize];

            mesh->GetAttributeTable(attributeTable, NULL);

        // Load Materials & Textures
            D3DXMATERIAL *materials = (D3DXMATERIAL*)materialBuf->GetBufferPointer();

            textures = new int[numMaterials];

            for (DWORD i = 0; i < numMaterials; i++)
            {
                if(materials[i].pTextureFilename)
                {
                    if (FAILED(pKSRInterface->LoadTextureFromFile(materials[i].pTextureFilename, &textures[i])))
                        return E_FAIL;
                }
            }

        // Determine index buffer size & format
            int indexSize = 0;
            DWORD indexFormat = 0;

            LPDIRECT3DINDEXBUFFER9 pIndexBuffer;
            D3DINDEXBUFFER_DESC desc;

            if (FAILED(mesh->GetIndexBuffer(&pIndexBuffer)))
                return E_FAIL;

            if (FAILED(pIndexBuffer->GetDesc(&desc)))
                return E_FAIL;

            if (desc.Format == D3DFMT_INDEX16)
            {
                indexSize = 2;
                indexFormat = IT_16;
            }
            else if (desc.Format == D3DFMT_INDEX32)
            {
                indexSize = 4;
                indexFormat = IT_32;
            }
            else
                return E_FAIL;

        // Lock buffers
            LPVOID pVertsLocked = NULL,
                   pIndicesLocked = NULL;

            if (FAILED(mesh->LockVertexBuffer(0, &pVertsLocked)))
                return E_FAIL;

            if (FAILED(mesh->LockIndexBuffer(0, &pIndicesLocked)))
                return E_FAIL;

        vertices = malloc(mesh->GetNumVertices() * mesh->GetNumBytesPerVertex());
        indices = malloc(mesh->GetNumFaces() * 3 * indexSize);

        memcpy(vertices, pVertsLocked, mesh->GetNumBytesPerVertex() * mesh->GetNumVertices());
        memcpy(indices, pIndicesLocked, mesh->GetNumFaces() * 3 * indexSize);

        int debugNumVerts = mesh->GetNumVertices();
        int debugNumFaces = mesh->GetNumFaces();

        for (i = 0; i < mesh->GetNumFaces() * 3; i++)
            ((int*)indices)[i] += pGeometry->GetNumVertices();

        Chunk *chunks = new Chunk[attributeTableSize];
        for (i = 0; i < attributeTableSize; i++)
        {
            chunks[i].idTexture0 = textures[i];
            chunks[i].idTexture1 = -1;
            chunks[i].effect = effectID;
            chunks[i].numVerts = attributeTable[i].FaceCount * 3;
            chunks[i].startIndex = pGeometry->GetNumVertexIndices() + attributeTable[i].FaceStart * 3;
            chunks[i].type = CT_TRIANGLELIST;
        }

        pGeometry->Init(mesh->GetNumBytesPerVertex(), mesh->GetFVF(), indexFormat);
        pGeometry->Insert(attributeTableSize, mesh->GetNumVertices(), 0, mesh->GetNumFaces() * 3, chunks, vertices, NULL, indices, 0);

        delete[] chunks;

        materialBuf->Release();
        adjacencyBuf->Release();

        if (attributeTable)
            delete[] attributeTable;

        mesh->UnlockVertexBuffer();
        mesh->UnlockIndexBuffer();

        free(vertices);
        free(indices);

        return S_OK;
    }

// EOF