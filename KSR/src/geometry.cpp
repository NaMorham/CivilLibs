#include "../include/ksr.h"

#include "../include/leakwatcher.h"

#ifdef _DO_MEMORY_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace KSR;

#ifdef _SHOW_KSR_SIZES_
	int RenderGroup::ms_rgCount = 0;
#endif

//-----------------------------------------------------------------------
	RenderGroup::RenderGroup(Scene *pScene)
		:	m_pGeometry(NULL), m_pOptimisedGeometry(NULL), m_pScene(pScene), m_startFaceIndex(-1), m_numFaces(0), m_order(0),
			m_effectID(-1), m_techniqueID(-1), m_materialID(-1), m_hasTransform(false)
	//-------------------------------------------------------------------
	{
		memset(m_lightIDs, -1, sizeof(int) * 8);

		D3DXMatrixIdentity(&m_transform);

#ifdef _SHOW_KSR_SIZES_
		ms_rgCount++;
		Logf(LL_LOWEST, _T("RenderGroup::RenderGroup(), count = %d"), ms_rgCount);
#endif
	}


//-----------------------------------------------------------------------
	RenderGroup::RenderGroup(const RenderGroup &rhs)
		:	m_pGeometry(rhs.m_pGeometry), m_pOptimisedGeometry(rhs.m_pOptimisedGeometry), m_pScene(rhs.m_pScene), m_startFaceIndex(rhs.m_startFaceIndex), m_numFaces(rhs.m_numFaces),
			m_order(rhs.m_order), m_effectID(rhs.m_effectID), m_techniqueID(rhs.m_techniqueID), m_materialID(rhs.m_materialID),
			m_hasTransform(rhs.m_hasTransform), m_transform(rhs.m_transform)
	//-------------------------------------------------------------------
	{
		memcpy(m_lightIDs, rhs.m_lightIDs, sizeof(int) * 8);

#ifdef _SHOW_KSR_SIZES
		ms_rgCount++;
		Logf(LL_LOWEST, _T("RenderGroup::RenderGroup(const RenderGroup &orig), count = %d"), ms_rgCount);
#endif
	}


//-----------------------------------------------------------------------
	RenderGroup::~RenderGroup()
	//-------------------------------------------------------------------
	{
#ifdef _SHOW_KSR_SIZES_
		ms_rgCount--;
		Logf(LL_LOWEST, _T("RenderGroup::~RenderGroup(), count = %d"), ms_rgCount);
#endif
	}


//-----------------------------------------------------------------------
	const RenderGroup &RenderGroup::operator =(const RenderGroup &rhs)
	//-------------------------------------------------------------------
	{
		m_pGeometry = rhs.m_pGeometry;
		m_pOptimisedGeometry = rhs.m_pOptimisedGeometry;
		m_pScene = rhs.m_pScene;

		m_startFaceIndex = rhs.m_startFaceIndex;
		m_numFaces = rhs.m_numFaces;
		m_order = rhs.m_order;

		m_effectID = rhs.m_effectID;
		m_techniqueID = rhs.m_techniqueID;
		m_materialID = rhs.m_materialID;
		memcpy(m_lightIDs, rhs.m_lightIDs, sizeof(int) * 8);

		m_hasTransform = rhs.m_hasTransform;
		m_transform = rhs.m_transform;

		return *this;
	}


//-----------------------------------------------------------------------
	const bool RenderGroup::Greater(const RenderGroup &rhs) const
	//-------------------------------------------------------------------
	{
		if (m_order > rhs.m_order)
			return true;

		if (m_order == rhs.m_order)
		{
			if (m_pOptimisedGeometry && !rhs.m_pOptimisedGeometry)
			{
				return true;
			}
			else if (m_pGeometry && !rhs.m_pGeometry)
			{
				return false;
			}
			else if (m_pOptimisedGeometry && m_pOptimisedGeometry > rhs.m_pOptimisedGeometry)
			{
				return true;
			}
			else if (m_pGeometry && m_pGeometry > rhs.m_pGeometry)
			{
				return true;
			}

			if ((m_pGeometry && m_pGeometry == rhs.m_pGeometry) || (m_pOptimisedGeometry && m_pOptimisedGeometry == rhs.m_pOptimisedGeometry))
			{
				if (m_effectID > rhs.m_effectID)
					return true;

				if (m_effectID == rhs.m_effectID)
				{
					if (m_techniqueID > rhs.m_techniqueID)
						return true;

					if (m_techniqueID == rhs.m_techniqueID)
					{
						if (m_materialID > rhs.m_materialID)
							return true;
					}
				}
			}
		}

		return false;
	}


//-----------------------------------------------------------------------
	const bool RenderGroup::Less(const RenderGroup &rhs) const
	//-------------------------------------------------------------------
	{
		if (m_order < rhs.m_order)
			return true;

		if (m_order == rhs.m_order)
		{
			if (m_pOptimisedGeometry && !rhs.m_pOptimisedGeometry)
			{
				return false;
			}
			else if (m_pGeometry && !rhs.m_pGeometry)
			{
				return true;
			}
			else if (m_pOptimisedGeometry && m_pOptimisedGeometry < rhs.m_pOptimisedGeometry)
			{
				return true;
			}
			else if (m_pGeometry && m_pGeometry < rhs.m_pGeometry)
			{
				return true;
			}

			if ((m_pGeometry && m_pGeometry == rhs.m_pGeometry) || (m_pOptimisedGeometry && m_pOptimisedGeometry == rhs.m_pOptimisedGeometry))
			{
				if (m_effectID < rhs.m_effectID)
					return true;

				if (m_effectID == rhs.m_effectID)
				{
					if (m_techniqueID < rhs.m_techniqueID)
						return true;

					if (m_techniqueID == rhs.m_techniqueID)
					{
						if (m_materialID < rhs.m_materialID)
							return true;
					}
				}
			}
		}

		return false;
	}


//-----------------------------------------------------------------------
	bool RenderGroup::GreaterOrder(const RenderGroup &r1, const RenderGroup &r2)
	//-------------------------------------------------------------------
	{
		return r1.m_order > r2.m_order;
	}


