/*! \file */
/*-----------------------------------------------------------------------
	scene.h

	Description: Scene and related class declarations
	Author:		 Julian McKinlay
	---------------------------------------------------------------------*/

#pragma once

#ifdef _DEBUG
#include <string>

//#define DEBUG_DUMP

	#ifdef DEBUG_DUMP
		#pragma message "DEBUG_DUMP is activated, performance will be severely curtailed."
	#endif
#endif

class OptimisedGeometry
{
public:
	OptimisedGeometry(Geometry *pSource)
	{
		m_pSource = pSource;

		m_indexStride = m_pSource->GetIndexFormat() == IT_32 ? 4 : 2;

		m_pChunkIndices = NULL;
		m_pIndexBuffer = NULL;

		m_numChunkIndices = NULL;
		m_numVertexIndices = NULL;

		m_indexBufferLocked = false;
	}

	~OptimisedGeometry()
	{
		if (m_pChunkIndices)
		{
			free(m_pChunkIndices);
		}

		std::map< unsigned int, std::vector< unsigned int > * >::iterator i = m_chunkIndexMap.begin();
		for (; i != m_chunkIndexMap.end(); i++)
		{
			delete i->second;
		}

		for (i = m_vertexIndexMap.begin(); i != m_vertexIndexMap.end(); i++)
		{
			delete i->second;
		}

		m_chunkIndexMap.clear();
		m_vertexIndexMap.clear();

		if (m_pIndexBuffer)
			m_pIndexBuffer->Release();
	}

	void InsertChunkIndex(unsigned int srcIndex, unsigned int chunkIndex)
	{
		GetChunkIndices(srcIndex)->push_back(chunkIndex);
		m_numChunkIndices++;
	}

	void InsertVertexIndex(unsigned int srcIndex, unsigned int vertexIndex)
	{
		GetVertexIndices(srcIndex)->push_back(vertexIndex);
		m_numVertexIndices++;
	}

	std::vector< unsigned int > *GetChunkIndices(unsigned int srcIndex)
	{
		if (!(!m_chunkIndexMap.empty() && m_chunkIndexMap.find(srcIndex) != m_chunkIndexMap.end()))
		{
			m_chunkIndexMap[srcIndex] = new std::vector< unsigned int >;
		}

		return m_chunkIndexMap[srcIndex];
	}

	void *GetChunkIndices() const
	{
		return m_pChunkIndices;
	}

	std::vector< unsigned int > *GetVertexIndices(unsigned int srcIndex)
	{
		if (!(!m_vertexIndexMap.empty() && m_vertexIndexMap.find(srcIndex) != m_vertexIndexMap.end()))
		{
			m_vertexIndexMap[srcIndex] = new std::vector< unsigned int >;
		}

		return m_vertexIndexMap[srcIndex];
	}

	void CreateVertexIndices(LPDIRECT3DDEVICE9 pDevice)
	{
		if (m_vertexIndexMap.empty() || !pDevice || !m_pSource)
			return;

		D3DFORMAT indexFormat = m_pSource->GetIndexFormat() == IT_32 ? D3DFMT_INDEX32 : D3DFMT_INDEX16;
		unsigned int indexStride = indexFormat == D3DFMT_INDEX32 ? 4 : 2;

		pDevice->CreateIndexBuffer(indexStride * m_numVertexIndices, 0, indexFormat, D3DPOOL_MANAGED, &m_pIndexBuffer, NULL);

		void *pIndicesLocked = NULL;
		m_pIndexBuffer->Lock(0, 0, (void **)&pIndicesLocked, 0);

		void *pIndicesLockedIndex = pIndicesLocked;
		std::map< unsigned int, std::vector< unsigned int > * >::iterator i = m_vertexIndexMap.begin();
		for (; i != m_vertexIndexMap.end(); i++)
		{
			if (!i->second)
				continue;

			unsigned int vectorSize = m_indexStride * i->second->size();
			memcpy(pIndicesLockedIndex, &(*i->second)[0], vectorSize);
			pIndicesLockedIndex = (void *)(((char *)pIndicesLockedIndex) + vectorSize);
		}

		m_pIndexBuffer->Unlock();
	}

