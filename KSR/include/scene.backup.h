/*! \file */
/*-----------------------------------------------------------------------
    scene.h

    Description: Scene and related class declarations
    Author:         Julian McKinlay
    ---------------------------------------------------------------------*/

#pragma once

#ifdef _DEBUG
#include <string>

//#define DEBUG_DUMP

    #ifdef DEBUG_DUMP
        #pragma message "DEBUG_DUMP is activated, performance will be severly curtailed."
    #endif
#endif

//! \struct Scene
struct Scene : public MemObject
{
private:
    typedef void (*ProgressCallbackFunc)(DWORD, float, void *);

    typedef int MaterialID;
    typedef int EffectID;
    typedef int EffectTechniqueID;
    typedef int TextureID;
    typedef unsigned int ChunkIndex;

    // datatypes for partitioning
    typedef std::vector< unsigned int > VertexIndexList;
    struct _VertexIndexList
    {
        _VertexIndexList()
        {
#ifdef DEBUG_DUMP
            char dbgBuf[256];

            dbgBuf[255] = 0;
            _snprintf(dbgBuf, 255, "alloc: \t\t\t\t_VertexIndexList - 0x%8.8X\n", (DWORD)this);
            OutputDebugString(dbgBuf);
#endif
        }

        _VertexIndexList(const _VertexIndexList &rhs)
            : m(rhs.m)
        {

        }

        ~_VertexIndexList()
        {
#ifdef DEBUG_DUMP
            char dbgBuf[256];

            dbgBuf[255] = 0;
            _snprintf(dbgBuf, 255, " free: \t\t\t\t_VertexIndexList - 0x%8.8X\n", (DWORD)this);
            OutputDebugString(dbgBuf);
#endif
        }

        VertexIndexList m;

        const bool IsEmpty() const
        {
            return m.empty();
        }

        void PushBack (const unsigned int val) { m.push_back(val); }
        const size_t Size() const { return m.size(); }
        const unsigned int operator[] (const unsigned int idx) const { return idx >= m.size() ? (size_t)-1 : m[idx]; }
        unsigned int operator[] (const unsigned int idx) { return idx >= m.size() ? (size_t)-1 : m[idx]; }
    };

    typedef std::map< ChunkIndex, _VertexIndexList * > ChunkIndexList;
    struct _ChunkIndexList
    {
        _ChunkIndexList()
        {
#ifdef DEBUG_DUMP
            char dbgBuf[256];

            dbgBuf[255] = 0;
            _snprintf(dbgBuf, 255, "alloc: \t\t\t_ChunkIndexList - 0x%8.8X\n", (DWORD)this);
            OutputDebugString(dbgBuf);
#endif
        }

        _ChunkIndexList(const _ChunkIndexList &rhs)
        {
            ChunkIndexList::const_iterator i = rhs.m.begin();
            for (; i != rhs.m.end(); i++)
            {
                _VertexIndexList *pNewVIL = new _VertexIndexList(*i->second);
                m[i->first] = pNewVIL;
            }
        }

        ChunkIndexList m;

        ~_ChunkIndexList()
        {
#ifdef DEBUG_DUMP
            char dbgBuf[256];

            dbgBuf[255] = 0;
            _snprintf(dbgBuf, 255, " free: \t\t\t_ChunkIndexList - 0x%8.8X\n", (DWORD)this);
            OutputDebugString(dbgBuf);
#endif
            for (ChunkIndexList::iterator i = m.begin(); i != m.end(); i++)
            {
                if (i->second)
                    delete i->second;
            }
        }
        const bool IsEmpty() const
        {
            ChunkIndexList::const_iterator i = m.begin();
            for (; i != m.end(); i++)
            {
                if (!i->second->IsEmpty())
                    return false;
            }

            return true;
        }
        const size_t Size() const { return m.size(); }
        const _VertexIndexList * operator[] (const ChunkIndex idx) const
        {
            ChunkIndexList::const_iterator i = m.find(idx);
            if (i == m.end())
                return NULL;
            else
                return i->second;
        }
        _VertexIndexList *GetPtr(const ChunkIndex idx)
        {
            if (m.find(idx) == m.end())
            {
                m[idx] = new _VertexIndexList;
#ifdef DEBUG_DUMP
                char buf[256];
                sprintf(buf, "Allocating new _VertexIndexList at 0x%8.8X.\n", (DWORD)m[idx]);
                OutputDebugString(buf);
#endif
            }
            return m[idx];
        }
    };