//-----------------------------------------------------------------------
	bool RenderGroup::LessOrder(const RenderGroup &r1, const RenderGroup &r2)
	//-------------------------------------------------------------------
	{
		return r1.m_order < r2.m_order;
	}


//-----------------------------------------------------------------------
	bool RenderGroup::Greater(const RenderGroup &r1, const RenderGroup &r2)
	//-------------------------------------------------------------------
	{
		return r1.Greater(r2);
	}


//-----------------------------------------------------------------------
	bool RenderGroup::Less(const RenderGroup &r1, const RenderGroup &r2)
	//-------------------------------------------------------------------
	{
		return r1.Less(r2);
	}


//-----------------------------------------------------------------------
	TreeNode::TreeNode()
	//-------------------------------------------------------------------
	{
		m_min = m_max = Vector3(0, 0, 0);

		m_numChildren = 0;
		m_leafIndex = -1;
		m_id = 0;

		m_partitionOutlineChunkId = -1;

		m_pParent = NULL;
		m_pChildren = NULL;
	}


//-----------------------------------------------------------------------
	TreeNode::TreeNode(const Vector3 &boundingMin, const Vector3 &boundingMax, TreeNode *parentNode, int uniqueId)
	//-------------------------------------------------------------------
	{
		m_min = boundingMin;
		m_max = boundingMax;

		m_numChildren = 0;
		m_leafIndex = -1;
		m_id = uniqueId;

		m_partitionOutlineChunkId = -1;

		m_pParent = parentNode;
		m_pChildren = NULL;
	}


//-----------------------------------------------------------------------
	TreeNode::~TreeNode()
	//-------------------------------------------------------------------
	{
		if (m_pChildren)
		{
			for (int i = 0; i < m_numChildren; i++)
			{
				if (m_pChildren[i])
				{
					delete m_pChildren[i];
					m_pChildren[i] = NULL;
				}
			}

			delete[] m_pChildren;
			m_pChildren = NULL;
		}
	}


//-----------------------------------------------------------------------
	TreeLeaf::TreeLeaf(TreeNode *pParent)
	//-------------------------------------------------------------------
	{
		m_pParent = pParent;
		m_min = Vector3(0, 0, 0);
		m_max = Vector3(0, 0, 0);

		m_renderList.clear();

#ifdef _SHOW_KSR_SIZES_
		m_tlCount++;
		Logf(LL_LOWEST, "TreeLeaf::TreeLeaf(), count = %d", tlCount);
#endif
	}


//-----------------------------------------------------------------------
	TreeLeaf::~TreeLeaf()
	//-------------------------------------------------------------------
	{
		std::list<RenderGroup *>::iterator i = m_renderList.begin();
		for (; i != m_renderList.end(); i++)
		{
			if (*i)
			{
				delete (*i);
				(*i) = NULL;
			}
			else
				Logf(LL_LOWEST, "Warning: Skipping NULL RenderGroup *");

			//i = renderList.erase(i);
		}

#ifdef _SHOW_KSR_SIZES_
		m_tlCount--;
		Logf(LL_LOWEST, "TreeLeaf::~TreeLeaf(), count = %d", tlCount);
#endif
	}


#ifdef _SHOW_KSR_SIZES_
	int TreeLeaf::m_tlCount = 0;

//-----------------------------------------------------------------------
	TreeLeaf::TreeLeaf(const TreeLeaf &treeLeaf)
	//-------------------------------------------------------------------
	{
		m_min = treeLeaf.m_min;
		m_max = treeLeaf.m_max;
		//renderList = treeLeaf.renderList;
		renderList.assign(treeLeaf.renderList.begin(), treeLeaf.renderList.end());

		tlCount++;
		Logf(LL_LOWEST, "TreeLeaf::TreeLeaf(const TreeLeaf &), count = %d", tlCount);
	}
#endif


//-----------------------------------------------------------------------
	const TreeLeaf &TreeLeaf::operator = (const TreeLeaf &rhs)
	//-------------------------------------------------------------------
	{
		m_min = rhs.m_min;
		m_max = rhs.m_max;

		std::list<RenderGroup *>::iterator i = m_renderList.begin();
		for (; i != m_renderList.end();)
		{
			if (*i)
			{
				delete (*i);
				(*i) = NULL;
			}
			else
				Logf(LL_LOWEST, "Warning: Skipping NULL RenderGroup *");

			i = m_renderList.erase(i);
		}

		m_renderList.assign(rhs.m_renderList.begin(), rhs.m_renderList.end());

		return *this;
	}


//-----------------------------------------------------------------------
	Geometry::Geometry(LPDIRECT3DDEVICE9 pDevice, DWORD resourceOwnershipFlags)
	//-------------------------------------------------------------------
	:	m_pScene(NULL), m_pDevice(pDevice), m_FVF(0), m_indexType(IT_32), m_vertexLength(0), m_indexLength(0), 
		m_indexGenerationFlags(0), m_numChunks(0), m_numVerts(0), m_numChunkIndices(0), m_numVertexIndices(0),
		m_pChunks(NULL), m_pChunkIndices(NULL), m_pVertexDeclaration(NULL),
		m_useVertexDeclarationFormat(false), m_vertexBufferLocked(false), m_indexBufferLocked(false), m_hide(false),
		m_pVertexBuffer(NULL), m_pIndexBuffer(NULL),
		m_min(0, 0, 0), m_max(0, 0, 0)
	//-------------------------------------------------------------------
	{
		AddUsedMemory(sizeof(Geometry), "Geometry::Geometry()");
	}


//-----------------------------------------------------------------------
	Geometry::~Geometry()
	//-------------------------------------------------------------------
	{
		FreeUsedMemory(sizeof(Geometry), "Geometry::~Geometry()");

		Clear();
	}


//-----------------------------------------------------------------------
	void Geometry::Release()
	//-------------------------------------------------------------------
	{
		if (m_pScene)
		{
			m_pScene->RemoveGeometry(this);
		}

		MemObject::Release();
	}


