/*-----------------------------------------------------------------------
    primitive.h

    Description: Geometry class definitions
    Author:         Julian McKinlay
    ---------------------------------------------------------------------*/

#pragma once

enum { PT_LINELIST = 0, PT_LINESTRIP, PT_TRIANGLELIST, PT_TRIANGLESTRIP, PT_TRIANGLEFAN, PT_FACELIST };

struct Face
{
    int textureID;

};


struct KSRGeometry
{
public:
    KSRGeometry();

    void SetPrimitiveType(int type) { primitiveType = type; };
    void SetVertexStride(DWORD s) { vertexStride = s;};
    void SetIndexStride(DWORD s) { indexStride = s;};
    void SetFVF(DWORD fvf) { FVF = fvf; };
    void SetRedraw(DWORD r) { redraw = r; };

    void SetVertexBuffer(LPDIRECT3DVERTEXBUFFER9 vb) { vertexBuffer = vb; };
    void SetIndexBuffer(LPDIRECT3DINDEXBUFFER9 ib) { indexBuffer = ib; };

    DWORD GetVertexStride() { return vertexStride; };
    DWORD GetIndexStride() { return indexStride; };
    DWORD GetVertexLength() { return vertexLength; };
    DWORD GetIndexLength() { return indexLength; };
    DWORD GetFVF() { return FVF; };
    DWORD GetRedraw() { return redraw; };

    HRESULT Lock(LPVOID *pVertices, LPVOID *pIndices);
    HRESULT Unlock();

private:
    DWORD primitiveType,
          vertexStride,
          indexStride,
          vertexLength,
          indexLength,
          FVF,
          redraw;

    bool verticesLocked,
         indicesLocked;

    LPDIRECT3DVERTEXBUFFER9 vertexBuffer;
    LPDIRECT3DINDEXBUFFER9  indexBuffer;
};

typedef KSRGeometry *LPKSRGEOMETRY;

// EOF