    typedef std::map< PGEOMETRY, _ChunkIndexList * > GeometryMap;
    struct _GeometryMap
    {
        _GeometryMap()
        {
#ifdef DEBUG_DUMP
            char dbgBuf[256];

            dbgBuf[255] = 0;
            _snprintf(dbgBuf, 255, "alloc: \t\t_GeometryMap - 0x%8.8X\n", (DWORD)this);
            OutputDebugString(dbgBuf);
#endif
        }

        _GeometryMap(const _GeometryMap &rhs)
        {
            GeometryMap::const_iterator i = rhs.m.begin();
            for (; i != rhs.m.end(); i++)
            {
                _ChunkIndexList *pNewCIL = new _ChunkIndexList(*i->second);
                m[i->first] = pNewCIL;
            }
        }

        GeometryMap m;

        ~_GeometryMap()
        {
#ifdef DEBUG_DUMP
            char dbgBuf[256];

            dbgBuf[255] = 0;
            _snprintf(dbgBuf, 255, " free: \t\t_GeometryMap - 0x%8.8X\n", (DWORD)this);
            OutputDebugString(dbgBuf);
#endif
            for (GeometryMap::iterator i = m.begin(); i != m.end(); i++)
            {
                if (i->second)
                    delete i->second;
            }
        }
        const bool IsEmpty() const
        {
            GeometryMap::const_iterator i = m.begin();
            for (; i != m.end(); i++)
            {
                if (!i->second->IsEmpty())
                    return false;
            }

            return true;
        }
        const size_t Size() const { return m.size(); }
        const _ChunkIndexList *operator[] (const PGEOMETRY idx) const
        {
            GeometryMap::const_iterator i = m.find(idx);
            if (i == m.end())
                return NULL;
            else
                return i->second;
        }
        _ChunkIndexList *GetPtr(const PGEOMETRY idx)
        {
            if (m.find(idx) == m.end())
            {
                m[idx] = new _ChunkIndexList;
#ifdef DEBUG_DUMP
                char buf[256];
                sprintf(buf, "Allocating new _ChunkIndexList at 0x%8.8X.\n", (DWORD)m[idx]);
                OutputDebugString(buf);
#endif
            }
            return m[idx];
        }
    };

    typedef std::map< unsigned int, _GeometryMap * > OrderedGeometryMap;
    struct _OrderedGeometryMap
    {
        _OrderedGeometryMap()
        {
#ifdef DEBUG_DUMP
            char dbgBuf[256];

            dbgBuf[255] = 0;
            _snprintf(dbgBuf, 255, "alloc: \t_OrderedGeometryMap - 0x%8.8X\n", (DWORD)this);
            OutputDebugString(dbgBuf);
#endif
        }

        _OrderedGeometryMap(const _OrderedGeometryMap &rhs)
        {
            OrderedGeometryMap::const_iterator i = rhs.m.begin();
            for (; i != rhs.m.end(); i++)
            {
                _GeometryMap *pNewGM = new _GeometryMap(*i->second);
                m[i->first] = pNewGM;
            }
        }

        OrderedGeometryMap m;

        ~_OrderedGeometryMap()
        {
#ifdef DEBUG_DUMP
            char dbgBuf[256];

            dbgBuf[255] = 0;
            _snprintf(dbgBuf, 255, " free: \t_OrderedGeometryMap - 0x%8.8X\n", (DWORD)this);
            OutputDebugString(dbgBuf);
#endif
            for (OrderedGeometryMap::iterator i = m.begin(); i != m.end(); i++)
            {
                if (i->second)
                    delete i->second;
            }
        }
        const bool IsEmpty() const
        {
            OrderedGeometryMap::const_iterator i = m.begin();
            for (; i != m.end(); i++)
            {
                if (!i->second->IsEmpty())
                    return false;
            }

            return true;
        }
        const size_t Size() const { return m.size(); }
        const _GeometryMap *operator[] (const unsigned int idx) const
        {
            OrderedGeometryMap::const_iterator i = m.find(idx);
            if (i == m.end())
                return NULL;
            else
                return i->second;
        }
        _GeometryMap *GetPtr(const unsigned int idx)
        {
            if (m.find(idx) == m.end())
            {
                m[idx] = new _GeometryMap;
#ifdef DEBUG_DUMP
                char buf[256];
                sprintf(buf, "Allocating new _GeometryMap at 0x%8.8X.\n", (DWORD)m[idx]);
                OutputDebugString(buf);
#endif
            }

            return m[idx];
        }
    };