	void CreateChunkIndices()
	{
		if (m_chunkIndexMap.empty() || !m_pSource)
			return;

		if (m_pChunkIndices)
		{
			free(m_pChunkIndices);
		}

		D3DFORMAT indexFormat = m_pSource->GetIndexFormat() == IT_32 ? D3DFMT_INDEX32 : D3DFMT_INDEX16;

		m_pChunkIndices = malloc(m_indexStride * m_numChunkIndices);
		void *m_pChunkIndicesIndex = m_pChunkIndices;

		std::map< unsigned int, std::vector< unsigned int > * >::iterator i = m_chunkIndexMap.begin();
		for (; i != m_chunkIndexMap.end(); i++)
		{
			if (!i->second)
				continue;

			unsigned int vectorSize = m_indexStride * i->second->size();
			memcpy(m_pChunkIndicesIndex, &((*i->second)[0]), vectorSize);
			m_pChunkIndicesIndex = (void *)(((char *)m_pChunkIndicesIndex) + vectorSize);
		}
	}

	const Chunk &GetChunk(unsigned int index) const { return m_chunks[index]; }

	unsigned int GetChunkIndex(unsigned int index) const 
	{ 
		return *((unsigned int *)(((char *)m_pChunkIndices) + index * m_indexStride));
	}

	LPDIRECT3DINDEXBUFFER9 GetIndexBuffer() const { return m_pIndexBuffer; }

	const unsigned int GetNumChunks() const { return (unsigned int)m_chunks.size(); }
	const unsigned int GetNumChunkIndices() const { return m_numChunkIndices; }
	const unsigned int GetNumVertexIndices() const { return m_numVertexIndices; }

//private:
	PGEOMETRY m_pSource;

	std::vector<Chunk> m_chunks;
	void *m_pChunkIndices;
	LPDIRECT3DINDEXBUFFER9 m_pIndexBuffer;

	// do we need some kind of struct at the chunk level of node chunk map
	std::vector<int> m_newIndices;	//? should we resize an array ourselves each partition?
	bool *m_pChunksUsingNewIndices; //? do we know how many chunks we'll have?

	unsigned int m_numChunkIndices,			//!< Number of indices contained in this geometry.
				 m_numVertexIndices,		//!< Number of vertex indices contained in this geometry.
				 m_indexStride;

	std::map< unsigned int, std::vector< unsigned int > * > m_chunkIndexMap;	// maps each original chunk index to a vector of the new chunk indices associated with that index.

	std::map< unsigned int, std::vector< unsigned int > * > m_vertexIndexMap;	// maps each new chunk index to a list of vertex indices

	bool m_indexBufferLocked;
};

typedef OptimisedGeometry * POPTIMISEDGEOMETRY;
typedef std::map< PGEOMETRY, POPTIMISEDGEOMETRY > OptimisedGeometryMap;

//! \struct Scene
struct Scene : public MemObject
{
private:
	typedef void (*ProgressCallbackFunc)(DWORD, float, void *);
	typedef void (*PVSGeneratorCallbackFunc)(void *, std::vector< byte * > *, ProgressCallbackFunc, void *);

	typedef int MaterialID;
	typedef int EffectID;
	typedef int EffectTechniqueID;
	typedef int TextureID;
	typedef unsigned int ChunkIndex;

	typedef std::vector< unsigned int > VertexIndexList;
	struct _VertexIndexList 
	{
		_VertexIndexList()
		{ }

		_VertexIndexList(const _VertexIndexList &rhs)
			: m(rhs.m)
		{ }

		~_VertexIndexList()
		{ }

		VertexIndexList m;

		const bool IsEmpty() const
		{
			return m.empty();
		}

		void PushBack(const unsigned int val) { m.push_back(val); }

		const size_t Size() const { return m.size(); }

		const unsigned int operator[] (const unsigned int idx) const { return idx >= m.size() ? (size_t)-1 : m[idx]; }

		unsigned int operator[] (const unsigned int idx) { return idx >= m.size() ? (size_t)-1 : m[idx]; }
	};

	typedef std::vector< _VertexIndexList * > IndexSet;
	struct _IndexSet
	{
		_IndexSet()
		{ }

		_IndexSet(const _IndexSet &rhs)
		{ }

		IndexSet m;

		~_IndexSet()
		{ 
			for (IndexSet::iterator i = m.begin(); i != m.end(); i++)
			{
				delete *i;
			}
		}

		const bool IsEmpty() const
		{
			IndexSet::const_iterator i = m.begin();
			for (; i != m.end(); i++)
			{
				if (!(*i)->IsEmpty())
					return false;
			}

			return true;
		}

		const size_t Size() const { return m.size(); }

		const _VertexIndexList * operator[] (const unsigned int idx) const 
		{ 
			if (idx > m.size())
				return NULL;
			else
				return m[idx];
		}

