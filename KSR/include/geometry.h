/*! \file */
/*-----------------------------------------------------------------------
	geometry.h

	Description: Geometry class definitions
	Author:		 Julian McKinlay
	---------------------------------------------------------------------*/

#pragma once

enum INDEXTYPE
{ 
	IT_16, /**< 16 bit indices. */
	IT_32, /**< 32 bit indices. */
};

//! \enum
enum CHUNKTYPE
{
	CT_POLYGON = 0,		/**< Polygon assembled as triangle fan. */
	CT_TRIANGLELIST,	/**< List of isolated triangles. */
	CT_TRIANGLESTRIP,	/**< Single triangle strip. For more information, see \ref triangles. */
	CT_LINELIST,		/**< Texture index for the first texture. */
	CT_LINESTRIP,		/**< Single triangle strip. For more information, see \ref triangles. */
	CT_POINTLIST,		/**< List of individual points. */
	CT_PATCH,			/**< Chunk defines the control vertices for a patch. */
};

static LPCTSTR CHUNKTYPENAMES[] = { "CT_POLYGON", "CT_TRIANGLELIST", "CT_TRIANGLESTRIP", "CT_LINELIST", "CT_LINESTRIP", "CT_POINTLIST", "CT_PATCH" };

//! \enum
enum VERTEXELEMENT_TYPE
{
	VET_FLOAT  = D3DDECLTYPE_FLOAT1,
	VET_FLOAT2 = D3DDECLTYPE_FLOAT2,
	VET_FLOAT3 = D3DDECLTYPE_FLOAT3,
	VET_FLOAT4 = D3DDECLTYPE_FLOAT4,
	VET_COLOUR = D3DDECLTYPE_D3DCOLOR,
};

//! \enum
enum VERTEXELEMENT_USAGE
{
	VEU_POSITION = D3DDECLUSAGE_POSITION,
	VEU_NORMAL = D3DDECLUSAGE_NORMAL,
	VEU_TEX = D3DDECLUSAGE_TEXCOORD,
	VEU_POINTSIZE = D3DDECLUSAGE_PSIZE,
	VEU_COLOUR = D3DDECLUSAGE_COLOR,
};

//! \enum
enum GEOMETRYFLAGS
{
	GF_ALLOWZEROCHUNKINDICES	= 1,	/**< If an index buffer is not specified, one will not be created. */
	GF_ALLOWZEROVERTEXINDICES	= 2,	/**< If an index buffer is not specified, one will not be created. */
	GF_APPENDCHUNKINDICES		= 4,	/**< The indices of the inserted geometry will be modified to start from the end of the existing indices.*/
	GF_APPENDVERTEXINDICES		= 8,	/**< The indices of the inserted geometry will be modified to start from the end of the existing indices.*/
};

//! \enum
enum RESOURCEOWNERSHIPFLAGS
{
	ROF_CHUNKS					= 1,
	ROF_VERTICES				= 2,
	ROF_CHUNKINDICES			= 4,
	ROF_VERTEXINDICES			= 8,
	ROF_ALL						= 15,	// The sum of all flags.
};

//! \enum
enum INTERSECTFLAGS
{
	IF_POLYGON			= 1 << CT_POLYGON,
	IF_TRIANGLELIST		= 1 << CT_TRIANGLELIST,
	IF_TRIANGLESTRIP	= 1 << CT_TRIANGLESTRIP,
	IF_ALL				= IF_POLYGON | IF_TRIANGLELIST | IF_TRIANGLESTRIP,
};

enum LOCKFLAGS
{
	LF_DISCARD = D3DLOCK_DISCARD,
	LF_READONLY = D3DLOCK_READONLY,
};

enum UNLOCKFLAGS
{
	UF_CHUNKINDICES = 1,
	UF_VERTEXINDICES = 2,
};

enum GEOMETRYCLEARFLAGS
{
	GCF_CHUNKS				= 1,
	GCF_CHUNKINDICES		= 2,
	GCF_VERTICES			= 4,
	GCF_VERTEXINDICES		= 8,
	GCF_SOURCEVERTEXINDICES = 16,
	GCF_ALL					= 31,	// The sum of all flags.
};

struct Geometry;
struct Scene;
struct OutputGeometry;
struct OutputScene;
struct OutputTreeNode;
struct OutputTreeLeaf;
struct OutputRenderGroup;
struct OutputFile;
class OptimisedGeometry;

//! \struct
struct RenderGroup
{
public:
	RenderGroup(Scene *pScene);

	RenderGroup(const RenderGroup &rhs);
	~RenderGroup();