    typedef std::vector< _OrderedGeometryMap * > NodeChunkMap;
    struct _NodeChunkMap
    {
        _NodeChunkMap()
        {
#ifdef DEBUG_DUMP
            char dbgBuf[256];

            dbgBuf[255] = 0;
            _snprintf(dbgBuf, 255, "alloc: _NodeChunkMap - 0x%8.8X\n", (DWORD)this);
            OutputDebugString(dbgBuf);
#endif
        }

        _NodeChunkMap(const _NodeChunkMap &rhs)
        {
            NodeChunkMap::const_iterator i = rhs.m.begin();
            for (; i != rhs.m.end(); i++)
            {
                _OrderedGeometryMap *pNewOGM = new _OrderedGeometryMap(*(*i));
                m.push_back(pNewOGM);
            }
        }

        ~_NodeChunkMap()
        {
#ifdef DEBUG_DUMP
            char dbgBuf[256];

            dbgBuf[255] = 0;
            _snprintf(dbgBuf, 255, " free: _NodeChunkMap - 0x%8.8X\n", (DWORD)this);
            OutputDebugString(dbgBuf);
#endif
            for (NodeChunkMap::iterator i = m.begin(); i != m.end(); i++)
            {
                if (*i)
                    delete *i;
            }
        }

        const bool IsEmpty() const
        {
            NodeChunkMap::const_iterator i = m.begin();
            for (; i != m.end(); i++)
            {
                if (!(*i)->IsEmpty())
                    return false;
            }

            return true;
        }

        NodeChunkMap m;
    };

    // datatypes for optimisation
    typedef std::vector< unsigned int > ChunkIndices;
    typedef std::map<TextureID, ChunkIndices> TextureChunkMap;
    typedef std::map<MaterialID, TextureChunkMap> MaterialTChunkMap;
    typedef std::map<EffectTechniqueID, MaterialTChunkMap> EffectTechniqueMTChunkMap;
    typedef std::map<EffectID, MaterialTChunkMap> EffectTMTChunkMap;
    typedef std::map<PGEOMETRY, EffectTMTChunkMap> PGeometryETMTChunkMap;
    typedef std::map<int, PGeometryETMTChunkMap> OrderPGETMTChunkMap;

    /*enum eChumkMapType { eCMT_TRIANGLES = 0, eCMT_LINES, eCMT_POINTS };
    typedef std::pair< eChumkMapType, std::vector< int > > ChunkMap;    //!< Stores chunk indices
    typedef std::vector< ChunkMap > todo2;
                                                                                //typedef std::vector< todo2 > todo3;
    typedef std::pair< PGEOMETRY, todo2 > GeometryMap;
    /*/
    //typedef std::pair< PGEOMETRY, std::vector< std::vector< std::vector< int > > > > GeometryMap;
    //typedef std::pair< PGEOMETRY, std::vector< std::vector< int > > > GeometryMap;
    //*/
    //typedef std::vector< GeometryMap > NodeChunkMap;

public:
    typedef std::list<PGEOMETRY>::const_iterator ConstGeometryIterator;

    Scene(PSCENESETTINGS pSettings, LPDIRECT3DDEVICE9 pDevice, PRESOURCEMANAGER pResourceManager);
    ~Scene();

    HRESULT InsertGeometry(PGEOMETRY pGeometry);
    HRESULT RemoveGeometry(PGEOMETRY pGeometry);
    HRESULT UpdateGeometry(PGEOMETRY pGeometry);

    //! \brief Inserts an Entity and attaches it to the scene graph root.
    /*! Returns S_OK on success and E_FAIL on failure.
        \param pEntity [out] Address of a pointer to a KSREntity structure, representing the entity to insert.*/
    HRESULT InsertEntity(PENTITY pEntity);

    //! \brief Removes an entity from the scene.
    /*! Returns S_OK on success and E_FAIL on failure.
        \param pEntity [in] Pointer to the KSREntity structure to remove.*/
    HRESULT RemoveEntity(PENTITY pEntity);

    //! \brief Inserts a Patch.
    /*! Returns S_OK on success and E_FAIL on failure.
        \param pPatch [in] Address of a pointer to a KSRPatch structure, representing the patch to insert.*/
    HRESULT InsertPatch(PPATCH pPatch);