//-----------------------------------------------------------------------
	HRESULT Geometry::Clear(DWORD flags)
	//-------------------------------------------------------------------
	{
		if (m_pChunks && (flags & GCF_CHUNKS))
		{
			delete m_pChunks;
			m_pChunks = NULL;

			FreeUsedMemory(m_numChunks * sizeof(Chunk), "Geometry::Clear() - Chunks");

			m_numChunks = 0;
		}

		if (m_pVertexBuffer && (flags & GCF_VERTICES))
		{
			m_pVertexBuffer->Release();
			m_pVertexBuffer = NULL;

			FreeUsedMemory(m_numVerts * m_vertexLength, "Geometry::Clear() - Verts");

			m_numVerts = 0;
		}

		if (m_pChunkIndices && (flags & GCF_CHUNKINDICES))
		{
			delete m_pChunkIndices;
			m_pChunkIndices = NULL;

			FreeUsedMemory(m_numChunkIndices * m_indexLength, "Geometry::Clear() - Chunk Indices");

			m_numChunkIndices = 0;
		}

		if (m_pIndexBuffer && (flags & GCF_VERTEXINDICES))
		{
			m_pIndexBuffer->Release();
			m_pIndexBuffer = NULL;

			FreeUsedMemory(m_numVertexIndices * m_indexLength, "Geometry::Clear() - Vert Indices");

			m_numVertexIndices = 0;
		}

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Geometry::Reset()
	//-------------------------------------------------------------------
	{
		Clear();

		m_vertexLength = m_indexLength = 0;

		m_FVF = 0;

		m_indexGenerationFlags = 0;

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Geometry::Init(int vertexSize, DWORD vertexFormat, DWORD indexFormat)
	//-------------------------------------------------------------------
	{
		m_useVertexDeclarationFormat = false;

		m_vertexLength = vertexSize;
		m_FVF = vertexFormat;
		m_indexType = indexFormat;

		m_indexGenerationFlags = 0;

		if (m_indexType == IT_32)
			m_indexLength = 4;
		else
			m_indexLength = 2;

		// TODO: Notify/Update all scenes that use this geometry so they can also Update the Sub-Geometries they use

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Geometry::Init(int vertexSize, const VertexDeclarationFormat &vertexFormat, DWORD indexFormat)
	//-------------------------------------------------------------------
	{
		m_useVertexDeclarationFormat = true;

		m_vertexDeclarationFormat = vertexFormat;

		m_vertexLength = vertexSize;
		m_FVF = 0;
		m_indexType = indexFormat;

		m_indexGenerationFlags = 0;

		if (m_indexType == IT_32)
			m_indexLength = 4;
		else
			m_indexLength = 2;

		// Setup vertex declaration
			int numElements = m_vertexDeclarationFormat.elements.size() + 1;
			D3DVERTEXELEMENT9 *vertexElements = new D3DVERTEXELEMENT9[numElements];
			ZeroMemory(vertexElements, sizeof(D3DVERTEXELEMENT9) * numElements);

			std::vector<VertexFormatElement>::iterator i = m_vertexDeclarationFormat.elements.begin();
			for (int n = 0; i != m_vertexDeclarationFormat.elements.end(); i++, n++)
			{
				vertexElements[n].Stream = 0;
				vertexElements[n].Method = D3DDECLMETHOD_DEFAULT;
				vertexElements[n].Offset = i->offset;
				vertexElements[n].UsageIndex = i->usageIndex;
				vertexElements[n].Type = i->type;
				vertexElements[n].Usage = i->usage;
			}

			vertexElements[n].Method = 0;
			vertexElements[n].Offset = 0;
			vertexElements[n].Stream = 0xFF;
			vertexElements[n].Type = D3DDECLTYPE_UNUSED;
			vertexElements[n].Usage = 0;
			vertexElements[n].UsageIndex = 0;

			if (FAILED(m_pDevice->CreateVertexDeclaration(vertexElements, &m_pVertexDeclaration)))
				return E_FAIL;

			delete vertexElements;

		// TODO: Notify/Update all scenes that use this geometry so they can also update the Sub-Geometries they use

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Geometry::Lock(void **ppChunks, void **ppVerts, void **ppChunkIndices, void **ppVertexIndices, DWORD flags)
	//-------------------------------------------------------------------
	{
		if (ppChunks)
		{
			if (m_numChunks && m_pChunks)
			{
				(*ppChunks) = m_pChunks;
			}
			else
				return E_FAIL;
		}

		if (ppVerts)
		{
			if (m_numVerts && m_pVertexBuffer && !m_vertexBufferLocked)
			{
				if (FAILED(m_pVertexBuffer->Lock(0, 0, ppVerts, flags)))
					return E_FAIL;

				m_vertexBufferLocked = true;
			}
			else
				return E_FAIL;
		}

		if (ppChunkIndices)
		{
			if (m_numChunkIndices && m_pChunkIndices)
			{
				(*ppChunkIndices) = m_pChunkIndices;
			}
			else
				return E_FAIL;
		}

		if (ppVertexIndices)
		{
			if (m_numVertexIndices && m_pIndexBuffer && !m_indexBufferLocked)
			{
				if (FAILED(m_pIndexBuffer->Lock(0, 0, ppVertexIndices, flags)))
					return E_FAIL;

				m_indexBufferLocked = true;
			}
			else
				return E_FAIL;
		}

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Geometry::Unlock(DWORD unlockFlags)
	//-------------------------------------------------------------------
	{
		if (unlockFlags & UF_CHUNKINDICES)
		{
			if (m_pVertexBuffer && m_vertexBufferLocked)
			{
				if (FAILED(m_pVertexBuffer->Unlock()))
					Logf("Failed Unlocking Vertex Buffer");

				m_vertexBufferLocked = false;
			}
		}

		if (unlockFlags & UF_VERTEXINDICES)
		{
			if (m_pIndexBuffer && m_indexBufferLocked)
			{
				if (FAILED(m_pIndexBuffer->Unlock()))
					Logf("Failed Unlocking Vertex Index Buffer");

				m_indexBufferLocked = false;
			}
		}

		return S_OK;
	}


//-----------------------------------------------------------------------
	const Vector3 Geometry::GetVertexPosition(LPVOID pVertsLocked, unsigned int index) const
	//-------------------------------------------------------------------
	{
		LPVOID pVertsLockedIndex = (LPVOID)((char*)pVertsLocked + m_vertexLength * index);
#ifdef _DEBUG
		float x = *((float *)pVertsLockedIndex);
		float y = *((float *)pVertsLockedIndex + 1);
		float z = *((float *)pVertsLockedIndex + 2);

		return Vector3(x, y, z);
#else
		return Vector3(*((float *)pVertsLockedIndex), *((float *)pVertsLockedIndex + 1), *((float *)pVertsLockedIndex + 2));
#endif
	}


//-----------------------------------------------------------------------
	HRESULT Geometry::Insert(Geometry *pGeometry, DWORD flags)
	//-------------------------------------------------------------------
	{
		if (!pGeometry)
			return E_FAIL;

		if (m_numVertexIndices && m_indexType != pGeometry->m_indexType)
			return E_FAIL;

		if (pGeometry->m_useVertexDeclarationFormat)
		{
			if (FAILED(Init(pGeometry->m_vertexLength, pGeometry->m_vertexDeclarationFormat, pGeometry->m_indexType)))
				return E_FAIL;
		}
		else
		{
			if (FAILED(Init(pGeometry->m_vertexLength, pGeometry->m_FVF, pGeometry->m_indexType)))
				return E_FAIL;
		}

		void *pVertsLocked = NULL,
			 *pIndicesLocked = NULL;

		void *vertices = NULL;
		void *vertexIndices = NULL;

		if (pGeometry->m_numVerts)
			vertices = malloc(pGeometry->m_numVerts * pGeometry->m_vertexLength);

		if (pGeometry->m_numVertexIndices)
			vertexIndices = malloc(pGeometry->m_numVertexIndices * pGeometry->m_indexLength);

		if (FAILED(pGeometry->Lock(NULL, &pVertsLocked, NULL, &pIndicesLocked)))
			return E_FAIL;

			memcpy(vertices, pVertsLocked, pGeometry->m_numVerts * pGeometry->m_vertexLength);
			memcpy(vertexIndices, pIndicesLocked, pGeometry->m_numVertexIndices * pGeometry->m_indexLength);

		if (FAILED(pGeometry->Unlock()))
			return E_FAIL;

		if (FAILED(Insert(pGeometry->m_numChunks, pGeometry->m_numVerts, pGeometry->m_numChunkIndices, pGeometry->m_numVertexIndices,
						  pGeometry->m_pChunks, vertices, pGeometry->m_pChunkIndices, vertexIndices, flags)))
			return E_FAIL;

		if (vertices)
			free(vertices);

		if (vertexIndices)
			free(vertexIndices);

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Geometry::Insert(int nChunks, int nVerts, void *pChunks, void *pVerts, DWORD flags)
	//-------------------------------------------------------------------
	{
		return Insert(nChunks, nVerts, 0, 0, pChunks, pVerts, NULL, NULL, flags);
	}


//-----------------------------------------------------------------------
	HRESULT Geometry::Insert(int nChunks, int nVerts, int nChunkIndices, int nVertexIndices,
							 void *pChunks, void *pVerts, void *pChunkIndices, void *pVertexIndices, DWORD flags)
	//-------------------------------------------------------------------
	{
		if (nChunks  && !pChunks  || pChunks  && !nChunks  ||
			nVerts   && !pVerts   || pVerts   && !nVerts   ||
			nVertexIndices && !pVertexIndices || pVertexIndices && !nVertexIndices ||
			nChunkIndices && !pChunkIndices || pChunkIndices && !nChunkIndices)
			return E_FAIL;

		m_indexGenerationFlags |= flags;

		if (nChunks)
		{
			m_pChunks = (Chunk*)realloc((void*)m_pChunks, (m_numChunks + nChunks) * sizeof(Chunk));

			if (!m_pChunks)
				return E_FAIL;

			memcpy(m_pChunks + m_numChunks, pChunks, nChunks * sizeof(Chunk));
		}

		if (nVerts)
		{
			LPVOID pVertsBuffered = NULL;
			LPVOID pVertsLocked = NULL;

			if (m_pVertexBuffer)
			{
				if (FAILED(m_pVertexBuffer->Lock(0, 0, (LPVOID*)&pVertsLocked, 0)))
					return E_FAIL;

				pVertsBuffered = malloc((m_numVerts + nVerts) * m_vertexLength);
				memcpy(pVertsBuffered, pVertsLocked, m_numVerts * m_vertexLength);

				if (FAILED(m_pVertexBuffer->Unlock()))
					return E_FAIL;

				m_pVertexBuffer->Release();
			}
			else
			{
				pVertsBuffered = malloc(nVerts * m_vertexLength);
			}

			if (!pVertsBuffered)
				return E_FAIL;

			memcpy((char *)pVertsBuffered + m_numVerts * m_vertexLength, pVerts, nVerts * m_vertexLength);

			m_pDevice->CreateVertexBuffer((m_numVerts + nVerts) * m_vertexLength,
										  0,
										  m_FVF,
										  D3DPOOL_MANAGED,
										  &m_pVertexBuffer,
										  NULL);

			if (FAILED(m_pVertexBuffer->Lock(0, 0, (LPVOID*)&pVertsLocked, 0)))
				return E_FAIL;

			memcpy(pVertsLocked, pVertsBuffered, (m_numVerts + nVerts) * m_vertexLength);

			LPVOID pVertsLockedIndex = 0;
			for (int i = 0; i < nVerts; i++)
			{
				Vector3 position = GetVertexPosition(pVertsLocked, m_numVerts + i);

				if (i == 0 && m_numVerts == 0)
				{
					m_min = position;
					m_max = position;
				}

				Math::SortMinMax(position, position, m_min, m_max);
			}

			if (FAILED(m_pVertexBuffer->Unlock()))
				return E_FAIL;

			if (pVertsBuffered)
				free(pVertsBuffered);
		}

		if (nChunkIndices)
		{
			m_pChunkIndices = realloc(m_pChunkIndices, (m_numChunkIndices + nChunkIndices) * m_indexLength);

			if (!m_pChunkIndices)
				return E_FAIL;

			memcpy((char*)m_pChunkIndices + m_numChunkIndices * m_indexLength, pChunkIndices, nChunkIndices * m_indexLength);
		}

		if (nVertexIndices)
		{
			LPVOID newVertexIndicesBuffered = NULL;
			LPVOID newVertexIndicesLocked = NULL;

			if (m_pIndexBuffer)
			{
				if (FAILED(m_pIndexBuffer->Lock(0, 0, (LPVOID*)&newVertexIndicesLocked, 0)))
					return E_FAIL;

				newVertexIndicesBuffered = malloc((m_numVertexIndices + nVertexIndices) * m_indexLength);
				memcpy(newVertexIndicesBuffered, newVertexIndicesLocked, m_numVertexIndices *m_indexLength);

				if (FAILED(m_pIndexBuffer->Unlock()))
					return E_FAIL;

				m_pIndexBuffer->Release();
			}
			else
				newVertexIndicesBuffered = malloc(nVertexIndices * m_indexLength);

			if (!newVertexIndicesBuffered)
				return E_FAIL;

			memcpy((char *)newVertexIndicesBuffered + m_numVertexIndices * m_indexLength, pVertexIndices, nVertexIndices * m_indexLength);

			D3DFORMAT indexD3DFormat = D3DFMT_INDEX32;

			if (m_indexType == IT_16)
				indexD3DFormat = D3DFMT_INDEX16;

			m_pDevice->CreateIndexBuffer((m_numVertexIndices + nVertexIndices) * m_indexLength,
									  0,
									  indexD3DFormat,
									  D3DPOOL_MANAGED,
									  &m_pIndexBuffer,
									  NULL);

			if (FAILED(m_pIndexBuffer->Lock(0, 0, (LPVOID*)&newVertexIndicesLocked, 0)))
				return E_FAIL;

			memcpy(newVertexIndicesLocked, newVertexIndicesBuffered, (m_numVertexIndices + nVertexIndices) * m_indexLength);

			if (FAILED(m_pIndexBuffer->Unlock()))
				return E_FAIL;

			if (newVertexIndicesBuffered)
				free(newVertexIndicesBuffered);
		}

		m_numChunks += nChunks;
		m_numVerts += nVerts;
		m_numChunkIndices += nChunkIndices;
		m_numVertexIndices += nVertexIndices;

		AddUsedMemory(nChunks * sizeof(Chunk), "Geometry::Insert() - Chunks...");
		AddUsedMemory(nVerts * m_vertexLength, "Geometry::Insert() - Verts...");
		AddUsedMemory(nChunkIndices * m_indexLength, "Geometry::Insert() - ChunkIndices...");
		AddUsedMemory(nVertexIndices * m_indexLength, "Geometry::Insert() - VertexIndices...");

		return S_OK;
	}


//-----------------------------------------------------------------------
	void Geometry::SetScene(PSCENE pScene, bool notify)
	//-------------------------------------------------------------------
	{
		if (notify && m_pScene)
		{
			m_pScene->RemoveGeometry(this, false);
		}

		m_pScene = pScene;
	}


/*
//-----------------------------------------------------------------------
	HRESULT Geometry::InsertScene(PSCENE pScene)
	//-------------------------------------------------------------------
	{
		if (!pScene || !m_pScenes)
			return E_FAIL;

		m_pScenes->push_back(pScene);
		m_pScenes->unique();

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Geometry::RemoveScene(PSCENE pScene)
	//-------------------------------------------------------------------
	{
		if (!pScene || !m_pScenes)
			return E_FAIL;

		m_pScenes->remove(pScene);

		return S_OK;
	}
*/

//-----------------------------------------------------------------------
	bool Geometry::Intersect(std::list<IntersectionResult> &results, const Vector3 &rayPos, const Vector3 &rayDir,
							 RenderGroup *pGroup, DWORD flags, bool findFirst)
	//-------------------------------------------------------------------
	// Note: returns only the first interection found
	//-------------------------------------------------------------------
	{
		if (!m_pVertexBuffer || !m_pIndexBuffer)
			return false;

		std::vector<IntersectionResult> newResults;

		LPVOID pVertsLocked = NULL;
		LPVOID pIndicesLocked = NULL;

		m_pVertexBuffer->Lock(0, 0, &pVertsLocked, D3DLOCK_READONLY);
		m_pIndexBuffer->Lock(0, 0, &pIndicesLocked, D3DLOCK_READONLY);

		// todo: currently assumes indexLength == 4
		unsigned int *pIndices = (unsigned int *)pIndicesLocked;

        for (unsigned int c = 0; c < (unsigned int)m_numChunks; c++)
		{
			Chunk &chunk = m_pChunks[c];

			if (chunk.type == CT_TRIANGLELIST && flags & IF_TRIANGLELIST)
			{
				for (unsigned int n = chunk.startIndex; n < chunk.numIndices; n += 3)
				{
					unsigned int index0 = pIndices[n];
					unsigned int index1 = pIndices[n + 1];
					unsigned int index2 = pIndices[n + 2];

					Vector3 v0 = GetVertexPosition(pVertsLocked, index0);
					Vector3 v1 = GetVertexPosition(pVertsLocked, index1);
					Vector3 v2 = GetVertexPosition(pVertsLocked, index2);
					float u = 0.0f;
					float v = 0.0f;
					float distance = 0.0f;

					if (D3DXIntersectTri(&v0, &v1, &v2, &rayPos, &rayDir, &u, &v, &distance))
					{
						IntersectionResult result(this, c, index0, index1, index2, u, v, distance);

						if (findFirst)
						{
							results.push_back(result);

							return true;
						}
						else
							newResults.push_back(result);
					}
				}
			}
			else if (chunk.type == CT_TRIANGLESTRIP && flags & IF_TRIANGLESTRIP)
			{
				unsigned int index0 = pIndices[0];
				unsigned int index1 = pIndices[1];
				unsigned int index2 = pIndices[2];

				Vector3 v0 = GetVertexPosition(pVertsLocked, index0);
				Vector3 v1 = GetVertexPosition(pVertsLocked, index1);
				Vector3 v2 = GetVertexPosition(pVertsLocked, index2);
				float u = 0.0f;
				float v = 0.0f;
				float distance = 0.0f;

				if (D3DXIntersectTri(&v0, &v1, &v2, &rayPos, &rayDir, &u, &v, &distance))
				{
					IntersectionResult result(this, c, index0, index1, index2, u, v, distance);

					if (findFirst)
					{
						results.push_back(result);

						return true;
					}
					else
						newResults.push_back(result);
				}

				for (unsigned int n = chunk.startIndex + 3; n < chunk.numIndices; n++)
				{
					index0 = pIndices[n];
					index1 = pIndices[n + 1];
					index2 = pIndices[n + 2];

					v0 = GetVertexPosition(pVertsLocked, index0);
					v1 = GetVertexPosition(pVertsLocked, index1);
					v2 = GetVertexPosition(pVertsLocked, index2);
					u = 0.0f;
					v = 0.0f;
					distance = 0.0f;

					if (D3DXIntersectTri(&v0, &v1, &v2, &rayPos, &rayDir, &u, &v, &distance))
					{
						IntersectionResult result(this, c, index0, index1, index2, u, v, distance);

						if (findFirst)
						{
							results.push_back(result);

							return true;
						}
						else
							newResults.push_back(result);
					}
				}
			}
			else if (chunk.type == CT_POLYGON && flags & IF_POLYGON)
			{

			}
		}

		m_pIndexBuffer->Unlock();
		m_pVertexBuffer->Unlock();

		if (newResults.size() > 0)
		{
			results.insert(results.end(), newResults.begin(), newResults.end());

			return true;
		}

		return false;
	}


//-----------------------------------------------------------------------
	bool Geometry::Intersect(std::list<IntersectionResult> &results, const Vector3 &rayPos, const Vector3 &rayDir, RenderGroup *pGroup, DWORD flags)
	// Note: returns all intersections found with this geometry object
	//-------------------------------------------------------------------
	{
		bool found = Intersect(results, rayPos, rayDir, pGroup, flags, false);

        return found;
	}


//-----------------------------------------------------------------------
	bool Geometry::Intersect(IntersectionResult &result, const Vector3 &rayPos, const Vector3 &rayDir, RenderGroup *pGroup, DWORD flags)
	// Note: returns only the first interection found
	//-------------------------------------------------------------------
	{
		std::list<IntersectionResult> results;

		bool found = Intersect(results, rayPos, rayDir, pGroup, flags, true);

		if (results.size() < 1)
			return false;

		result = *results.begin();

		return found;
	}


//-----------------------------------------------------------------------
	HRESULT Geometry::GenerateIndexBuffers(DWORD flags)
	//-------------------------------------------------------------------
	{
		DWORD generationFlags = m_indexGenerationFlags | flags;

		if (!m_numChunkIndices && !(generationFlags & GF_ALLOWZEROCHUNKINDICES))
		{
			if (!m_numChunks)
				return E_FAIL;

			if (m_pChunkIndices)
				free(m_pChunkIndices);

			m_numChunkIndices = m_numChunks;

			m_pChunkIndices = malloc(m_numChunkIndices * m_indexLength);

			std::vector<int> chunkIndexList;

			for (unsigned int i = 0; i < m_numChunkIndices; i++)
				chunkIndexList.push_back(i);

			void *newChunkIndices = NULL;

			if (m_indexType == IT_32)
			{
				newChunkIndices = new unsigned int[m_numChunkIndices];

				for (unsigned int v = 0; v < m_numChunkIndices; v++)
					((unsigned int *)newChunkIndices)[v] = chunkIndexList[v];
			}
			else if (m_indexType == IT_16)
			{
				newChunkIndices = new unsigned short int[m_numChunkIndices];

				for (unsigned int v = 0; v < m_numChunkIndices; v++)
					((unsigned short int*)newChunkIndices)[v] = chunkIndexList[v];
			}

			memcpy(m_pChunkIndices, newChunkIndices, m_numChunkIndices * m_indexLength);

			if (newChunkIndices)
				delete[] newChunkIndices;
		}

		if (!m_numVertexIndices && !(generationFlags & GF_ALLOWZEROVERTEXINDICES))
		{
			if (!m_numVerts)
				return E_FAIL;

			if (m_pIndexBuffer)
				m_pIndexBuffer->Release();

			m_numVertexIndices = m_numVerts;

			std::vector<int> vertexIndexList;

			for (unsigned int i = 0; i < m_numVertexIndices; i++)
				vertexIndexList.push_back(i);

			void *newVertexIndices = NULL;

			if (m_indexType == IT_32)
			{
				newVertexIndices = new int[m_numVertexIndices];

				for (unsigned int v = 0; v < m_numVertexIndices; v++)
					((unsigned int *)newVertexIndices)[v] = vertexIndexList[v];
			}
			else if (m_indexType == IT_16)
			{
				newVertexIndices = new short int[m_numVertexIndices];

				for (unsigned int v = 0; v < m_numVertexIndices; v++)
					((unsigned short int*)newVertexIndices)[v] = vertexIndexList[v];
			}

			D3DFORMAT indexD3DFormat = D3DFMT_INDEX32;

			if (m_indexType == IT_16)
				indexD3DFormat = D3DFMT_INDEX16;

			m_pDevice->CreateIndexBuffer(m_numVertexIndices * m_indexLength,
									  0,
									  indexD3DFormat,
									  D3DPOOL_MANAGED,
									  &m_pIndexBuffer,
									  NULL);

			void *newVertexIndicesLocked = NULL;

			if (FAILED(m_pIndexBuffer->Lock(0, 0, (LPVOID*)&newVertexIndicesLocked, 0)))
				return E_FAIL;

			memcpy(newVertexIndicesLocked, newVertexIndices, m_numVertexIndices * m_indexLength);

			if (FAILED(m_pIndexBuffer->Unlock()))
				return E_FAIL;

			if (newVertexIndices)
				delete[] newVertexIndices;
		}

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Geometry::Serialise(OutputGeometry *pOutputGeometry)
	//-------------------------------------------------------------------
	{
		pOutputGeometry->size = sizeof(OutputGeometry) + m_numChunks * sizeof(Chunk) + m_numVerts * m_vertexLength +
								m_numChunkIndices * m_indexLength + m_numVertexIndices * m_indexLength;

		pOutputGeometry->FVF = m_FVF;
		//pOutputGeometry->vertexDeclarationFormat = vertexDeclarationFormat; // TODO: not that simple!
		pOutputGeometry->vertexLength = m_vertexLength;
		pOutputGeometry->indexLength = m_indexLength;
		pOutputGeometry->indexFormat = m_indexType;
		pOutputGeometry->min = m_min;
		pOutputGeometry->max = m_max;

		pOutputGeometry->numChunks = m_numChunks;
		pOutputGeometry->numVertices = m_numVerts;
		pOutputGeometry->numChunkIndices = m_numChunkIndices;
		pOutputGeometry->numVertexIndices = m_numVertexIndices;

		pOutputGeometry->chunks = NULL;
		pOutputGeometry->vertices = NULL;
		pOutputGeometry->chunkIndices = NULL;
		pOutputGeometry->vertexIndices = NULL;

		if (pOutputGeometry->numChunks)
		{
			pOutputGeometry->chunks = malloc(m_numChunks * sizeof(Chunk));
			memcpy(pOutputGeometry->chunks, m_pChunks, m_numChunks * sizeof(Chunk));
		}

		if (pOutputGeometry->numVertices && m_pVertexBuffer)
		{
			void *pVertsLocked = NULL;
			m_pVertexBuffer->Lock(0, 0, &pVertsLocked, 0);

			pOutputGeometry->vertices = malloc(m_numVerts * m_vertexLength);
			memcpy(pOutputGeometry->vertices, pVertsLocked, m_numVerts * m_vertexLength);

			m_pVertexBuffer->Unlock();
		}

		if (pOutputGeometry->numChunkIndices)
		{
			pOutputGeometry->chunkIndices = malloc(m_numChunkIndices * m_indexLength);
			memcpy(pOutputGeometry->chunkIndices, m_pChunkIndices, m_numChunkIndices * m_indexLength);
		}

		if (pOutputGeometry->numVertexIndices && m_pIndexBuffer)
		{
			void *pIndicesLocked = NULL;
			m_pIndexBuffer->Lock(0, 0, &pIndicesLocked, 0);

			pOutputGeometry->vertexIndices = malloc(m_numVertexIndices * m_indexLength);
			memcpy(pOutputGeometry->vertexIndices, pIndicesLocked, m_numVertexIndices * m_indexLength);

			m_pIndexBuffer->Unlock();
		}

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Geometry::Reassemble(OutputGeometry *pOutputGeometry)
	//-------------------------------------------------------------------
	{
		if (!pOutputGeometry)
			return E_FAIL;

		if (pOutputGeometry->useVertexDeclarationFormat)
		{
			/*if (FAILED(Init(pOutputGeometry->vertexLength, pOutputGeometry->vertexDeclarationFormat,
							pOutputGeometry->indexFormat)))
				return E_FAIL;*/

			// TODO: not that simple!
		}
		else
		{
			if (FAILED(Init(pOutputGeometry->vertexLength, pOutputGeometry->FVF, pOutputGeometry->indexFormat)))
				return E_FAIL;
		}

		if (FAILED(Insert(pOutputGeometry->numChunks, pOutputGeometry->numVertices, pOutputGeometry->numChunkIndices,
						  pOutputGeometry->numVertexIndices, pOutputGeometry->chunks, pOutputGeometry->vertices,
						  pOutputGeometry->chunkIndices, pOutputGeometry->vertexIndices, 0)))
			return E_FAIL;

		return S_OK;
	}


//-----------------------------------------------------------------------
	LPDIRECT3DVERTEXBUFFER9 Geometry::GetVertexBuffer()
	//-------------------------------------------------------------------
	{
		return m_pVertexBuffer;
	}


//-----------------------------------------------------------------------
	LPDIRECT3DINDEXBUFFER9 Geometry::GetIndexBuffer()
	//-------------------------------------------------------------------
	{
		return m_pIndexBuffer;
	}

/*
//-----------------------------------------------------------------------
	PartitionTree::PartitionTree()
	//-------------------------------------------------------------------
	{
		AddUsedMemory(sizeof(PartitionTree));

		numLeafFaces = 0;

		leafFaces = NULL;

		treeNodes = NULL;
		treeLeafs = NULL;
	}


//-----------------------------------------------------------------------
	PartitionTree::~PartitionTree()
	//-------------------------------------------------------------------
	{
		FreeUsedMemory(sizeof(PartitionTree));

		delete[] leafFaces;
		delete[] treeNodes;
		delete[] treeLeafs;

		FreeUsedMemory(numTreeNodes * sizeof(TreeNode));
		FreeUsedMemory(numTreeLeafs * sizeof(TreeLeaf));
		FreeUsedMemory(numLeafFaces * sizeof(int));
	}


//-----------------------------------------------------------------------
	HRESULT PartitionTree::Insert(int nNodes, int nLeafs, int nLeafFaces, void *pNodes, void *pLeafs, void *pLeafFaces)
	//-------------------------------------------------------------------
	{
		if (nNodes      && !pNodes    || pNodes     && !nNodes ||
			nLeafs      && !pLeafs    || pLeafs     && !nLeafs ||
			nLeafFaces  && !pLeafFaces|| pLeafFaces && !nLeafFaces)
			return E_FAIL;

		if (nLeafFaces)
		{
			leafFaces = (int*)realloc((void*)leafFaces, (numLeafFaces + nLeafFaces) * sizeof(int));
			memcpy(leafFaces + numLeafFaces, pLeafFaces, nLeafFaces * sizeof(int));
		}

		if (pNodes)
		{
			treeNodes = (TreeNodeOutput*)realloc((void*)treeNodes, (numTreeNodes + nNodes) * sizeof(TreeNode));
			memcpy(treeNodes + numTreeNodes, pNodes, nNodes * sizeof(TreeNode));
		}

		if (pLeafs)
		{
			treeLeafs = (TreeLeaf*)realloc((void*)treeLeafs, (numTreeLeafs + nLeafs) * sizeof(TreeLeaf));
			memcpy(treeLeafs + numTreeLeafs, pLeafs, nLeafs * sizeof(TreeLeaf));
		}

		numTreeNodes += nNodes;
		numTreeLeafs += nLeafs;
		numLeafFaces += nLeafFaces;

		AddUsedMemory(nNodes * sizeof(TreeNode));
		AddUsedMemory(nLeafs * sizeof(TreeLeaf));
		AddUsedMemory(nLeafFaces * sizeof(int));

		return S_OK;
	}
*/

//-----------------------------------------------------------------------
	Chunk::Chunk()
	//-------------------------------------------------------------------
	{
		idTexture0 = -1;
		idTexture1 = -1;
		material = -1;
		startIndex = 0;
		numVerts = 0;
		effect = -1;
		technique = -1;
		order = 0;
		render = true;
		depthBias = 0;
		type = CT_POLYGON;
	}


//-----------------------------------------------------------------------
	Chunk::Chunk(const Chunk &rhs)
	//-------------------------------------------------------------------
	{
		idTexture0 = rhs.idTexture0;
		idTexture1 = rhs.idTexture1;
		material = rhs.material;
		startIndex = rhs.startIndex;
		numVerts = rhs.numVerts;
		effect = rhs.effect;
		technique = rhs.technique;
		order = rhs.order;
		render = rhs.render;
		depthBias = rhs.depthBias;
		type = rhs.type;
	}


//-----------------------------------------------------------------------
	Chunk::Chunk(int texture0, int texture1, int materialID, int effectID, int techniqueId,
				 int startVertexIndex, int numVertexIndices, int depthOrder, CHUNKTYPE chunkType, bool renderChunk)
	//-------------------------------------------------------------------
	{
		idTexture0 = texture0;
		idTexture1 = texture1;
		material = materialID;
		effect = effectID;
		technique = techniqueId;
		startIndex = startVertexIndex;
		numVerts = numVertexIndices;
		order = 0;
		render = renderChunk;
		depthBias = depthOrder;
		type = chunkType;
	}


//-----------------------------------------------------------------------
	const Chunk &Chunk::operator =(const Chunk &rhs)
	//-------------------------------------------------------------------
	{
		idTexture0 = rhs.idTexture0;
		idTexture1 = rhs.idTexture1;
		material = rhs.material;
		startIndex = rhs.startIndex;
		numVerts = rhs.numVerts;
		effect = rhs.effect;
		technique = rhs.technique;
		order = rhs.order;
		render = rhs.render;
		depthBias = rhs.depthBias;
		type = rhs.type;

		return *this;
	}


//-----------------------------------------------------------------------
	bool Chunk::IsEquivalent(const Chunk &rhs)
	//-------------------------------------------------------------------
	{
		return type == rhs.type && order == rhs.order && effect == rhs.effect && idTexture0 == rhs.idTexture0 && 
			   idTexture1 == rhs.idTexture1 && material == rhs.material && technique == rhs.technique && 
			   render == rhs.render && depthBias == rhs.depthBias;
	}


//-----------------------------------------------------------------------
	IntersectionResult::IntersectionResult()
	//-------------------------------------------------------------------
	{
		m_pGeometry = NULL;

		m_chunkID = m_vertexID0 = m_vertexID1 = m_vertexID2 = 0;

		m_u = m_v = m_distance = 0.0f;
	}


//-----------------------------------------------------------------------
	IntersectionResult::IntersectionResult(const IntersectionResult &result)
	//-------------------------------------------------------------------
	{
		m_pGeometry = result.m_pGeometry;

		m_chunkID = result.m_chunkID;
		m_vertexID0 = result.m_vertexID0;
		m_vertexID1 = result.m_vertexID1;
		m_vertexID2 = result.m_vertexID2;

		m_u = result.m_u;
		m_v = result.m_v;
		m_distance = result.m_distance;
	}


//-----------------------------------------------------------------------
	IntersectionResult::IntersectionResult(Geometry *pGeometry, unsigned int chunkID, unsigned int vertexID0,
										   unsigned int vertexID1, unsigned int vertexID2,
										   float u, float v, float distance)
	//-------------------------------------------------------------------
	{
		m_pGeometry = pGeometry;

		m_chunkID = chunkID;
		m_vertexID0 = vertexID0;
		m_vertexID1 = vertexID1;
		m_vertexID2 = vertexID2;

		m_u = u;
		m_v = v;
		m_distance = distance;
	}


//-----------------------------------------------------------------------
	const IntersectionResult &IntersectionResult::operator = (const IntersectionResult &rhs)
	//-------------------------------------------------------------------
	{
		m_pGeometry = rhs.m_pGeometry;

		m_chunkID = rhs.m_chunkID;
		m_vertexID0 = rhs.m_vertexID0;
		m_vertexID1 = rhs.m_vertexID1;
		m_vertexID2 = rhs.m_vertexID2;

		m_u = rhs.m_u;
		m_v = rhs.m_v;
		m_distance = rhs.m_distance;

		return *this;
	}


//-----------------------------------------------------------------------
	bool IntersectionResult::operator < (const IntersectionResult &rhs)
	//-------------------------------------------------------------------
	{
		return m_distance < rhs.m_distance;
	}


//-----------------------------------------------------------------------
	bool IntersectionResult::operator > (const IntersectionResult &rhs)
	//-------------------------------------------------------------------
	{
		return m_distance > rhs.m_distance;
	}


//-----------------------------------------------------------------------
	bool IntersectionResult::operator <= (const IntersectionResult &rhs)
	//-------------------------------------------------------------------
	{
		return m_distance <= rhs.m_distance;
	}


//-----------------------------------------------------------------------
	bool IntersectionResult::operator >= (const IntersectionResult &rhs)
	//-------------------------------------------------------------------
	{
		return m_distance >= rhs.m_distance;
	}


//-----------------------------------------------------------------------
	bool IntersectionResult::operator == (const IntersectionResult &rhs)
	//-------------------------------------------------------------------
	{
		return m_distance == rhs.m_distance;
	}


//-----------------------------------------------------------------------
	bool IntersectionResult::operator != (const IntersectionResult &rhs)
	//-------------------------------------------------------------------
	{
		return m_distance != rhs.m_distance;
	}

// EOF