	const RenderGroup &operator =(const RenderGroup &rhs);

#ifdef _SHOW_KSR_SIZES_
	static int ms_rgCount;
#endif

	const bool Greater(const RenderGroup &rhs) const;
	const bool Less(const RenderGroup &rhs) const;

	static bool GreaterOrder(const RenderGroup &r1, const RenderGroup &r2);
	static bool LessOrder(const RenderGroup &r1, const RenderGroup &r2);

	static bool Greater(const RenderGroup &r1, const RenderGroup &r2);
	static bool Less(const RenderGroup &r1, const RenderGroup &r2);

	Geometry *m_pGeometry;
	OptimisedGeometry *m_pOptimisedGeometry;
	Scene *m_pScene;
	unsigned int m_startFaceIndex;
	unsigned int m_numFaces;
	unsigned int m_order;
	int m_effectID;
	int m_techniqueID;
	int m_materialID;
	int m_lightIDs[8];

	bool m_hasTransform;
	Matrix m_transform;
};


struct TreeNode
{
public:
	TreeNode();
	TreeNode(const Vector3 &boundingMin, const Vector3 &boundingMax, TreeNode *parentNode, int uniqueId);
	~TreeNode();

	Vector3 m_min, m_max;

	int m_numChildren;
	int m_leafIndex;
	int m_id;

	int m_partitionOutlineChunkId;

	TreeNode *m_pParent;
	TreeNode **m_pChildren;
};


struct TreeLeaf
{
public:
	TreeLeaf(TreeNode *pParent = NULL);
	~TreeLeaf();

#ifdef _SHOW_KSR_SIZES_
	TreeLeaf(const TreeLeaf &orig);
	static int m_tlCount;
#endif

	TreeNode *m_pParent;

	const TreeLeaf &operator = (const TreeLeaf &rhs);

	Vector3 m_min, m_max;

	std::list<RenderGroup *> m_renderList;
};


//! \struct
struct Chunk
{
	Chunk();
	Chunk(const Chunk &rhs);
	Chunk(int texture0, int texture1, int materialID, int effectID, int techniqueId, int startVertexIndex, 
		  int numVertexIndices, int depthOrder, CHUNKTYPE chunkType, bool renderChunk = true);

	const Chunk &operator = (const Chunk &rhs);

	bool IsEquivalent(const Chunk &rhs);

	bool render;

	int idTexture0,		/**< Texture index for the first texture. */
		idTexture1,		/**< Texture index for the second texture. */
		material,		/**< Material index for this chunk. */
		effect,			/**< Effect index for this chunk. */
		technique;		/**< Id of the string handle of the effect technique for this chunk. */

	unsigned int startIndex;	/**< Vertex Index to start reading from. */

	union 
	{
		unsigned int numVerts;		/**< Number of Vertex Indices to read. */
		unsigned int numIndices;	/**< Number of Vertex Indices to read. */
	};

	int depthBias;		/**< Coplanar rendering order. Must not exceed the numDepthLevels value of KSRViewportSettings for
							 any Viewport that will render this chunk. */

	unsigned int order;	/**< global rendering order. If used incorrectly, this variable can significantly reduce performance. */

	CHUNKTYPE type;		/**< Primitive Type. For more information, see \ref geometry. */
};

// Maps old chunk indices with the vector of new chunk indices that replace them.
typedef std::map< unsigned int, std::vector< unsigned int > > PartitionChunkMap;

struct IntersectionResult;

//! \struct
struct IntersectionResult
{
	IntersectionResult();
	IntersectionResult(const IntersectionResult &result);
	IntersectionResult(Geometry *pGeometry, unsigned int chunkID, unsigned int vertexID0,
					   unsigned int vertexID1, unsigned int vertexID2, float u, float v, float distance);

	const IntersectionResult &operator = (const IntersectionResult &rhs);
	bool operator < (const IntersectionResult &rhs);
	bool operator > (const IntersectionResult &rhs);
	bool operator <= (const IntersectionResult &rhs);
	bool operator >= (const IntersectionResult &rhs);
	bool operator == (const IntersectionResult &rhs);
	bool operator != (const IntersectionResult &rhs);

	Geometry *m_pGeometry;
	unsigned int m_chunkID,
				 m_vertexID0,
				 m_vertexID1,
				 m_vertexID2;

	float m_u, m_v, m_distance;
};

//! \struct
struct VertexFormatElement
{
public:
	VertexFormatElement(unsigned int _offset = 0, VERTEXELEMENT_TYPE _type = VET_FLOAT3,
						VERTEXELEMENT_USAGE _usage = VEU_POSITION, unsigned int _usageIndex = 0)
	{
		offset = _offset;
		type = _type;
		usage = _usage;
		usageIndex = _usageIndex;
	}