    //! \brief Retrieves collision information from the most recent call to Update().
    /*! Returns S_OK on success and E_FAIL on failure.
        \param pCollisionInfo [out] Pointer to an array of KSRCollisionInfo structures,
               representing the collision data.
        \param pNumCollisions [out] Pointer to an int containing the number of KSRCollisionInfo structures stored in
               pCollisionInfo array. */
    HRESULT GetCollisionInfo(CollisionInfo **pCollisionInfo, int *pNumCollisions);

    //! \brief Updates Settings
    /*! Returns S_OK on success and E_FAIL on failure.
        \param pSettings [in] Pointer to a KSRSceneSettings structure, representing the new settings.*/
    HRESULT SetSettings(PSCENESETTINGS pSettings);

    //! \brief Sets the scene's boundaries
    /*! Returns S_OK on success and E_FAIL on failure.
        \param min 3D Vector defining the lower-left corner of the world.
        \param max 3D Vector defining the upper-right corner of the world.*/
    HRESULT SetWorldBounds(Vector3 min, Vector3 max);

    //! \brief Sets the scene's origin position
    /*! Returns S_OK on success and E_FAIL on failure.
        \param position 3D Vector defining the origin of the world.*/
    HRESULT SetPosition(Vector3 position);

    //! Erases all geometry, entities and space partitioning for this scene.
    /*! Returns S_OK on success and E_FAIL on failure.*/
    HRESULT ResetWorld();

    //! Subdivides this scene
    /*! Returns S_OK on success and E_FAIL on failure.*/
    HRESULT SubdivideWorld();

    HRESULT SetProgressCallbackPayload(void *pPayload);

    HRESULT GetGeometry(ConstGeometryIterator *begin, ConstGeometryIterator *end);

    HRESULT GetSpacePartitionOutlineGeometry(PGEOMETRY *ppGeometry);

    HRESULT GetSceneGraphRootEntity(PENTITY *ppEntity);

    HRESULT Serialise(OutputScene *pOutputScene, std::vector< std::pair<PGEOMETRY, int> > &geometryMap,
                   std::vector< std::pair<PENTITY, int> > &entityMap, DWORD flags);

    HRESULT Reassemble(OutputScene *pOutputScene, std::vector< std::pair<PGEOMETRY, int> > &geometryMap,
                   std::vector< std::pair<PENTITY, int> > &entityMap, DWORD flags);

    //! \brief Creates a list of chunks to render
    /*! Returns S_OK on success and E_FAIL on failure.
        \param renderList Vector of render lists to be rendered.
        \param pViewport [in] Pointer to a Viewport structure, representing the viewport to render to.*/
    HRESULT CreateRenderList(std::vector<RenderGroup> &renderList, PVIEWPORT pViewport);

    //! \brief Sets the scene's boundaries
    /*! Returns S_OK on success and E_FAIL on failure.
        \param min 3D Vector defining the lower-left corner of the world.
        \param max 3D Vector defining the upper-right corner of the world.*/
    HRESULT Update(const float deltaTime);

    HRESULT SetProgressCallback(ProgressCallbackFunc progressCallbackFunc, void *pPayload);

    bool Intersect(IntersectionResult &result, const Vector3 &rayPos, const Vector3 &rayDir, DWORD flags);
    bool Intersect(std::list<IntersectionResult> &results, const Vector3 &rayPos, const Vector3 &rayDir, DWORD flags);

    void SetLightID(unsigned int index, int lightID) { if (index < 8) lightIDs[index] = lightID; }

    //! Updates Redraw state
    /*! \param r New Redraw state.*/
    void SetRedraw(DWORD r) { sceneGraphRoot->SetRedraw(r); };

    //! Updates Update state
    /*! \param r New Update state.*/
    void SetUpdate(DWORD u) { sceneGraphRoot->SetUpdate(u); };

    void SetActiveLeaf(int id) { activeLeafID = id; }
    int GetActiveLeaf() { return activeLeafID; }

    //! \brief Gets the scene's boundaries
    /*! Returns S_OK on success and E_FAIL on failure.
        \param min 3D Vector defining the lower-left corner of the world.
        \param max 3D Vector defining the upper-right corner of the world.*/
    HRESULT GetWorldBounds(Vector3 *pMin, Vector3 *pMax);

    DWORD GetRedraw() { return sceneGraphRoot->GetRedraw(); };
    DWORD GetUpdate() { return sceneGraphRoot->GetUpdate(); };

    int    GetSubdivisionsExecuted();
    int    GetNumLeafs();

    void *GetProgressCallbackPayload() { return pProgressCallbackPayload; }

