#include "../include/ksr.h"

#include "../include/leakwatcher.h"

#define VERBOSE_PARTITIONING

#ifdef _DO_MEMORY_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace KSR;

std::vector< Chunk > *g_pSortingChunks;

//-----------------------------------------------------------------------
	bool SortByTechniqueID(int lhs, int rhs)
	//-------------------------------------------------------------------
	{
		if (!g_pSortingChunks || g_pSortingChunks->empty())
			return false;

		std::vector< Chunk > &v = *g_pSortingChunks;

		return v[lhs].technique > v[rhs].technique;
	}

	/*void CreateRenderGroups(std::vector< RenderGroup > &renderGroups)
		{
			std::vector< RenderGroup > result;

			NodeChunkMap::iterator i = m.begin();
			for (; i != m.end(); i++)
			{
				if ((*i)->m.empty())
					continue;

				OrderedGeometryMap &orderedGeomMapRef = (*i)->m;
				OrderedGeometryMap::iterator j = orderedGeomMapRef.begin();
				for (; j != orderedGeomMapRef.end(); j++)
				{
					GeometryMap &gm = j->second->m;
					GeometryMap::iterator k = gm.begin();
					for (; k != gm.end(); k++)
					{
						if (!k->first)
							continue;

						if (k->second->m.empty())
							continue;

						ChunkIndexList &chunkIndexList = k->second->m;
						ChunkIndexList::iterator c = chunkIndexList.begin();
						for (; c != chunkIndexList.end(); c++)
						{
							Chunk &srcChunk = 

							Chunk newChunk(
						}
					}
				}
			}

			//result.sort(result.begin(), result.end(),);
			renderGroups.insert(renderGroups.end(), result.begin(), result.end());
		}*/


//-----------------------------------------------------------------------
	Scene::Scene(PSCENESETTINGS pSettings, PDIRECT3DDEVICE9 pDevice, PRESOURCEMANAGER pResourceManager)
	//-------------------------------------------------------------------
	:	m_pFnProgressCallback(NULL), m_pProgressCallbackPayload(NULL), m_pSpacePartitionOutline(NULL),
		m_pD3DDevice(pDevice), m_pResourceManager(pResourceManager)
	//-------------------------------------------------------------------
	{
		AddUsedMemory(sizeof(Scene), "Scene::Scene()");

		m_pSettings = new SceneSettings;
		AddUsedMemory(sizeof(SceneSettings), "Scene::Scene() - SceneSettings");

		memset(m_lightIDs, -1, sizeof(int) * 8);

		SetSettings(pSettings);

		m_pSceneGraphRoot = NULL;

		Init();
	}


//-----------------------------------------------------------------------
	Scene::~Scene()
	//-------------------------------------------------------------------
	{
		Logf("Destroying Scene...");

		FreeUsedMemory(sizeof(Scene), "Scene::~Scene()");

		if (m_pSettings)
		{
			delete m_pSettings;
			FreeUsedMemory(sizeof(SceneSettings), "Scene::~Scene() - SceneSettings");
		}

		OptimisedGeometryMap::iterator itr = m_optimisedGeometry.begin();
		for (; itr != m_optimisedGeometry.end(); itr++)
		{
			delete itr->second;
		}
		m_optimisedGeometry.clear();

		Clear();
	}


//-----------------------------------------------------------------------
	void Scene::Init()
	//-------------------------------------------------------------------
	{
		m_subdivisionsExecuted = 0;

		m_numClusters = 0;
		m_bytesPerCluster = 0;

		m_pRoot = new TreeNode;
		m_pRoot->m_min = Vector3(0, 0, 0);
		m_pRoot->m_max = Vector3(0, 0, 0);
		m_numNodes = 1;

		m_activeLeafID = -1;

		m_pSceneGraphRoot = new Entity;
		m_pSceneGraphRoot->SetPosition(Vector3(0, 0, 0));
		m_pSceneGraphRoot->SetRotation(Vector3(0, 0, 0));

		m_spacePartitionProgress = 0;
		m_optimiseChunksProgress = 0;
		m_pvsProgress = 0;
	}


//-----------------------------------------------------------------------
	void Scene::Reset(bool recreate, bool resetLights)
	//-------------------------------------------------------------------
	{
		if (m_pRoot)
		{
			delete m_pRoot;
			m_pRoot = NULL;
			m_numNodes = 0;
		}

		std::vector<TreeLeaf *>::iterator leafItr = m_leafs.begin();
		for (; leafItr != m_leafs.end(); leafItr++)
		{
			delete *leafItr;
			*leafItr = NULL;
		}

		m_leafs.clear();

		std::vector< byte * >::iterator p = m_PVS.begin();
		for (; p != m_PVS.end(); p++)
		{
			delete[] *p;
			*p = NULL;

			FreeUsedMemory(m_bytesPerCluster, "Scene()::Clear() - PVS Cluster");
		}

		m_PVS.clear();

		std::vector< _NodeChunkMap * >::iterator r = m_nodeChunkMap.begin();
		for (; r != m_nodeChunkMap.end(); r++)
		{
			delete *r;
			*r = NULL;
		}

		m_nodeChunkMap.clear();

		m_numClusters = 0;
		m_bytesPerCluster = 0;

		m_subdivisionsExecuted = 0;
		m_numNodes = 0;

		m_activeLeafID = -1;

		if (resetLights)
		{
			memset(m_lightIDs, -1, sizeof(int) * 8);
		}

		if (recreate)
		{
			m_pRoot = new TreeNode;
			m_pRoot->m_min = Vector3(0, 0, 0);
			m_pRoot->m_max = Vector3(0, 0, 0);
			m_numNodes = 1;
		}
	}


//-----------------------------------------------------------------------
	void Scene::Clear()
	//-------------------------------------------------------------------
	{
		Reset();

		if (m_pSpacePartitionOutline)
			m_pSpacePartitionOutline->Clear();

		if (m_pSceneGraphRoot)
		{
			m_pSceneGraphRoot->Release();
			m_pSceneGraphRoot = NULL;
		}

		m_worldCollisions.clear();
		m_bodyCollisions.clear();
	}