	unsigned int offset;

	VERTEXELEMENT_TYPE type;
	VERTEXELEMENT_USAGE usage;

	unsigned int usageIndex;
};

//! \struct
struct VertexDeclarationFormat
{
public:
	std::vector<VertexFormatElement> elements;
};

//! \struct Geometry
struct Geometry : public MemObject
{
public:
	typedef std::vector< unsigned int > VertexIndexList;
	typedef std::vector< std::pair< unsigned int, VertexIndexList > > ChunkIndexList;
	typedef std::vector< std::pair< unsigned int, ChunkIndexList > > OrderedChunkIndexLists;

	Geometry(LPDIRECT3DDEVICE9 pDevice, DWORD resourceOwnershipFlags = 0);
	~Geometry();

	const Chunk &operator [](const unsigned int index) const { return m_pChunks[index]; }

	void Release();

	//! \brief Initialises formatting
	/*! Returns S_OK on success and E_FAIL on failure.
		\param vertexSize Stride of each vertex, in bytes.
		\param vertexFormat Vertex format.
		\param indexFormat Index format.*/
	HRESULT Init(int vertexSize, DWORD vertexFormat, DWORD indexFormat);

	//! \brief Initialises formatting
	/*! Returns S_OK on success and E_FAIL on failure.
		\param vertexSize Stride of each vertex, in bytes.
		\param vertexFormat Vertex format.
		\param indexFormat Index format.*/
	HRESULT Init(int vertexSize, const VertexDeclarationFormat &vertexFormat, DWORD indexFormat);

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
	HRESULT Insert(int nChunks, int nVerts, int nChunkIndices, int nVertexIndices,
				   void *pChunks, void *pVerts, void *pChunkIndices, void *pVertexIndices, DWORD flags);

	//! \brief Inserts geometry data
	/*! Returns S_OK on success and E_FAIL on failure.
		\param nChunks Number of chunks stored in the pChunks array.
		\param nVerts Number of vertices stored in the pVerts array.
		\param pChunks [in] An array of chunks to be inserted.
		\param pVertices [in] An array of pVertices to be inserted.*/
	HRESULT Insert(int nChunks, int nVerts, void *pChunks, void *pVerts, DWORD flags);

	//! \brief Inserts geometry data
	/*! Returns S_OK on success and E_FAIL on failure.
		\param pGeometry [in] Pointer to a Geometry structure, containing the geometry data to insert. */
	HRESULT Insert(Geometry *pGeometry, DWORD flags);

	//! \brief Inserts a Scene that uses this Geometry object.
	/*! Returns S_OK on success and E_FAIL on failure.
		\param pScene [in] Pointer to a Scene structure, representing the scene to insert. */
	//HRESULT InsertScene(Scene *pScene);

	//! \brief Removes a Scene that uses this Geometry object.
	/*! Returns S_OK on success and E_FAIL on failure.
		\param pScene [in] Pointer to a Scene structure, representing the scene to remove. */
	//HRESULT RemoveScene(Scene *pScene);

	//! \brief Genereates default index buffers
	/*! Returns S_OK on success and E_FAIL on failure.*/
	HRESULT GenerateIndexBuffers(DWORD flags);

	//! \brief Locks and retrieves access to geometry data.
	/*! Returns S_OK on success and E_FAIL on failure.
		\param pChunks [out] Pointer to a buffer representing the locked chunk data.
		\param pVerts [out] Pointer to a buffer representing the locked vertex data.
		\param pChunkIndices [out] Pointer to a buffer representing the locked chunk index data
		\param pVertexIndices [out] Pointer to a buffer representing the locked vertex index data.*/
	//HRESULT Lock(void **pChunks, void **pVerts, void **pChunkIndices, void **pVertexIndices, void **pSourceVertexIndices = NULL, DWORD flags = 0);
	HRESULT Lock(void **ppChunks, void **ppVerts, void **ppChunkIndices, void **ppVertexIndices, DWORD flags = 0);

	//! \brief Unlocks and geometry data.
	/*! Returns S_OK on success and E_FAIL on failure.*/
	HRESULT Unlock(DWORD unlockFlags = UF_CHUNKINDICES | UF_VERTEXINDICES);

	//! \brief Erases stored geometry.
	/*! Returns S_OK on success and E_FAIL on failure.*/
	HRESULT Clear(DWORD flags = GCF_ALL);

	//! \brief Erases stored geometry and resets formatting.
	/*! Returns S_OK on success and E_FAIL on failure.*/
	HRESULT Reset();

	//! \brief Allows or disallows the geometry object to render
	/*! \param hide [in] flag to indicate if the geometry should be prevented from rendering.*/
	void Hide(bool hide) { m_hide = hide; }