    const int GetLightID(unsigned int index) const { return index < 8 ? lightIDs[index] : -1; }

private:
    void Reset(bool recreate = false, bool resetLights = true);
    void Clear();
    void Init();

    void Partition(TreeNode *node, int numSubdivisions, _NodeChunkMap *pNodeChunkMap);
    _NodeChunkMap *CreateNodeChunkMap();
    _NodeChunkMap *CreateNodeChunkMap(_NodeChunkMap *pNodeChunkMap, _NodeChunkMap **ppRemainder,
                                      const Math::AABB &bounds, unsigned int *pNumPolygons);

    void GenerateLeafs(TreeNode *node, std::map< PGEOMETRY, Chunk * > &srcChunks,
                       std::map< PGEOMETRY, std::vector< unsigned int > > &newVertexIndices);

    void UpdateSpacePartitionProgress(TreeNode *node);
    void UpdateOptimiseChunksProgress(TreeNode *node);

    bool OptimiseByOrder(const unsigned int chunkIndex, PGEOMETRY pGeometry, const Chunk &chunk, PGeometryETMTChunkMap &chunkMap);
    bool OptimiseByGeometry(const unsigned int chunkIndex, const Chunk &chunk, EffectTMTChunkMap &chunkMap);
    bool OptimiseByEffect(const unsigned int chunkIndex, const Chunk &chunk, MaterialTChunkMap &chunkMap);
    bool OptimiseByMaterial(const unsigned int chunkIndex, const Chunk &chunk, TextureChunkMap &chunkMap);
    bool OptimiseByTexture(const unsigned int chunkIndex, const Chunk &chunk, ChunkIndices &chunkMap);

    bool   TestVisibilityLeaf(TreeLeaf *origin, TreeLeaf *target);
    bool   TestVisibility(TreeLeaf *target, Matrix frustum);
    Matrix CreateVisibilityFrustum(TreeLeaf *origin, Vector3 forward, Vector3 up);

    void CreateNodeRenderList(TreeNode *node, std::vector<RenderGroup> &renderList, PVIEWPORT pViewport);
    void CreateLeafRenderList(TreeLeaf *leaf, std::vector<RenderGroup> &renderList);
    void CreateEntityRenderList(PENTITY entity, std::vector<RenderGroup> &renderList, PVIEWPORT pViewport);

    void RecurseDrawPartition(TreeNode *&node, std::vector<Vector3> &verts, std::vector<Chunk> &chunks);
    void RecurseOutputPartition(TreeNode *node, std::vector<OutputTreeNode> &outputNodes);
    void RecurseInputPartition(TreeNode *node, OutputTreeNode *outputNodes, int &numNodes);
    void RecurseSphereCollisionPartition(TreeNode *node, Math::Sphere sphere1, Math::Sphere sphere2, std::vector<TreeLeaf *> &leafs, std::vector<TreeLeaf *> &collidingLeafs);
    void RecursePVS(TreeNode *node, TreeLeaf *leaf);
    void RecursePVSSetOccluded(TreeNode *node, int leafIndex, Matrix occlusionFrustum);
    bool RecurseTestIntersect(TreeNode *node, std::list<IntersectionResult> &results,
                              const Vector3 &rayPos, const Vector3 &rayDir, DWORD flags, bool findFirst);

private:
    ProgressCallbackFunc progressCallback;
    void *pProgressCallbackPayload;
    float spacePartitionProgress,
          optimiseChunksProgress,
          pvsProgress;

    PSCENESETTINGS settings;

    LPDIRECT3DDEVICE9 D3DDevice;
    PRESOURCEMANAGER  resourceManager;

    int subdivisionsExecuted,
        numNodes;
    TreeNode *root;

    int numClusters,
        bytesPerCluster;
    byte *PVS;

    int lightIDs[8];

    std::vector< TreeLeaf * > leafs;
    int activeLeafID;

    std::map< int, int > leafPartitionOutlineOffsets;

    std::list< PGEOMETRY > geometry;
    std::list< std::pair < PGEOMETRY, PGEOMETRY > > optimisedGeometry;

    PGEOMETRY spacePartitionOutline;

    PENTITY sceneGraphRoot;
    std::vector< CollisionInfo > bodyCollisions;
    std::vector< CollisionInfo > worldCollisions;

    std::vector< _NodeChunkMap * > nodeChunkMap;
};

//! \typedef Scene *PSCENE
typedef Scene *PSCENE;

// EOF