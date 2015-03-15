/*! \file */
/*-----------------------------------------------------------------------
    patch.h

    Description: Continuous Level of Detail Patch and related class declarations
    Author:         Julian McKinlay
    ---------------------------------------------------------------------*/

#pragma once

struct PatchBlock
{
    PatchBlock();

    float x, y, size;
    float deltaMax;

    int stride;

    bool visible;

    bool splitChildren;
    int childUpdateCount;

    PatchBlock *parent;
    PatchBlock **children;

    Chunk chunk;
};


//! \struct Patch
struct Patch : public MemObject
{
public:
    Patch(LPDIRECT3DDEVICE9 pDevice);
    ~Patch();

    HRESULT SetGeometry(PGEOMETRY pGeometry);

    HRESULT Reset();
    HRESULT Generate(int patchQuality);

    HRESULT Update(PVIEWPORT pViewport);
    HRESULT CreateRenderList(std::vector<RenderGroup> &renderList);

    //! \brief Initialises geometry formatting
    /*! Returns S_OK on success and E_FAIL on failure.
        \param vertexSize Stride of each vertex, in bytes.
        \param vertexFormat Vertex format.
        \param indexFormat Index format.*/
    //HRESULT InitGeometry(int vertexSize, DWORD vertexFormat, DWORD indexFormat);

    //! \brief Inserts geometry data
    /*! Returns S_OK on success and E_FAIL on failure.
        \param nChunks Number of chunks stored in the pChunks array.
        \param nVerts Number of vertices stored in the pVerts array.
        \param nChunkIndices Number of chunk indices stored in the pChunkIndices array.
        \param nVertexIndices Number of vertex indices stored in the pVertexIndices array.
        \param pChunks [in] An array of chunks to be inserted.
        \param pVertices [in] An array of pVertices to be inserted.
        \param pChunkIndices [in] An array of chunk indices to be inserted
        \param pVertexIndices [in] An array of vertex indices to be inserted. */
    //HRESULT InsertGeometry(int nChunks, int nVerts, int nChunkIndices, int nVertexIndices,
    //                      void *pChunks, void *pVerts, void *pChunkIndices, void *pVertexIndices, DWORD flags);

    //! \brief Inserts geometry data
    /*! Returns S_OK on success and E_FAIL on failure.
        \param nChunks Number of chunks stored in the pChunks array.
        \param nVerts Number of vertices stored in the pVerts array.
        \param pChunks [in] An array of chunks to be inserted.
        \param pVertices [in] An array of pVertices to be inserted.*/
    //HRESULT InsertGeometry(int nChunks, int nVerts, void *pChunks, void *pVerts, DWORD flags);

    //! \brief Inserts geometry data
        /*! Returns S_OK on success and E_FAIL on failure.
            \param pGeometry Pointer to a KSRGeometry structure, representing the geometry to insert. */
    //HRESULT InsertGeometry(PGEOMETRY pGeometry, DWORD flags);

    //PGEOMETRY geometry;
    //PGEOMETRY renderGeometry;

private:
    float CalculateBlockDelta(PatchBlock *block, float width, float height);
    float CalculateVertexDelta(Vector3 p, Vector3 neighboursAvg, float width, float height);

    void CalculateBlockDeltas(float width, float height);
    void UpdateBlocks();
    void RenderGeometry();

    void RecurseGenerateBlocks(PatchBlock *block, LPVOID pVertsLocked, LPVOID pIndicesLocked);
    void RecurseUpdateBlocks(PatchBlock *block, float width, float height);
    void RecurseGetActiveBlocks(PatchBlock *block);

    int quality;
    int blockWidth;
    int vertexWidth;
    int controlVertexWidth;
    int controlBlockWidth;

    int *divisionTree;
    int *updateBlocks;

    Matrix frustum;

    PGEOMETRY controlGeometry;
    PGEOMETRY renderGeometry;

    PatchBlock *root;
    int numStaticBlocks;

    std::list<PatchBlock *> blocks;
    std::vector<PatchBlock *> activeBlocks;

    LPDIRECT3DDEVICE9 D3DDevice;
};

//! \typedef Patch *PPATCH
typedef Patch *PPATCH;

/*
struct KSRPatch : public MemObject
{
public:
    KSRPatch(LPDIRECT3DDEVICE9 pDevice);
    ~KSRPatch();

    HRESULT SetErrorThreshold(float errorThreshold);

    HRESULT Create(int q);
    HRESULT Render(KSRChunk *pChunk, LPKSRGEOMETRY pGeometry, LPKSRVIEWPORT pViewport);

private:
    float CalculateBlockDelta(int x, int y, int size);
    float CalculateVertexDelta(Vector3 p, Vector3 neighboursAvg);
    float CalculateUncertaintyInterval(int x, int y, int size);
    bool  DetermineVisibility(int x, int y, int size);

    void RecurseCreateDivisionTree(int x, int y, int size);
    void RecurseRenderPatch(int x, int y, int size);
    void RenderChunk(KSRChunk *pChunk);

    int quality;
    int width;
    float threshold;

    int *divisionTree;
    float *morphTree;
    float *roughTree;

    LPDIRECT3DDEVICE9 D3DDevice;
    LPKSRGEOMETRY renderGeometry;

    // Variables used only during render
    int vertexWidth;

    KSRChunk *chunk;
    LPVOID vertices;
    LPVOID indices;

    Matrix frustum;
    LPKSRGEOMETRY geometry;
    LPKSRVIEWPORT viewport;
};

typedef KSRPatch *LPKSRPATCH;*/

// EOF