	//! \brief Allows or disallows the geometry object to render
	/*! Returns true if the Geometry object should be prevented from rendering.*/
	const bool IsHidden() const { return m_hide; }

	void SetScene(PSCENE pScene, bool notify = false);

	HRESULT Serialise(OutputGeometry *pOutputGeometry);
	HRESULT Reassemble(OutputGeometry *pOutputGeometry);

	bool Intersect(IntersectionResult &result, const Vector3 &rayPos, const Vector3 &rayDir, RenderGroup *pGroup, DWORD flags);
	bool Intersect(std::list<IntersectionResult> &results, const Vector3 &rayPos, const Vector3 &rayDir, RenderGroup *pGroup, DWORD flags);

	const DWORD GetIndexGenerationFlags() const { return m_indexGenerationFlags; }
	const DWORD GetOwnershipFlags() const { return m_resourceOwnershipFlags; }
	const DWORD GetVertexFormat() const { return m_FVF; };
	const unsigned int GetIndexFormat() const { return m_indexType; };
	const unsigned int GetVertexSize() const { return m_vertexLength; };
	const unsigned int GetIndexSize() const { return m_indexLength; };

	const unsigned int GetNumChunks() const { return m_numChunks; };
	const unsigned int GetNumVertices() const { return m_numVerts; };
	const unsigned int GetNumChunkIndices() const { return m_numChunkIndices; };
	const unsigned int GetNumVertexIndices() const { return m_numVertexIndices; };

	const Vector3 &GetMin() const { return m_min; };
	const Vector3 &GetMax() const { return m_max; };

	Chunk &GetChunk(const unsigned int index) { return m_pChunks[index]; }
	const Chunk &GetChunk(const unsigned int index) const { return m_pChunks[index]; }
	
	void *GetChunkIndices() { return m_pChunkIndices; }
	const void *GetChunkIndices() const { return m_pChunkIndices; }

	const unsigned int GetChunkIndex(const unsigned int index) const
	{
		return *((unsigned int *)(((char *)m_pChunkIndices) + index * m_indexLength));
	}

	const PSCENE GetScene() const { return m_pScene; }
	PSCENE GetScene() { return m_pScene; }

	const Vector3 GetVertexPosition(LPVOID pVertsLocked, unsigned int index) const;

	const bool GetUseVertexDeclarationFormat() const { return m_useVertexDeclarationFormat; }
	const VertexDeclarationFormat &GetVertexDeclarationFormat() const { return m_vertexDeclarationFormat; }
	const LPDIRECT3DVERTEXDECLARATION9 GetVertexDeclaration() const { return m_pVertexDeclaration; }

	LPDIRECT3DVERTEXBUFFER9 GetVertexBuffer();
	LPDIRECT3DINDEXBUFFER9  GetIndexBuffer();

	LPDIRECT3DDEVICE9 GetD3DDevice() { return m_pDevice; }

private:
	PSCENE m_pScene;

	DWORD m_FVF,					/**< Vertex format used by the vertices of this geometry. */
		  m_indexType;				/**< Index format used by the indices of this geometry. */

	unsigned int m_vertexLength,			/**< Length of each vertex in this geometry, in bytes. */
				 m_indexLength,				/**< Length of each index in this geometry, in bytes. */
				 m_numChunks,				/**< Number of chunks contained in this geometry. */
				 m_numVerts,				/**< Number of vertices contained in this geometry. */
				 m_numChunkIndices,			/**< Number of indices contained in this geometry. */
				 m_numVertexIndices;		/**< Number of vertex indices contained in this geometry. */

	DWORD m_indexGenerationFlags,
		  m_resourceOwnershipFlags;

	Vector3 m_min,
			m_max;

	VertexDeclarationFormat m_vertexDeclarationFormat;
	bool m_useVertexDeclarationFormat;

	Chunk *m_pChunks;
	void *m_pChunkIndices;

	bool m_vertexBufferLocked,
		 m_indexBufferLocked,
		 m_hide;

	LPDIRECT3DVERTEXDECLARATION9 m_pVertexDeclaration;

	LPDIRECT3DVERTEXBUFFER9 m_pVertexBuffer;
	LPDIRECT3DINDEXBUFFER9 m_pIndexBuffer;
	LPDIRECT3DDEVICE9 m_pDevice;

	//Geometry *m_pSharedResources[4];

//private:
	bool Intersect(std::list<IntersectionResult> &results, const Vector3 &rayPos, const Vector3 &rayDir, RenderGroup *pGroup, DWORD flags, bool findFirst);
};

// EOF