//-----------------------------------------------------------------------
	HRESULT Scene::SetSettings(PSCENESETTINGS pSettings)
	//-------------------------------------------------------------------
	{
		if (!pSettings)
			return E_FAIL;

		memcpy(m_pSettings, pSettings, sizeof(SceneSettings));

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Scene::SetProgressCallback(ProgressCallbackFunc progressCallbackFunc, void *pPayload)
	//-------------------------------------------------------------------
	{
		if (!progressCallbackFunc)
			return E_FAIL;

		m_pFnProgressCallback = progressCallbackFunc;
		m_pProgressCallbackPayload = pPayload;

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Scene::SetProgressCallbackPayload(void *pPayload)
	//-------------------------------------------------------------------
	{
		m_pProgressCallbackPayload = pPayload;

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Scene::SetWorldBounds(Vector3 min, Vector3 max)
	//-------------------------------------------------------------------
	{
		if (!m_pRoot)
			return E_FAIL;

		m_pRoot->m_min = min;
		m_pRoot->m_max = max;

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Scene::SetPosition(Vector3 position)
	//-------------------------------------------------------------------
	{
		if (!m_pSceneGraphRoot)
			return E_FAIL;

		m_pSceneGraphRoot->SetPosition(position);

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Scene::ResetWorld()
	//-------------------------------------------------------------------
	{
		Clear();

		Init();

		return S_OK;
	}


//-----------------------------------------------------------------------
	bool Scene::TestVisibility(TreeLeaf *target, Matrix frustum)
	//-------------------------------------------------------------------
	{
		if (!target)
			return false;

		Plane frustumPlanes[6];
		frustumPlanes[0] = Plane(frustum._14 + frustum._11,
						   frustum._24 + frustum._21,
						   frustum._34 + frustum._31,
						   frustum._44 + frustum._41);
		frustumPlanes[1] = Plane(frustum._14 - frustum._11,
						   frustum._24 - frustum._21,
						   frustum._34 - frustum._31,
						   frustum._44 - frustum._41);
		frustumPlanes[2] = Plane(frustum._14 - frustum._12,
						   frustum._24 - frustum._22,
						   frustum._34 - frustum._32,
						   frustum._44 - frustum._42);
		frustumPlanes[3] = Plane(frustum._14 + frustum._12,
						   frustum._24 + frustum._22,
						   frustum._34 + frustum._32,
						   frustum._44 + frustum._42);
		frustumPlanes[4] = Plane(frustum._13,
						   frustum._23,
						   frustum._33,
						   frustum._43);
		frustumPlanes[5] = Plane(frustum._14 + frustum._13,
						   frustum._24 + frustum._23,
						   frustum._34 + frustum._33,
						   frustum._44 + frustum._43);

		GeometryList::iterator g = m_sourceGeometry.begin();
		for (; g != m_sourceGeometry.end(); g++)
		{
			PVOID pVertsLocked = NULL;
			int *pIndicesLocked = NULL;
			Chunk *chunks = NULL;

			bool useIndices = true;

			if ((*g)->GetNumVertexIndices() < 1)
				useIndices = false;

			if (useIndices)
			{
				if ((*g)->GetIndexFormat() == IT_32)
				{
					if (FAILED((*g)->Lock((PVOID*)&chunks, &pVertsLocked, NULL, (PVOID*)&pIndicesLocked)))
						return true;
				}
				else if ((*g)->GetIndexFormat() == IT_16)
				{
					int numIndices = (*g)->GetNumVertexIndices();
					pIndicesLocked = new int[numIndices];

					PVOID pIndicesLockedTemp = NULL;

					if (FAILED((*g)->Lock((PVOID*)&chunks, &pVertsLocked, NULL, &pIndicesLockedTemp)))
						return true;

					for (int i = 0; i < numIndices; i++)
						pIndicesLocked[i] = (int)((short int *)pIndicesLockedTemp)[i];
				}
			}
			else
			{
				if (FAILED((*g)->Lock((PVOID*)&chunks, &pVertsLocked, NULL, NULL)))
					return true;
			}

			if (Math::AABBFrustumCollision(Math::AABB(target->m_min, target->m_max), frustumPlanes))
			{
				// Build a 2D Polygon from the projected target node
					Vector3 boxPoints[8];
					boxPoints[0] = Vector3(target->m_min.x, target->m_min.y, target->m_max.z);
					boxPoints[1] = Vector3(target->m_min.x, target->m_max.y, target->m_max.z);
					boxPoints[2] = Vector3(target->m_max.x, target->m_max.y, target->m_max.z);
					boxPoints[3] = Vector3(target->m_max.x, target->m_min.y, target->m_max.z);
					boxPoints[4] = Vector3(target->m_min.x, target->m_min.y, target->m_min.z);
					boxPoints[5] = Vector3(target->m_min.x, target->m_max.y, target->m_min.z);
					boxPoints[6] = Vector3(target->m_max.x, target->m_max.y, target->m_min.z);
					boxPoints[7] = Vector3(target->m_max.x, target->m_min.y, target->m_min.z);

					Vector2 boxPoints2D[8];
					float	boxPoints2DAngles[8][8];
					std::vector<Vector2> boxProjection;

					//for (int a = 0; a < 8; a++)
					//	boxPoints2D[a] = Math::Project(boxPoints[a], frustum, 1000.0f, 1000.0f);

					for (int a = 0; a < 8; a++)
					{
						float total = 0;

						for (int b = 0; b < 8; b++)
						{
							if (boxPoints2D[b].y != boxPoints2D[a].y)
							{
								float y = boxPoints2D[b].y - boxPoints2D[a].y;
								float f = Math::Length(boxPoints2D[a] - boxPoints2D[b]);
								boxPoints2DAngles[a][b] = (float)asin(y / f);

								//if (y < 0)
								//	boxPoints2DAngles[a][b] += D3DX_PI * 2.0f;
							}
							else if (boxPoints2D[b].x < boxPoints2D[a].x)
								boxPoints2DAngles[a][b] = D3DX_PI;
							/*else if (boxPoints2D[b].x != boxPoints2D[a].x)
							{
								float x = boxPoints2D[b].x - boxPoints2D[a].x;
								float f = Length(boxPoints2D[a] - boxPoints2D[b]);
								boxPoints2DAngles[a][b] = acos(x / f);

								if (x < 0)
									boxPoints2DAngles[a][b] += D3DX_PI * 2.0f;
							}*/
							else
								boxPoints2DAngles[a][b] = 0;

							total += boxPoints2DAngles[a][b];
						}

						if (total < D3DX_PI * 2.0f)
							boxProjection.push_back(boxPoints2D[a]);
					}
/*
					if (boxProjection.size() != 6)
					{
						int poo = boxProjection.size();

						int foo = 0;
					}
*/
				/*std::vector<TreeLeaf *>::iterator i = leafs.begin();
				for (; i != leafs.end(); i++)
				{
					if (Math::AABBFrustumCollision(Math::AABB((*i)->min, (*i)->max), frustumPlanes))
					{
						for (int n = 0; n < (*i)->numFaces; n++)
						{
							Chunk chunk = chunks[n];

							if (chunk.type == CT_LINELIST  ||
								chunk.type == CT_LINESTRIP ||
								chunk.type == CT_POINTLIST)
								continue;

							if (chunk.type == CT_POLYGON)
							{
								Vector3 *vertices = new Vector3[chunk.numVerts];

								//for (int v = 0; v < chunk.numVerts; v++)
								//	vertices[v] = Project(

								delete[] vertices;
							}
						}
					}
				}*/
			}

			if (useIndices && (*g)->GetIndexFormat() == IT_16)
			{
				delete[] pIndicesLocked;
				pIndicesLocked = NULL;
			}
		}

		return false;
	}


//-----------------------------------------------------------------------
	Matrix Scene::CreateVisibilityFrustum(TreeLeaf *origin, Vector3 forward, Vector3 up)
	//-------------------------------------------------------------------
	{
		Vector3 extends = origin->m_max - origin->m_min;
		Vector3 originCenter = origin->m_min + extends * 0.5f;

		float w = 0;
		float h = 0;
		float d = 0;

		Vector3 upVector;

		if (forward == Vector3(1, 0, 0) || forward == Vector3(-1, 0, 0))
		{
			w = extends.z;
			h = extends.y;
			d = extends.x;
			upVector = Vector3(0, 1, 0);
		}
		else if (forward == Vector3(0, 1, 0) || forward == Vector3(0, -1, 0))
		{
			w = extends.x;
			h = extends.z;
			d = extends.y;
			upVector = Vector3(0, 0, 1);
		}
		else if (forward == Vector3(0, 0, 1) || forward == Vector3(0, 0, -1))
		{
			w = extends.x;
			h = extends.y;
			d = extends.z;
			upVector = Vector3(0, 1, 0);
		}

		float fovX = 2 * (float)atan(w / d);
		float fovY = 2 * (float)atan(h / d);
		float a = fovY / fovX;

		Matrix view, projection;
		D3DXMatrixLookAtLH(&view, &originCenter, &(originCenter + forward), &upVector);
		D3DXMatrixPerspectiveFovLH(&projection, fovX, a, d * 0.5f, FLT_MAX);

		return view * projection;
	}


//-----------------------------------------------------------------------
	bool Scene::TestVisibilityLeaf(TreeLeaf *origin, TreeLeaf *target)
	//-------------------------------------------------------------------
	{
		if (!origin || !target)
			return false;

		if (origin == target)
			return true;

		Vector3 upVector;

		if (m_pSettings->spacePartitionAxis == SS_AXIS_X)
			upVector = Vector3(1, 0, 0);
		else if (m_pSettings->spacePartitionAxis == SS_AXIS_Y)
			upVector = Vector3(0, 1, 0);
		else if (m_pSettings->spacePartitionAxis == SS_AXIS_Z)
			upVector = Vector3(0, 0, 1);

		if (m_pSettings->spacePartitionMode == SS_QUADTREE)
		{

		}
		else if (m_pSettings->spacePartitionMode == SS_OCTTREE)
		{
			if (TestVisibility(target, CreateVisibilityFrustum(origin, Vector3(1,  0,  0), upVector)))
				return true;

			if (TestVisibility(target, CreateVisibilityFrustum(origin, Vector3(0,  1,  0), upVector)))
				return true;

			if (TestVisibility(target, CreateVisibilityFrustum(origin, Vector3(0,  0,  1), upVector)))
				return true;

			if (TestVisibility(target, CreateVisibilityFrustum(origin, Vector3(-1,  0,  0), upVector)))
				return true;

			if (TestVisibility(target, CreateVisibilityFrustum(origin, Vector3(0, -1,  0), upVector)))
				return true;

			if (TestVisibility(target, CreateVisibilityFrustum(origin, Vector3(0,  0, -1), upVector)))
				return true;
		}

		return false;
	}


//-----------------------------------------------------------------------
	HRESULT Scene::SubdivideWorld()
	//-------------------------------------------------------------------
	{
		m_spacePartitionProgress = 0;

		// Reset Partitioning information
			Reset(true, false);

			m_pSpacePartitionOutline = new Geometry(m_pD3DDevice);

		if (m_sourceGeometry.size() < 1)
			return S_OK;

		// Generate the vertex index buffers
			GeometryList::iterator g = m_sourceGeometry.begin();

			m_pRoot->m_min = (*g)->GetMin();
			m_pRoot->m_max = (*g)->GetMax();

			for (; g != m_sourceGeometry.end(); g++)
			{
				if ((*g)->GetNumVertices() < 1)
					continue;

				(*g)->GenerateIndexBuffers(GF_ALLOWZEROCHUNKINDICES);

				Math::SortMinMax((*g)->GetMin(), (*g)->GetMax(), m_pRoot->m_min, m_pRoot->m_max);
			}

		// Make the space partition cubic
			float m = m_pRoot->m_max.x - m_pRoot->m_min.x;

			if (m_pRoot->m_max.y - m_pRoot->m_min.y > m)
				m = m_pRoot->m_max.y - m_pRoot->m_min.y;

			if (m_pRoot->m_max.z - m_pRoot->m_min.z > m)
				m = m_pRoot->m_max.z - m_pRoot->m_min.z;

			if (m_pSettings->spacePartitionMode == SS_QUADTREE)
			{
				if (m_pSettings->spacePartitionAxis == SS_AXIS_X)
				{
					m_pRoot->m_max.y = m_pRoot->m_min.y + m;
					m_pRoot->m_max.z = m_pRoot->m_min.z + m;
				}
				else if (m_pSettings->spacePartitionAxis == SS_AXIS_Y)
				{
					m_pRoot->m_max.x = m_pRoot->m_min.x + m;
					m_pRoot->m_max.z = m_pRoot->m_min.z + m;
				}
				else if (m_pSettings->spacePartitionAxis == SS_AXIS_Z)
				{
					m_pRoot->m_max.x = m_pRoot->m_min.x + m;
					m_pRoot->m_max.y = m_pRoot->m_min.y + m;
				}
			}
			else if (m_pSettings->spacePartitionMode == SS_OCTTREE)
			{
				m_pRoot->m_max.x = m_pRoot->m_min.x + m;
				m_pRoot->m_max.y = m_pRoot->m_min.y + m;
				m_pRoot->m_max.z = m_pRoot->m_min.z + m;
			}

		// If space partition mode is none, force space partition depths to 0.
			if (m_pSettings->spacePartitionMode == SS_NONE)
			{
				m_pSettings->subdivisionDepth = 0;
				m_pSettings->polygonDepth = 0;
				m_pSettings->sizeDepth = 0;
			}
			else if (m_pSettings->subdivisionDepth == 0 && m_pSettings->polygonDepth == 0 && m_pSettings->sizeDepth == 0)
			{
				m_pSettings->spacePartitionMode = SS_NONE;
			}

		// setup optimised geometry from geometry to optimised geometry
			if (m_pSettings->optimiseChunks)
			{
				/*GeometryList::iterator g = m_sourceGeometry.begin();
				for (; g != m_sourceGeometry.end(); g++)
				{
					std::list< std::pair<PGEOMETRY, PGEOMETRY> >::iterator o = optimisedGeometry.begin();
					for (; o != optimisedGeometry.end(); o++)
					{
						if (o->first == (*g))
						{
							if ((*g)->GetNumVertices() < 1)
								continue;

							LPVOID pVertsLocked = NULL;
							(*g)->Lock(NULL, &pVertsLocked, NULL, NULL);

							if (o->second->GetUseVertexDeclarationFormat())
							{
								if (FAILED(o->second->Init((*g)->GetVertexSize(), (*g)->GetVertexDeclarationFormat(), (*g)->GetIndexFormat())))
									return E_FAIL;
							}
							else
							{
								if (FAILED(o->second->Init((*g)->GetVertexSize(), (*g)->GetVertexFormat(), (*g)->GetIndexFormat())))
									return E_FAIL;
							}

							if (FAILED(o->second->Insert(0, (*g)->GetNumVertices(), 0, 0, NULL, pVertsLocked, NULL, NULL, 0)))
								return E_FAIL;

							(*g)->Unlock();
						}
					}
				}*/
			}

		// Partition World
			if (m_pFnProgressCallback)
				m_pFnProgressCallback(PC_SCENE_SPACEPARTITIONING, m_spacePartitionProgress, m_pProgressCallbackPayload);

			// Build a temporary m_pRoot _NodeChunkMap * that contains all the geometry.
			unsigned int numPolygons = 0;

			_NodeChunkMap *pRootNodeChunkMap = CreateNodeChunkMap(&numPolygons);

			if (m_pSettings->spacePartitionMode == VS_NONE)
			{
				GenerateLeaf(m_pRoot, pRootNodeChunkMap);
			}
			else
			{
				Partition(m_pRoot, 0, pRootNodeChunkMap, NULL, numPolygons);
			}

			delete pRootNodeChunkMap;
			pRootNodeChunkMap = NULL;

			OptimisedGeometryMap::iterator og = m_optimisedGeometry.begin();
			for (; og != m_optimisedGeometry.end(); ++og)
			{
				if (!og->first || !og->second)
					continue;

				og->second->CreateChunkIndices();
				og->second->CreateVertexIndices(og->first->GetD3DDevice());

				/*std::map< unsigned int, std::vector< unsigned int > * >::iterator v = og->second->m_vertexIndexMap.begin();
				for (; v != og->second->m_vertexIndexMap.end(); ++v)
				{
					if (!v->second)
						continue;

					og->first->Insert(0, 0, 0, v->second->size(), NULL, NULL, NULL, v->second, 0);
				}*/
			}

		// Generate Leafs
			/*std::map< PGEOMETRY, Chunk * > srcChunks;
			std::map< PGEOMETRY, std::vector< unsigned int > > newVertexIndices;

			GeometryList::iterator geometryItr = m_sourceGeometry.begin();
			for (; geometryItr != m_sourceGeometry.end(); geometryItr++)
			{
				srcChunks[*geometryItr] = NULL;
			}

			GenerateLeafs(m_pRoot, srcChunks, newVertexIndices);

			for (geometryItr = m_sourceGeometry.begin(); geometryItr != m_sourceGeometry.end(); geometryItr++)
			{
				delete srcChunks[*geometryItr];
			}

			std::map< PGEOMETRY, std::vector< unsigned int > >::iterator nviItr = newVertexIndices.begin();
			for (; nviItr != newVertexIndices.end(); nviItr++)
			{
				PGEOMETRY pGeometry = nviItr->first;

				if (!pGeometry)
					continue;

				if (pGeometry->GetIndexGenerationFlags() & GF_ALLOWZEROVERTEXINDICES)
					continue;

				pGeometry->Insert(0, 0, 0, nviItr->second.size(), NULL, NULL, NULL, (void *)&nviItr->second[0], 0);
			}*/

		// Empty source geometry to save memory and copy optimised Geometry buffers to old geometry buffers to retain mapping
			if (m_pSettings->optimiseChunks)
			{
				if (m_pFnProgressCallback)
					m_pFnProgressCallback(PC_SCENE_OPTIMISECHUNKS, m_optimiseChunksProgress, m_pProgressCallbackPayload);

				/*GeometryList::iterator g = m_sourceGeometry.begin();
				for (; g != m_sourceGeometry.end(); g++)
				{
					std::list< std::pair<PGEOMETRY, PGEOMETRY> >::iterator o = optimisedGeometry.begin();
					for (; o != optimisedGeometry.end();)
					{
						if (o->first == (*g))
						{
							if (FAILED((*g)->Clear()))
								return E_FAIL;

							if (o->second->GetUseVertexDeclarationFormat())
							{
								if (FAILED(o->second->Init(o->second->GetVertexSize(), o->second->GetVertexDeclarationFormat(), o->second->GetIndexFormat())))
									return E_FAIL;
							}
							else
							{
								if (FAILED(o->second->Init(o->second->GetVertexSize(), o->second->GetVertexFormat(), o->second->GetIndexFormat())))
									return E_FAIL;
							}

							if (FAILED((*g)->Insert(o->second, 0)))
								return E_FAIL;

							if (o->second)
								o->second->Release();

							o = optimisedGeometry.erase(o);
						}
						else
						{
							o++;
						}
					}
				}*/
			}

		// Generate PVS Data
			if (m_pSettings->visibilityMode == SS_ENABLED)
			{

			}

		// Create Draw Partition
			if (m_pSpacePartitionOutline)
			{
				TreeNode *node = m_pRoot;
				std::vector<Vector3> verts;
				std::vector<Chunk> lineChunks;
				std::vector<Chunk> faceChunks;
				std::vector<unsigned int> vertexIndices;
				vertexIndices.resize(m_leafs.size() * 60);

				RecurseCreateDrawPartition(m_pRoot, verts, lineChunks, faceChunks, vertexIndices);

				int doubleVertsSize = (int)verts.size() * 2;

				VERTEX_DIFFUSE *vertices = new VERTEX_DIFFUSE[doubleVertsSize];

				for (int i = 0; i < (int)verts.size(); i++)
				{
					vertices[i].position = verts[i];
					vertices[i].color = 0xffff00ff;//m_pSettings->spacePartitionColour;
				}

				for (; i < doubleVertsSize; i++)
				{
					vertices[i].position = verts[i - (int)verts.size()];
					vertices[i].color = 0x7fff00ff;
				}

				if (FAILED(m_pSpacePartitionOutline->Init(sizeof(VERTEX_DIFFUSE), FVF_DIFFUSE, IT_32)))
					return E_FAIL;

				if (FAILED(m_pSpacePartitionOutline->Insert((unsigned int)lineChunks.size(), doubleVertsSize, 0, (unsigned int)vertexIndices.size(), 
															&lineChunks[0], vertices, NULL, &vertexIndices[0], 0)))
					return E_FAIL;

				if (FAILED(m_pSpacePartitionOutline->Insert((unsigned int)faceChunks.size(), 0, &faceChunks[0], NULL, 0)))
					return E_FAIL;

				if (FAILED(m_pSpacePartitionOutline->GenerateIndexBuffers(0)))
					return E_FAIL;

				delete[] vertices;
			}

		return S_OK;
	}

// Notes
/* -------------------------------

pNodeChunkMap [in] source NodeChunkMap or NULL.
pRemainder [out] the portion of pNodeChunkMap that is not included in pResult (so, empty if pNodeChunkMap is NULL). Can be NULL, in which case it is ignored.
min, max [in] bounds for the resulting NodeChunkMap.
pResult (return value) [out] NodeChunkMap contains all geometry within the given bounds found in pNodeChunkMap (if not NULL).

---------------------------------- */

//-----------------------------------------------------------------------
	Scene::_NodeChunkMap *Scene::CreateNodeChunkMap(unsigned int *pNumPolygons)
	//-------------------------------------------------------------------
	{
		_NodeChunkMap *pResult = new _NodeChunkMap;

		Chunk *chunks = NULL;
		PVOID pVertsLocked = NULL;
		int *pIndicesLocked = NULL;
		unsigned int numPolygons = 0;

		// There is no parent to read from, then use all available geometry.
			bool useIndices = true;

			GeometryList::iterator g = m_sourceGeometry.begin();
			for (; g != m_sourceGeometry.end(); g++)
			{
				PGEOMETRY pCurGeometry = *g;

				if (pCurGeometry->GetNumChunks() < 1 || pCurGeometry->GetNumVertices() < 1)
					continue;

				if (pCurGeometry->GetNumVertexIndices() < 1)
					useIndices = false;

				_OrderedGeometryMap *pOrderedGeometryMap = new _OrderedGeometryMap;

				// Lock
					if (useIndices)
					{
						if (pCurGeometry->GetIndexFormat() == IT_32)
						{
							if (FAILED(pCurGeometry->Lock((PVOID*)&chunks, &pVertsLocked, NULL, (PVOID*)&pIndicesLocked)))
								return NULL;
						}
						else if (pCurGeometry->GetIndexFormat() == IT_16)
						{
							int numIndices = pCurGeometry->GetNumVertexIndices();
							pIndicesLocked = new int[numIndices];

							PVOID pIndicesLockedTemp = NULL;

							if (FAILED(pCurGeometry->Lock((PVOID*)&chunks, &pVertsLocked, NULL, &pIndicesLockedTemp)))
								return NULL;

							for (int i = 0; i < numIndices; i++)
								pIndicesLocked[i] = (int)((short int *)pIndicesLockedTemp)[i];
						}
					}
					else
					{
						if (FAILED(pCurGeometry->Lock((PVOID*)&chunks, &pVertsLocked, NULL, NULL)))
							return NULL;
					}

				// Extract geometry
					for (unsigned int i = 0; i < (unsigned int)pCurGeometry->GetNumChunks(); i++)
					{
						Chunk &chunk = chunks[i];

						// TODO: Handle failures below

						_GeometryMap *pGeometryMap = pOrderedGeometryMap->GetPtr(chunk.order);

						_ChunkIndexList *pChunkIndexList = pGeometryMap->GetPtr(pCurGeometry);

						//_VertexIndexList *pVertexIndexList = pChunkIndexList->GetPtr(i);

						_IndexSet *pIndexSet = pChunkIndexList->GetPtr(i);

						unsigned int vertexIndexListId = pIndexSet->Create();

						switch (chunk.type)
						{
						case CT_POLYGON:
							numPolygons += chunk.numIndices - 2;
							break;

						case CT_TRIANGLELIST:
							numPolygons += chunk.numIndices / 3;
							break;

						case CT_TRIANGLESTRIP:
							numPolygons += chunk.numIndices - 2;
							break;

						case CT_LINELIST:
							numPolygons += chunk.numIndices / 2;
							break;

						case CT_LINESTRIP:
							numPolygons += chunk.numIndices - 1;
							break;

						case CT_POINTLIST:
							numPolygons += chunk.numIndices;
							break;
						}

						if (useIndices)
						{
							for (unsigned int v = 0; v < chunk.numIndices; v++)
							{
								//pVertexIndexList->PushBack(pIndicesLocked[chunk.startIndex + v]);
								((*pIndexSet)[vertexIndexListId])->PushBack(pIndicesLocked[chunk.startIndex + v]);
							}
						}
						else
						{
							for (unsigned int v = 0; v < chunk.numVerts; v++)
							{
								//pVertexIndexList->PushBack(chunk.startIndex + v);
								((*pIndexSet)[vertexIndexListId])->PushBack(chunk.startIndex + v);
							}
						}
					}

				pResult->m.push_back(pOrderedGeometryMap);

				// Unlock
					(*g)->Unlock();

					if (useIndices && (*g)->GetIndexFormat() == IT_16)
					{
						delete[] pIndicesLocked;
						pIndicesLocked = NULL;
					}
			}

		if (pNumPolygons)
		{
			*pNumPolygons = numPolygons;
		}

		if (pResult->m.empty())
		{
			delete pResult;
			return NULL;
		}

		return pResult;
	}


//-----------------------------------------------------------------------
	Scene::_NodeChunkMap *Scene::CreateNodeChunkMap(Scene::_NodeChunkMap *pNodeChunkMap, Scene::_NodeChunkMap *pRemainder, 
													const Math::AABB &bounds, unsigned int *pNumPolygons)
	//-------------------------------------------------------------------
	{
		typedef bool (* SimpleAABBTriangleIntersectionFuncPtr)(const Math::AABB &, const Vector3 *);

		SimpleAABBTriangleIntersectionFuncPtr pSimpleAABBTriangleIntersection = Math::SimpleAABBTriangleIntersection;

		if (m_pSettings->spacePartitionMode == SS_QUADTREE)
		{
			if (m_pSettings->spacePartitionAxis == SS_AXIS_Z)
			{
				pSimpleAABBTriangleIntersection = Math::SimpleAABBTriangleIntersectionXY;
			}
			else if (m_pSettings->spacePartitionAxis == SS_AXIS_Y)
			{
				pSimpleAABBTriangleIntersection = Math::SimpleAABBTriangleIntersectionXZ;
			}
			else if (m_pSettings->spacePartitionAxis == SS_AXIS_X)
			{
				pSimpleAABBTriangleIntersection = Math::SimpleAABBTriangleIntersectionYZ;
			}
		}

#ifdef VERBOSE_PARTITIONING
		DebugPrintf(_T("CreateNodeChunkMap: (%.3f, %.3f, %.3f) - (%.3f, %.3f, %.3f)\n"), bounds.min.x, bounds.min.y, bounds.min.z, bounds.max.x, bounds.max.y, bounds.max.z);
#endif

		_NodeChunkMap *pResult = new _NodeChunkMap;

		Chunk *chunks = NULL;
		PVOID pVertsLocked = NULL;
		int *pIndicesLocked = NULL;
		unsigned int numPolygons = 0;

		bool useIndices = true;

#ifdef VERBOSE_PARTITIONING
		DebugPrintf(_T("\nCreateNodeChunkMap()\n\tPartitioning Geometry for node (min = (%.2f, %.2f, %.2f), max = (%.2f, %.2f, %.2f))\n"),
				   bounds.min.x, bounds.min.y, bounds.min.z, bounds.max.x, bounds.max.y, bounds.max.z);

		DebugPrintf(_T("\tNodeChunkMap size = %d\n"), pNodeChunkMap->m.size());
#endif

		NodeChunkMap::const_iterator ncmItr = pNodeChunkMap->m.begin();
		for (; ncmItr != pNodeChunkMap->m.end(); ncmItr++)
		{
			if ((*ncmItr)->m.empty())
				continue;

			const OrderedGeometryMap &ogm = (*ncmItr)->m;

#ifdef VERBOSE_PARTITIONING
			DebugPrintf(_T("\t\tOrderedGeometryMap size = %d\n"), ogm.size());
#endif

			OrderedGeometryMap::const_iterator ogmItr = ogm.begin();
			for (; ogmItr != ogm.end(); ogmItr++)
			{
				if (ogmItr->second->m.empty())
					continue;

#ifdef VERBOSE_PARTITIONING
				DebugPrintf(_T("\tSearching OrderedGeometryMap at order = %d\n"), ogmItr->first);
#endif
				const GeometryMap &gm = ogmItr->second->m;

#ifdef VERBOSE_PARTITIONING
				DebugPrintf(_T("\t\t\tGeometryMap size = %d\n"), gm.size());
#endif

				GeometryMap::const_iterator gmItr = gm.begin();
				for (; gmItr != gm.end(); gmItr++)
				{
					PGEOMETRY pGeometry = gmItr->first;

					if (!pGeometry)
					{
#ifdef VERBOSE_PARTITIONING
						DebugPrintf(_T("\tNo Geometry to Partition.\n"), ogmItr->first);
#endif
						continue;
					}

					if (pGeometry->GetNumChunks() < 1 || pGeometry->GetNumVertices() < 1)
					{
#ifdef VERBOSE_PARTITIONING
						DebugPrintf(_T("\tNo Geometry to Partition.\n"), ogmItr->first);
#endif
						continue;
					}

					if (pGeometry->GetNumVertexIndices() < 1)
						useIndices = false;

					if (gmItr->second->m.empty())
					{
#ifdef VERBOSE_PARTITIONING
						DebugPrintf(_T("\tNo Geometry to Partition.\n"), ogmItr->first);
#endif
						continue;
					}

					_OrderedGeometryMap *pOrderedGeometryMap = new _OrderedGeometryMap;
					_OrderedGeometryMap *pRemainderOrderedGeometryMap = NULL;

					if (pRemainder)
					{
						pRemainderOrderedGeometryMap = new _OrderedGeometryMap;
					}

					// Lock
						if (useIndices)
						{
							if (pGeometry->GetIndexFormat() == IT_32)
							{
								if (FAILED(pGeometry->Lock((PVOID*)&chunks, &pVertsLocked, NULL, (PVOID*)&pIndicesLocked)))
									return NULL;
							}
							else if (pGeometry->GetIndexFormat() == IT_16)
							{
								int numIndices = pGeometry->GetNumVertexIndices();
								pIndicesLocked = new int[numIndices];

								PVOID pIndicesLockedTemp = NULL;

								if (FAILED(pGeometry->Lock((PVOID*)&chunks, &pVertsLocked, NULL, &pIndicesLockedTemp)))
									return NULL;

								for (int i = 0; i < numIndices; i++)
									pIndicesLocked[i] = (int)((short int *)pIndicesLockedTemp)[i];
							}
						}
						else
						{
							if (FAILED(pGeometry->Lock((PVOID*)&chunks, &pVertsLocked, NULL, NULL)))
								return NULL;
						}

#ifdef VERBOSE_PARTITIONING
						for (int v = 0; v < (int)pGeometry->GetNumVertices(); v++)
						{
							const Vector3 &vertex = pGeometry->GetVertexPosition(pVertsLocked, v);
							DebugPrintf("Vertex %d = (%.3f, %.3f, %.3f)\n", v, vertex.x, vertex.y, vertex.z);
						}

						DebugPrintf("\n");

						for (v = 0; v < (int)pGeometry->GetNumVertexIndices(); v++)
						{
							DebugPrintf("Vertex Index %d = %d\n", v, pIndicesLocked[v]);
						}
#endif

					// Extract geometry with bounds testing
						const ChunkIndexList &cil = gmItr->second->m;

#ifdef VERBOSE_PARTITIONING
						DebugPrintf(_T("\t\t\t\tChunkIndexList size = %d\n"), cil.size());
#endif

						ChunkIndexList::const_iterator cilItr = cil.begin();
						for (; cilItr != cil.end(); cilItr++)
						{
#ifdef VERBOSE_PARTITIONING
							DebugPrintf(_T("\t\tSearching Chunk %d.\n"), cilItr->first);
#endif
							Chunk &chunk = chunks[cilItr->first];

							// TODO: handle failure
							_GeometryMap *pGeometryMap = pOrderedGeometryMap->GetPtr(chunk.order);
							_GeometryMap *pRemainderGeometryMap = NULL;

							if (pRemainder)
							{
								pRemainderGeometryMap = pRemainderOrderedGeometryMap->GetPtr(chunk.order);
							}

							// TODO: handle failure
							_ChunkIndexList *pChunkIndexList = pGeometryMap->GetPtr(pGeometry);
							_ChunkIndexList *pRemainderChunkIndexList = NULL;

							if (pRemainder)
							{
								pRemainderChunkIndexList = pRemainderGeometryMap->GetPtr(pGeometry);
							}

							const IndexSet &srcIndexSet = cilItr->second->m;

							if (srcIndexSet.empty())
							{
								continue;
							}

							_IndexSet *pIndexSet = pChunkIndexList->GetPtr(cilItr->first);
							_IndexSet *pRemainderIndexSet = NULL;

							if (pRemainder)
							{
								pRemainderIndexSet = pRemainderChunkIndexList->GetPtr(cilItr->first);
							}

							if (useIndices)
							{
								switch (chunk.type)
								{
								case CT_POLYGON:
									{
										int poo = 0;
									}
									break;

								case CT_TRIANGLELIST:
									{
										IndexSet::const_iterator isItr = srcIndexSet.begin();
										for (; isItr != srcIndexSet.end(); isItr++)
										{
											_VertexIndexList *pSrcVertexIndexList = *isItr;

											unsigned int vertexIndexListId = INT_MAX;
											unsigned int remainderVertexIndexListId = INT_MAX;

											for (unsigned int p = 0; p < pSrcVertexIndexList->m.size(); p += 3)
											{
												Vector3 vertices[3];

												unsigned int t0 = pSrcVertexIndexList->m[p];
												unsigned int t1 = pSrcVertexIndexList->m[p + 1];
												unsigned int t2 = pSrcVertexIndexList->m[p + 2];

												vertices[0] = pGeometry->GetVertexPosition(pVertsLocked, t0);
												vertices[1] = pGeometry->GetVertexPosition(pVertsLocked, t1);
												vertices[2] = pGeometry->GetVertexPosition(pVertsLocked, t2);

												bool intersects = false;
												bool contains = false;

												if (Math::AABBContainsTriangle(bounds, vertices))
												{
													intersects = true;
													contains = true;
												}

												if (!contains && pSimpleAABBTriangleIntersection(bounds, vertices))
												{
													intersects = true;
												}

												if (intersects)
												{
													if (vertexIndexListId == INT_MAX)
													{
                                                        vertexIndexListId = pIndexSet->Create();
													}

													_VertexIndexList *pVertexIndexList = (*pIndexSet)[vertexIndexListId];
													pVertexIndexList->PushBack(t0);
													pVertexIndexList->PushBack(t1);
													pVertexIndexList->PushBack(t2);

													if (contains)
													{
														continue;
													}
												}

												if (pRemainder)
												{
													if (remainderVertexIndexListId == INT_MAX)
													{
														remainderVertexIndexListId = pRemainderIndexSet->Create();
													}

													_VertexIndexList *pRemainderVertexIndexList = (*pRemainderIndexSet)[pRemainderIndexSet->Create()];

													pRemainderVertexIndexList->PushBack(t0);
													pRemainderVertexIndexList->PushBack(t1);
													pRemainderVertexIndexList->PushBack(t2);
												}
											}
										}
									}
									break;

								case CT_TRIANGLESTRIP:
									{
										IndexSet::const_iterator isItr = srcIndexSet.begin();
										for (; isItr != srcIndexSet.end(); isItr++)
										{
											_VertexIndexList *pSrcVertexIndexList = *isItr;

											unsigned int vertexIndexListId = INT_MAX;
											unsigned int remainderVertexIndexListId = INT_MAX;

											for (unsigned int p = 2; p < (*isItr)->m.size(); p++)
											{
												Vector3 vertices[3];

												unsigned int t0 = (*isItr)->m[p];
												unsigned int t1 = (*isItr)->m[p - 1];
												unsigned int t2 = (*isItr)->m[p - 2];

												vertices[0] = pGeometry->GetVertexPosition(pVertsLocked, t0);
												vertices[1] = pGeometry->GetVertexPosition(pVertsLocked, t1);
												vertices[2] = pGeometry->GetVertexPosition(pVertsLocked, t2);

												bool intersects = false;
												bool contains = false;

												if (Math::AABBContainsTriangle(bounds, vertices))
												{
													intersects = true;
													contains = true;
												}
												else if (!contains && pSimpleAABBTriangleIntersection(bounds, vertices))
												{
													intersects = true;
												}

												if (intersects)
												{
													if (vertexIndexListId == INT_MAX)
													{
                                                        vertexIndexListId = pIndexSet->Create();
														_VertexIndexList *pVertexIndexList = (*pIndexSet)[vertexIndexListId];

														pVertexIndexList->PushBack(t2);
														pVertexIndexList->PushBack(t1);
														pVertexIndexList->PushBack(t0);
													}
													else
													{
														_VertexIndexList *pVertexIndexList = (*pIndexSet)[vertexIndexListId];

														pVertexIndexList->PushBack(t0);
													}

													if (contains)
													{
														remainderVertexIndexListId = INT_MAX;
														continue;
													}
												}
												else
												{
													vertexIndexListId = INT_MAX;
												}

												if (pRemainder)
												{
													if (remainderVertexIndexListId == INT_MAX)
													{
														remainderVertexIndexListId = pRemainderIndexSet->Create();
														_VertexIndexList *pRemainderVertexIndexList = (*pRemainderIndexSet)[remainderVertexIndexListId];

														pRemainderVertexIndexList->PushBack(t2);
														pRemainderVertexIndexList->PushBack(t1);
														pRemainderVertexIndexList->PushBack(t0);
													}
													else
													{
														_VertexIndexList *pRemainderVertexIndexList = (*pRemainderIndexSet)[remainderVertexIndexListId];

														pRemainderVertexIndexList->PushBack(t0);
													}
												}
											}
										}
									}
									break;

								case CT_LINELIST:
									{
										int poo = 0;
									}
									break;

								case CT_LINESTRIP:
									{
										int poo = 0;
									}
									break;

								case CT_POINTLIST:
									{
										IndexSet::const_iterator isItr = srcIndexSet.begin();
										for (; isItr != srcIndexSet.end(); isItr++)
										{
											_VertexIndexList *pSrcVertexIndexList = *isItr;

											unsigned int vertexIndexListId = INT_MAX;
											unsigned int remainderVertexIndexListId = INT_MAX;

											for (unsigned int p = 0; p < pSrcVertexIndexList->m.size(); p++)
											{
												unsigned int t = pSrcVertexIndexList->m[p];

												const Vector3 &vertex = pGeometry->GetVertexPosition(pVertsLocked, t);

												if (Math::AABBPointCollision(bounds, vertex))
												{
													if (vertexIndexListId == INT_MAX)
													{
                                                        vertexIndexListId = pIndexSet->Create();
													}

													_VertexIndexList *pVertexIndexList = (*pIndexSet)[vertexIndexListId];
													pVertexIndexList->PushBack(t);

													if (pRemainder)
													{
														if (remainderVertexIndexListId == INT_MAX)
														{
															remainderVertexIndexListId = pRemainderIndexSet->Create();
														}

														_VertexIndexList *pRemainderVertexIndexList = (*pRemainderIndexSet)[pRemainderIndexSet->Create()];
														pRemainderVertexIndexList->PushBack(t);
													}
												}
											}
										}
									}
									break;
								}
							}
						}

					pResult->m.push_back(pOrderedGeometryMap);
					
					if (pRemainder)
					{
						pRemainder->m.push_back(pRemainderOrderedGeometryMap);
					}

					// Unlock
						pGeometry->Unlock();

						if (useIndices && pGeometry->GetIndexFormat() == IT_16)
						{
							delete[] pIndicesLocked;
							pIndicesLocked = NULL;
						}
				}
			}
		}

		if (pResult->m.empty())
		{
			delete pResult;
			return NULL;
		}

		if (pNumPolygons)
			*pNumPolygons = numPolygons;

		return pResult;
	}


//-----------------------------------------------------------------------
	void Scene::Partition(TreeNode *node, int numSubdivisions, _NodeChunkMap *pNodeChunkMap, _NodeChunkMap *pRemainingNodeChunkMap, unsigned int numPrimitives)
	//-------------------------------------------------------------------
	{
#ifdef VERBOSE_PARTITIONING
		DebugPrintf(_T("\nPartition()\n\tNode (min = (%.2f, %.2f, %.2f), max = (%.2f, %.2f, %.2f))\n"),
				node->m_min.x, node->m_min.y, node->m_min.z, node->m_max.x, node->m_max.y, node->m_max.z);
#endif

		if (!pNodeChunkMap)
			return;

		Vector3 min = node->m_min;
		Vector3 max = node->m_max;

		if (pNodeChunkMap->IsEmpty())
		{
			// There is no geometry for this leaf to attempt to partition, so we can create an
			// empty leaf and return early. Must take a copy of pNodeChunkMap, so that it is cleared up properly.

#ifdef VERBOSE_PARTITIONING
			DebugPrintf(_T("\tpNodeChunkMap is empty. Storing empty leaf.\n"));
#endif

			GenerateLeaf(node, NULL);

			UpdateSpacePartitionProgress(node);

			return;
		}

		// Setup local variables
			int numDivisions = numSubdivisions;

			if (numDivisions > m_subdivisionsExecuted)
				m_subdivisionsExecuted = numDivisions;

			unsigned int numPolygons = numPrimitives;
			int numEffectGroups = m_pResourceManager->GetNumEffects();

			Chunk *chunks = NULL;
			PVOID pVertsLocked = NULL;
			int *pIndicesLocked = NULL;

			node->m_leafIndex = -1;

		// Partition Geometry
			_NodeChunkMap *pResultingNodeChunkMap = NULL;

			if (node->m_pParent)
			{
				if (m_pSettings->optimiseChunks)
				{
					
				}
				else
				{
					pResultingNodeChunkMap = CreateNodeChunkMap(pNodeChunkMap, pRemainingNodeChunkMap, Math::AABB(min, max), &numPolygons);
				}
			}
			else
			{
				pResultingNodeChunkMap = pNodeChunkMap;
			}

		// Determine if further subdivision is necessary
			float sizeX = max.x - min.x;
			float sizeY = max.y - min.y;
			float sizeZ = max.z - min.z;

			bool subdivideSize = false;
			bool subdividePoly = false;
			bool subdivideSub  = false;

			if (m_pSettings->sizeDepth > 0)
			{
				if (m_pSettings->spacePartitionAxis == SS_AXIS_X)
				{
					if (m_pSettings->sizeDepth < sizeY || m_pSettings->sizeDepth < sizeZ || (m_pSettings->spacePartitionMode == SS_OCTTREE && m_pSettings->sizeDepth < sizeX))
						subdivideSize = true;
				}
				else if (m_pSettings->spacePartitionAxis == SS_AXIS_Y)
				{
					if (m_pSettings->sizeDepth < sizeX || m_pSettings->sizeDepth < sizeZ || (m_pSettings->spacePartitionMode == SS_OCTTREE && m_pSettings->sizeDepth < sizeY))
						subdivideSize = true;
				}
				else if (m_pSettings->spacePartitionAxis == SS_AXIS_Z)
				{
					if (m_pSettings->sizeDepth < sizeX || m_pSettings->sizeDepth < sizeY || (m_pSettings->spacePartitionMode == SS_OCTTREE && m_pSettings->sizeDepth < sizeZ))
						subdivideSize = true;
				}
			}
			else
				subdivideSize = true;

			if (m_pSettings->polygonDepth > 0)
			{
				if (m_pSettings->polygonDepth < (int)numPolygons)
					subdividePoly = true;
			}
			else
				subdividePoly = true;

			if (m_pSettings->subdivisionDepth > 0)
			{
				if (numDivisions < m_pSettings->subdivisionDepth)
					subdivideSub = true;
			}
			else
				subdivideSub = true;

		// Subdivide if so
			if (subdivideSize && subdividePoly && subdivideSub && (m_pSettings->polygonDepth || m_pSettings->sizeDepth || m_pSettings->subdivisionDepth))
			{
#ifdef VERBOSE_PARTITIONING
				DebugPrintf(_T("\tSubdividing.\n"));
#endif
				numDivisions++;

				Vector3 mid(0, 0, 0);

				if (m_pSettings->spacePartitionMode == SS_QUADTREE)
				{
					node->m_pChildren = new TreeNode *[4];
					node->m_numChildren = 4;

					if (m_pSettings->spacePartitionAxis == SS_AXIS_X)
						mid = min + Vector3(sizeX, sizeY * 0.5f, sizeZ * 0.5f);
					else if (m_pSettings->spacePartitionAxis == SS_AXIS_Y)
						mid = min + Vector3(sizeX * 0.5f, sizeY, sizeZ * 0.5f);
					else if (m_pSettings->spacePartitionAxis == SS_AXIS_Z)
						mid = min + Vector3(sizeX * 0.5f, sizeY * 0.5f, sizeZ);
				}
				else if (m_pSettings->spacePartitionMode == SS_OCTTREE)
				{
					node->m_pChildren = new TreeNode *[8];
					node->m_numChildren = 8;
					mid = min + Vector3(sizeX * 0.5f, sizeY * 0.5f, sizeZ * 0.5f);
				}

				if (m_pSettings->spacePartitionMode == SS_QUADTREE)
				{
					if (m_pSettings->spacePartitionAxis == SS_AXIS_X)
					{
						// Node 0
							node->m_pChildren[0] = new TreeNode(min, mid, node, m_numNodes);
							m_numNodes++;

                            _NodeChunkMap *pRemainingNodeChunkMap0 = new _NodeChunkMap;
							Partition(node->m_pChildren[0], numDivisions, pResultingNodeChunkMap, pRemainingNodeChunkMap0);

						// Node 1
							node->m_pChildren[1] = new TreeNode(Vector3(min.x, mid.y, min.z), Vector3(mid.x, max.y, mid.z), node, m_numNodes);
							m_numNodes++;

							_NodeChunkMap *pRemainingNodeChunkMap1 = new _NodeChunkMap;
							Partition(node->m_pChildren[1], numDivisions, pRemainingNodeChunkMap0, pRemainingNodeChunkMap1);

						// Node 2
							node->m_pChildren[2] = new TreeNode(Vector3(min.x, min.y, mid.z), Vector3(mid.x, mid.y, max.z), node, m_numNodes);
							m_numNodes++;

							_NodeChunkMap *pRemainingNodeChunkMap2 = new _NodeChunkMap;
							Partition(node->m_pChildren[2], numDivisions, pRemainingNodeChunkMap1, pRemainingNodeChunkMap2);

						// Node 3
							node->m_pChildren[3] = new TreeNode(Vector3(min.x, mid.y, mid.z), Vector3(mid.x, max.y, max.z), node, m_numNodes);
							m_numNodes++;

							Partition(node->m_pChildren[3], numDivisions, pRemainingNodeChunkMap2, NULL);

						delete pRemainingNodeChunkMap0;
						delete pRemainingNodeChunkMap1;
						delete pRemainingNodeChunkMap2;
					}
					else if (m_pSettings->spacePartitionAxis == SS_AXIS_Y)
					{
						// Node 0
							node->m_pChildren[0] = new TreeNode(min, mid, node, m_numNodes);
							m_numNodes++;

                            _NodeChunkMap *pRemainingNodeChunkMap0 = new _NodeChunkMap;
							Partition(node->m_pChildren[0], numDivisions, pResultingNodeChunkMap, pRemainingNodeChunkMap0);

						// Node 1
							node->m_pChildren[1] = new TreeNode(Vector3(mid.x, min.y, min.z), Vector3(max.x, mid.y, mid.z), node, m_numNodes);
							m_numNodes++;

							_NodeChunkMap *pRemainingNodeChunkMap1 = new _NodeChunkMap;
							Partition(node->m_pChildren[1], numDivisions, pRemainingNodeChunkMap0, pRemainingNodeChunkMap1);

						// Node 2
							node->m_pChildren[2] = new TreeNode(Vector3(min.x, min.y, mid.z), Vector3(mid.x, mid.y, max.z), node, m_numNodes);
							m_numNodes++;

							_NodeChunkMap *pRemainingNodeChunkMap2 = new _NodeChunkMap;
							Partition(node->m_pChildren[2], numDivisions, pRemainingNodeChunkMap1, pRemainingNodeChunkMap2);

						// Node 3
							node->m_pChildren[3] = new TreeNode(Vector3(mid.x, min.y, mid.z), Vector3(max.x, mid.y, max.z), node, m_numNodes);
							m_numNodes++;

							Partition(node->m_pChildren[3], numDivisions, pRemainingNodeChunkMap2, NULL);

						delete pRemainingNodeChunkMap0;
						delete pRemainingNodeChunkMap1;
						delete pRemainingNodeChunkMap2;
					}
					else if (m_pSettings->spacePartitionAxis == SS_AXIS_Z)
					{
						// Node 0
							node->m_pChildren[0] = new TreeNode(min, mid, node, m_numNodes);
							m_numNodes++;

                            _NodeChunkMap *pRemainingNodeChunkMap0 = new _NodeChunkMap;
							Partition(node->m_pChildren[0], numDivisions, pResultingNodeChunkMap, pRemainingNodeChunkMap0);

						// Node 1
							node->m_pChildren[1] = new TreeNode(Vector3(mid.x, min.y, min.z), Vector3(max.x, mid.y, mid.z), node, m_numNodes);
							m_numNodes++;

							_NodeChunkMap *pRemainingNodeChunkMap1 = new _NodeChunkMap;
							Partition(node->m_pChildren[1], numDivisions, pRemainingNodeChunkMap0, pRemainingNodeChunkMap1);

						// Node 2
							node->m_pChildren[2] = new TreeNode(Vector3(min.x, mid.y, min.z), Vector3(mid.x, max.y, mid.z), node, m_numNodes);
							m_numNodes++;

							_NodeChunkMap *pRemainingNodeChunkMap2 = new _NodeChunkMap;
							Partition(node->m_pChildren[2], numDivisions, pRemainingNodeChunkMap1, pRemainingNodeChunkMap2);

						// Node 3
							node->m_pChildren[3] = new TreeNode(Vector3(mid.x, mid.y, min.z), Vector3(max.x, max.y, mid.z), node, m_numNodes);
							m_numNodes++;

							Partition(node->m_pChildren[3], numDivisions, pRemainingNodeChunkMap2, NULL);

						delete pRemainingNodeChunkMap0;
						delete pRemainingNodeChunkMap1;
						delete pRemainingNodeChunkMap2;
					}
				}
				else
				{
					// Node 0
						node->m_pChildren[0] = new TreeNode(min, mid, node, m_numNodes);
						m_numNodes++;

                        _NodeChunkMap *pRemainingNodeChunkMap0 = new _NodeChunkMap;
						Partition(node->m_pChildren[0], numDivisions, pResultingNodeChunkMap, pRemainingNodeChunkMap0);

					// Node 1
						node->m_pChildren[1] = new TreeNode(Vector3(mid.x, min.y, min.z), Vector3(max.x, mid.y, mid.z), node, m_numNodes);
						m_numNodes++;

						_NodeChunkMap *pRemainingNodeChunkMap1 = new _NodeChunkMap;
						Partition(node->m_pChildren[1], numDivisions, pRemainingNodeChunkMap0, pRemainingNodeChunkMap1);

					// Node 2
						node->m_pChildren[2] = new TreeNode(Vector3(min.x, min.y, mid.z), Vector3(mid.x, mid.y, max.z), node, m_numNodes);
						m_numNodes++;

						_NodeChunkMap *pRemainingNodeChunkMap2 = new _NodeChunkMap;
						Partition(node->m_pChildren[2], numDivisions, pRemainingNodeChunkMap1, pRemainingNodeChunkMap2);

					// Node 3
						node->m_pChildren[3] = new TreeNode(Vector3(mid.x, min.y, mid.z), Vector3(max.x, mid.y, max.z), node, m_numNodes);
						m_numNodes++;

						_NodeChunkMap *pRemainingNodeChunkMap3 = new _NodeChunkMap;
						Partition(node->m_pChildren[3], numDivisions, pRemainingNodeChunkMap2, pRemainingNodeChunkMap3);

					// Node 4
						node->m_pChildren[4] = new TreeNode(Vector3(min.x, mid.y, min.z), Vector3(mid.x, max.y, mid.z), node, m_numNodes);
						m_numNodes++;

                        _NodeChunkMap *pRemainingNodeChunkMap4 = new _NodeChunkMap;
						Partition(node->m_pChildren[4], numDivisions, pRemainingNodeChunkMap3, pRemainingNodeChunkMap4);

					// Node 5
						node->m_pChildren[5] = new TreeNode(Vector3(mid.x, mid.y, min.z), Vector3(max.x, max.y, mid.z), node, m_numNodes);
						m_numNodes++;

						_NodeChunkMap *pRemainingNodeChunkMap5 = new _NodeChunkMap;
						Partition(node->m_pChildren[5], numDivisions, pRemainingNodeChunkMap4, pRemainingNodeChunkMap5);

					// Node 6
						node->m_pChildren[6] = new TreeNode(Vector3(min.x, mid.y, mid.z), Vector3(mid.x, max.y, max.z), node, m_numNodes);
						m_numNodes++;

						_NodeChunkMap *pRemainingNodeChunkMap6 = new _NodeChunkMap;
						Partition(node->m_pChildren[6], numDivisions, pRemainingNodeChunkMap5, pRemainingNodeChunkMap6);

					// Node 7
						node->m_pChildren[7] = new TreeNode(Vector3(mid.x, mid.y, mid.z), Vector3(max.x, max.y, max.z), node, m_numNodes);
						m_numNodes++;

						Partition(node->m_pChildren[7], numDivisions, pRemainingNodeChunkMap6, NULL);

					delete pRemainingNodeChunkMap0;
					delete pRemainingNodeChunkMap1;
					delete pRemainingNodeChunkMap2;
					delete pRemainingNodeChunkMap3;
					delete pRemainingNodeChunkMap4;
					delete pRemainingNodeChunkMap5;
					delete pRemainingNodeChunkMap6;
				}
			}
			else
			{
				// The node is not being partitioned, so place the current geometry in this leaf.

#ifdef VERBOSE_PARTITIONING
				DebugPrintf(_T("\tNot Subdividing. Creating the leaf.\n"));
#endif

				GenerateLeaf(node, pResultingNodeChunkMap);

				UpdateSpacePartitionProgress(node);
			}

		if (node->m_pParent)
		{
			delete pResultingNodeChunkMap;
		}
	}


#if 0
//-----------------------------------------------------------------------
	void Scene::Partition(TreeNode *node, int numSubdivisions)
	//-------------------------------------------------------------------
	{
#ifdef DEBUG_DUMP
		static int depth = 0;

		char buf[256];
		sprintf(buf, "\nLevel %d ----------------------------------------------------------\n", depth);
		OutputDebugString(buf);

		depth++;
#endif

		if (!node)
		{
#ifdef DEBUG_DUMP
			depth--;
#endif
			return;
		}

#ifdef DEBUG_DUMP
		OutputDebugString("Partition() :: Entry Point\n");
#endif

		int numDivisions = numSubdivisions;

		if (numDivisions > subdivisionsExecuted)
			subdivisionsExecuted = numDivisions;

		int numPolygons = 0;
		int numEffectGroups = resourceManager->GetNumEffects();

		Vector3 min = node->min;
		Vector3 max = node->max;

		float x = max.x - min.x;
		float y = max.y - min.y;
		float z = max.z - min.z;

		Chunk *chunks = NULL;
		PVOID pVertsLocked = NULL;
		int *pIndicesLocked = NULL;

		node->leafIndex = -1;

		_NodeChunkMap *pNodeGeometryChunkMap = new _NodeChunkMap;

#ifdef DEBUG_DUMP
		OutputDebugString("Partition() :: Allocating new _NodeChunkMap\n");
#endif

		bool useIndices = true;

		GeometryList::iterator g = geometry.begin();
		for (; g != geometry.end(); g++)
		{
			PGEOMETRY pCurGeometry = *g;

			if (pCurGeometry->GetNumChunks() < 1 || pCurGeometry->GetNumVertices() < 1)
				continue;

			if (pCurGeometry->GetNumVertexIndices() < 1)
				useIndices = false;

			_OrderedGeometryMap *pOrderedGeometryMap = new _OrderedGeometryMap;

#ifdef DEBUG_DUMP
			OutputDebugString("Partition() :: Allocating new _OrderedGeometryMap\n");
#endif
			if (useIndices)
			{
				if (pCurGeometry->indexType == IT_32)
				{
					if (FAILED(pCurGeometry->Lock((PVOID*)&chunks, &pVertsLocked, NULL, (PVOID*)&pIndicesLocked)))
					{
#ifdef DEBUG_DUMP
						OutputDebugString("Partition() :: returning due to FAILED(pCurGeometry->Lock)\n");
						depth--;
#endif
						return;
					}
				}
				else if (pCurGeometry->indexType == IT_16)
				{
					int numIndices = pCurGeometry->GetNumVertexIndices();
					pIndicesLocked = new int[numIndices];

					PVOID pIndicesLockedTemp = NULL;

					if (FAILED(pCurGeometry->Lock((PVOID*)&chunks, &pVertsLocked, NULL, &pIndicesLockedTemp)))
					{
#ifdef DEBUG_DUMP
						OutputDebugString("Partition() :: returning due to FAILED(pCurGeometry->Lock)\n");
						depth--;
#endif
						return;
					}

					for (int i = 0; i < numIndices; i++)
						pIndicesLocked[i] = (int)((short int *)pIndicesLockedTemp)[i];
				}
			}
			else
			{
				if (FAILED(pCurGeometry->Lock((PVOID*)&chunks, &pVertsLocked, NULL, NULL)))
				{
#ifdef DEBUG_DUMP
					OutputDebugString("Partition() :: returning due to FAILED(pCurGeometry->Lock)\n");
					depth--;
#endif
					return;
				}
			}

			for (unsigned int i = 0; i < (unsigned int)pCurGeometry->numChunks; i++)
			{
				//_GeometryMap *pGeometryMap = new _GeometryMap;

				Chunk &chunk = chunks[i];

				_GeometryMap *pGeometryMap = pOrderedGeometryMap->GetPtr(chunk.order);

#ifdef DEBUG_DUMP
				char buf[256];
				sprintf(buf, "Partitioning on order = %d", chunk.order);
				OutputDebugString(buf);
#endif

				// TODO handle failure

				_ChunkIndexList *pChunkIndexList = pGeometryMap->GetPtr(pCurGeometry);

				// TODO handle failure

				_VertexIndexList *pVertexIndexList = pChunkIndexList->GetPtr(i);

				// TODO handle failure

				if (chunk.type == CT_POLYGON)
				{
					for (unsigned int t = 0; t < chunk.numVerts - 2; t++)
					{
						Vector3 vertices[3];

						if (useIndices)
						{
							vertices[0] = (*g)->GetVertexPosition(pVertsLocked, pIndicesLocked[chunk.startIndex]);
							vertices[1] = (*g)->GetVertexPosition(pVertsLocked, pIndicesLocked[chunk.startIndex + t + 1]);
							vertices[2] = (*g)->GetVertexPosition(pVertsLocked, pIndicesLocked[chunk.startIndex + t + 2]);
						}
						else
						{
							vertices[0] = (*g)->GetVertexPosition(pVertsLocked, chunk.startIndex);
							vertices[1] = (*g)->GetVertexPosition(pVertsLocked, chunk.startIndex + t + 1);
							vertices[2] = (*g)->GetVertexPosition(pVertsLocked, chunk.startIndex + t + 2);
						}

						Vector3 normal = Math::Normalise(Math::Cross(vertices[1] - vertices[0], vertices[2] - vertices[0]));

						if (Math::AABBPolygonCollision(Math::AABB(node->min, node->max), 
													   Math::Polygon(normal, vertices, 3), NULL))
						{
							if (m_pSettings->optimiseChunks)
							{
								numPolygons++;

								if (useIndices)
								{
									// [0]
									pVertexIndexList->PushBack(pIndicesLocked[chunk.startIndex]);
									pVertexIndexList->PushBack(pIndicesLocked[chunk.startIndex + t + 1]);
									pVertexIndexList->PushBack(pIndicesLocked[chunk.startIndex + t + 2]);
								}
								else
								{
									// [0]
									pVertexIndexList->PushBack(chunk.startIndex);
									pVertexIndexList->PushBack(chunk.startIndex + t + 1);
									pVertexIndexList->PushBack(chunk.startIndex + t + 2);
								}
							}
							else
							{
								numPolygons += chunk.numVerts - 2;
								break;
							}
						}
					}
				}
				else if (chunk.type == CT_TRIANGLELIST)
				{
					// for each triangle in the chunk
					for (unsigned int p = 0, t = 0; p < chunk.numVerts / 3; p++, t += 3)
					{
						Vector3 vertices[3];

						// get each set of 3 triangle verts for testing
						if (useIndices)
						{
							for (int v = 0; v < 3; v++)
							{
								vertices[v] = (*g)->GetVertexPosition(pVertsLocked, pIndicesLocked[chunk.startIndex + t + v]);
							}
						}
						else
						{
							for (int v = 0; v < 3; v++)
							{
								vertices[v] = (*g)->GetVertexPosition(pVertsLocked, chunk.startIndex + t + v);
							}
						}

						// calc the normal for the triangle and test to see if it is in the bounds of this node
						Vector3 normal = Math::Normalise(Math::Cross(vertices[1] - vertices[0], vertices[2] - vertices[0]));
						if (Math::AABBPolygonCollision(Math::AABB(node->min, node->max), 
													   Math::Polygon(normal, vertices, 3), NULL))
						{
							if (m_pSettings->optimiseChunks)
							{
								numPolygons++;

								if (useIndices)
								{
									// [0]
									pVertexIndexList->PushBack(pIndicesLocked[chunk.startIndex + t]);
									pVertexIndexList->PushBack(pIndicesLocked[chunk.startIndex + t + 1]);
									pVertexIndexList->PushBack(pIndicesLocked[chunk.startIndex + t + 2]);
								}
								else
								{
									// [0]
									pVertexIndexList->PushBack(chunk.startIndex + t);
									pVertexIndexList->PushBack(chunk.startIndex + t + 1);
									pVertexIndexList->PushBack(chunk.startIndex + t + 2);
								}
							}
							else
							{
								numPolygons += chunk.numVerts / 3;
								break;
							}
						}
					}
				}
				else if (chunk.type == CT_TRIANGLESTRIP)
				{
					for (unsigned int p = 2; p < chunk.numVerts; p++)
					{
						Vector3 vertices[3];

						if (useIndices)
						{
							for (int v = 0; v < 3; v++)
							{
								vertices[v] = (*g)->GetVertexPosition(pVertsLocked, pIndicesLocked[chunk.startIndex + p - v]);
							}
						}
						else
						{
							for (int v = 0; v < 3; v++)
							{
								vertices[v] = (*g)->GetVertexPosition(pVertsLocked, chunk.startIndex + p - v);
							}
						}

						Vector3 normal = Math::Normalise(Math::Cross(vertices[1] - vertices[0], vertices[2] - vertices[0]));
						if (Math::AABBPolygonCollision(Math::AABB(node->min, node->max), Math::Polygon(normal, vertices, 3), NULL))
						{
							if (m_pSettings->optimiseChunks)
							{
								numPolygons++;

								if (useIndices)
								{
									// [0]
									pVertexIndexList->PushBack(pIndicesLocked[chunk.startIndex + p]);
									pVertexIndexList->PushBack(pIndicesLocked[chunk.startIndex + p - 1]);
									pVertexIndexList->PushBack(pIndicesLocked[chunk.startIndex + p - 2]);
								}
								else
								{
									// [0]
									pVertexIndexList->PushBack(chunk.startIndex + p);
									pVertexIndexList->PushBack(chunk.startIndex + p - 1);
									pVertexIndexList->PushBack(chunk.startIndex + p - 2);
								}
							}
							else
							{
								numPolygons += chunk.numVerts - 2;
								break;
							}
						}
					}
				}
				else if (chunk.type == CT_LINELIST)
				{
					for (unsigned int p = 0, t = 0; p < chunk.numVerts / 2; p++, t += 2)
					{
						Vector3 vertices[2];

						if (useIndices)
						{
							PVOID pVertsLockedIndex = 0;
							for (int v = 0; v < 2; v++)
							{
								vertices[v] = (*g)->GetVertexPosition(pVertsLocked, pIndicesLocked[chunk.startIndex + t + v]);
							}
						}
						else
						{
							for (int v = 0; v < 2; v++)
							{
								vertices[v] = (*g)->GetVertexPosition(pVertsLocked, chunk.startIndex + t + v);
							}
						}

						// HACK: AABBPolygonCollision is inefficient for lines
						Vector3 normal = Math::Normalise(Math::Cross(vertices[1] - vertices[0], vertices[1] - vertices[0]));
						if (Math::AABBPolygonCollision(Math::AABB(node->min, node->max), Math::Polygon(normal, vertices, 2), NULL))
						{
							if (m_pSettings->optimiseChunks)
							{
								numPolygons++;

								if (useIndices)
								{
									// [1]
									pVertexIndexList->PushBack(pIndicesLocked[chunk.startIndex + t]);
									pVertexIndexList->PushBack(pIndicesLocked[chunk.startIndex + t + 1]);
								}
								else
								{
									// [1]
									pVertexIndexList->PushBack(chunk.startIndex + t);
									pVertexIndexList->PushBack(chunk.startIndex + t + 1);
								}
							}
							else
							{
								numPolygons += chunk.numVerts - 2;

								break;
							}
						}
					}
				}
				else if (chunk.type == CT_LINESTRIP)
				{
					bool inLeaf = false;

					for (unsigned int p = 1; p < chunk.numVerts; p++)
					{
						Vector3 vertices[2];

						if (useIndices)
						{
							PVOID pVertsLockedIndex = 0;
							for (int v = 0; v < 2; v++)
							{
								vertices[v] = (*g)->GetVertexPosition(pVertsLocked, pIndicesLocked[chunk.startIndex + p - v]);
							}
						}
						else
						{
							PVOID pVertsLockedIndex = 0;
							for (int v = 0; v < 2; v++)
							{
								vertices[v] = (*g)->GetVertexPosition(pVertsLocked, chunk.startIndex + p - v);
							}
						}

						// HACK: AABBPolygonCollision is inefficient for lines
						Vector3 normal = Math::Normalise(Math::Cross(vertices[1] - vertices[0], vertices[1] - vertices[0]));
						if (Math::AABBPolygonCollision(Math::AABB(node->min, node->max), Math::Polygon(normal, vertices, 2), NULL))
						{
							if (m_pSettings->optimiseChunks)
							{
								numPolygons++;

								if (useIndices)
								{
									// [1]
									pVertexIndexList->PushBack(pIndicesLocked[chunk.startIndex + p]);
									pVertexIndexList->PushBack(pIndicesLocked[chunk.startIndex + p - 1]);
								}
								else
								{
									// [1]
									pVertexIndexList->PushBack(chunk.startIndex + p);
									pVertexIndexList->PushBack(chunk.startIndex + p - 1);
								}
							}
							else
							{
								numPolygons += chunk.numVerts - 1;
								break;
							}
						}
					}
				}
				else if (chunk.type == CT_POINTLIST)
				{
					bool inLeaf = false;

					for (unsigned int p = 0; p < chunk.numVerts; p++)
					{
						Vector3 vertex;

						if (useIndices)
						{
							PVOID pVertsLockedIndex = 0;
							vertex = (*g)->GetVertexPosition(pVertsLocked, pIndicesLocked[chunk.startIndex + p]);
						}
						else
						{
							vertex = (*g)->GetVertexPosition(pVertsLocked, chunk.startIndex + p);
						}

						if (Math::AABBPointCollision(Math::AABB(node->min, node->max), vertex))
						{
							if (m_pSettings->optimiseChunks)
							{
								numPolygons++;

								// [2]
								if (useIndices)
									pVertexIndexList->PushBack(pIndicesLocked[chunk.startIndex + p]);
								else
									pVertexIndexList->PushBack(chunk.startIndex + p);
							}
							else
							{
								numPolygons += chunk.numVerts;
								break;
							}
						}
					}
				}
			} // end for (unsigned int i = 0; i < (*g)->numChunks; i++)

			//map.push_back(orderedGeometryMap);
			pNodeGeometryChunkMap->m.push_back(pOrderedGeometryMap);

			(*g)->Unlock();

			if (useIndices && (*g)->indexType == IT_16)
			{
				delete[] pIndicesLocked;
				pIndicesLocked = NULL;
			}
		}

		bool subdivideSize = false;
		bool subdividePoly = false;
		bool subdivideSub  = false;

		if (m_pSettings->sizeDepth > 0)
		{
			if (m_pSettings->spacePartitionAxis == SS_AXIS_X)
			{
				if (m_pSettings->sizeDepth < y || m_pSettings->sizeDepth < z || (m_pSettings->spacePartitionMode == SS_OCTTREE && m_pSettings->sizeDepth < x))
					subdivideSize = true;
			}
			else if (m_pSettings->spacePartitionAxis == SS_AXIS_Y)
			{
				if (m_pSettings->sizeDepth < x || m_pSettings->sizeDepth < z || (m_pSettings->spacePartitionMode == SS_OCTTREE && m_pSettings->sizeDepth < y))
					subdivideSize = true;
			}
			else if (m_pSettings->spacePartitionAxis == SS_AXIS_Z)
			{
				if (m_pSettings->sizeDepth < x || m_pSettings->sizeDepth < y || (m_pSettings->spacePartitionMode == SS_OCTTREE && m_pSettings->sizeDepth < z))
					subdivideSize = true;
			}
		}
		else
			subdivideSize = true;

		if (m_pSettings->polygonDepth > 0)
		{
			if (m_pSettings->polygonDepth < numPolygons)
				subdividePoly = true;
		}
		else
			subdividePoly = true;

		if (m_pSettings->subdivisionDepth > 0)
		{
			if (numDivisions < m_pSettings->subdivisionDepth)
				subdivideSub = true;
		}
		else
			subdivideSub = true;

		if (subdivideSize && subdividePoly && subdivideSub && (m_pSettings->polygonDepth || m_pSettings->sizeDepth || m_pSettings->subdivisionDepth))
		{
			int childNumDivisions = numDivisions + 1;

			delete pNodeGeometryChunkMap;
			pNodeGeometryChunkMap = NULL;

			Vector3 mid(0, 0, 0);

			if (m_pSettings->spacePartitionMode == SS_QUADTREE)
			{
				node->children = new TreeNode *[4];
				node->numChildren = 4;

				if (m_pSettings->spacePartitionAxis == SS_AXIS_X)
					mid = min + Vector3(x, y * 0.5f, z * 0.5f);
				else if (m_pSettings->spacePartitionAxis == SS_AXIS_Y)
					mid = min + Vector3(x * 0.5f, y, z * 0.5f);
				else if (m_pSettings->spacePartitionAxis == SS_AXIS_Z)
					mid = min + Vector3(x * 0.5f, y * 0.5f, z);
			}
			else if (m_pSettings->spacePartitionMode == SS_OCTTREE)
			{
				node->children = new TreeNode *[8];
				node->numChildren = 8;
				mid = min + Vector3(x * 0.5f, y * 0.5f, z * 0.5f);
			}

			if (m_pSettings->spacePartitionMode == SS_QUADTREE)
			{
				if (m_pSettings->spacePartitionAxis == SS_AXIS_X)
				{
					node->children[0] = new TreeNode(min, mid, node, m_numNodes);
					m_numNodes++;
					Partition(node->children[0], childNumDivisions);

					node->children[1] = new TreeNode(Vector3(min.x, mid.y, min.z), Vector3(mid.x, max.y, mid.z), node, m_numNodes);
					m_numNodes++;
					Partition(node->children[1], childNumDivisions);

					node->children[2] = new TreeNode(Vector3(min.x, min.y, mid.z), Vector3(mid.x, mid.y, max.z), node, m_numNodes);
					m_numNodes++;
					Partition(node->children[2], childNumDivisions);

					node->children[3] = new TreeNode(Vector3(min.x, mid.y, mid.z), Vector3(mid.x, max.y, max.z), node, m_numNodes);
					m_numNodes++;
					Partition(node->children[3], childNumDivisions);
				}
				else if (m_pSettings->spacePartitionAxis == SS_AXIS_Y)
				{
					node->children[0] = new TreeNode(min, mid, node, m_numNodes);
					m_numNodes++;
					Partition(node->children[0], childNumDivisions);

					node->children[1] = new TreeNode(Vector3(mid.x, min.y, min.z), Vector3(max.x, mid.y, mid.z), node, m_numNodes);
					m_numNodes++;
					Partition(node->children[1], childNumDivisions);

					node->children[2] = new TreeNode(Vector3(min.x, min.y, mid.z), Vector3(mid.x, mid.y, max.z), node, m_numNodes);
					m_numNodes++;
					Partition(node->children[2], childNumDivisions);

					node->children[3] = new TreeNode(Vector3(mid.x, min.y, mid.z), Vector3(max.x, mid.y, max.z), node, m_numNodes);
					m_numNodes++;
					Partition(node->children[3], childNumDivisions);
				}
				else if (m_pSettings->spacePartitionAxis == SS_AXIS_Z)
				{
					node->children[0] = new TreeNode(min, mid, node, m_numNodes);
					m_numNodes++;
					Partition(node->children[0], childNumDivisions);

					node->children[1] = new TreeNode(Vector3(mid.x, min.y, min.z), Vector3(max.x, mid.y, mid.z), node, m_numNodes);
					m_numNodes++;
					Partition(node->children[1], childNumDivisions);

					node->children[2] = new TreeNode(Vector3(min.x, mid.y, min.z), Vector3(mid.x, max.y, mid.z), node, m_numNodes);
					m_numNodes++;
					Partition(node->children[2], childNumDivisions);

					node->children[3] = new TreeNode(Vector3(mid.x, mid.y, min.z), Vector3(max.x, max.y, mid.z), node, m_numNodes);
					m_numNodes++;
					Partition(node->children[3], childNumDivisions);
				}
			}
			else
			{
				node->children[0] = new TreeNode(min, mid, node, m_numNodes);
				m_numNodes++;
				Partition(node->children[0], childNumDivisions);

				node->children[1] = new TreeNode(Vector3(mid.x, min.y, min.z), Vector3(max.x, mid.y, mid.z), node, m_numNodes);
				m_numNodes++;
				Partition(node->children[1], childNumDivisions);

				node->children[2] = new TreeNode(Vector3(min.x, min.y, mid.z), Vector3(mid.x, mid.y, max.z), node, m_numNodes);
				m_numNodes++;
				Partition(node->children[2], childNumDivisions);

				node->children[3] = new TreeNode(Vector3(mid.x, min.y, mid.z), Vector3(max.x, mid.y, max.z), node, m_numNodes);
				m_numNodes++;
				Partition(node->children[3], childNumDivisions);

				node->children[4] = new TreeNode(Vector3(min.x, mid.y, min.z), Vector3(mid.x, max.y, mid.z), node, m_numNodes);
				m_numNodes++;
				Partition(node->children[4], childNumDivisions);

				node->children[5] = new TreeNode(Vector3(mid.x, mid.y, min.z), Vector3(max.x, max.y, mid.z), node, m_numNodes);
				m_numNodes++;
				Partition(node->children[5], childNumDivisions);

				node->children[6] = new TreeNode(Vector3(min.x, mid.y, mid.z), Vector3(mid.x, max.y, max.z), node, m_numNodes);
				m_numNodes++;
				Partition(node->children[6], childNumDivisions);

				node->children[7] = new TreeNode(Vector3(mid.x, mid.y, mid.z), Vector3(max.x, max.y, max.z), node, m_numNodes);
				m_numNodes++;
				Partition(node->children[7], childNumDivisions);
			}
		}
		else
		{
			node->leafIndex = m_nodeChunkMap.size();
			m_nodeChunkMap.push_back(pNodeGeometryChunkMap);

			if (node->leafIndex > -1)
			{
				float nodeVolume = (node->max.x - node->min.x) * (node->max.y - node->min.y) * (node->max.z - node->min.z);
				float rootVolume = (m_pRoot->max.x - m_pRoot->min.x) * (m_pRoot->max.y - m_pRoot->min.y) * (m_pRoot->max.z - m_pRoot->min.z);

				if (rootVolume > 0)
				{
					spacePartitionProgress += (nodeVolume / rootVolume) * 100.0f;

					if (progressCallback)
						progressCallback(PC_SCENE_SPACEPARTITIONING, spacePartitionProgress, pProgressCallbackPayload);
				}
			}
		}
	}
#endif


//-----------------------------------------------------------------------
	void Scene::GenerateLeaf(TreeNode *pNode, _NodeChunkMap *pNodeChunkMap)
	//-------------------------------------------------------------------
	{
		if (!pNode)
			return;

		int numEffectGroups = m_pResourceManager->GetNumEffects();

		TreeLeaf *pLeaf = new TreeLeaf(pNode);
		pLeaf->m_min = pNode->m_min;
		pLeaf->m_max = pNode->m_max;

		pNode->m_leafIndex = m_leafs.size();

		m_leafs.push_back(pLeaf);
		AddUsedMemory(sizeof(TreeLeaf), "Scene::GenerateLeafs() - TreeLeaf...");

		// if pNodeChunkMap is NULL, the leaf is created, but empty.
		if (!pNodeChunkMap)
			return;

#ifdef VERBOSE_PARTITIONING
		DebugPrintf(_T("\n"));
		DumpNodeChunkMap((const _NodeChunkMap *)pNodeChunkMap, pNode->m_id);
		DebugPrintf(_T("\n"));
#endif

		NodeChunkMap &ncm = pNodeChunkMap->m;

		NodeChunkMap::iterator i = ncm.begin();
		for (; i != ncm.end(); ++i)
		{
			if ((*i)->m.empty())
				continue;
			
			//DebugPrintf("NodeChunkMap size() = %d\n", (*i)->m.size());

			OrderedGeometryMap &orderedGeomMapRef = (*i)->m;
			OrderedGeometryMap::iterator j = orderedGeomMapRef.begin();
			for (; j != orderedGeomMapRef.end(); ++j)
			{
				GeometryMap &gm = j->second->m;

				//DebugPrintf("\tGeometryMap size() = %d\n", gm);

				GeometryMap::iterator k = gm.begin();
				for (; k != gm.end(); ++k)
				{
					PGEOMETRY pGeometry = k->first;

					if (k->second->m.empty() || pGeometry->GetNumChunks() < 1 || pGeometry->GetNumVertices() < 1)
						continue;

					OptimisedGeometryMap::iterator og = m_optimisedGeometry.find(pGeometry);
					POPTIMISEDGEOMETRY pOptimisedGeometry = NULL;

					if (og == m_optimisedGeometry.end())
					{
						//DebugPrintf("\t\tOptimisedGeometry created.\n");
						pOptimisedGeometry = new OptimisedGeometry(pGeometry);
						m_optimisedGeometry[pGeometry] = pOptimisedGeometry;
					}
					else
					{
						//DebugPrintf("\t\tOptimisedGeometry retrieved.\n");
						pOptimisedGeometry = og->second;
					}

					std::vector< std::vector<int> > effectGroups;
					effectGroups.resize(numEffectGroups + 1);

					ChunkIndexList &cil = k->second->m;

					//DebugPrintf("\t\t\tChunkIndexList size() = %d.\n", cil.size());

					ChunkIndexList::iterator c = cil.begin();
					for (; c != cil.end(); ++c)
					{
						if (c->second->m.empty())
							continue;

						unsigned int srcChunkIndex = c->first;
						const Chunk &srcChunk = pGeometry->GetChunk(srcChunkIndex);

						//DebugPrintf("\t\t\t\tsrcChunkIndex = %d.\n", srcChunkIndex);

						IndexSet &is = c->second->m;

						if (is.empty())
							continue;

						IndexSet::iterator s = is.begin();
						for (; s != is.end(); ++s)
						{
							unsigned int chunkIndex = (unsigned int)pOptimisedGeometry->m_chunks.size();
							effectGroups[srcChunk.effect + 1].push_back(chunkIndex);

							Chunk newChunk(srcChunk);
							newChunk.startIndex = pOptimisedGeometry->m_numVertexIndices;
							newChunk.numIndices = 0;

							VertexIndexList &vil = (*s)->m;

							//DebugPrintf("Creating new chunk of type %s...\n", CHUNKTYPENAMES[newChunk.type]);

							for (unsigned int p = 0; p < vil.size(); p++)
							{
								pOptimisedGeometry->InsertVertexIndex(chunkIndex, vil[p]);
								++newChunk.numIndices;
							}

							pOptimisedGeometry->m_chunks.push_back(newChunk);
						}

						//DebugPrintf("\t\t\t\tIndexSet size() = %d.\n", is.size());

						/*switch (srcChunk.type)
						{
						case CT_TRIANGLELIST:
						case CT_LINELIST:
						case CT_POINTLIST:
							{
								unsigned int chunkIndex = (unsigned int)pOptimisedGeometry->m_chunks.size();
								effectGroups[srcChunk.effect + 1].push_back(chunkIndex);

								Chunk newChunk(srcChunk);
								newChunk.startIndex = pOptimisedGeometry->m_numVertexIndices;
								newChunk.numIndices = 0;

								VertexIndexList &vil = (*is.begin())->m;

								//DebugPrintf("Creating new chunk of type %s...\n", CHUNKTYPENAMES[newChunk.type]);

								for (unsigned int p = 0; p < vil.size(); p++)
								{
									pOptimisedGeometry->InsertVertexIndex(chunkIndex, vil[p]);
									++newChunk.numIndices;
								}

								pOptimisedGeometry->m_chunks.push_back(newChunk);
							}
							break;

						case CT_TRIANGLESTRIP:
						case CT_LINESTRIP:
						case CT_POLYGON:
							{
								IndexSet::iterator s = is.begin();
								for (; s != is.end(); ++s)
								{
									unsigned int chunkIndex = (unsigned int)pOptimisedGeometry->m_chunks.size();
									effectGroups[srcChunk.effect + 1].push_back(chunkIndex);

									Chunk newChunk(srcChunk);
									newChunk.startIndex = pOptimisedGeometry->m_numVertexIndices;
									newChunk.numIndices = 0;

									VertexIndexList &vil = (*s)->m;

									//DebugPrintf("Creating new chunk of type %s...\n", CHUNKTYPENAMES[newChunk.type]);

									for (unsigned int p = 0; p < vil.size(); p++)
									{
										pOptimisedGeometry->InsertVertexIndex(chunkIndex, vil[p]);
										++newChunk.numIndices;
									}

									pOptimisedGeometry->m_chunks.push_back(newChunk);
								}
							}
							break;
						}*/
/*
						// this isn't quite right; lists (as opposed to strips and polygon) can just use the first element.
						IndexSet::iterator s = is.begin();
						for (; s != is.end(); ++s)
						{
		                    unsigned int chunkIndex = (unsigned int)pOptimisedGeometry->m_chunks.size();
							effectGroups[srcChunk.effect + 1].push_back(chunkIndex);

							Chunk newChunk(srcChunk);
							newChunk.startIndex = pOptimisedGeometry->m_numVertexIndices;
							newChunk.numIndices = 0;

							VertexIndexList &vil = (*s)->m;

							//DebugPrintf("\t\t\t\t\tVertexIndexList size() = %d.\n", vil.size());
							//DebugPrintf("\t\t\t\t\t\t");

							DebugPrintf("Creating new chunk of type %s...\n", CHUNKTYPENAMES[newChunk.type]);

							switch (newChunk.type)
							{
							case CT_TRIANGLELIST:
							case CT_LINELIST:
							case CT_POINTLIST:
								{
									for (unsigned int p = 0; p < vil.size(); p++)
									{
										pOptimisedGeometry->InsertVertexIndex(chunkIndex, vil[p]);
										++newChunk.numIndices;
									}
								}
								break;

							case CT_TRIANGLESTRIP:
							case CT_LINESTRIP:
								{
									for (unsigned int p = 0; p < vil.size(); p++)
									{
										pOptimisedGeometry->InsertVertexIndex(chunkIndex, vil[p]);
										++newChunk.numIndices;
									}
								}
								break;

							case CT_POLYGON:
								{

								}
								break;
							}

							if (newChunk.numIndices < 1)
								DebugPrintf("new chunk has 0 indices!\n");

							//DebugPrintf("\n", is.size());

							pOptimisedGeometry->m_chunks.push_back(newChunk);
						}*/
					}

					/*ChunkIndexList &cil = k->second->m;
					ChunkIndexList::iterator c = cil.begin();
					for (; c != cil.end(); c++)
					{
						if (c->second->m.empty())
							continue;

						unsigned int srcChunkIndex = c->first;
						const Chunk &srcChunk = pGeometry->GetChunk(srcChunkIndex);

                        unsigned int chunkIndex = (unsigned int)pOptimisedGeometry->m_chunks.size();
						effectGroups[srcChunk.effect + 1].push_back(chunkIndex);

						Chunk newChunk(srcChunk);
						newChunk.startIndex = pOptimisedGeometry->m_numVertexIndices;
						newChunk.numIndices = 0;
						newChunk.type = CT_TRIANGLELIST;

						VertexIndexList &vil = c->second->m;
						VertexIndexList::iterator v = c->second->m.begin();
						for (; v != vil.end(); v++)
						{
							pOptimisedGeometry->InsertVertexIndex(chunkIndex, *v);
							++newChunk.numIndices;
						}

						pOptimisedGeometry->m_chunks.push_back(newChunk);
					}*/

					std::vector< std::vector<int> >::iterator e = effectGroups.begin();
					for (int effectId = -1; e != effectGroups.end(); ++e, ++effectId)
					{
						if (e->empty())
							continue;

						RenderGroup *pRenderGroup = new RenderGroup(this);
						pRenderGroup->m_order = j->first;
						pRenderGroup->m_pGeometry = NULL;
						pRenderGroup->m_pOptimisedGeometry = pOptimisedGeometry;
						pRenderGroup->m_effectID = effectId;
						pRenderGroup->m_techniqueID = -1;
						pRenderGroup->m_hasTransform = false;
						pRenderGroup->m_materialID = -1;
						pRenderGroup->m_startFaceIndex = *e->begin();
						pRenderGroup->m_numFaces = e->size();

						pLeaf->m_renderList.push_back(pRenderGroup);

						for (unsigned int m = 0; m < pRenderGroup->m_numFaces; m++)
						{
							pOptimisedGeometry->InsertChunkIndex(0, effectGroups[effectId + 1][m]);
						}
					}
				}
			}
		}
	}

#if 0
//-----------------------------------------------------------------------
	void Scene::GenerateLeaf(TreeNode *pNode, _NodeChunkMap *pNodeChunkMap)
	//-------------------------------------------------------------------
	{
		if (!pNode)
			return;

		int numEffectGroups = m_pResourceManager->GetNumEffects();

		TreeLeaf *pLeaf = new TreeLeaf(pNode);
		pLeaf->m_min = pNode->m_min;
		pLeaf->m_max = pNode->m_max;

		pNode->m_leafIndex = m_leafs.size();

		m_leafs.push_back(pLeaf);
		AddUsedMemory(sizeof(TreeLeaf), "Scene::GenerateLeafs() - TreeLeaf...");

		// if pNodeChunkMap is NULL, the leaf is created, but empty.
		if (!pNodeChunkMap)
			return;

		NodeChunkMap &ncm = pNodeChunkMap->m;

		NodeChunkMap::iterator i = ncm.begin();
		for (; i != ncm.end(); i++)
		{
			if ((*i)->m.empty())
				continue;

			OrderedGeometryMap &orderedGeomMapRef = (*i)->m;
			OrderedGeometryMap::iterator j = orderedGeomMapRef.begin();
			for (; j != orderedGeomMapRef.end(); j++)
			{
				GeometryMap &gm = j->second->m;
				GeometryMap::iterator k = gm.begin();
				for (; k != gm.end(); k++)
				{
					PGEOMETRY pGeometry = k->first;

					if (k->second->m.empty() || pGeometry->GetNumChunks() < 1 || pGeometry->GetNumVertices() < 1)
						continue;

					// see if pGeometry has an OptimisedGeometry instance in a map
					// if not, create one. If so, use it.

					// Let og be the OptimisedGeometry instance.

					// for each chunk c in k
						// create a new chunk cn and add to og
						// add a chunk index to og
						// for each vertex v in c
							// add a vertex index to og

					// create a RenderGroup rg and add it to leaf

					// ---

					OptimisedGeometryMap::iterator og = m_optimisedGeometry.find(pGeometry);
					POPTIMISEDGEOMETRY pOptimisedGeometry = NULL;

					if (og == m_optimisedGeometry.end())
					{
						pOptimisedGeometry = new OptimisedGeometry(pGeometry);
						m_optimisedGeometry[pGeometry] = pOptimisedGeometry;
					}
					else
					{
						pOptimisedGeometry = og->second;
					}

					std::vector< std::vector<int> > effectGroups;
					effectGroups.resize(numEffectGroups + 1);

					ChunkIndexList &cil = k->second->m;
					ChunkIndexList::iterator c = cil.begin();
					for (; c != cil.end(); c++)
					{
						if (c->second->m.empty())
							continue;

						unsigned int srcChunkIndex = c->first;
						unsigned int chunkIndex = pOptimisedGeometry->GetNumChunks();

						const Chunk &srcChunk = pGeometry->GetChunk(srcChunkIndex);

						//pOptimisedGeometry->GetChunkIndices(srcChunkIndex)->push_back(chunkIndex);
						pOptimisedGeometry->InsertChunkIndex(srcChunkIndex, chunkIndex);

						effectGroups[srcChunk.effect + 1].push_back(chunkIndex);

						Chunk newChunk(srcChunk);
						newChunk.startIndex = pOptimisedGeometry->m_numVertexIndices;
						newChunk.numIndices = 0;
						newChunk.type = CT_TRIANGLELIST;

						VertexIndexList &vil = c->second->m;
						VertexIndexList::iterator v = c->second->m.begin();
						for (; v != vil.end(); v++)
						{
							unsigned int vertexIndex = pOptimisedGeometry->m_numVertexIndices;

							//pOptimisedGeometry->GetVertexIndices(chunkIndex)->push_back(vertexIndex);
							//++pOptimisedGeometry->m_numVertexIndices;

							pOptimisedGeometry->InsertVertexIndex(chunkIndex, vertexIndex);
							++newChunk.numIndices;
						}

						pOptimisedGeometry->m_chunks.push_back(newChunk);
					}

					std::vector<unsigned int> chunkIndices;

					std::vector< std::vector<int> >::iterator e = effectGroups.begin();
					for (int effectId = -1; e != effectGroups.end(); ++e, ++effectId)
					{
						if (e->empty())
							continue;

						RenderGroup *pRenderGroup = new RenderGroup(this);
						pRenderGroup->m_order = j->first;
						pRenderGroup->m_pGeometry = NULL;
						pRenderGroup->m_pOptimisedGeometry = pOptimisedGeometry;
						pRenderGroup->m_effectID = effectId;
						pRenderGroup->m_techniqueID = -1;
						pRenderGroup->m_hasTransform = false;
						pRenderGroup->m_materialID = -1;
						pRenderGroup->m_startFaceIndex = *e->begin();
						pRenderGroup->m_numFaces = e->size();

						pLeaf->m_renderList.push_back(pRenderGroup);

						for (unsigned int m = 0; m < pRenderGroup->m_numFaces; m++)
						{
							chunkIndices.push_back(effectGroups[effectId + 1][m]);
						}
					}

					pOptimisedGeometry->m_numChunkIndices = chunkIndices.size();
					pOptimisedGeometry->m_pChunkIndices = new unsigned int[pOptimisedGeometry->m_numChunkIndices];

					memcpy(pOptimisedGeometry->m_pChunkIndices, &chunkIndices[0], sizeof(unsigned int) * pOptimisedGeometry->m_numChunkIndices);
				}
			}
		}
	}
#endif

#if 0
//-----------------------------------------------------------------------
	void Scene::GenerateLeafs(TreeNode *node, std::map< PGEOMETRY, Chunk * > &srcChunks, std::map< PGEOMETRY, std::vector< unsigned int > > &newVertexIndices)
	//-------------------------------------------------------------------
	{
		if (!node)
			return;

		int numEffectGroups = m_pResourceManager->GetNumEffects();

		if (node->m_numChildren == 4)
		{
			GenerateLeafs(node->m_pChildren[0], srcChunks, newVertexIndices);
			GenerateLeafs(node->m_pChildren[1], srcChunks, newVertexIndices);
			GenerateLeafs(node->m_pChildren[2], srcChunks, newVertexIndices);
			GenerateLeafs(node->m_pChildren[3], srcChunks, newVertexIndices);
		}
		else if (node->m_numChildren == 8)
		{
			GenerateLeafs(node->m_pChildren[0], srcChunks, newVertexIndices);
			GenerateLeafs(node->m_pChildren[1], srcChunks, newVertexIndices);
			GenerateLeafs(node->m_pChildren[2], srcChunks, newVertexIndices);
			GenerateLeafs(node->m_pChildren[3], srcChunks, newVertexIndices);
			GenerateLeafs(node->m_pChildren[4], srcChunks, newVertexIndices);
			GenerateLeafs(node->m_pChildren[5], srcChunks, newVertexIndices);
			GenerateLeafs(node->m_pChildren[6], srcChunks, newVertexIndices);
			GenerateLeafs(node->m_pChildren[7], srcChunks, newVertexIndices);
		}
		else if (node->m_leafIndex > -1)
		{
			// Setup Leaf
				TreeLeaf *leaf = new TreeLeaf(node);
				leaf->m_min = node->m_min;
				leaf->m_max = node->m_max;

				node->m_leafIndex = m_leafs.size();

				m_leafs.push_back(leaf);
				AddUsedMemory(sizeof(TreeLeaf), "Scene::GenerateLeafs() - TreeLeaf...");

			if (m_pSettings->optimiseChunks)
			{
				DebugPrintf("GenerateLeafs(): Skipping Optimise Chunks case.");
			}
			else
			{
				NodeChunkMap &ncm = m_nodeChunkMap[node->m_leafIndex]->m;

				NodeChunkMap::iterator i = ncm.begin();
				for (; i != ncm.end(); i++)
				{
					if ((*i)->m.empty())
						continue;

					OrderedGeometryMap &orderedGeomMapRef = (*i)->m;
					OrderedGeometryMap::iterator j = orderedGeomMapRef.begin();
					for (; j != orderedGeomMapRef.end(); j++)
					{
						GeometryMap &gm = j->second->m;
						GeometryMap::iterator k = gm.begin();
						for (; k != gm.end(); k++)
						{
							PGEOMETRY pGeometry = k->first;

							if (pGeometry->GetNumChunks() < 1)
								continue;

							Chunk *pSrcChunks = srcChunks[pGeometry];
							unsigned int numChunks = pGeometry->GetNumChunks();

							if (!pSrcChunks)
							{
								Chunk *pChunksLocked = NULL;
								if (FAILED(pGeometry->Lock((void **)&pChunksLocked, NULL, NULL, NULL)))
								{
									DebugPrintf("Error: pGeometry->Lock() failed.");
									return;
								}

								pSrcChunks = new Chunk[numChunks];
								memcpy(pSrcChunks, pChunksLocked, sizeof(Chunk) * numChunks);

								srcChunks[pGeometry] = pSrcChunks;

								if (FAILED(pGeometry->Unlock()))
								{
									DebugPrintf("Error: pGeometry->Unlock() failed.");
									return;
								}

								pGeometry->Clear(GCF_CHUNKS | GCF_VERTEXINDICES);
							}

							std::vector< Chunk > newChunks;

							std::vector< std::vector<int> > effectGroups;
							effectGroups.resize(numEffectGroups + 1);

							ChunkIndexList::iterator c = k->second->m.begin();

							for (; c != k->second->m.end(); c++)
							{
								Chunk &srcChunk = pSrcChunks[c->first];

								effectGroups[srcChunk.effect + 1].push_back(pGeometry->GetNumChunks() + newChunks.size());

								Chunk newChunk(srcChunk);
								newChunk.startIndex = newVertexIndices[pGeometry].size();
								newChunk.numIndices = c->second->m.size();
								newChunks.push_back(newChunk);

								VertexIndexList::iterator v = c->second->m.begin();
								for (; v != c->second->m.end(); v++)
								{
									newVertexIndices[pGeometry].push_back(*v);
								}
							}

							pGeometry->Insert(newChunks.size(), 0, 0, 0, &newChunks[0], NULL, NULL, NULL, 0);

							//g_pSortingChunks = pSrcChunks;

							for (int e = 0; e < numEffectGroups + 1; e++)
							{
								if (effectGroups[e].size() < 1)
									continue;

								//sort(effectGroups[e].begin(), effectGroups[e].end(), SortByTechniqueID);

								RenderGroup *group = new RenderGroup(this);
								group->m_order = j->first;
								group->m_pGeometry = pGeometry;
								group->m_startFaceIndex = pGeometry->GetNumChunkIndices();
								group->m_numFaces = effectGroups[e].size();
								group->m_effectID = e - 1;
								group->m_hasTransform = false;

								leaf->m_renderList.push_back(group);

								int *chunkIndices = new int[effectGroups[e].size()];

								for (unsigned int m = 0; m < group->m_numFaces; m++)
									chunkIndices[m] = effectGroups[e][m];

								pGeometry->Insert(0, 0, effectGroups[e].size(), 0, NULL, NULL, chunkIndices, NULL, 0);

								delete[] chunkIndices;
							}

							//g_pSortingChunks = NULL;
						}
					}
				}
			}
		}
	}


#endif
#if 0
//-----------------------------------------------------------------------
	void Scene::GenerateLeafs(TreeNode *node, std::map< PGEOMETRY, Chunk * > &srcChunks)
	//-------------------------------------------------------------------
	{
		if (!node)
			return;

		int numEffectGroups = resourceManager->GetNumEffects();

		if (node->m_numChildren == 4)
		{
			GenerateLeafs(node->m_pChildren[0], srcChunks);
			GenerateLeafs(node->m_pChildren[1], srcChunks);
			GenerateLeafs(node->m_pChildren[2], srcChunks);
			GenerateLeafs(node->m_pChildren[3], srcChunks);
		}
		else if (node->m_numChildren == 8)
		{
			GenerateLeafs(node->m_pChildren[0], srcChunks);
			GenerateLeafs(node->m_pChildren[1], srcChunks);
			GenerateLeafs(node->m_pChildren[2], srcChunks);
			GenerateLeafs(node->m_pChildren[3], srcChunks);
			GenerateLeafs(node->m_pChildren[4], srcChunks);
			GenerateLeafs(node->m_pChildren[5], srcChunks);
			GenerateLeafs(node->m_pChildren[6], srcChunks);
			GenerateLeafs(node->m_pChildren[7], srcChunks);
		}
		else if (node->m_leafIndex > -1)
		{
			int chunkMapIndex = node->m_leafIndex;

			// Setup Leaf
				TreeLeaf *leaf = new TreeLeaf(node);
				leaf->m_min = node->m_min;
				leaf->m_max = node->m_max;

				node->m_leafIndex = leafs.size();

				leafs.push_back(leaf);
				AddUsedMemory(sizeof(TreeLeaf), "Scene::GenerateLeafs() - TreeLeaf...");

			if (m_pSettings->optimiseChunks)
			{
				DebugPrintf("GenerateLeafs(): Skipping Optimise Chunks case.");
			}
			else
			{
				if (chunkMapIndex >= (int)m_nodeChunkMap.size())
				{
					DebugPrintf("Error: chunkMapIndex exceeds m_nodeChunkMap.size().");
					return;
				}

				if (m_nodeChunkMap[chunkMapIndex]->m.empty())
				{
					DebugPrintf("Error: m_nodeChunkMap[chunkMapIndex] is empty.");
					return;
				}

				std::map< PGEOMETRY, std::vector< unsigned int > > newVertexIndices;

				NodeChunkMap &ncm = m_nodeChunkMap[chunkMapIndex]->m;

				NodeChunkMap::iterator i = ncm.begin();
				for (; i != ncm.end(); i++)
				{
					if ((*i)->m.empty())
						continue;

					OrderedGeometryMap &orderedGeomMapRef = (*i)->m;
					OrderedGeometryMap::iterator j = orderedGeomMapRef.begin();
					for (; j != orderedGeomMapRef.end(); j++)
					{
						int order = j->first;

						GeometryMap &gm = j->second->m;
						GeometryMap::iterator k = gm.begin();
						for (; k != gm.end(); k++)
						{
							PGEOMETRY pGeometry = k->first;

							if (pGeometry->GetNumChunks() < 1 || pGeometry->GetNumVertices() < 1)
								continue;

							std::vector< std::vector<int> > effectGroups;
							effectGroups.resize(numEffectGroups + 1);

							Chunk *pSrcChunks = srcChunks[pGeometry];

							if (!pSrcChunks)
							{
								unsigned int numChunks = pGeometry->GetNumChunks();
								Chunk *pChunksLocked = NULL;
								if (FAILED(pGeometry->Lock((void **)&pChunksLocked, NULL, NULL, NULL)))
								{
									DebugPrintf("Error: pGeometry->Lock() failed.");
									return;
								}

								pSrcChunks = new Chunk[numChunks];
								memcpy(pSrcChunks, pChunksLocked, sizeof(Chunk) * numChunks);

								srcChunks[pGeometry] = pSrcChunks;

								if (FAILED(pGeometry->Unlock()))
								{
									DebugPrintf("Error: pGeometry->Unlock() failed.");
									return;
								}

								pGeometry->Clear(GCF_CHUNKS);
							}

							std::vector< Chunk > newChunks;

							ChunkIndexList::iterator c = k->second->m.begin();

							for (; c != k->second->m.end(); c++)
							{
								//if (c->second->m.empty())
								//	continue;

								if (c->first >= pGeometry->GetNumChunks())
									continue;

								const Chunk &srcChunk = pSrcChunks[c->first];

								effectGroups[srcChunk.effect + 1].push_back(pGeometry->GetNumChunks() + newChunks.size());

								newChunks.push_back(srcChunk);

								/*Chunk newChunk(srcChunk);
								newChunk.startIndex = newVertexIndices[pGeometry].size();
								newChunk.numIndices = c->second->m.size();
								newChunks.push_back(newChunk);*/

								/*VertexIndexList::iterator v = c->second->m.begin();
								for (; v != c->second->m.end(); v++)
								{
									newVertexIndices[pGeometry].push_back(*v);
								}*/
							}

							Chunk *pChunks = (Chunk *)&newChunks[0];

							g_pSortingChunks = pChunks;

							for (int e = 0; e < numEffectGroups + 1; e++)
							{
								if (effectGroups[e].size() < 1)
									continue;

								sort(effectGroups[e].begin(), effectGroups[e].end(), SortByTechniqueID);

								RenderGroup *group = new RenderGroup(this);
								group->m_order = j->first;
								group->m_pGeometry = pGeometry;
								group->m_startFaceIndex = pGeometry->GetNumChunkIndices();
								group->m_numFaces = effectGroups[e].size();
								group->m_effectID = e - 1;
								group->m_hasTransform = false;

								leaf->m_renderList.push_back(group);

								int *chunkIndices = new int[effectGroups[e].size()];

								for (unsigned int m = 0; m < group->m_numFaces; m++)
									chunkIndices[m] = effectGroups[e][m];

								pGeometry->Insert(0, 0, effectGroups[e].size(), 0, 0, NULL, NULL, chunkIndices, NULL, NULL, 0);

								delete[] chunkIndices;
							}

							g_pSortingChunks = NULL;

							pGeometry->Insert(newChunks.size(), 0, 0, 0, 0, &newChunks[0], NULL, NULL, NULL, NULL, 0);
						}
					}
				}

				/*std::map< PGEOMETRY, std::vector< unsigned int > >::iterator nviItr = newVertexIndices.begin();
				for (; nviItr != newVertexIndices.end(); nviItr++)
				{
					PGEOMETRY pGeometry = nviItr->first;

					if (!pGeometry)
						continue;

					if (pGeometry->GetNumVertexIndices() > 0 && 0) // Not ready yet
					{
						void *pSourceVertIndices = NULL;
						pGeometry->Lock(NULL, NULL, NULL, (void **)&pSourceVertIndices, NULL, 0);

						unsigned int numSourceVertexIndices = pGeometry->GetNumVertexIndices();
						size_t sourceVertIndicesSize = numSourceVertexIndices * pGeometry->GetIndexSize();

						void *pTempSourceVertIndices = malloc(sourceVertIndicesSize);
						memcpy(pTempSourceVertIndices, pSourceVertIndices, sourceVertIndicesSize);

						pGeometry->Clear(GCF_VERTEXINDICES);

						pGeometry->Insert(0, 0, 0, nviItr->second.size(), numSourceVertexIndices,
										  NULL, NULL, NULL, (void *)&nviItr->second[0], pTempSourceVertIndices, 0);

						free(pTempSourceVertIndices);
					}
					else
					{
						pGeometry->Clear(GCF_VERTEXINDICES);

						pGeometry->Insert(0, 0, 0, nviItr->second.size(), 0, NULL, NULL, NULL, (void *)&nviItr->second[0], NULL, 0);
					}
				}*/
			}
		}
	}


#endif


//-----------------------------------------------------------------------
	void Scene::UpdateSpacePartitionProgress(TreeNode *node)
	//-------------------------------------------------------------------
	{
		if (!node || !m_pRoot)
			return;

		if (node->m_leafIndex > -1)
		{
			float nodeVolume = (node->m_max.x - node->m_min.x) * (node->m_max.y - node->m_min.y) * (node->m_max.z - node->m_min.z);
			float rootVolume = (m_pRoot->m_max.x - m_pRoot->m_min.x) * (m_pRoot->m_max.y - m_pRoot->m_min.y) * (m_pRoot->m_max.z - m_pRoot->m_min.z);

			if (rootVolume > 0)
			{
				m_spacePartitionProgress += (nodeVolume / rootVolume) * 100.0f;

				if (m_pFnProgressCallback)
					m_pFnProgressCallback(PC_SCENE_SPACEPARTITIONING, m_spacePartitionProgress, m_pProgressCallbackPayload);
			}
		}
	}


//-----------------------------------------------------------------------
	void Scene::UpdateOptimiseChunksProgress(TreeNode *node)
	//-------------------------------------------------------------------
	{
		if (!node || !m_pRoot)
			return;

		if (node->m_leafIndex > -1 && m_pSettings->optimiseChunks)
		{
			float nodeVolume = (node->m_max.x - node->m_min.x) * (node->m_max.y - node->m_min.y) * (node->m_max.z - node->m_min.z);
			float rootVolume = (m_pRoot->m_max.x - m_pRoot->m_min.x) * (m_pRoot->m_max.y - m_pRoot->m_min.y) * (m_pRoot->m_max.z - m_pRoot->m_min.z);

			if (rootVolume > 0)
			{
				m_optimiseChunksProgress += (nodeVolume / rootVolume) * 100.0f;

				if (m_pFnProgressCallback)
					m_pFnProgressCallback(PC_SCENE_OPTIMISECHUNKS, m_optimiseChunksProgress, m_pProgressCallbackPayload);
			}
		}
	}


//-----------------------------------------------------------------------
	bool Scene::OptimiseByOrder(const unsigned int chunkIndex, PGEOMETRY pGeometry, const Chunk &chunk, PGeometryETMTChunkMap &chunkMap)
	//-------------------------------------------------------------------
	{
		if (!pGeometry || (chunk.numVerts < 1))
			return false;

		return OptimiseByGeometry(chunkIndex, chunk, chunkMap[pGeometry]);
	}


//-----------------------------------------------------------------------
	bool Scene::OptimiseByGeometry(const unsigned int chunkIndex, const Chunk &chunk, EffectTMTChunkMap &chunkMap)
	//-------------------------------------------------------------------
	{
		return OptimiseByEffect(chunkIndex, chunk, chunkMap[chunk.effect]);
	}


//-----------------------------------------------------------------------
	bool Scene::OptimiseByEffect(const unsigned int chunkIndex, const Chunk &chunk, MaterialTChunkMap &chunkMap)
	//-------------------------------------------------------------------
	{
		return OptimiseByMaterial(chunkIndex, chunk, chunkMap[chunk.material]);
	}


//-----------------------------------------------------------------------
	bool Scene::OptimiseByMaterial(const unsigned int chunkIndex, const Chunk &chunk, TextureChunkMap &chunkMap)
	//-------------------------------------------------------------------
	{
		return OptimiseByTexture(chunkIndex, chunk, chunkMap[chunk.material]);
	}


//-----------------------------------------------------------------------
	bool Scene::OptimiseByTexture(const unsigned int chunkIndex, const Chunk &chunk, ChunkIndices &chunkMap)
	//-------------------------------------------------------------------
	{
		chunkMap.push_back(chunkIndex);

		return true;
	}

/*
//-----------------------------------------------------------------------
	HRESULT Scene::Lock(void **pChunks, void **pVerts, void **pChunkIndices, void **pVertexIndices)
	//-------------------------------------------------------------------
	{
		return geometry->Lock(pChunks, pVerts, pChunkIndices, pVertexIndices);
	}


//-----------------------------------------------------------------------
	HRESULT Scene::Unlock()
	//-------------------------------------------------------------------
	{
		return geometry->Unlock();
	}


//-----------------------------------------------------------------------
	HRESULT Scene::InitGeometry(int vertexSize, DWORD vertexFormat, DWORD indexFormat)
	//-------------------------------------------------------------------
	{
		return geometry->Init(vertexSize, vertexFormat, indexFormat);
	}


//-----------------------------------------------------------------------
	HRESULT Scene::InsertGeometry(int nChunks, int nVerts, int nChunkIndices, int nVertexIndices,
								  void *pChunks, void *pVerts, void *pChunkIndices, void *pVertexIndices, DWORD flags)
	//-------------------------------------------------------------------
	{
		geometry->Insert(nChunks, nVerts, nChunkIndices, nVertexIndices, 
						 pChunks, pVerts, pChunkIndices, pVertexIndices, flags);

		m_pRoot->min = geometry->min;
		m_pRoot->max = geometry->max;

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Scene::InsertGeometry(int nChunks, int nVerts, void *pChunks, void *pVerts, DWORD flags)
	//-------------------------------------------------------------------
	{
		geometry->Insert(nChunks, nVerts, pChunks, pVerts, flags);

		m_pRoot->min = geometry->min;
		m_pRoot->max = geometry->max;

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Scene::InsertGeometry(PGEOMETRY pGeometry, DWORD flags)
	//-------------------------------------------------------------------
	{
		geometry->Insert(pGeometry, flags);

		m_pRoot->min = geometry->min;
		m_pRoot->max = geometry->max;

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Scene::OutputGeometry(PGEOMETRY pGeometry, PPARTITIONTREE pTree)
	//-------------------------------------------------------------------
	{
		// NOTE: Do Insert geometry flags need to be passed to this function?

		if (!pGeometry && !pTree)
			return E_FAIL;

		if (pGeometry)
		{
			if (FAILED(pGeometry->Insert(geometry, 0)))
				return E_FAIL;
		}

		//pTree->numLeafFaces = numLeafFaces;
		pTree->numTreeNodes = m_numNodes;
		pTree->numTreeLeafs = leafs.size();

		if (pTree)
		{
			//memcpy(pTree->leafFaces, leafFaceIndices, numLeafFaces * sizeof(int));

			if (m_pRoot)
			{
				std::vector<TreeNodeOutput> outputNodes;
				RecurseOutputPartition(m_pRoot, outputNodes);
				pTree->treeNodes = new TreeNodeOutput[outputNodes.size()];

				std::vector<TreeNodeOutput>::iterator i = outputNodes.begin();
				for (int n = 0; i != outputNodes.end(); i++, n++)
					pTree->treeNodes[n] = (*i);
			}

			if (leafs.size())
			{
				pTree->treeLeafs = new TreeLeaf[leafs.size()];

				std::vector<TreeLeaf *>::iterator i = leafs.begin();
				for (int n = 0; i != leafs.end(); i++, n++)
					pTree->treeLeafs[n] = **i;
			}
		}

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Scene::GeneratePatch(PPATCH pPatch)
	//-------------------------------------------------------------------
	{
		if (!pPatch)
			return E_FAIL;

		return S_OK;
	}
*/


//-----------------------------------------------------------------------
	HRESULT Scene::InsertGeometry(PGEOMETRY pGeometry)
	//-------------------------------------------------------------------
	{
		if (!pGeometry)
			return E_FAIL;

		pGeometry->SetScene(this, true);

		m_sourceGeometry.push_back(pGeometry);

		if (m_pSettings->optimiseChunks)
		{
			/*Geometry *pOptimisedGeometry = new Geometry(D3DDevice);
			optimisedGeometry.push_back(std::make_pair(pGeometry, pOptimisedGeometry));*/
		}

		if (m_sourceGeometry.size() == 1)
		{
			m_pRoot->m_min = pGeometry->GetMin();
			m_pRoot->m_max = pGeometry->GetMax();
		}
		else
		{
			Math::SortMinMax(pGeometry->GetMin(), pGeometry->GetMax(), m_pRoot->m_min, m_pRoot->m_max);
		}

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Scene::RemoveGeometry(PGEOMETRY pGeometry, bool notify)
	//-------------------------------------------------------------------
	{
		// TODO: Go through Sub-Geometries and remove any that reference pGeometry.

		if (!pGeometry)
			return E_FAIL;

		GeometryList::iterator i = m_sourceGeometry.begin();
		for (; i != m_sourceGeometry.end();)
		{
			if ((*i) == pGeometry)
			{
				if (notify)
				{
					(*i)->SetScene(NULL, false);
				}
				i = m_sourceGeometry.erase(i);
			}
			else
			{
				i++;
			}
		}

		if (m_pSettings->optimiseChunks)
		{
			/*std::list< std::pair<PGEOMETRY, PGEOMETRY> >::iterator o = optimisedGeometry.begin();
			for (; o != optimisedGeometry.end();)
			{
				if (o->first == pGeometry)
				{
					if (o->second)
						o->second->Release();

					o = optimisedGeometry.erase(o);
				}
				else
				{
					o++;
				}
			}*/
		}

		std::vector<TreeLeaf *>::iterator leaf = m_leafs.begin();
		for (; leaf != m_leafs.end(); leaf++)
		{
			if (!(*leaf))
				continue;

			std::list<RenderGroup *>::iterator r = (*leaf)->m_renderList.begin();
			for (; r != (*leaf)->m_renderList.end();)
			{
				if ((*r)->m_pGeometry == pGeometry)
				{
					delete (*r);
					(*r) = NULL;

					r = (*leaf)->m_renderList.erase(r);
				}
				else
				{
					r++;
				}
			}
		}

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Scene::UpdateGeometry(PGEOMETRY pGeometry)
	//-------------------------------------------------------------------
	{
		if (!pGeometry)
			return E_FAIL;

		RemoveGeometry(pGeometry);
		InsertGeometry(pGeometry);

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Scene::InsertEntity(PENTITY pEntity)
	//-------------------------------------------------------------------
	{
 		if (!pEntity || !m_pSceneGraphRoot)
			return E_FAIL;

		m_pSceneGraphRoot->AttachChild(pEntity);

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Scene::RemoveEntity(PENTITY pEntity)
	//-------------------------------------------------------------------
	{
		if (!pEntity || !m_pSceneGraphRoot)
			return E_FAIL;

		m_pSceneGraphRoot->DetachChild(pEntity);

		return S_OK;
	}


#if 0
//-----------------------------------------------------------------------
	HRESULT Scene::InsertPatch(PPATCH pPatch)
	//-------------------------------------------------------------------
	{
		if (!pPatch)
			return E_FAIL;

		//patches.push_back(pPatch);

		return S_OK;
	}
#endif

//-----------------------------------------------------------------------
	HRESULT Scene::GetWorldBounds(Vector3 *pMin, Vector3 *pMax)
	//-------------------------------------------------------------------
	{
		if (!m_pRoot)
			return E_FAIL;

		if (pMin)
			*pMin = m_pRoot->m_min;

		if (pMax)
			*pMax = m_pRoot->m_max;

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Scene::GetCollisionInfo(CollisionInfo **pCollisionInfo, int *pNumCollisions)
	//-------------------------------------------------------------------
	{
		// this function leaks.

		if (!pCollisionInfo)
			return E_FAIL;
/*
		int numCollisions = m_bodyCollisions.size() + m_worldCollisions.size();
		*pCollisionInfo = new CollisionInfo[numCollisions];

		for (int i = 0; i < m_bodyCollisions.size(); i++)
		{
			CollisionInfo info = m_bodyCollisions[i];
			memcpy((*pCollisionInfo) + sizeof(CollisionInfo) * i, &info, sizeof(CollisionInfo));
		}

		for (; i < numCollisions; i++)
		{
			CollisionInfo info = m_worldCollisions[i - m_bodyCollisions.size()];
			memcpy((*pCollisionInfo) + sizeof(CollisionInfo) * i, &info, sizeof(CollisionInfo));
		}

		if (pNumCollisions)
			*pNumCollisions = numCollisions;

		m_bodyCollisions.clear();
		m_worldCollisions.clear();
*/
		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Scene::GetGeometry(ConstGeometryIterator *begin, ConstGeometryIterator *end)
	//-------------------------------------------------------------------
	{
		*begin = m_sourceGeometry.begin();
		*end = m_sourceGeometry.end();

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Scene::GetSpacePartitionOutlineGeometry(PGEOMETRY *ppGeometry)
	//-------------------------------------------------------------------
	{
		if (!ppGeometry || !m_pSpacePartitionOutline)
			return E_FAIL;

		(*ppGeometry) = m_pSpacePartitionOutline;

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Scene::GetSceneGraphRootEntity(PENTITY *ppEntity)
	//-------------------------------------------------------------------
	{
		if (!ppEntity || !m_pSceneGraphRoot)
			return E_FAIL;

		(*ppEntity) = m_pSceneGraphRoot;

		return S_OK;
	}


//-----------------------------------------------------------------------
	int Scene::GetSubdivisionsExecuted()
	//-------------------------------------------------------------------
	{
		return m_subdivisionsExecuted;
	}


//-----------------------------------------------------------------------
	int Scene::GetNumLeafs()
	//-------------------------------------------------------------------
	{
		return m_leafs.size();
	}


//-----------------------------------------------------------------------
	HRESULT Scene::Serialise(OutputScene *pOutputScene, std::vector< std::pair<PGEOMETRY, int> > &geometryMap, 
				   std::vector< std::pair<PENTITY, int> > &entityMap, DWORD flags)
	//-------------------------------------------------------------------
	{
		if (!pOutputScene)
			return E_FAIL;

		ZeroMemory(pOutputScene, sizeof(OutputScene));

		pOutputScene->size = sizeof(OutputScene);
		pOutputScene->settings = *m_pSettings;

		pOutputScene->numGeometry = m_sourceGeometry.size();
		pOutputScene->pGeometryIDs = new unsigned int[pOutputScene->numGeometry];

		GeometryList::iterator g = m_sourceGeometry.begin();
		for (int n = 0; g != m_sourceGeometry.end(); g++, n++)
		{
			std::vector< std::pair< PGEOMETRY, int > >::iterator i = geometryMap.begin();
			for (; i != geometryMap.end(); i++)
			{
				if ((*g) == i->first)
				{
					pOutputScene->pGeometryIDs[n] = i->second;
					break;
				}
			}
		}

		std::vector< std::pair< PGEOMETRY, int > >::iterator i = geometryMap.begin();
		for (; i != geometryMap.end(); i++)
		{
			if (m_pSpacePartitionOutline == i->first)
			{
				pOutputScene->spacePartitionGeometryID = i->second;
				break;
			}
		}

		if (1) // (flags & SPACEPARTITION)
		{
			// Tree Nodes
				pOutputScene->numTreeNodes = m_numNodes;

				std::vector<OutputTreeNode> outputTreeNodes;

				RecurseOutputPartition(m_pRoot, outputTreeNodes);

				pOutputScene->nodes = new OutputTreeNode[outputTreeNodes.size()];
				pOutputScene->size += outputTreeNodes.size() * sizeof(OutputTreeNode);

				std::vector<OutputTreeNode>::iterator i = outputTreeNodes.begin();
				for (int n = 0; i != outputTreeNodes.end(); i++, n++)
					pOutputScene->nodes[n] = *i;

			// Tree Leafs
				pOutputScene->numTreeLeafs = m_leafs.size();

				pOutputScene->leafs = new OutputTreeLeaf[m_leafs.size()];
				pOutputScene->size += m_leafs.size() * sizeof(OutputTreeLeaf);

				std::vector<TreeLeaf *>::iterator leafItr = m_leafs.begin();
				for (int n = 0; leafItr != m_leafs.end(); leafItr++, n++)
				{
					OutputTreeLeaf leaf;
					leaf.min = (*leafItr)->m_min;
					leaf.max = (*leafItr)->m_max;
					leaf.renderList = new OutputRenderGroup[(*leafItr)->m_renderList.size()];
					leaf.numRenderGroups = (*leafItr)->m_renderList.size();
					leaf.size = (*leafItr)->m_renderList.size() * sizeof(OutputRenderGroup);
					pOutputScene->size += leaf.size;

					std::list<RenderGroup *>::iterator r = (*leafItr)->m_renderList.begin();
					for (int v = 0; r != (*leafItr)->m_renderList.end(); r++)
					{
						OutputRenderGroup group;

						std::vector< std::pair<PGEOMETRY, int> >::iterator g = geometryMap.begin();
						for (; g != geometryMap.end(); g++)
						{
							if (g->first == (*r)->m_pGeometry)
							{
								group.geometryID = g->second;
								break;
							}
						}

						group.geometryID = 0;
						group.startFaceIndex = (*r)->m_startFaceIndex;
						group.numFaces = (*r)->m_numFaces;
						group.effectID = (*r)->m_effectID;
						leaf.renderList[v] = group;
					}

					pOutputScene->leafs[n] = leaf;
			}
		}

		if (1) // (flags & PVS)
		{
			/*pOutputScene->numPVSClusters = numClusters;
			pOutputScene->bytesPerPVSCluster = bytesPerCluster;

			pOutputScene->PVS = new byte[numClusters * bytesPerCluster];
			memcpy(pOutputScene->PVS, PVS, numClusters * bytesPerCluster);

			pOutputScene->size += numClusters * bytesPerCluster;*/
		}

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Scene::Reassemble(OutputScene *pOutputScene, std::vector< std::pair<PGEOMETRY, int> > &geometryMap, 
				   std::vector< std::pair<PENTITY, int> > &entityMap, DWORD flags)
	//-------------------------------------------------------------------
	{
		if (!pOutputScene)
			return E_FAIL;

		ResetWorld();

		std::vector< std::pair<PGEOMETRY, int> >::iterator g = geometryMap.begin();
		for (; g != geometryMap.end(); g++)
		{
			for (unsigned int i = 0; i < pOutputScene->numGeometry; i++)
			{
				if (pOutputScene->pGeometryIDs[i] == g->second)
					m_sourceGeometry.push_back(g->first);

				if (pOutputScene->spacePartitionGeometryID == g->second)
					m_pSpacePartitionOutline = g->first;
			}
		}

		std::vector< std::pair<PENTITY, int> >::iterator e = entityMap.begin();
		for (; e != entityMap.end(); e++)
		{
			if (pOutputScene->sceneGraphRootEntityID == e->second)
				m_pSceneGraphRoot = e->first;
		}

		if (1) // (flags & SPACEPARTITION)
		{
			if (pOutputScene->numTreeNodes)
			{
				if (m_pRoot)
				{
					delete m_pRoot;
					m_pRoot = new TreeNode;

					m_numNodes = 1;
				}

				m_pRoot->m_numChildren = pOutputScene->nodes[0].numChildren;
				m_pRoot->m_leafIndex = pOutputScene->nodes[0].leafIndex;
				m_pRoot->m_min = pOutputScene->nodes[0].min;
				m_pRoot->m_max = pOutputScene->nodes[0].max;
				m_pRoot->m_pParent = NULL;
				m_pRoot->m_id = 0;

				RecurseInputPartition(m_pRoot, pOutputScene->nodes, m_numNodes);
			}

			if (pOutputScene->numTreeLeafs)
			{
				for (unsigned int i = 0; i < pOutputScene->numTreeLeafs; i++)
				{
					OutputTreeLeaf &outputLeaf = pOutputScene->leafs[i];

					// TODO: give it its parent node.
					TreeLeaf *leaf = new TreeLeaf;
					leaf->m_min = outputLeaf.min;
					leaf->m_max = outputLeaf.max;

					for (unsigned int r = 0; r < outputLeaf.numRenderGroups; r++)
					{
						OutputRenderGroup &outputGroup = outputLeaf.renderList[r];

						RenderGroup *group = new RenderGroup(this);

						group->m_pGeometry = NULL;

						std::vector< std::pair<PGEOMETRY, int> >::iterator g = geometryMap.begin();
						for (; g != geometryMap.end(); g++)
						{
							if (g->second == outputGroup.geometryID)
							{
								group->m_pGeometry = g->first;
								break;
							}
						}

						group->m_startFaceIndex = outputGroup.startFaceIndex;
						group->m_numFaces = outputGroup.numFaces;
						group->m_effectID = outputGroup.effectID;
						group->m_hasTransform = false;

						leaf->m_renderList.push_back(group);
					}

					m_leafs.push_back(leaf);
				}
			}
		}

		if (/*flags & PVS &&*/ pOutputScene->numPVSClusters && pOutputScene->bytesPerPVSCluster)
		{
			/*numClusters = pOutputScene->numPVSClusters;
			bytesPerCluster = pOutputScene->bytesPerPVSCluster;

			PVS = new byte[numClusters * bytesPerCluster];
			memcpy(PVS, pOutputScene->PVS, numClusters * bytesPerCluster);*/
		}

		return S_OK;
	}


//-----------------------------------------------------------------------
	void Scene::CreateNodeRenderList(TreeNode *node, std::vector<RenderGroup> &renderList, PVIEWPORT pViewport)
	//-------------------------------------------------------------------
	{
		if (pViewport)
		{
			if (!pViewport->GetCamera()->CheckBoxFrustum(node->m_min, node->m_max))
				return;
		}

		if (node->m_numChildren)
		{
			for (int i = 0; i < node->m_numChildren; i++)
				CreateNodeRenderList(node->m_pChildren[i], renderList, pViewport);
		}
		else if (!m_leafs.empty())
		{
			if (node->m_leafIndex > -1)
				CreateLeafRenderList(m_leafs[node->m_leafIndex], renderList);
		}
	}


//-----------------------------------------------------------------------
	void Scene::CreateEntityRenderList(PENTITY entity, std::vector<RenderGroup> &renderList, PVIEWPORT pViewport)
	//-------------------------------------------------------------------
	{
		ViewportSettings vps;
		pViewport->GetSettings(&vps);

		if (entity->GetRedraw())
		{
			if (entity->GetRedraw() == RT_ONCE)
				entity->SetRedraw(RT_NONE);

			if (entity->GetVisualID() > -1)
			{
				PVISUAL pVisual = m_pResourceManager->GetVisual(entity->GetVisualID());

				Vector3 visualMin = pVisual->GetMin();
				Vector3 visualMax = pVisual->GetMax();
				const Matrix &entityTransform = entity->GetAbsoluteTransformMatrix();

				D3DXVec3TransformCoord(&visualMin, &visualMin, &entityTransform);
				D3DXVec3TransformCoord(&visualMax, &visualMax, &entityTransform);

				if (pViewport->GetCamera()->CheckBoxFrustum(visualMin, visualMax))
				{
					pVisual->CreateRenderList(renderList, entityTransform, vps.drawVisualBoundingBox);
				}
			}
		}

		// Create child render lists
		std::list<PENTITY>::iterator e = entity->m_children.begin();
		for (; e != entity->m_children.end(); e++)
			CreateEntityRenderList((*e), renderList, pViewport);
	}


//-----------------------------------------------------------------------
	void Scene::CreateLeafRenderList(TreeLeaf *leaf, std::vector<RenderGroup> &renderList)
	//-------------------------------------------------------------------
	{
		if (leaf->m_renderList.empty())
			return;

		std::list<RenderGroup *>::iterator r = leaf->m_renderList.begin();
		for (; r != leaf->m_renderList.end(); r++)
		{
			if (!(*r))
				continue;

			if (!((*r)->m_pGeometry || (*r)->m_pOptimisedGeometry))
				continue;

			if ((*r)->m_pGeometry && (*r)->m_pGeometry->IsHidden())
				continue;

			renderList.push_back(*(*r));
		}
	}


//-----------------------------------------------------------------------
	HRESULT Scene::CreateRenderList(std::vector<RenderGroup> &renderList, PVIEWPORT pViewport)
	//-------------------------------------------------------------------
	{
		if (!GetRedraw())
			return S_OK;

		if (GetRedraw() == RT_ONCE)
			SetRedraw(RT_NONE);

		ViewportSettings vps;

		if (FAILED(pViewport->GetSettings(&vps)))
			return E_FAIL;

		if (vps.drawSpacePartition && m_pSpacePartitionOutline)
		{
			if (m_activeLeafID > -1 && m_leafs[m_activeLeafID]->m_pParent)
			{
				if (vps.spacePartitionMode & VS_EDGE)
				{
					RenderGroup outlineListLines(this);
					outlineListLines.m_order = 0;
					outlineListLines.m_pGeometry = m_pSpacePartitionOutline;
					outlineListLines.m_startFaceIndex = m_leafs[m_activeLeafID]->m_pParent->m_partitionOutlineChunkId;
					outlineListLines.m_effectID = vps.boundingEdgeEffectId;
					outlineListLines.m_numFaces = 1;

					renderList.push_back(outlineListLines);
				}

				if (vps.spacePartitionMode & VS_FACE)
				{
					RenderGroup outlineListFaces(this);
					outlineListFaces.m_order = 9;
					outlineListFaces.m_pGeometry = m_pSpacePartitionOutline;
					outlineListFaces.m_startFaceIndex = m_leafs[m_activeLeafID]->m_pParent->m_partitionOutlineChunkId + m_leafs.size();
					outlineListFaces.m_effectID = vps.boundingFaceEffectId;
					outlineListFaces.m_numFaces = 1;

					renderList.push_back(outlineListFaces);
				}
			}
			else
			{
				if (vps.spacePartitionMode & VS_EDGE)
				{
					RenderGroup outlineListLines(this);
					outlineListLines.m_order = 0;
					outlineListLines.m_pGeometry = m_pSpacePartitionOutline;
					outlineListLines.m_startFaceIndex = 0;
					outlineListLines.m_effectID = vps.boundingEdgeEffectId;
					outlineListLines.m_numFaces = m_leafs.size();

					renderList.push_back(outlineListLines);
				}

				if (vps.spacePartitionMode & VS_FACE)
				{
					RenderGroup outlineListFaces(this);
					outlineListFaces.m_order = 9;
					outlineListFaces.m_pGeometry = m_pSpacePartitionOutline;
					outlineListFaces.m_startFaceIndex = m_leafs.size();
					outlineListFaces.m_effectID = vps.boundingFaceEffectId;
					outlineListFaces.m_numFaces = m_leafs.size();

					renderList.push_back(outlineListFaces);
				}
			}
		}

		if (m_pSettings->visibilityMode == SS_ENABLED)
		{
			for (unsigned int i = 0; i < m_leafs.size(); i++)
			{
				byte *pCluster = m_PVS[i];

				if (!pCluster)
					continue;

				if (Math::AABBPointCollision(Math::AABB(m_leafs[i]->m_min, m_leafs[i]->m_max), pViewport->GetCamera()->GetPosition()))
				{
					for (unsigned int n = 0; n < m_leafs.size(); n++)
					{
						if (*(pCluster + (n / 8)) & (1 << (n & 7)))
						{
							Vector3 cMin = m_leafs[n]->m_min;
							Vector3 cMax = m_leafs[n]->m_max;

							if (!pViewport->GetCamera()->CheckBoxFrustum(cMin, cMax))
								continue;

							CreateLeafRenderList(m_leafs[n], renderList);
						}
					}

					break;
				}
			}
		}
		else
		{
			if (m_activeLeafID == -1)
			{
				CreateNodeRenderList(m_pRoot, renderList, pViewport);
			}
			else if (m_activeLeafID < (int)m_leafs.size())
			{
				CreateLeafRenderList(m_leafs[m_activeLeafID], renderList);
			}
		}

		CreateEntityRenderList(m_pSceneGraphRoot, renderList, pViewport);

		// TODO: sort render list by depth bias

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Scene::Update(const float deltaTime)
	//-------------------------------------------------------------------
	{
		if (m_pSceneGraphRoot)
			m_pSceneGraphRoot->Update(deltaTime);

		/*
		m_bodyCollisions.clear();
		m_worldCollisions.clear();

		std::vector<CollisionInfo> collisions;
		collisions.clear();

		//DebugPrintf("Frame %d at %f\n\n", frameNumber, deltaTime);

		// Update dynamic states: Pre Collision Response
		std::list<Entity *>::iterator i = entities.begin();
		for (; i != entities.end(); i++)
		{
			if ((*i)->GetUpdate())
			{
				//DebugPrintf("Pre Response\n\n");

				RigidBody *body = (*i)->GetRigidBody();

				//body->force = Vector3(0, -800, 0);

				body->acceleration = body->force / body->mass;
				body->rotAcceleration = body->torque / body->mass;

				body->velocity += body->acceleration * deltaTime;
				body->rotVelocity += body->rotAcceleration * deltaTime;

				body->lastPosition = body->position;
				body->lastRotation = body->rotation;

				body->position += body->velocity * deltaTime;
				body->rotation += body->rotVelocity * deltaTime;
			}
		}

		std::vector<TreeLeaf *>::iterator leaf = leafs.begin();
		for (; leaf != leafs.end(); leaf++)
		{
			// Make a list of entites in this leaf
				std::list<Entity *> leafEntities;
				std::vector<int> numEntityCollisions;

				for (i = entities.begin(); i != entities.end(); i++)
				{
					RigidBody *body = (*i)->GetRigidBody();

					if (AABBSphereCollision(AABB((*leaf)->min, (*leaf)->max), 
											Sphere(body->lastPosition, body->totalBoundingRadius), NULL) ||
						AABBSphereCollision(AABB((*leaf)->min, (*leaf)->max), 
											Sphere(body->position,	  body->totalBoundingRadius), NULL))
						leafEntities.push_back(*i);
				}

				PVOID pVertsLocked = NULL;
				int *pIndicesLocked = NULL;

				if (leafEntities.size() && (*leaf)->numFaces)
				{
					if (FAILED(geometry->Lock(NULL, &pVertsLocked, (PVOID*)&pIndicesLocked)))
						return E_FAIL;
				}

				numEntityCollisions.resize(leafEntities.size());

			// Entity-world collision detection
				int iEntity = 0;
				for (i = leafEntities.begin(); i != leafEntities.end(); i++, iEntity++)
				{
					RigidBody *body = (*i)->GetRigidBody();

					for (int f = (*leaf)->startFaceIndex; f < (*leaf)->startFaceIndex + (*leaf)->numFaces; f++)
					{
						Chunk face = geometry->chunks[leafFaceIndices[f]];

						Vector3 *faceVerts = new Vector3[face.numVerts];
						for (int v = 0; v < face.numVerts; v++)
							faceVerts[v] = geometry->GetVertexPosition(pVertsLocked, pIndicesLocked[face.startIndex + v]);

						Vector3 faceNormal = Normalise(Cross(faceVerts[1] - faceVerts[0], faceVerts[2] - faceVerts[0]));

						// If it doesn't have any bounding volumes, body is a rigid particle
						std::vector<BoundingVolume>::iterator b = body->boundingVolumes.begin();
						for (; b != body->boundingVolumes.end(); b++)
						{
							CollisionInfo *result = NULL;

							if (b->sphereRadius)
							{
								Vector3 point, pos;
								float time;
								if (SweepSphereStaticPolygon(Sphere(body->lastPosition, b->sphereRadius),
															 Sphere(body->position, b->sphereRadius),
															 Polygon(faceNormal, faceVerts, face.numVerts), 
															 &time, &pos, &point))
								{
									body->position = pos;
									result = new CollisionInfo;
									result->collisionNormal = faceNormal;
									result->contactPointWorld = point;
								}
							}
							else if (b->boundingBox)
							{

							}
							else if (b->cylinderHeight && b->cylinderRadius)
							{

							}

							if (result)
							{
								result->type = CT_BODYWORLD;
								result->body[0] = (*i);
								result->body[1] = NULL;
								result->bodyID[0] = iEntity;
								result->bodyID[1] = -1;
								result->contactPointBody[0] = result->contactPointWorld - body->position;
								result->relativeVelocity = body->velocity;
								result->relativeNormalVelocity = Dot(result->relativeVelocity, result->collisionNormal);

								m_worldCollisions.push_back(*result);
								collisions.push_back(*result);
								numEntityCollisions[iEntity]++;
								delete result;
							}
						}

						delete[] faceVerts;
					}
				}

				if (leafEntities.size() && (*leaf)->numFaces)
				{
					geometry->Unlock();
				}

			// Collision Response - Body/World
				std::vector<CollisionInfo>::iterator n = collisions.begin();
				for (; n != collisions.end(); n++)
				{
					if (n->body[0] && n->bodyID[0] > -1)
					{
						RigidBody *body = NULL;
						if (n->body[0]->GetRigidBody())
							body = n->body[0]->GetRigidBody();
						else
							continue;

						float impulse = (-(1.0f + 0.25f) * n->relativeNormalVelocity) / (1/body->mass);

						body->velocity += ((impulse * n->collisionNormal) / body->mass) / numEntityCollisions[n->bodyID[0]];
						body->rotVelocity += Cross(n->contactPointBody[0], (impulse * n->collisionNormal));
					}
				}
				collisions.clear();

			// Entity-Entity collision detection
				for (i = leafEntities.begin(), iEntity = 0; i != leafEntities.end(); i++, iEntity++)
				{
					int jEntity = iEntity;
					std::list<Entity *>::iterator j;
					for (j = i; j != leafEntities.end(); j++, jEntity++)
					{
						if (j == i)
							continue;

						RigidBody *body = (*i)->GetRigidBody();
						RigidBody *body2 = (*j)->GetRigidBody();

						if (!body2)
							continue;

						if (AABBSphereCollision(AABB((*leaf)->min, (*leaf)->max), 
							Sphere(body->position, body->totalBoundingRadius), NULL))
						{
							if (SweepSphereSphere(Sphere(body->lastPosition, body->totalBoundingRadius),
												  Sphere(body->position,		body->totalBoundingRadius),
												  Sphere(body2->lastPosition, body2->totalBoundingRadius),
												  Sphere(body2->position,	 body2->totalBoundingRadius),
												  NULL, NULL, NULL, NULL))
							{
								std::vector<BoundingVolume>::iterator b = body->boundingVolumes.begin();
								for (; b != body->boundingVolumes.end(); b++)
								{
									std::vector<BoundingVolume>::iterator c = body2->boundingVolumes.begin();
									for (; c != body2->boundingVolumes.end(); c++)
									{
										CollisionInfo *result = NULL;

										if (b->sphereRadius && c->sphereRadius)
										{
											Vector3 point, pos1, pos2;
											float time;

											if (SweepSphereSphere(Sphere(body->lastPosition, b->sphereRadius),
																  Sphere(body->position,		b->sphereRadius),
																  Sphere(body2->lastPosition, c->sphereRadius),
																  Sphere(body2->position,	 c->sphereRadius),
																  &time, &pos1, &pos2, &point))
											{
												/*
												DebugPrintf("Collision Detected: Body %d and Body %d at %f\n", iEntity, jEntity, time);
												DebugPrintf("Body %d Position: %f, %f, %f\n", iEntity, body->position.x,
																									   body->position.y,
																									   body->position.z);
												DebugPrintf("Body %d Position: %f, %f, %f\n\n", jEntity, body2->position.x,
																										 body2->position.y,
																										 body2->position.z);

												body->position = pos1;
												body2->position = pos2;

												result = new CollisionInfo;
												result->contactPointWorld = point;
												result->contactPointBody[0] = point - body->position;
												result->contactPointBody[1] = point - body2->position;
												result->collisionNormal = Normalise(body->position - body2->position);
											}
										}
										else if (b->boundingBox && c->boundingBox)
										{
											Vector3 point, pos1, pos2;
											//float time;

											/*if (AABBAABBCollision(AABB(body->position + b->offset - b->boundingBox * 0.5f,
																 		  body->position + b->offset + b->boundingBox * 0.5f),
																  AABB(body2->position + c->offset - c->boundingBox * 0.5f,
																		  body2->position + c->offset + c->boundingBox * 0.5f),
																		  &point))
											{
											if (SweepAABBAABB(AABB(body->lastPosition + b->offset - b->boundingBox * 0.5f,
																	  body->lastPosition + b->offset + b->boundingBox * 0.5f),
															  AABB(body->position + b->offset - b->boundingBox * 0.5f,
																	  body->position + b->offset + b->boundingBox * 0.5f),
															  AABB(body2->lastPosition + c->offset - c->boundingBox * 0.5f,
																	  body2->lastPosition + c->offset + c->boundingBox * 0.5f),
															  AABB(body2->position + c->offset - c->boundingBox * 0.5f,
																	  body2->position + c->offset + c->boundingBox * 0.5f),
																	  &time, &pos1, &pos2))
											{
												body->position = pos1;
												body2->position = pos2;
												point = body->position + (body2->position - body->position) * time;

												result = new CollisionInfo;
												result->contactPointWorld = point;
												result->contactPointBody[0] = point - body->position;
												result->contactPointBody[1] = point - body2->position;
												result->collisionNormal = Normalise(body->position - body2->position);
											}
											if (OBBOBBCollision(OBB(body->lastPosition + b->offset - b->boundingBox * 0.5f,
																	   body->lastPosition + b->offset + b->boundingBox * 0.5f,
																	   body->rotation),
																OBB(body2->lastPosition + c->offset - c->boundingBox * 0.5f,
																	   body2->lastPosition + c->offset + c->boundingBox * 0.5f,
																	   body2->rotation), NULL))
											{
												body->velocity = Vector3(0, 0, 0);
												body2->velocity = Vector3(0, 0, 0);
											}
										}
										else if (b->cylinderHeight && b->cylinderRadius)
										{

										}

										if (result)
										{
											result->type = CT_BODYBODY;
											result->body[0] = (*i);
											result->body[1] = (*j);
											result->bodyID[0] = iEntity;
											result->bodyID[1] = jEntity;

											result->relativeVelocity = body->velocity - body2->velocity;
											result->relativeNormalVelocity = Dot(result->relativeVelocity, result->collisionNormal);

											m_bodyCollisions.push_back(*result);
											collisions.push_back(*result);
											delete result;
										}
									}
								}
							}
						}
					}
				}

			// Collision Response - Body
			/*
				n = collisions.begin();
				for (; n != collisions.end(); n++)
				{
					if (n->body[0] && n->bodyID[0] > -1 && n->body[1] && n->bodyID[1] > -1)
					{
						RigidBody *body1 = NULL;
						RigidBody *body2 = NULL;

						if (n->body[0]->GetRigidBody())
							body1 = n->body[0]->GetRigidBody();
						else
							continue;

						if (n->body[1]->GetRigidBody())
							body2 = n->body[1]->GetRigidBody();
						else
							continue;

						float impulse = (-(1.0f + 0.5f) * n->relativeNormalVelocity) /
										((1/body1->mass + 1/body2->mass));

						body1->velocity += (impulse * n->collisionNormal) / body1->mass;
						//body1->rotVelocity += Cross(n->contactPointBody[0], (impulse * n->collisionNormal));

						body2->velocity -= (impulse * n->collisionNormal) / body2->mass;
						//body2->rotVelocity -= Cross(n->contactPointBody[1], (impulse * n->collisionNormal));

						DebugPrintf("Collision Resolved: Body %d and Body %d\n", n->bodyID[0], n->bodyID[1]);
						DebugPrintf("Body %d Position: %f, %f, %f\n", n->bodyID[0], n->body[0]->GetPosition().x,
																				  n->body[0]->GetPosition().y,
																				  n->body[0]->GetPosition().z);
						DebugPrintf("Body %d Position: %f, %f, %f\n\n", n->bodyID[1], n->body[1]->GetPosition().x,
																				  n->body[1]->GetPosition().y,
																				  n->body[1]->GetPosition().z);
					}
				}
				collisions.clear();
		}

		// Update dynamic states: Post Collision Response
		/*i = entities.begin();
		int bodyNum = 0;
		for (; i != entities.end(); i++, bodyNum++)
		{
			if ((*i)->GetUpdate())
			{
				if ((*i)->GetUpdate() == RT_ONCE)
					(*i)->SetUpdate(RT_ONCE);

				RigidBody *body = (*i)->GetRigidBody();

				DebugPrintf("Post Response\n");
				DebugPrintf("Body %d\n", bodyNum);
				DebugPrintf("Velocity: %f, %f, %f\n", body->velocity.x, body->velocity.y, body->velocity.z);
				DebugPrintf("Position: %f, %f, %f\n\n", body->position.x, body->position.y, body->position.z);
			}
		}

		DebugPrintf("-------------------------------------------------\n\n");
*/
		return S_OK;
	}


//-----------------------------------------------------------------------
	void Scene::RecurseCreateDrawPartition(TreeNode *pNode, std::vector<Vector3> &verts, std::vector<Chunk> &lineChunks,
										   std::vector<Chunk> &faceChunks, std::vector<unsigned int> &vertexIndices)
	//-------------------------------------------------------------------
	{
		if (!pNode)
			return;

		if (pNode->m_leafIndex > -1)
		{
			pNode->m_partitionOutlineChunkId = lineChunks.size();

			unsigned int lineVertexOffset = lineChunks.size() * 8;
			unsigned int chunkVertexOffset = m_leafs.size() * 8 + lineVertexOffset;

			// Vertices
				Vector3 &a = pNode->m_min;
				Vector3 &b = pNode->m_max;

				verts.push_back(Vector3(a.x, b.y, b.z));
				verts.push_back(Vector3(a.x, a.y, b.z));
				verts.push_back(Vector3(b.x, a.y, b.z));
				verts.push_back(b);

				verts.push_back(Vector3(a.x, b.y, a.z));
				verts.push_back(a);
				verts.push_back(Vector3(b.x, a.y, a.z));
				verts.push_back(Vector3(b.x, b.y, a.z));

			// Lines Chunk
				Chunk lines(-1, -1, -1, -1, -1, (int)lineChunks.size() * 24, 24, 1, CT_LINELIST);
				lineChunks.push_back(lines);

			// Faces Chunk
				Chunk faces(lines);
				faces.order = 9;
				faces.startIndex = (int)m_leafs.size() * 24 + (int)faceChunks.size() * 36;
				faces.numIndices = 36;
				faces.type = CT_TRIANGLELIST;
				faces.depthBias = 0;
				faceChunks.push_back(faces);

			// Line Vert Indices
				// Top
					vertexIndices[lines.startIndex] = lineVertexOffset + 0;			vertexIndices[lines.startIndex + 1] = lineVertexOffset + 1;
					vertexIndices[lines.startIndex + 2] = lineVertexOffset + 1;		vertexIndices[lines.startIndex + 3] = lineVertexOffset + 2;
					vertexIndices[lines.startIndex + 4] = lineVertexOffset + 2;		vertexIndices[lines.startIndex + 5] = lineVertexOffset + 3;
					vertexIndices[lines.startIndex + 6] = lineVertexOffset + 3;		vertexIndices[lines.startIndex + 7] = lineVertexOffset + 0;

				// Sides
					vertexIndices[lines.startIndex + 8] = lineVertexOffset + 0;		vertexIndices[lines.startIndex + 9] = lineVertexOffset + 4;
					vertexIndices[lines.startIndex + 10] = lineVertexOffset + 1;	vertexIndices[lines.startIndex + 11] = lineVertexOffset + 5;
					vertexIndices[lines.startIndex + 12] = lineVertexOffset + 2;	vertexIndices[lines.startIndex + 13] = lineVertexOffset + 6;
					vertexIndices[lines.startIndex + 14] = lineVertexOffset + 3;	vertexIndices[lines.startIndex + 15] = lineVertexOffset + 7;

				// Bottom
					vertexIndices[lines.startIndex + 16] = lineVertexOffset + 4;	vertexIndices[lines.startIndex + 17] = lineVertexOffset + 5;
					vertexIndices[lines.startIndex + 18] = lineVertexOffset + 5;	vertexIndices[lines.startIndex + 19] = lineVertexOffset + 6;
					vertexIndices[lines.startIndex + 20] = lineVertexOffset + 6;	vertexIndices[lines.startIndex + 21] = lineVertexOffset + 7;
					vertexIndices[lines.startIndex + 22] = lineVertexOffset + 7;	vertexIndices[lines.startIndex + 23] = lineVertexOffset + 4;

			// Face Vert Indices
				// Top
					vertexIndices[faces.startIndex] = chunkVertexOffset + 0;	vertexIndices[faces.startIndex + 1] = chunkVertexOffset + 2;	vertexIndices[faces.startIndex + 2] = chunkVertexOffset + 1;
					vertexIndices[faces.startIndex + 3] = chunkVertexOffset + 0;	vertexIndices[faces.startIndex + 4] = chunkVertexOffset + 3;	vertexIndices[faces.startIndex + 5] = chunkVertexOffset + 2;

				// Bottom
					vertexIndices[faces.startIndex + 6] = chunkVertexOffset + 7;	vertexIndices[faces.startIndex + 7] = chunkVertexOffset + 5;	vertexIndices[faces.startIndex + 8] = chunkVertexOffset + 6;
					vertexIndices[faces.startIndex + 9] = chunkVertexOffset + 7;	vertexIndices[faces.startIndex + 10] = chunkVertexOffset + 4;	vertexIndices[faces.startIndex + 11] = chunkVertexOffset + 5;

				// Left
					vertexIndices[faces.startIndex + 12] = chunkVertexOffset + 4;	vertexIndices[faces.startIndex + 13] = chunkVertexOffset + 1;	vertexIndices[faces.startIndex + 14] = chunkVertexOffset + 5;
					vertexIndices[faces.startIndex + 15] = chunkVertexOffset + 4;	vertexIndices[faces.startIndex + 16] = chunkVertexOffset + 0;	vertexIndices[faces.startIndex + 17] = chunkVertexOffset + 1;

				// Right
					vertexIndices[faces.startIndex + 18] = chunkVertexOffset + 3;	vertexIndices[faces.startIndex + 19] = chunkVertexOffset + 6;	vertexIndices[faces.startIndex + 20] = chunkVertexOffset + 2;
					vertexIndices[faces.startIndex + 21] = chunkVertexOffset + 3;	vertexIndices[faces.startIndex + 22] = chunkVertexOffset + 7;	vertexIndices[faces.startIndex + 23] = chunkVertexOffset + 6;

				// Front
					vertexIndices[faces.startIndex + 24] = chunkVertexOffset + 1;	vertexIndices[faces.startIndex + 25] = chunkVertexOffset + 6;	vertexIndices[faces.startIndex + 26] = chunkVertexOffset + 5;
					vertexIndices[faces.startIndex + 27] = chunkVertexOffset + 1;	vertexIndices[faces.startIndex + 28] = chunkVertexOffset + 2;	vertexIndices[faces.startIndex + 29] = chunkVertexOffset + 6;

				// Back
					vertexIndices[faces.startIndex + 30] = chunkVertexOffset + 4;	vertexIndices[faces.startIndex + 31] = chunkVertexOffset + 3;	vertexIndices[faces.startIndex + 32] = chunkVertexOffset + 0;
					vertexIndices[faces.startIndex + 33] = chunkVertexOffset + 4;	vertexIndices[faces.startIndex + 34] = chunkVertexOffset + 7;	vertexIndices[faces.startIndex + 35] = chunkVertexOffset + 3;
		}
		else
		{
			for (int i = 0; i < pNode->m_numChildren; i++)
			{
				RecurseCreateDrawPartition(pNode->m_pChildren[i], verts, lineChunks, faceChunks, vertexIndices);
			}
		}
	}


//-----------------------------------------------------------------------
	void Scene::RecurseOutputPartition(TreeNode *node, std::vector<OutputTreeNode> &outputNodes)
	//-------------------------------------------------------------------
	{
		OutputTreeNode outputNode;
		outputNode.min = node->m_min;
		outputNode.max = node->m_max;
		outputNode.numChildren = node->m_numChildren;
		outputNode.leafIndex = node->m_leafIndex;

		if (node->m_pParent)
			outputNode.parent = node->m_pParent->m_id;
		else
			outputNode.parent = -1;

		for (int i = 0; i < node->m_numChildren; i++)
			outputNode.children[i] = node->m_pChildren[i]->m_id;

		outputNodes.push_back(outputNode);

		for (i = 0; i < node->m_numChildren; i++)
			RecurseOutputPartition(node->m_pChildren[i], outputNodes);
	}


//-----------------------------------------------------------------------
	void Scene::RecurseInputPartition(TreeNode *node, OutputTreeNode *outputNodes, int &m_numNodes)
	//-------------------------------------------------------------------
	{
		if (!node || !outputNodes)
			return;

		if (node->m_numChildren)
			node->m_pChildren = new TreeNode *[node->m_numChildren];

		for (int i = 0; i < node->m_numChildren; i++)
		{
			int childID = outputNodes[node->m_id].children[i];

			node->m_pChildren[i] = new TreeNode(outputNodes[childID].min, outputNodes[childID].max, node, m_numNodes);
			node->m_pChildren[i]->m_leafIndex = outputNodes[childID].leafIndex;
			node->m_pChildren[i]->m_numChildren = outputNodes[childID].numChildren;
			node->m_pChildren[i]->m_min = outputNodes[childID].min;
			node->m_pChildren[i]->m_max = outputNodes[childID].max;
			node->m_pChildren[i]->m_pParent = node;
			node->m_pChildren[i]->m_id = m_numNodes;

			m_numNodes++;

			RecurseInputPartition(node->m_pChildren[i], outputNodes, m_numNodes);
		}
	}


//-----------------------------------------------------------------------
	void Scene::RecurseSphereCollisionPartition(TreeNode *node, Math::Sphere sphere1, Math::Sphere sphere2, std::vector<TreeLeaf *> &leafs, std::vector<TreeLeaf *> &collidingLeafs)
	//-------------------------------------------------------------------
	{
		if (!node)
			return;

		if (!Math::AABBSphereCollision(Math::AABB(node->m_min, node->m_max), sphere1, NULL) &&
			!Math::AABBSphereCollision(Math::AABB(node->m_min, node->m_max), sphere2, NULL))
			return;

		if (!node->m_numChildren)
		{
			collidingLeafs.push_back(leafs[node->m_leafIndex]);
			return;
		}

		for (int i = 0; i < node->m_numChildren; i++)
			RecurseSphereCollisionPartition(node->m_pChildren[i], sphere1, sphere2, leafs, collidingLeafs);
	}


//-----------------------------------------------------------------------
	void Scene::RecursePVS(TreeNode *node, TreeLeaf *leaf)
	//-------------------------------------------------------------------
	{
		if (!node || !leaf)
			return;

		if (node->m_leafIndex == -1)
		{
			for (int i = 0; i < node->m_numChildren; i++)
				RecursePVS(node->m_pChildren[i], leaf);

			return;
		}

		// If node is opaque
		if (1)
		{
			// Find maximal points on boxes
				Vector3 leafCenter, nodeCenter;
				leafCenter = leaf->m_min + (leaf->m_max - leaf->m_min) * 0.5f;
				nodeCenter = node->m_min + (node->m_max - node->m_min) * 0.5f;

				Vector3 leafPoints[8];
				leafPoints[0] = Vector3(leaf->m_min.x, leaf->m_min.y, leaf->m_max.z);
				leafPoints[1] = Vector3(leaf->m_min.x, leaf->m_max.y, leaf->m_max.z);
				leafPoints[2] = Vector3(leaf->m_max.x, leaf->m_max.y, leaf->m_max.z);
				leafPoints[3] = Vector3(leaf->m_max.x, leaf->m_min.y, leaf->m_max.z);
				leafPoints[4] = Vector3(leaf->m_min.x, leaf->m_min.y, leaf->m_min.z);
				leafPoints[5] = Vector3(leaf->m_min.x, leaf->m_max.y, leaf->m_min.z);
				leafPoints[6] = Vector3(leaf->m_max.x, leaf->m_max.y, leaf->m_min.z);
				leafPoints[7] = Vector3(leaf->m_max.x, leaf->m_min.y, leaf->m_min.z);

				Vector3 nodePoints[8];
				nodePoints[0] = Vector3(node->m_min.x, node->m_min.y, node->m_max.z);
				nodePoints[1] = Vector3(node->m_min.x, node->m_max.y, node->m_max.z);
				nodePoints[2] = Vector3(node->m_max.x, node->m_max.y, node->m_max.z);
				nodePoints[3] = Vector3(node->m_max.x, node->m_min.y, node->m_max.z);
				nodePoints[4] = Vector3(node->m_min.x, node->m_min.y, node->m_min.z);
				nodePoints[5] = Vector3(node->m_min.x, node->m_max.y, node->m_min.z);
				nodePoints[6] = Vector3(node->m_max.x, node->m_max.y, node->m_min.z);
				nodePoints[7] = Vector3(node->m_max.x, node->m_min.y, node->m_min.z);

				Vector3 leafMaxPoints[4];
				Vector3 nodeMaxPoints[4];

				float leafDistances[8];
				float nodeDistances[8];

				for (int i = 0; i < 8; i++)
				{
					leafDistances[i] = Math::Length(leafPoints[i] - Math::ClosestPointOnLine(leafCenter, nodeCenter, leafPoints[i]));
					nodeDistances[i] = Math::Length(nodePoints[i] - Math::ClosestPointOnLine(leafCenter, nodeCenter, nodePoints[i]));
				}

				for (int n = 0; n < 4; n++)
				{
					for (int i = 0; i < 8; i++)
					{
						int numGreaterLeaf = 0;
						int numGreaterNode = 0;

						for (int j = 0; j < 8; j++)
						{
							if (i == j)
								continue;

							if (leafDistances[i] > leafDistances[j])
								numGreaterLeaf++;
							else if (leafDistances[i] == leafDistances[j])
							{
								float di = Math::Length(nodeCenter - leafPoints[i]);
								float dj = Math::Length(nodeCenter - leafPoints[j]);

								if (di > dj)
									numGreaterLeaf++;
							}

							if (nodeDistances[i] > nodeDistances[j])
								numGreaterNode++;
							else if (nodeDistances[i] == nodeDistances[j])
							{
								float di = Math::Length(leafCenter - nodePoints[i]);
								float dj = Math::Length(leafCenter - nodePoints[j]);

								if (di > dj)
									numGreaterNode++;
							}
						}

						if (numGreaterLeaf >= 4)
							leafMaxPoints[n] = leafPoints[i];

						if (numGreaterNode >= 4)
							nodeMaxPoints[n] = nodePoints[i];
					}
				}

			// Sort points
				Vector3 leafFrustumFace[4];

				Vector3 leafMin = leafMaxPoints[0],
						leafMax = leafMaxPoints[0],
						nodeMin = nodeMaxPoints[0],
						nodeMax = nodeMaxPoints[0];

				for (i = 0; i < 4; i++)
				{
					Math::SortMinMax(leafMaxPoints[i], leafMaxPoints[i], leafMin, leafMax);
					Math::SortMinMax(nodeMaxPoints[i], nodeMaxPoints[i], nodeMin, nodeMax);
				}

			// Build occlusion frustum matrix
				Vector3 upVector;
				float leafWidth = 0;
				float leafHeight = 0;

				if (m_pSettings->spacePartitionAxis == SS_AXIS_X)
					upVector = Vector3(1, 0, 0);
				else if (m_pSettings->spacePartitionAxis == SS_AXIS_Y)
					upVector = Vector3(0, 1, 0);
				else if (m_pSettings->spacePartitionAxis == SS_AXIS_Z)
					upVector = Vector3(0, 0, 1);

				Vector3 f = Math::Normalise(leafCenter - nodeCenter);

				Matrix view, projection, frustum;
				//D3DXMatrixLookAtLH(&view, &(leafCenter - f), &leafCenter, &upVector);
				//D3DXMatrixPerspectiveLH(&projection, w, h, 1, FLT_MAX);

				frustum = view * projection;
/*
				Vector3 vUp = Normalise(nodeCenter - leafCenter);
				Vector3 vf = Normalise(Cross(vUp, upVector));

				if (vf == vUp)
					return;

				Vector3 d[4];
				Matrix rot[4];
				D3DXMatrixRotationAxis(&rot[0], &vUp, D3DX_PI/4);
				D3DXMatrixRotationAxis(&rot[1], &vUp, D3DX_PI/4 + D3DX_PI/2);
				D3DXMatrixRotationAxis(&rot[2], &vUp, D3DX_PI/4 + D3DX_PI);
				D3DXMatrixRotationAxis(&rot[3], &vUp, 2*D3DX_PI - D3DX_PI/4);

				for (int i = 0; i < 4; i++)
				{
					D3DXVec3TransformNormal(&v[i], &vf, &rot[i]);

					for (int n = 0; n < 8; n++)
					{
						Vector3 dv = Normalise(leafPoints[n] - leafCenter);

						if (Dot(v[i], dv) >= 0.5f)
						{
							//if (!(Normalise())7
							for (int j = 0; j < i; j++)
							{
								if (d[j] == leafPoints[n])
									continue;
							}

							d[i] = leafPoints[n];
						}
					}
				}*/
		}
	}


//-----------------------------------------------------------------------
	bool Scene::RecurseTestIntersect(TreeNode *node, std::list<IntersectionResult> &results, 
									 const Vector3 &rayPos, const Vector3 &rayDir, DWORD flags, bool findFirst)
	//-------------------------------------------------------------------
	{
		if (!node)
			return false;

		bool found = false;

		if (!Math::AABBRayCollision(Math::AABB(node->m_min, node->m_max), rayPos, rayDir))
			return false;

		if (node->m_leafIndex > -1 && node->m_leafIndex < (int)m_leafs.size())
		{
			TreeLeaf *pLeaf = m_leafs[node->m_leafIndex];

			if (pLeaf)
			{
				std::list<RenderGroup *>::iterator i = pLeaf->m_renderList.begin();
				for (; i != pLeaf->m_renderList.end(); i++)
				{
					if (!(*i) || !(*i)->m_pGeometry)
						continue;

					if (findFirst)
					{
						IntersectionResult result;

						found = (*i)->m_pGeometry->Intersect(result, rayPos, rayDir, *i, flags);

						results.push_back(result);

						break;
					}
					else
					{
						found = (*i)->m_pGeometry->Intersect(results, rayPos, rayDir, *i, flags);
					}
				}
			}
		}
		else
		{
			for (unsigned int c = 0; c < (unsigned)node->m_numChildren; c++)
			{
				if (node->m_pChildren[c])
				{
					found |= RecurseTestIntersect(node->m_pChildren[c], results, rayPos, rayDir, flags, findFirst);

					if (found && findFirst)
						return found;
				}
			}
		}

		return found;
	}


//-----------------------------------------------------------------------
	bool Scene::Intersect(std::list<IntersectionResult> &results, const Vector3 &rayPos, const Vector3 &rayDir, DWORD flags)
	//-------------------------------------------------------------------
	{
		if (!m_pRoot)
			return false;

		bool found = RecurseTestIntersect(m_pRoot, results, rayPos, rayDir, flags, false);

		return found;
	}


//-----------------------------------------------------------------------
	bool Scene::Intersect(IntersectionResult &result, const Vector3 &rayPos, const Vector3 &rayDir, DWORD flags)
	//-------------------------------------------------------------------
	{
		if (!m_pRoot)
			return false;

		std::list<IntersectionResult> results;

		bool found = RecurseTestIntersect(m_pRoot, results, rayPos, rayDir, flags, true);

		if (results.size() < 1)
			return false;

		result = *results.begin();

		return found;
	}


//-----------------------------------------------------------------------
	void Scene::GeneratePVS(void *pData, std::vector< byte * > *pResults, 
							ProgressCallbackFunc progressCallbackFunc, void *pProgressCallbackPayload)
	//-------------------------------------------------------------------
	{
/*
		if (progressCallbackFunc)
			progressCallbackFunc(PC_SCENE_PVS, pvsProgress, pProgressCallbackPayload);

		numClusters = leafs.size();
		//numClusters = m_numNodes;

		bytesPerCluster = (int)ceil((float)numClusters / 8);
		m_PVS = new byte[numClusters * bytesPerCluster];
		AddUsedMemory(numClusters * bytesPerCluster, "Scene::SubdivideWorld() - Clusters...");

		//ZeroMemory(PVS, numClusters * bytesPerCluster);
		memset(PVS, 0xffffffff, numClusters * bytesPerCluster);

		int x = 0;
		std::vector<TreeLeaf *>::iterator i;
		for (i = leafs.begin(); i != leafs.end(); i++, x++)
		{
			pvsProgress = ((float)x / (leafs.size())) * 100.0f;

			if (progressCallback)
				progressCallback(PC_SCENE_PVS, pvsProgress, pProgressCallbackPayload);

			RecursePVS(m_pRoot, (*i));
		}
*/
/*
		int x = 0;
		std::vector<TreeLeaf *>::iterator i, j;
		for (i = leafs.begin(); i != leafs.end(); i++, x++)
		{
			int y = 0;
			for (j = leafs.begin(); j != leafs.end(); j++, y++)
			{
				DWORD progress = ((float)(x * leafs.size() + y) / (leafs.size() * leafs.size())) * 100.0f;

				if (progressCallback)
					progressCallback(PC_SCENE_PVS, progress);

				if (TestVisibilityLeaf((*i), (*j)))
					PVS[x * bytesPerCluster + (y / 8)] |= (1 << (y & 7));
			}
		}
*/
	}

#ifdef _DEBUG
//-----------------------------------------------------------------------
	void Scene::DumpNodeChunkMap(const _NodeChunkMap *pNodeChunkMap, const unsigned int index) const
	//-------------------------------------------------------------------
	{
		if (!pNodeChunkMap)
			return;

		const NodeChunkMap &ncm = pNodeChunkMap->m;
		NodeChunkMap::const_iterator ncmItr = ncm.begin();

		DebugPrintf(_T("\tNodeChunkMap [%3d] : size = %d\n"), index, ncm.size());

		unsigned int ogmIndex = 0;
		for (; ncmItr != ncm.end(); ++ncmItr, ++ogmIndex)
		{
			const OrderedGeometryMap &ogm = (*ncmItr)->m;
			OrderedGeometryMap::const_iterator ogmItr = ogm.begin();

			DebugPrintf(_T("\t\tOrderedGeometryMap [%3d] : size = %d\n"), ogmIndex, ogm.size());

			unsigned int gmIndex = 0;
			for (; ogmItr != ogm.end(); ++ogmItr, ++gmIndex)
			{
				const unsigned int order = ogmItr->first;
				const GeometryMap &gm = ogmItr->second->m;

				GeometryMap::const_iterator gmItr = gm.begin();

				DebugPrintf(_T("\t\t\tGeometryMap [%3d] : order = %d, size = %d\n"), gmIndex, order, gm.size());

				unsigned int cilIndex = 0;
				for (; gmItr != gm.end(); ++gmItr, ++cilIndex)
				{
					const PGEOMETRY pGM = gmItr->first;
					const ChunkIndexList &cil = gmItr->second->m;
					ChunkIndexList::const_iterator cilItr = cil.begin();

					DebugPrintf(_T("\t\t\t\tChunkIndexList [%3d] : PGEOMETRY = 0x%8.8x, size = %d\n"), cilIndex, DWORD(pGM), cil.size());

					unsigned int isIndex = 0;
					for (; cilItr != cil.end(); ++cilItr, ++isIndex)
					{
						const ChunkIndex chunkIndex = cilItr->first;
						const IndexSet &is = cilItr->second->m;
						IndexSet::const_iterator isItr = is.begin();

						DebugPrintf(_T("\t\t\t\t\tIndexSet [%3d] : ChunkIndex = %d, size = %d\n"), isIndex, chunkIndex, is.size());

						unsigned int vilIndex = 0;
						for (; isItr != is.end(); ++isItr)
						{
							const VertexIndexList &vil = (*isItr)->m;
							VertexIndexList::const_iterator vilItr = vil.begin();

							DebugPrintf(_T("\t\t\t\t\t\tVertexIndexList [%3d] : size = %d\n"), vilIndex, vil.size());

							unsigned int indexCount = 0;
							for (; vilItr != vil.end(); ++vilItr)
							{
								if (indexCount == 0)
								{
									DebugPrintf(_T("\t\t\t\t\t\t\t"));
								}

								DebugPrintf(_T("%d, "), *vilItr);

								++indexCount;
								if (indexCount >= 10)
								{
									DebugPrintf(_T("\n"));
									indexCount = 0;
								}
							}
						}
					}
				}
			}
		}
	}


//-----------------------------------------------------------------------
	void Scene::DumpNodeChunkMap() const
	//-------------------------------------------------------------------
	{
		DebugPrintf(_T("Scene 0x%8.8x : NodeChunkMap : size = %d\n"), DWORD(this), m_nodeChunkMap.size());

		unsigned int ncmIndex = 0;
		std::vector< _NodeChunkMap * >::const_iterator ncmvItr = m_nodeChunkMap.begin();
		for (; ncmvItr != m_nodeChunkMap.end(); ++ncmvItr, ++ncmIndex)
		{
			DumpNodeChunkMap(*ncmvItr, ncmIndex);
		}
	}


//-----------------------------------------------------------------------
	void Scene::DumpPartitionTree() const
	//-------------------------------------------------------------------
	{
	}


//-----------------------------------------------------------------------
	void Scene::DumpLeafs() const
	//-------------------------------------------------------------------
	{
	}


#endif

// EOF