		_VertexIndexList * operator[] (const unsigned int idx)
		{ 
			if (idx > m.size())
				return NULL;
			else
				return m[idx];
		}

		const unsigned int Create()
		{
			m.push_back(new _VertexIndexList);

			return (unsigned int)m.size() - 1;
		}
	};

	typedef std::map< ChunkIndex, _IndexSet * > ChunkIndexList;
	struct _ChunkIndexList
	{
		_ChunkIndexList()
		{ }

		_ChunkIndexList(const _ChunkIndexList &rhs)
		{ }

		ChunkIndexList m;

		~_ChunkIndexList()
		{ 
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

		const _IndexSet * operator[] (const ChunkIndex idx) const 
		{ 
			ChunkIndexList::const_iterator i = m.find(idx);
			if (i == m.end())
				return NULL;
			else
				return i->second;
		}

		_IndexSet *GetPtr(const ChunkIndex idx)
		{
			if (m.find(idx) == m.end())
			{
				m[idx] = new _IndexSet;
			}
			return m[idx];
		}
	};

	typedef std::map< PGEOMETRY, _ChunkIndexList * > GeometryMap;
	struct _GeometryMap
	{
		_GeometryMap()
		{ }

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
			}
			return m[idx];
		}
	};

	typedef std::map< unsigned int, _GeometryMap * > OrderedGeometryMap;
	struct _OrderedGeometryMap
	{
		_OrderedGeometryMap()
		{ }

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
			}

			return m[idx];
		}
	};

	typedef std::vector< _OrderedGeometryMap * > NodeChunkMap;
	struct _NodeChunkMap
	{
		_NodeChunkMap()
		{ }

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
	typedef std::map< int, int > LeafPartitionOutlineOffsetMap;

	/*enum eChumkMapType { eCMT_TRIANGLES = 0, eCMT_LINES, eCMT_POINTS };
	typedef std::pair< eChumkMapType, std::vector< int > > ChunkMap;	//!< Stores chunk indices
	typedef std::vector< ChunkMap > todo2;
																				//typedef std::vector< todo2 > todo3;
	typedef std::pair< PGEOMETRY, todo2 > GeometryMap;
	/*/
	//typedef std::pair< PGEOMETRY, std::vector< std::vector< std::vector< int > > > > GeometryMap;
	//typedef std::pair< PGEOMETRY, std::vector< std::vector< int > > > GeometryMap;
	//*/
	//typedef std::vector< GeometryMap > NodeChunkMap;

public:
	//typedef GeometryList::const_iterator ConstGeometryIterator;
	typedef GeometryList::const_iterator ConstGeometryIterator;

	Scene(PSCENESETTINGS pSettings, LPDIRECT3DDEVICE9 pDevice, PRESOURCEMANAGER pResourceManager);
	~Scene();

	HRESULT InsertGeometry(PGEOMETRY pGeometry);
	HRESULT RemoveGeometry(PGEOMETRY pGeometry, bool notify = false);
	HRESULT UpdateGeometry(PGEOMETRY pGeometry);

	//! \brief Inserts an Entity and attaches it to the scene graph root.
	/*! Returns S_OK on success and E_FAIL on failure.
		\param pEntity [out] Address of a pointer to a KSREntity structure, representing the entity to insert.*/
	HRESULT InsertEntity(PENTITY pEntity);

	//! \brief Removes an entity from the scene.
	/*! Returns S_OK on success and E_FAIL on failure.
		\param pEntity [in] Pointer to the KSREntity structure to remove.*/
	HRESULT RemoveEntity(PENTITY pEntity);

#if 0
	//! \brief Inserts a Patch.
	/*! Returns S_OK on success and E_FAIL on failure.
		\param pPatch [in] Address of a pointer to a KSRPatch structure, representing the patch to insert.*/
	HRESULT InsertPatch(PPATCH pPatch);
#endif

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

	HRESULT SetPVSGeneratorCallback(PVSGeneratorCallbackFunc pvsGeneratorCallbackFunc);

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

	void SetLightID(unsigned int index, int lightID) { if (index < 8) m_lightIDs[index] = lightID; }

	//! Updates Redraw state
	/*! \param r New Redraw state.*/
	void SetRedraw(DWORD r) { m_pSceneGraphRoot->SetRedraw(r); };

	//! Updates Update state
	/*! \param r New Update state.*/
	void SetUpdate(DWORD u) { m_pSceneGraphRoot->SetUpdate(u); };

	void SetActiveLeaf(int id) { m_activeLeafID = id; }
	int GetActiveLeaf() { return m_activeLeafID; }

	//! \brief Gets the scene's boundaries
	/*! Returns S_OK on success and E_FAIL on failure.
		\param min 3D Vector defining the lower-left corner of the world.
		\param max 3D Vector defining the upper-right corner of the world.*/
	HRESULT GetWorldBounds(Vector3 *pMin, Vector3 *pMax);

	DWORD GetRedraw() { return m_pSceneGraphRoot->GetRedraw(); };
	DWORD GetUpdate() { return m_pSceneGraphRoot->GetUpdate(); };

	int	GetSubdivisionsExecuted();
	int	GetNumLeafs();

	void *GetProgressCallbackPayload() { return m_pProgressCallbackPayload; }

	const int GetLightID(unsigned int index) const { return index < 8 ? m_lightIDs[index] : -1; }

	static void GeneratePVS(void *pData, std::vector< byte * > *pResults, ProgressCallbackFunc, void *);

private:
	void Reset(bool recreate = false, bool resetLights = true);
	void Clear();
	void Init();

	void Partition(TreeNode *node, int numSubdivisions, _NodeChunkMap *pNodeChunkMap, _NodeChunkMap *pRemainderNodeChunkMap, unsigned int numPolygons = 0);
	_NodeChunkMap *CreateNodeChunkMap(unsigned int *pNumPolygons);
	_NodeChunkMap *CreateNodeChunkMap(_NodeChunkMap *pNodeChunkMap, _NodeChunkMap *pRemainder, 
								      const Math::AABB &bounds, unsigned int *pNumPolygons);

	void GenerateLeaf(TreeNode *pNode, _NodeChunkMap *NodeChunkMap);
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

	void RecurseCreateDrawPartition(TreeNode *node, std::vector<Vector3> &verts, std::vector<Chunk> &lineChunks,
									std::vector<Chunk> &faceChunks, std::vector<unsigned int> &vertexIndices);
	void RecurseOutputPartition(TreeNode *node, std::vector<OutputTreeNode> &outputNodes);
	void RecurseInputPartition(TreeNode *node, OutputTreeNode *outputNodes, int &numNodes);
	void RecurseSphereCollisionPartition(TreeNode *node, Math::Sphere sphere1, Math::Sphere sphere2, std::vector<TreeLeaf *> &leafs, std::vector<TreeLeaf *> &collidingLeafs);
	void RecursePVS(TreeNode *node, TreeLeaf *leaf);
	void RecursePVSSetOccluded(TreeNode *node, int leafIndex, Matrix occlusionFrustum);
	bool RecurseTestIntersect(TreeNode *node, std::list<IntersectionResult> &results, 
							  const Vector3 &rayPos, const Vector3 &rayDir, DWORD flags, bool findFirst);

#ifdef _DEBUG
	void DumpNodeChunkMap(const _NodeChunkMap *pNodeChunkMap, const unsigned int index) const;
	void DumpNodeChunkMap() const;
	void DumpPartitionTree() const;
	void DumpLeafs() const;
#endif

private:
	ProgressCallbackFunc m_pFnProgressCallback;
	void *m_pProgressCallbackPayload;
	float m_spacePartitionProgress,
		  m_optimiseChunksProgress,
		  m_pvsProgress;

	PSCENESETTINGS m_pSettings;

	LPDIRECT3DDEVICE9 m_pD3DDevice;
	PRESOURCEMANAGER  m_pResourceManager;

	int m_subdivisionsExecuted,
		m_numNodes;
	TreeNode *m_pRoot;

	int m_numClusters,
		m_bytesPerCluster;
	//byte *PVS;
	std::vector< byte * > m_PVS;

	int m_lightIDs[8];

	std::vector< TreeLeaf * > m_leafs;
	int m_activeLeafID;

	LeafPartitionOutlineOffsetMap m_leafPartitionOutlineOffsets;

	GeometryList m_sourceGeometry;
	OptimisedGeometryMap m_optimisedGeometry;

	PGEOMETRY m_pSpacePartitionOutline;

	PENTITY m_pSceneGraphRoot;
	std::vector< CollisionInfo > m_bodyCollisions;
	std::vector< CollisionInfo > m_worldCollisions;

	std::vector< _NodeChunkMap * > m_nodeChunkMap;
};

//! \typedef Scene *PSCENE
typedef Scene *PSCENE;

// EOF