#include "ksr.h"

#include "leakwatcher.h"

#ifdef _DO_MEMORY_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace KSR;

std::vector< Chunk > *g_pSortingChunks;

//#define VERBOSE_PARTITIONING

//-----------------------------------------------------------------------
	bool SortByTechniqueID(int lhs, int rhs)
	//-------------------------------------------------------------------
	{
		if (!g_pSortingChunks || g_pSortingChunks->empty())
			return false;

		std::vector< Chunk > &v = *g_pSortingChunks;

		return v[lhs].technique > v[rhs].technique;
	}


//-----------------------------------------------------------------------
	Scene::Scene(PSCENESETTINGS pSettings, PDIRECT3DDEVICE9 pDevice, PRESOURCEMANAGER pResourceManager)
	//-------------------------------------------------------------------
	{
		AddUsedMemory(sizeof(Scene), "Scene::Scene()");

		D3DDevice = pDevice;
		resourceManager = pResourceManager;

		settings = new SceneSettings;
		AddUsedMemory(sizeof(SceneSettings), "Scene::Scene() - SceneSettings");

		memset(lightIDs, -1, sizeof(int) * 8);

		SetSettings(pSettings);

		progressCallback = NULL;
		pProgressCallbackPayload = NULL;

		sceneGraphRoot = NULL;
		spacePartitionOutline = NULL;

		Init();
	}


//-----------------------------------------------------------------------
	Scene::~Scene()
	//-------------------------------------------------------------------
	{
		Logf("Destroying Scene...");

		FreeUsedMemory(sizeof(Scene), "Scene::~Scene()");

		if (settings)
		{
			delete settings;
			FreeUsedMemory(sizeof(SceneSettings), "Scene::~Scene() - SceneSettings");
		}

		Clear();
	}


//-----------------------------------------------------------------------
	void Scene::Init()
	//-------------------------------------------------------------------
	{
		subdivisionsExecuted = 0;

		numClusters = 0;
		bytesPerCluster = 0;
		PVS = NULL;

		root = new TreeNode;
		root->m_min = Vector3(0, 0, 0);
		root->m_max = Vector3(0, 0, 0);
		numNodes = 1;

		activeLeafID = -1;

		sceneGraphRoot = new Entity;
		sceneGraphRoot->SetPosition(Vector3(0, 0, 0));
		sceneGraphRoot->SetRotation(Vector3(0, 0, 0));

		spacePartitionProgress = 0;
		optimiseChunksProgress = 0;
		pvsProgress = 0;
	}


//-----------------------------------------------------------------------
	void Scene::Reset(bool recreate, bool resetLights)
	//-------------------------------------------------------------------
	{
		if (root)
		{
			delete root;
			root = NULL;
			numNodes = 0;
		}

		if (PVS)
		{
			delete[] PVS;
			PVS = NULL;

			FreeUsedMemory(numClusters * bytesPerCluster, "Scene()::Clear() - PVS - Clusters");
		}

		std::vector<TreeLeaf *>::iterator q = leafs.begin();
		for (; q != leafs.end(); q++)
		{
			delete *q;
			*q = NULL;
		}

		std::vector< _NodeChunkMap * >::iterator r = nodeChunkMap.begin();
		for (; r != nodeChunkMap.end(); r++)
		{
			delete *r;
			*r = NULL;
		}

		numClusters = 0;
		bytesPerCluster = 0;
		PVS = NULL;

		subdivisionsExecuted = 0;
		numNodes = 0;

		activeLeafID = -1;

		if (resetLights)
		{
			memset(lightIDs, -1, sizeof(int) * 8);
		}

		if (recreate)
		{
			root = new TreeNode;
			root->m_min = Vector3(0, 0, 0);
			root->m_max = Vector3(0, 0, 0);
			numNodes = 1;
		}
	}


//-----------------------------------------------------------------------
	void Scene::Clear()
	//-------------------------------------------------------------------
	{
		Reset();

		if (spacePartitionOutline)
			spacePartitionOutline->Clear();

		if (sceneGraphRoot)
		{
			sceneGraphRoot->Release();
			sceneGraphRoot = NULL;
		}

		worldCollisions.clear();
		bodyCollisions.clear();
	}


//-----------------------------------------------------------------------
	HRESULT Scene::SetSettings(PSCENESETTINGS pSettings)
	//-------------------------------------------------------------------
	{
		if (!pSettings)
			return E_FAIL;

		memcpy(settings, pSettings, sizeof(SceneSettings));

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Scene::SetProgressCallback(ProgressCallbackFunc progressCallbackFunc, void *pPayload)
	//-------------------------------------------------------------------
	{
		if (!progressCallbackFunc)
			return E_FAIL;

		progressCallback = progressCallbackFunc;
		pProgressCallbackPayload = pPayload;

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Scene::SetProgressCallbackPayload(void *pPayload)
	//-------------------------------------------------------------------
	{
		pProgressCallbackPayload = pPayload;

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Scene::SetWorldBounds(Vector3 min, Vector3 max)
	//-------------------------------------------------------------------
	{
		if (!root)
			return E_FAIL;

		root->m_min = min;
		root->m_max = max;

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Scene::SetPosition(Vector3 position)
	//-------------------------------------------------------------------
	{
		if (!sceneGraphRoot)
			return E_FAIL;

		sceneGraphRoot->SetPosition(position);

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

		std::list<PGEOMETRY>::iterator g = geometry.begin();
		for (; g != geometry.end(); g++)
		{
			PVOID pVertsLocked = NULL;
			int *pIndicesLocked = NULL;
			Chunk *chunks = NULL;

			bool useIndices = true;

			if ((*g)->GetNumVertexIndices() < 1)
				useIndices = false;

			if (useIndices)
			{
				if ((*g)->m_indexType == IT_32)
				{
					if (FAILED((*g)->Lock((PVOID*)&chunks, &pVertsLocked, NULL, (PVOID*)&pIndicesLocked)))
						return true;
				}
				else if ((*g)->m_indexType == IT_16)
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

			if (useIndices && (*g)->m_indexType == IT_16)
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

		if (settings->spacePartitionAxis == SS_AXIS_X)
			upVector = Vector3(1, 0, 0);
		else if (settings->spacePartitionAxis == SS_AXIS_Y)
			upVector = Vector3(0, 1, 0);
		else if (settings->spacePartitionAxis == SS_AXIS_Z)
			upVector = Vector3(0, 0, 1);

		if (settings->spacePartitionMode == SS_QUADTREE)
		{

		}
		else if (settings->spacePartitionMode == SS_OCTTREE)
		{
			if (TestVisibility(target, CreateVisibilityFrustum(origin, Vector3( 1,  0,  0), upVector)))
				return true;

			if (TestVisibility(target, CreateVisibilityFrustum(origin, Vector3( 0,  1,  0), upVector)))
				return true;

			if (TestVisibility(target, CreateVisibilityFrustum(origin, Vector3( 0,  0,  1), upVector)))
				return true;

			if (TestVisibility(target, CreateVisibilityFrustum(origin, Vector3(-1,  0,  0), upVector)))
				return true;

			if (TestVisibility(target, CreateVisibilityFrustum(origin, Vector3( 0, -1,  0), upVector)))
				return true;

			if (TestVisibility(target, CreateVisibilityFrustum(origin, Vector3( 0,  0, -1), upVector)))
				return true;
		}

		return false;
	}


//-----------------------------------------------------------------------
	HRESULT Scene::SubdivideWorld()
	//-------------------------------------------------------------------
	{
		// Reset Partitioning information
			Reset(true, false);

			spacePartitionOutline = new Geometry(D3DDevice);

		if (geometry.size() < 1)
			return S_OK;

		// Generate the vertex index buffers
			std::list<PGEOMETRY>::iterator g = geometry.begin();

			root->m_min = (*g)->m_min;
			root->m_max = (*g)->m_max;

			for (; g != geometry.end(); g++)
			{
				if ((*g)->GetNumVertices() < 1)
					continue;

				(*g)->GenerateIndexBuffers(GF_ALLOWZEROCHUNKINDICES);

				Math::SortMinMax((*g)->m_min, (*g)->m_max, root->m_min, root->m_max);
			}

		// Make the space partition cubic
			float m = root->m_max.x - root->m_min.x;

			if (root->m_max.y - root->m_min.y > m)
				m = root->m_max.y - root->m_min.y;

			if (root->m_max.z - root->m_min.z > m)
				m = root->m_max.z - root->m_min.z;

			if (settings->spacePartitionMode == SS_QUADTREE)
			{
				if (settings->spacePartitionAxis == SS_AXIS_X)
				{
					root->m_max.y = root->m_min.y + m;
					root->m_max.z = root->m_min.z + m;
				}
				else if (settings->spacePartitionAxis == SS_AXIS_Y)
				{
					root->m_max.x = root->m_min.x + m;
					root->m_max.z = root->m_min.z + m;
				}
				else if (settings->spacePartitionAxis == SS_AXIS_Z)
				{
					root->m_max.x = root->m_min.x + m;
					root->m_max.y = root->m_min.y + m;
				}
			}
			else if (settings->spacePartitionMode == SS_OCTTREE)
			{
				root->m_max.x = root->m_min.x + m;
				root->m_max.y = root->m_min.y + m;
				root->m_max.z = root->m_min.z + m;
			}

		// If space partition mode is none, force space partition depths to 0.
			if (settings->spacePartitionMode == SS_NONE)
			{
				settings->subdivisionDepth = 0;
				settings->polygonDepth = 0;
				settings->sizeDepth = 0;
			}

		// setup optimised geometry from geometry to optimised geometry
			if (settings->optimiseChunks)
			{
				std::list<PGEOMETRY>::iterator g = geometry.begin();
				for (; g != geometry.end(); g++)
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
				}
			}

		// Partition World
			if (progressCallback)
				progressCallback(PC_SCENE_SPACEPARTITIONING, spacePartitionProgress, pProgressCallbackPayload);

			// Build a temporary root _NodeChunkMap * that contains all the geometry.
			_NodeChunkMap *pRootNodeChunkMap = CreateNodeChunkMap();

			Partition(root, 0, pRootNodeChunkMap);

			delete pRootNodeChunkMap;
			pRootNodeChunkMap = NULL;

			if (progressCallback && settings->optimiseChunks)
				progressCallback(PC_SCENE_OPTIMISECHUNKS, optimiseChunksProgress, pProgressCallbackPayload);

		// Generate Leafs
			std::map< PGEOMETRY, Chunk * > srcChunks;
			std::map< PGEOMETRY, std::vector< unsigned int > > newVertexIndices;

			std::list< PGEOMETRY >::iterator geometryItr = geometry.begin();
			for (; geometryItr != geometry.end(); geometryItr++)
			{
				srcChunks[*geometryItr] = NULL;
			}

			GenerateLeafs(root, srcChunks, newVertexIndices);

			for (geometryItr = geometry.begin(); geometryItr != geometry.end(); geometryItr++)
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

				pGeometry->Insert(0, 0, 0, nviItr->second.size(), 0, NULL, NULL, NULL, (void *)&nviItr->second[0], NULL, 0);
			}

		// Empty source geometry to save memory and copy optimised Geometry buffers to old geometry buffers to retain mapping
			if (settings->optimiseChunks)
			{
				std::list<PGEOMETRY>::iterator g = geometry.begin();
				for (; g != geometry.end(); g++)
				{
					std::list< std::pair<PGEOMETRY, PGEOMETRY> >::iterator o = optimisedGeometry.begin();
					for (; o != optimisedGeometry.end(); )
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
				}
			}

		// Generate PVS Data
			if (settings->visibilityMode == SS_ENABLED)
			{
				if (progressCallback)
					progressCallback(PC_SCENE_PVS, pvsProgress, pProgressCallbackPayload);

				numClusters = leafs.size();
				//numClusters = numNodes;

				bytesPerCluster = (int)ceil((float)numClusters / 8);
				PVS = new byte[numClusters * bytesPerCluster];
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

					RecursePVS(root, (*i));
				}
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

		// Create Draw Partition
			if(spacePartitionOutline)
			{
				TreeNode *node = root;
				std::vector<Vector3> verts;
				std::vector<Chunk> chunks;

				RecurseDrawPartition(root, verts, chunks);

				VERTEX_DIFFUSE_TEX1 *vertices = new VERTEX_DIFFUSE_TEX1[verts.size()];

				for (int i = 0; i < (int)verts.size(); i++)
				{
					vertices[i].position = verts[i];
					vertices[i].color = settings->spacePartitionColour;
					vertices[i].uv = D3DXVECTOR2(0, 0);
				}

				if (FAILED(spacePartitionOutline->Init(sizeof(VERTEX_DIFFUSE_TEX1), FVF_DIFFUSE_TEX1, IT_32)))
					return E_FAIL;

				if (FAILED(spacePartitionOutline->Insert((int)chunks.size(), (int)verts.size(), &chunks[0], vertices, 0)))
					return E_FAIL;

				if (FAILED(spacePartitionOutline->GenerateIndexBuffers(0)))
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
	Scene::_NodeChunkMap *Scene::CreateNodeChunkMap()
	//-------------------------------------------------------------------
	{
		_NodeChunkMap *pResult = new _NodeChunkMap;

		Chunk *chunks = NULL;
		PVOID pVertsLocked = NULL;
		int *pIndicesLocked = NULL;

		// If there is no parent to read from, then use all available geometry.
			bool useIndices = true;

			std::list<PGEOMETRY>::iterator g = geometry.begin();
			for (; g != geometry.end(); g++)
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
						if (pCurGeometry->m_indexType == IT_32)
						{
							if (FAILED(pCurGeometry->Lock((PVOID*)&chunks, &pVertsLocked, NULL, (PVOID*)&pIndicesLocked)))
								return NULL;
						}
						else if (pCurGeometry->m_indexType == IT_16)
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
					for (unsigned int i = 0; i < (unsigned int)pCurGeometry->m_numChunks; i++)
					{
						Chunk &chunk = chunks[i];

						// TODO handle failure

						_GeometryMap *pGeometryMap = pOrderedGeometryMap->GetPtr(chunk.order);

						// TODO handle failure

						_ChunkIndexList *pChunkIndexList = pGeometryMap->GetPtr(pCurGeometry);

						// TODO handle failure

						_VertexIndexList *pVertexIndexList = pChunkIndexList->GetPtr(i);

						if (useIndices)
						{
							for (unsigned int v = 0; v < chunk.numIndices; v++)
							{
								pVertexIndexList->PushBack(pIndicesLocked[chunk.startIndex + v]);
							}
						}
						else
						{
							for (unsigned int v = 0; v < chunk.numVerts; v++)
							{
								pVertexIndexList->PushBack(chunk.startIndex + v);
							}
						}
					}

				pResult->m.push_back(pOrderedGeometryMap);

				// Unlock
					(*g)->Unlock();

					if (useIndices && (*g)->m_indexType == IT_16)
					{
						delete[] pIndicesLocked;
						pIndicesLocked = NULL;
					}
			}

		if (pResult->m.empty())
		{
			delete pResult;
			return NULL;
		}

		return pResult;
	}


//-----------------------------------------------------------------------
	Scene::_NodeChunkMap *Scene::CreateNodeChunkMap(Scene::_NodeChunkMap *pNodeChunkMap, Scene::_NodeChunkMap **ppRemainder, 
													const Math::AABB &bounds, unsigned int *pNumPolygons)
	//-------------------------------------------------------------------
	{
		_NodeChunkMap *pResult = new _NodeChunkMap;

		Chunk *chunks = NULL;
		PVOID pVertsLocked = NULL;
		int *pIndicesLocked = NULL;
		int numPolygons = 0;

		bool useIndices = true;

#ifdef VERBOSE_PARTITIONING
		char buf[256];
		sprintf(buf, "\nCreateNodeChunkMap()\n\tPartitioning Geomtery for node ( min = (%.2f, %.2f, %.2f), max = (%.2f, %.2f, %.2f) )\n",
				bounds.min.x, bounds.min.y, bounds.min.z, bounds.max.x, bounds.max.y, bounds.max.z);
		OutputDebugString(buf);
#endif

		NodeChunkMap::const_iterator ncmItr = pNodeChunkMap->m.begin();
		for (; ncmItr != pNodeChunkMap->m.end(); ncmItr++)
		{
			if ((*ncmItr)->m.empty())
				continue;

			const OrderedGeometryMap &ogm = (*ncmItr)->m;

			OrderedGeometryMap::const_iterator ogmItr = ogm.begin();
			for (; ogmItr != ogm.end(); ogmItr++)
			{
				if (ogmItr->second->m.empty())
					continue;

#ifdef VERBOSE_PARTITIONING
				sprintf(buf, "\tSearching OrderedGeometryMap at order = %d\n", ogmItr->first);
				OutputDebugString(buf);
#endif
				const GeometryMap &gm = ogmItr->second->m;

				GeometryMap::const_iterator gmItr = gm.begin();
				for (; gmItr != gm.end(); gmItr++)
				{
					PGEOMETRY pGeometry = gmItr->first;

					if (!pGeometry)
					{
#ifdef VERBOSE_PARTITIONING
						sprintf(buf, "\tNo Geometry to Partition.\n", ogmItr->first);
						OutputDebugString(buf);
#endif
						continue;
					}

					if (pGeometry->GetNumChunks() < 1 || pGeometry->GetNumVertices() < 1)
					{
#ifdef VERBOSE_PARTITIONING
						sprintf(buf, "\tNo Geometry to Partition.\n", ogmItr->first);
						OutputDebugString(buf);
#endif
						continue;
					}

					if (pGeometry->GetNumVertexIndices() < 1)
						useIndices = false;

					if (gmItr->second->m.empty())
					{
#ifdef VERBOSE_PARTITIONING
						sprintf(buf, "\tNo Geometry to Partition.\n", ogmItr->first);
						OutputDebugString(buf);
#endif
						continue;
					}

					_OrderedGeometryMap *pOrderedGeometryMap = new _OrderedGeometryMap;

					// Lock
						if (useIndices)
						{
							if (pGeometry->m_indexType == IT_32)
							{
								if (FAILED(pGeometry->Lock((PVOID*)&chunks, &pVertsLocked, NULL, (PVOID*)&pIndicesLocked)))
									return NULL;
							}
							else if (pGeometry->m_indexType == IT_16)
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

					// Extract geometry with bounds testing
						const ChunkIndexList &cil = gmItr->second->m;

						ChunkIndexList::const_iterator cilItr = cil.begin();
						for (; cilItr != cil.end(); cilItr++)
						{
#ifdef VERBOSE_PARTITIONING
							sprintf(buf, "\t\tSearching Chunk %d.\n", cilItr->first);
							OutputDebugString(buf);
#endif
							Chunk &chunk = chunks[cilItr->first];

							// TODO: handle failure
							_GeometryMap *pGeometryMap = pOrderedGeometryMap->GetPtr(chunk.order);

							// TODO: handle failure
							_ChunkIndexList *pChunkIndexList = pGeometryMap->GetPtr(pGeometry);

							if (useIndices)
							{
								switch (chunk.type)
								{
								case CT_POLYGON:
									{
										for (unsigned int t = 0; t < chunk.numVerts - 2; t++)
										{
											Vector3 vertices[3];

											unsigned int t0 = pIndicesLocked[chunk.startIndex];
											unsigned int t1 = pIndicesLocked[chunk.startIndex + t + 1];
											unsigned int t2 = pIndicesLocked[chunk.startIndex + t + 2];

											vertices[0] = pGeometry->GetVertexPosition(pVertsLocked, t0);
											vertices[1] = pGeometry->GetVertexPosition(pVertsLocked, t1);
											vertices[2] = pGeometry->GetVertexPosition(pVertsLocked, t2);

											Vector3 normal = Math::Normalise(Math::Cross(vertices[1] - vertices[0], vertices[2] - vertices[0]));

											if (Math::AABBPolygonCollision(bounds, Math::Polygon(normal, vertices, 3), NULL))
											{
												_VertexIndexList *pVertexIndexList = pChunkIndexList->GetPtr(cilItr->first);

												pVertexIndexList->PushBack(t0);
												pVertexIndexList->PushBack(t1);
												pVertexIndexList->PushBack(t2);

												numPolygons++;
											}
										}
									}
									break;

								case CT_TRIANGLELIST:
									{
										for (unsigned int p = 0, t = 0; p < chunk.numVerts / 3; p++, t += 3)
										{
											Vector3 vertices[3];

											unsigned int t0 = pIndicesLocked[chunk.startIndex + t];
											unsigned int t1 = pIndicesLocked[chunk.startIndex + t + 1];
											unsigned int t2 = pIndicesLocked[chunk.startIndex + t + 2];

											vertices[0] = pGeometry->GetVertexPosition(pVertsLocked, t0);
											vertices[1] = pGeometry->GetVertexPosition(pVertsLocked, t1);
											vertices[2] = pGeometry->GetVertexPosition(pVertsLocked, t2);

											// calc the normal for the triangle and test to see if it is in the bounds of this node
											Vector3 normal = Math::Normalise(Math::Cross(vertices[1] - vertices[0], vertices[2] - vertices[0]));
											if (Math::AABBPolygonCollision(bounds, Math::Polygon(normal, vertices, 3), NULL))
											{
												_VertexIndexList *pVertexIndexList = pChunkIndexList->GetPtr(cilItr->first);

												pVertexIndexList->PushBack(t0);
												pVertexIndexList->PushBack(t1);
												pVertexIndexList->PushBack(t2);

												numPolygons++;
											}
										}
									}
									break;

								case CT_TRIANGLESTRIP:
									{
										for (unsigned int p = 2; p < chunk.numVerts; p++)
										{
											Vector3 vertices[3];

											unsigned int t0 = pIndicesLocked[chunk.startIndex + p];
											unsigned int t1 = pIndicesLocked[chunk.startIndex + p - 1];
											unsigned int t2 = pIndicesLocked[chunk.startIndex + p - 2];

											vertices[0] = pGeometry->GetVertexPosition(pVertsLocked, t0);
											vertices[1] = pGeometry->GetVertexPosition(pVertsLocked, t1);
											vertices[2] = pGeometry->GetVertexPosition(pVertsLocked, t2);

											Vector3 normal = Math::Normalise(Math::Cross(vertices[1] - vertices[0], vertices[2] - vertices[0]));
											if (Math::AABBPolygonCollision(bounds, Math::Polygon(normal, vertices, 3), NULL))
											{
												_VertexIndexList *pVertexIndexList = pChunkIndexList->GetPtr(cilItr->first);

												pVertexIndexList->PushBack(t0);
												pVertexIndexList->PushBack(t1);
												pVertexIndexList->PushBack(t2);

												numPolygons++;
											}
										}
									}
									break;

								case CT_LINELIST:
									{
										for (unsigned int p = 0, t = 0; p < chunk.numVerts / 2; p++, t += 2)
										{
											Vector3 vertices[2];

											unsigned int t0 = pIndicesLocked[chunk.startIndex + t];
											unsigned int t1 = pIndicesLocked[chunk.startIndex + t + 1];

											vertices[0] = pGeometry->GetVertexPosition(pVertsLocked, t0);
											vertices[1] = pGeometry->GetVertexPosition(pVertsLocked, t1);

											// HACK: AABBPolygonCollision is inefficient for lines
											Vector3 normal = Math::Normalise(Math::Cross(vertices[1] - vertices[0], vertices[1] - vertices[0]));
											if (Math::AABBPolygonCollision(bounds, Math::Polygon(normal, vertices, 2), NULL))
											{
												_VertexIndexList *pVertexIndexList = pChunkIndexList->GetPtr(cilItr->first);

												pVertexIndexList->PushBack(t0);
												pVertexIndexList->PushBack(t1);

												numPolygons++;
											}
										}
									}
									break;

								case CT_LINESTRIP:
									{
										for (unsigned int p = 1; p < chunk.numVerts; p++)
										{
											Vector3 vertices[2];

											unsigned int t0 = pIndicesLocked[chunk.startIndex + p];
											unsigned int t1 = pIndicesLocked[chunk.startIndex + p - 1];

											vertices[0] = pGeometry->GetVertexPosition(pVertsLocked, t0);
											vertices[1] = pGeometry->GetVertexPosition(pVertsLocked, t1);
	
											// HACK: AABBPolygonCollision is inefficient for lines
											Vector3 normal = Math::Normalise(Math::Cross(vertices[1] - vertices[0], vertices[1] - vertices[0]));
											if (Math::AABBPolygonCollision(bounds, Math::Polygon(normal, vertices, 2), NULL))
											{
												_VertexIndexList *pVertexIndexList = pChunkIndexList->GetPtr(cilItr->first);

												pVertexIndexList->PushBack(t0);
												pVertexIndexList->PushBack(t1);

												numPolygons++;
											}
										}
									}
									break;

								case CT_POINTLIST:
									{
										for (unsigned int p = 0; p < chunk.numVerts; p++)
										{
											Vector3 vertex;

											unsigned int t = pIndicesLocked[chunk.startIndex + p];

											vertex = pGeometry->GetVertexPosition(pVertsLocked, t);

											if (Math::AABBPointCollision(bounds, vertex))
											{
												_VertexIndexList *pVertexIndexList = pChunkIndexList->GetPtr(cilItr->first);

												pVertexIndexList->PushBack(t);

												numPolygons++;
											}
										}
									}
									break;
								}
							}
							else
							{
								switch (chunk.type)
								{
								case CT_POLYGON:
									{
										for (unsigned int t = 0; t < chunk.numVerts - 2; t++)
										{
											Vector3 vertices[3];

											unsigned int t0 = chunk.startIndex;
											unsigned int t1 = t0 + t + 1;
											unsigned int t2 = t1 + 1;

											vertices[0] = pGeometry->GetVertexPosition(pVertsLocked, t0);
											vertices[1] = pGeometry->GetVertexPosition(pVertsLocked, t1);
											vertices[2] = pGeometry->GetVertexPosition(pVertsLocked, t2);

											Vector3 normal = Math::Normalise(Math::Cross(vertices[1] - vertices[0], vertices[2] - vertices[0]));

											if (Math::AABBPolygonCollision(bounds, Math::Polygon(normal, vertices, 3), NULL))
											{
												// Returns the current pointer for this chunk's vertex list. Will allocate it if it doesn't exist.
												_VertexIndexList *pVertexIndexList = pChunkIndexList->GetPtr(cilItr->first);

												pVertexIndexList->PushBack(t0);
												pVertexIndexList->PushBack(t1);
												pVertexIndexList->PushBack(t2);

												numPolygons++;
											}
										}
									}
									break;

								case CT_TRIANGLELIST:
									{
										for (unsigned int p = 0, t = 0; p < chunk.numVerts / 3; p++, t += 3)
										{
											Vector3 vertices[3];

											unsigned int t0 = chunk.startIndex + t;
											unsigned int t1 = t0 + 1;
											unsigned int t2 = t0 + 2;

											vertices[0] = pGeometry->GetVertexPosition(pVertsLocked, t0);
											vertices[1] = pGeometry->GetVertexPosition(pVertsLocked, t1);
											vertices[2] = pGeometry->GetVertexPosition(pVertsLocked, t2);

											// calc the normal for the triangle and test to see if it is in the bounds of this node
											Vector3 normal = Math::Normalise(Math::Cross(vertices[1] - vertices[0], vertices[2] - vertices[0]));
											if (Math::AABBPolygonCollision(bounds, Math::Polygon(normal, vertices, 3), NULL))
											{
												_VertexIndexList *pVertexIndexList = pChunkIndexList->GetPtr(cilItr->first);

												pVertexIndexList->PushBack(t0);
												pVertexIndexList->PushBack(t1);
												pVertexIndexList->PushBack(t2);

												numPolygons++;
											}
										}
									}
									break;

								case CT_TRIANGLESTRIP:
									{
										for (unsigned int p = 2; p < chunk.numVerts; p++)
										{
											Vector3 vertices[3];

											unsigned int t0 = chunk.startIndex + p;
											unsigned int t1 = t0 - 1;
											unsigned int t2 = t0 - 2;

											vertices[0] = pGeometry->GetVertexPosition(pVertsLocked, t0);
											vertices[1] = pGeometry->GetVertexPosition(pVertsLocked, t1);
											vertices[2] = pGeometry->GetVertexPosition(pVertsLocked, t2);

											Vector3 normal = Math::Normalise(Math::Cross(vertices[1] - vertices[0], vertices[2] - vertices[0]));
											if (Math::AABBPolygonCollision(bounds, Math::Polygon(normal, vertices, 3), NULL))
											{
												_VertexIndexList *pVertexIndexList = pChunkIndexList->GetPtr(cilItr->first);

												pVertexIndexList->PushBack(t0);
												pVertexIndexList->PushBack(t1);
												pVertexIndexList->PushBack(t2);

												numPolygons++;
											}
										}
									}
									break;

								case CT_LINELIST:
									{
										for (unsigned int p = 0, t = 0; p < chunk.numVerts / 2; p++, t += 2)
										{
											Vector3 vertices[2];

											unsigned int t0 = chunk.startIndex + t;
											unsigned int t1 = t0 + 1;

											vertices[0] = pGeometry->GetVertexPosition(pVertsLocked, t0);
											vertices[1] = pGeometry->GetVertexPosition(pVertsLocked, t1);

											// HACK: AABBPolygonCollision is inefficient for lines
											Vector3 normal = Math::Normalise(Math::Cross(vertices[1] - vertices[0], vertices[1] - vertices[0]));
											if (Math::AABBPolygonCollision(bounds, Math::Polygon(normal, vertices, 2), NULL))
											{
												_VertexIndexList *pVertexIndexList = pChunkIndexList->GetPtr(cilItr->first);

												pVertexIndexList->PushBack(t0);
												pVertexIndexList->PushBack(t1);

												numPolygons++;
											}
										}
									}
									break;

								case CT_LINESTRIP:
									{
										for (unsigned int p = 1; p < chunk.numVerts; p++)
										{
											Vector3 vertices[2];

											unsigned int t0 = chunk.startIndex + p;
											unsigned int t1 = t0 - 1;

											vertices[0] = pGeometry->GetVertexPosition(pVertsLocked, t0);
											vertices[1] = pGeometry->GetVertexPosition(pVertsLocked, t1);
	
											// HACK: AABBPolygonCollision is inefficient for lines
											Vector3 normal = Math::Normalise(Math::Cross(vertices[1] - vertices[0], vertices[1] - vertices[0]));
											if (Math::AABBPolygonCollision(bounds, Math::Polygon(normal, vertices, 2), NULL))
											{
												_VertexIndexList *pVertexIndexList = pChunkIndexList->GetPtr(cilItr->first);

												pVertexIndexList->PushBack(t0);
												pVertexIndexList->PushBack(t1);

												numPolygons++;
											}
										}
									}
									break;

								case CT_POINTLIST:
									{
										for (unsigned int p = 0; p < chunk.numVerts; p++)
										{
											Vector3 vertex;

											unsigned int t = chunk.startIndex + p;

											vertex = pGeometry->GetVertexPosition(pVertsLocked, t);

											if (Math::AABBPointCollision(bounds, vertex))
											{
												_VertexIndexList *pVertexIndexList = pChunkIndexList->GetPtr(cilItr->first);

												pVertexIndexList->PushBack(t);

												numPolygons++;
											}
										}
									}
									break;
								}
							}
						}

					pResult->m.push_back(pOrderedGeometryMap);

					// Unlock
						pGeometry->Unlock();

						if (useIndices && pGeometry->m_indexType == IT_16)
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
	void Scene::Partition(TreeNode *node, int numSubdivisions, _NodeChunkMap *pNodeChunkMap)
	//-------------------------------------------------------------------
	{
#ifdef VERBOSE_PARTITIONING
		char buf[256];
		sprintf(buf, "\nPartition()\n\tNode ( min = (%.2f, %.2f, %.2f), max = (%.2f, %.2f, %.2f) )\n",
				node->m_min.x, node->m_min.y, node->m_min.z, node->m_max.x, node->m_max.y, node->m_max.z);
		OutputDebugString(buf);
#endif

		if (!pNodeChunkMap)
			return;

		if (pNodeChunkMap->IsEmpty())
		{
			// There is no geometry for this leaf to attempt to partition, so we can create an
			// empty leaf and return early. Must take a copy of pNodeChunkMap, so that it is cleared up properly.

#ifdef VERBOSE_PARTITIONING
			sprintf(buf, "\tpNodeChunkMap is empty. Storing empty leaf.\n");
			OutputDebugString(buf);
#endif

			node->m_leafIndex = nodeChunkMap.size();

			_NodeChunkMap *pNewNodeChunkMap = new _NodeChunkMap(*pNodeChunkMap);

			nodeChunkMap.push_back(pNewNodeChunkMap);

			UpdateSpacePartitionProgress(node);

			return;
		}

		// Setup local variables
			int numDivisions = numSubdivisions;

			if (numDivisions > subdivisionsExecuted)
				subdivisionsExecuted = numDivisions;

			unsigned int numPolygons = 0;
			int numEffectGroups = resourceManager->GetNumEffects();

			Vector3 min = node->m_min;
			Vector3 max = node->m_max;

			Chunk *chunks = NULL;
			PVOID pVertsLocked = NULL;
			int *pIndicesLocked = NULL;

			node->m_leafIndex = -1;

		// Partition Geometry
			_NodeChunkMap *pResultingNodeChunkMap = NULL;

			if (settings->optimiseChunks)
			{
				
			}
			else
			{
				pResultingNodeChunkMap = CreateNodeChunkMap(pNodeChunkMap, NULL, Math::AABB(min, max), &numPolygons);
			}

			if (!pResultingNodeChunkMap)
			{
				// TODO: Handle error. Cannot continue.
			}

		// Determine if further subdivision is necessary
			float sizeX = max.x - min.x;
			float sizeY = max.y - min.y;
			float sizeZ = max.z - min.z;

			bool subdivideSize = false;
			bool subdividePoly = false;
			bool subdivideSub  = false;

			if (settings->sizeDepth > 0)
			{
				if (settings->spacePartitionAxis == SS_AXIS_X)
				{
					if (settings->sizeDepth < sizeY || settings->sizeDepth < sizeZ || (settings->spacePartitionMode == SS_OCTTREE && settings->sizeDepth < sizeX))
						subdivideSize = true;
				}
				else if (settings->spacePartitionAxis == SS_AXIS_Y)
				{
					if (settings->sizeDepth < sizeX || settings->sizeDepth < sizeZ || (settings->spacePartitionMode == SS_OCTTREE && settings->sizeDepth < sizeY))
						subdivideSize = true;
				}
				else if (settings->spacePartitionAxis == SS_AXIS_Z)
				{
					if (settings->sizeDepth < sizeX || settings->sizeDepth < sizeY || (settings->spacePartitionMode == SS_OCTTREE && settings->sizeDepth < sizeZ))
						subdivideSize = true;
				}
			}
			else
				subdivideSize = true;

			if (settings->polygonDepth > 0)
			{
				if (settings->polygonDepth < (int)numPolygons)
					subdividePoly = true;
			}
			else
				subdividePoly = true;

			if (settings->subdivisionDepth > 0)
			{
				if (numDivisions < settings->subdivisionDepth)
					subdivideSub = true;
			}
			else
				subdivideSub = true;

		// Subdivide if so
			if (subdivideSize && subdividePoly && subdivideSub && (settings->polygonDepth || settings->sizeDepth || settings->subdivisionDepth))
			{
#ifdef VERBOSE_PARTITIONING
				sprintf(buf, "\tSubdividing.\n");
				OutputDebugString(buf);
#endif
				numDivisions++;

				Vector3 mid(0, 0, 0);

				if (settings->spacePartitionMode == SS_QUADTREE)
				{
					node->m_pChildren = new TreeNode *[4];
					node->m_numChildren = 4;

					if (settings->spacePartitionAxis == SS_AXIS_X)
						mid = min + Vector3(sizeX, sizeY * 0.5f, sizeZ * 0.5f);
					else if (settings->spacePartitionAxis == SS_AXIS_Y)
						mid = min + Vector3(sizeX * 0.5f, sizeY, sizeZ * 0.5f);
					else if (settings->spacePartitionAxis == SS_AXIS_Z)
						mid = min + Vector3(sizeX * 0.5f, sizeY * 0.5f, sizeZ);
				}
				else if (settings->spacePartitionMode == SS_OCTTREE)
				{
					node->m_pChildren = new TreeNode *[8];
					node->m_numChildren = 8;
					mid = min + Vector3(sizeX * 0.5f, sizeY * 0.5f, sizeZ * 0.5f);
				}

				if (settings->spacePartitionMode == SS_QUADTREE)
				{
					if (settings->spacePartitionAxis == SS_AXIS_X)
					{
						node->m_pChildren[0] = new TreeNode(min, mid, node, numNodes);
						numNodes++;
						Partition(node->m_pChildren[0], numDivisions, pResultingNodeChunkMap);

						node->m_pChildren[1] = new TreeNode(Vector3(min.x, mid.y, min.z), Vector3(mid.x, max.y, mid.z), node, numNodes);
						numNodes++;
						Partition(node->m_pChildren[1], numDivisions, pResultingNodeChunkMap);

						node->m_pChildren[2] = new TreeNode(Vector3(min.x, min.y, mid.z), Vector3(mid.x, mid.y, max.z), node, numNodes);
						numNodes++;
						Partition(node->m_pChildren[2], numDivisions, pResultingNodeChunkMap);

						node->m_pChildren[3] = new TreeNode(Vector3(min.x, mid.y, mid.z), Vector3(mid.x, max.y, max.z), node, numNodes);
						numNodes++;
						Partition(node->m_pChildren[3], numDivisions, pResultingNodeChunkMap);
					}
					else if (settings->spacePartitionAxis == SS_AXIS_Y)
					{
						node->m_pChildren[0] = new TreeNode(min, mid, node, numNodes);
						numNodes++;
						Partition(node->m_pChildren[0], numDivisions, pResultingNodeChunkMap);

						node->m_pChildren[1] = new TreeNode(Vector3(mid.x, min.y, min.z), Vector3(max.x, mid.y, mid.z), node, numNodes);
						numNodes++;
						Partition(node->m_pChildren[1], numDivisions, pResultingNodeChunkMap);

						node->m_pChildren[2] = new TreeNode(Vector3(min.x, min.y, mid.z), Vector3(mid.x, mid.y, max.z), node, numNodes);
						numNodes++;
						Partition(node->m_pChildren[2], numDivisions, pResultingNodeChunkMap);

						node->m_pChildren[3] = new TreeNode(Vector3(mid.x, min.y, mid.z), Vector3(max.x, mid.y, max.z), node, numNodes);
						numNodes++;
						Partition(node->m_pChildren[3], numDivisions, pResultingNodeChunkMap);
					}
					else if (settings->spacePartitionAxis == SS_AXIS_Z)
					{
						node->m_pChildren[0] = new TreeNode(min, mid, node, numNodes);
						numNodes++;
						Partition(node->m_pChildren[0], numDivisions, pResultingNodeChunkMap);

						node->m_pChildren[1] = new TreeNode(Vector3(mid.x, min.y, min.z), Vector3(max.x, mid.y, mid.z), node, numNodes);
						numNodes++;
						Partition(node->m_pChildren[1], numDivisions, pResultingNodeChunkMap);

						node->m_pChildren[2] = new TreeNode(Vector3(min.x, mid.y, min.z), Vector3(mid.x, max.y, mid.z), node, numNodes);
						numNodes++;
						Partition(node->m_pChildren[2], numDivisions, pResultingNodeChunkMap);

						node->m_pChildren[3] = new TreeNode(Vector3(mid.x, mid.y, min.z), Vector3(max.x, max.y, mid.z), node, numNodes);
						numNodes++;
						Partition(node->m_pChildren[3], numDivisions, pResultingNodeChunkMap);
					}
				}
				else
				{
					node->m_pChildren[0] = new TreeNode(min, mid, node, numNodes);
					numNodes++;
					Partition(node->m_pChildren[0], numDivisions, pResultingNodeChunkMap);

					node->m_pChildren[1] = new TreeNode(Vector3(mid.x, min.y, min.z), Vector3(max.x, mid.y, mid.z), node, numNodes);
					numNodes++;
					Partition(node->m_pChildren[1], numDivisions, pResultingNodeChunkMap);

					node->m_pChildren[2] = new TreeNode(Vector3(min.x, min.y, mid.z), Vector3(mid.x, mid.y, max.z), node, numNodes);
					numNodes++;
					Partition(node->m_pChildren[2], numDivisions, pResultingNodeChunkMap);

					node->m_pChildren[3] = new TreeNode(Vector3(mid.x, min.y, mid.z), Vector3(max.x, mid.y, max.z), node, numNodes);
					numNodes++;
					Partition(node->m_pChildren[3], numDivisions, pResultingNodeChunkMap);

					node->m_pChildren[4] = new TreeNode(Vector3(min.x, mid.y, min.z), Vector3(mid.x, max.y, mid.z), node, numNodes);
					numNodes++;
					Partition(node->m_pChildren[4], numDivisions, pResultingNodeChunkMap);

					node->m_pChildren[5] = new TreeNode(Vector3(mid.x, mid.y, min.z), Vector3(max.x, max.y, mid.z), node, numNodes);
					numNodes++;
					Partition(node->m_pChildren[5], numDivisions, pResultingNodeChunkMap);

					node->m_pChildren[6] = new TreeNode(Vector3(min.x, mid.y, mid.z), Vector3(mid.x, max.y, max.z), node, numNodes);
					numNodes++;
					Partition(node->m_pChildren[6], numDivisions, pResultingNodeChunkMap);

					node->m_pChildren[7] = new TreeNode(Vector3(mid.x, mid.y, mid.z), Vector3(max.x, max.y, max.z), node, numNodes);
					numNodes++;
					Partition(node->m_pChildren[7], numDivisions, pResultingNodeChunkMap);
				}
			}
			else
			{
				// The node is not being partitioned, so place the current geometry in this leaf.

#ifdef VERBOSE_PARTITIONING
				sprintf(buf, "\tNot Subdividing. Creating the leaf.\n");
				OutputDebugString(buf);
#endif

				node->m_leafIndex = nodeChunkMap.size();

				_NodeChunkMap *pNewNodeChunkMap = new _NodeChunkMap(*pResultingNodeChunkMap);

				nodeChunkMap.push_back(pNewNodeChunkMap);

				UpdateSpacePartitionProgress(node);
			}

		delete pResultingNodeChunkMap;
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

		std::list<PGEOMETRY>::iterator g = geometry.begin();
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
							if (settings->optimiseChunks)
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
							if (settings->optimiseChunks)
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
							if (settings->optimiseChunks)
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
							if (settings->optimiseChunks)
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
							if (settings->optimiseChunks)
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
							if (settings->optimiseChunks)
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

		if (settings->sizeDepth > 0)
		{
			if (settings->spacePartitionAxis == SS_AXIS_X)
			{
				if (settings->sizeDepth < y || settings->sizeDepth < z || (settings->spacePartitionMode == SS_OCTTREE && settings->sizeDepth < x))
					subdivideSize = true;
			}
			else if (settings->spacePartitionAxis == SS_AXIS_Y)
			{
				if (settings->sizeDepth < x || settings->sizeDepth < z || (settings->spacePartitionMode == SS_OCTTREE && settings->sizeDepth < y))
					subdivideSize = true;
			}
			else if (settings->spacePartitionAxis == SS_AXIS_Z)
			{
				if (settings->sizeDepth < x || settings->sizeDepth < y || (settings->spacePartitionMode == SS_OCTTREE && settings->sizeDepth < z))
					subdivideSize = true;
			}
		}
		else
			subdivideSize = true;

		if (settings->polygonDepth > 0)
		{
			if (settings->polygonDepth < numPolygons)
				subdividePoly = true;
		}
		else
			subdividePoly = true;

		if (settings->subdivisionDepth > 0)
		{
			if (numDivisions < settings->subdivisionDepth)
				subdivideSub = true;
		}
		else
			subdivideSub = true;

		if (subdivideSize && subdividePoly && subdivideSub && (settings->polygonDepth || settings->sizeDepth || settings->subdivisionDepth))
		{
			int childNumDivisions = numDivisions + 1;

			delete pNodeGeometryChunkMap;
			pNodeGeometryChunkMap = NULL;

			Vector3 mid(0, 0, 0);

			if (settings->spacePartitionMode == SS_QUADTREE)
			{
				node->children = new TreeNode *[4];
				node->numChildren = 4;

				if (settings->spacePartitionAxis == SS_AXIS_X)
					mid = min + Vector3(x, y * 0.5f, z * 0.5f);
				else if (settings->spacePartitionAxis == SS_AXIS_Y)
					mid = min + Vector3(x * 0.5f, y, z * 0.5f);
				else if (settings->spacePartitionAxis == SS_AXIS_Z)
					mid = min + Vector3(x * 0.5f, y * 0.5f, z);
			}
			else if (settings->spacePartitionMode == SS_OCTTREE)
			{
				node->children = new TreeNode *[8];
				node->numChildren = 8;
				mid = min + Vector3(x * 0.5f, y * 0.5f, z * 0.5f);
			}

			if (settings->spacePartitionMode == SS_QUADTREE)
			{
				if (settings->spacePartitionAxis == SS_AXIS_X)
				{
					node->children[0] = new TreeNode(min, mid, node, numNodes);
					numNodes++;
					Partition(node->children[0], childNumDivisions);

					node->children[1] = new TreeNode(Vector3(min.x, mid.y, min.z), Vector3(mid.x, max.y, mid.z), node, numNodes);
					numNodes++;
					Partition(node->children[1], childNumDivisions);

					node->children[2] = new TreeNode(Vector3(min.x, min.y, mid.z), Vector3(mid.x, mid.y, max.z), node, numNodes);
					numNodes++;
					Partition(node->children[2], childNumDivisions);

					node->children[3] = new TreeNode(Vector3(min.x, mid.y, mid.z), Vector3(mid.x, max.y, max.z), node, numNodes);
					numNodes++;
					Partition(node->children[3], childNumDivisions);
				}
				else if (settings->spacePartitionAxis == SS_AXIS_Y)
				{
					node->children[0] = new TreeNode(min, mid, node, numNodes);
					numNodes++;
					Partition(node->children[0], childNumDivisions);

					node->children[1] = new TreeNode(Vector3(mid.x, min.y, min.z), Vector3(max.x, mid.y, mid.z), node, numNodes);
					numNodes++;
					Partition(node->children[1], childNumDivisions);

					node->children[2] = new TreeNode(Vector3(min.x, min.y, mid.z), Vector3(mid.x, mid.y, max.z), node, numNodes);
					numNodes++;
					Partition(node->children[2], childNumDivisions);

					node->children[3] = new TreeNode(Vector3(mid.x, min.y, mid.z), Vector3(max.x, mid.y, max.z), node, numNodes);
					numNodes++;
					Partition(node->children[3], childNumDivisions);
				}
				else if (settings->spacePartitionAxis == SS_AXIS_Z)
				{
					node->children[0] = new TreeNode(min, mid, node, numNodes);
					numNodes++;
					Partition(node->children[0], childNumDivisions);

					node->children[1] = new TreeNode(Vector3(mid.x, min.y, min.z), Vector3(max.x, mid.y, mid.z), node, numNodes);
					numNodes++;
					Partition(node->children[1], childNumDivisions);

					node->children[2] = new TreeNode(Vector3(min.x, mid.y, min.z), Vector3(mid.x, max.y, mid.z), node, numNodes);
					numNodes++;
					Partition(node->children[2], childNumDivisions);

					node->children[3] = new TreeNode(Vector3(mid.x, mid.y, min.z), Vector3(max.x, max.y, mid.z), node, numNodes);
					numNodes++;
					Partition(node->children[3], childNumDivisions);
				}
			}
			else
			{
				node->children[0] = new TreeNode(min, mid, node, numNodes);
				numNodes++;
				Partition(node->children[0], childNumDivisions);

				node->children[1] = new TreeNode(Vector3(mid.x, min.y, min.z), Vector3(max.x, mid.y, mid.z), node, numNodes);
				numNodes++;
				Partition(node->children[1], childNumDivisions);

				node->children[2] = new TreeNode(Vector3(min.x, min.y, mid.z), Vector3(mid.x, mid.y, max.z), node, numNodes);
				numNodes++;
				Partition(node->children[2], childNumDivisions);

				node->children[3] = new TreeNode(Vector3(mid.x, min.y, mid.z), Vector3(max.x, mid.y, max.z), node, numNodes);
				numNodes++;
				Partition(node->children[3], childNumDivisions);

				node->children[4] = new TreeNode(Vector3(min.x, mid.y, min.z), Vector3(mid.x, max.y, mid.z), node, numNodes);
				numNodes++;
				Partition(node->children[4], childNumDivisions);

				node->children[5] = new TreeNode(Vector3(mid.x, mid.y, min.z), Vector3(max.x, max.y, mid.z), node, numNodes);
				numNodes++;
				Partition(node->children[5], childNumDivisions);

				node->children[6] = new TreeNode(Vector3(min.x, mid.y, mid.z), Vector3(mid.x, max.y, max.z), node, numNodes);
				numNodes++;
				Partition(node->children[6], childNumDivisions);

				node->children[7] = new TreeNode(Vector3(mid.x, mid.y, mid.z), Vector3(max.x, max.y, max.z), node, numNodes);
				numNodes++;
				Partition(node->children[7], childNumDivisions);
			}
		}
		else
		{
			node->leafIndex = nodeChunkMap.size();
			nodeChunkMap.push_back(pNodeGeometryChunkMap);

			if (node->leafIndex > -1)
			{
				float nodeVolume = (node->max.x - node->min.x) * (node->max.y - node->min.y) * (node->max.z - node->min.z);
				float rootVolume = (root->max.x - root->min.x) * (root->max.y - root->min.y) * (root->max.z - root->min.z);

				if (rootVolume > 0)
				{
					spacePartitionProgress += (nodeVolume / rootVolume) * 100.0f;

					if (progressCallback)
						progressCallback(PC_SCENE_SPACEPARTITIONING, spacePartitionProgress, pProgressCallbackPayload);
				}
			}
		}

#ifdef DEBUG_DUMP
		{
			OutputDebugString("\n-----------------------------------------------------------\n");
			std::vector< _NodeChunkMap * >::iterator z = nodeChunkMap.begin();
			for (int n0 = 0; z != nodeChunkMap.end(); z++, n0++)
			{
				char buf[256];

				sprintf(buf, "NodeChunkMap(0x%8.8X)[%d]: =\n", *z, n0);
				OutputDebugString(buf);

				_NodeChunkMap *pNode = *z;

				NodeChunkMap::iterator y = pNode->m.begin();
				for (int n1 = 0; y != pNode->m.end(); y++, n1++)
				{
					_OrderedGeometryMap *pOGM = (*y);
					sprintf(buf, "\tOrderedGeometryMap(0x%8.8X)[%d]: =\n", pOGM, n1);
					OutputDebugString(buf);

					OrderedGeometryMap::iterator ogmItr = pOGM->m.begin();
					for (; ogmItr != pOGM->m.end(); ogmItr++)
					{
						_GeometryMap *pGM = ogmItr->second;
						sprintf(buf, "\t\tOrder(0x%8.8X) = %d (size = %d)\n", pGM, ogmItr->first, pGM->Size());
						OutputDebugString(buf);

						GeometryMap::iterator gmItr = pGM->m.begin();
						for (; gmItr != pGM->m.end(); gmItr++)
						{
							_ChunkIndexList *pCIList = gmItr->second;
							sprintf(buf, "\t\t\tPGEOMETRY = 0x%8.8X (size = %d), ChunkIndexList = (0x%8.8X)\n", gmItr->first, pCIList->Size(), pCIList);
							OutputDebugString(buf);

							ChunkIndexList::iterator ciListItr = pCIList->m.begin();
							for (; ciListItr != pCIList->m.end(); ciListItr++)
							{
								_VertexIndexList *pVIList = ciListItr->second;
								sprintf(buf, "\t\t\t\tChunk Index = %d (size = %d), VIList = (0x%8.8X)\n", ciListItr->first, pVIList->Size(), pVIList);
								OutputDebugString(buf);

								VertexIndexList::iterator viListItr = pVIList->m.begin();
								OutputDebugString("\t\t\t\t\t[");
								for(int n2 = 0; viListItr != pVIList->m.end(); viListItr++, n2++)
								{
									sprintf(buf, " %d,", *viListItr);
								}

								OutputDebugString("]\n");
							}
						}
					}
				}
			}

			OutputDebugString("\n===========================================================\n");
		}

		depth--;
		OutputDebugString("Partition() :: returning due completion.\n");
#endif
	}
#endif


//-----------------------------------------------------------------------
	void Scene::GenerateLeafs(TreeNode *node, std::map< PGEOMETRY, Chunk * > &srcChunks, std::map< PGEOMETRY, std::vector< unsigned int > > &newVertexIndices)
	//-------------------------------------------------------------------
	{
		if (!node)
			return;

		int numEffectGroups = resourceManager->GetNumEffects();

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

				node->m_leafIndex = leafs.size();

				leafs.push_back(leaf);
				AddUsedMemory(sizeof(TreeLeaf), "Scene::GenerateLeafs() - TreeLeaf...");

			if (settings->optimiseChunks)
			{
				DebugPrintf("GenerateLeafs(): Skipping Optimise Chunks case.");
			}
			else
			{
				NodeChunkMap &ncm = nodeChunkMap[node->m_leafIndex]->m;

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

							pGeometry->Insert(newChunks.size(), 0, 0, 0, 0, &newChunks[0], NULL, NULL, NULL, NULL, 0);

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

								pGeometry->Insert(0, 0, effectGroups[e].size(), 0, 0, NULL, NULL, chunkIndices, NULL, NULL, 0);

								delete[] chunkIndices;
							}

							//g_pSortingChunks = NULL;
						}
					}
				}
			}
		}
	}


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

			if (settings->optimiseChunks)
			{
				DebugPrintf("GenerateLeafs(): Skipping Optimise Chunks case.");
			}
			else
			{
				if (chunkMapIndex >= (int)nodeChunkMap.size())
				{
					DebugPrintf("Error: chunkMapIndex exceeds nodeChunkMap.size().");
					return;
				}

				if (nodeChunkMap[chunkMapIndex]->m.empty())
				{
					DebugPrintf("Error: nodeChunkMap[chunkMapIndex] is empty.");
					return;
				}

				std::map< PGEOMETRY, std::vector< unsigned int > > newVertexIndices;

				NodeChunkMap &ncm = nodeChunkMap[chunkMapIndex]->m;

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

#if 0
//-----------------------------------------------------------------------
	void Scene::GenerateLeafs(TreeNode *node)
	//-------------------------------------------------------------------
	{
		if (!node)
			return;

		int numEffectGroups = resourceManager->GetNumEffects();

		if (node->numChildren == 4)
		{
			GenerateLeafs(node->children[0]);
			GenerateLeafs(node->children[1]);
			GenerateLeafs(node->children[2]);
			GenerateLeafs(node->children[3]);
		}
		else if (node->numChildren == 8)
		{
			GenerateLeafs(node->children[0]);
			GenerateLeafs(node->children[1]);
			GenerateLeafs(node->children[2]);
			GenerateLeafs(node->children[3]);
			GenerateLeafs(node->children[4]);
			GenerateLeafs(node->children[5]);
			GenerateLeafs(node->children[6]);
			GenerateLeafs(node->children[7]);
		}
		else if (node->leafIndex > -1)
		{
			DebugPrintf("\nCreating leaf %d for node ( min = (%.2f, %.2f, %.2f), max = (%.2f, %.2f, %.2f) )\n",
						node->leafIndex, node->min.x, node->min.y, node->min.z, node->max.x, node->max.y, node->max.z);

			int chunkMapIndex = node->leafIndex;

			// Setup Leaf
				TreeLeaf *leaf = new TreeLeaf(node);
				leaf->min = node->min;
				leaf->max = node->max;

				node->leafIndex = leafs.size();

				leafs.push_back(leaf);
				AddUsedMemory(sizeof(TreeLeaf), "Scene::GenerateLeafs() - TreeLeaf...");

			if (settings->optimiseChunks)
			{
				//OptimiseByOrder(node);

				NodeChunkMap &ncm = nodeChunkMap[chunkMapIndex]->m;

				NodeChunkMap::iterator i = ncm.begin();
				for (; i != ncm.end(); i++)
				{
					OrderedGeometryMap &ogm = (*i)->m;

					OrderedGeometryMap::iterator j = ogm.begin();
					for (; j != ogm.end(); j++)
					{
						GeometryMap &gm = j->second->m;

						GeometryMap::iterator k = gm.begin();
						for (; k != gm.end(); k++)
						{
							if (!k->first || k->first->GetNumChunks() < 1 || k->first->GetNumVertices() < 1)
								continue;

							PGEOMETRY pGeometry = NULL;

							std::list< std::pair< PGEOMETRY, PGEOMETRY > >::iterator o = optimisedGeometry.begin();
							for (; o != optimisedGeometry.end(); o++)
							{
								std::pair< PGEOMETRY, PGEOMETRY > &gPair = *o;

								if (o->first == k->first)
								{
									pGeometry = o->second;
									break;
								}

								Chunk *chunks = NULL;

								if (FAILED(k->first->Lock((PVOID*)&chunks, NULL, NULL, NULL)))
									return;

								std::vector< std::vector<int> > effectGroups;
								effectGroups.resize(numEffectGroups + 1);

								for (int e = 0; e < numEffectGroups + 1; e++)
								{
									
								}
							}
						}
					}
				}
#if 0
				{
				std::vector< GeometryMap >::iterator i = nodeChunkMap[chunkMapIndex].begin();

				for (; i != nodeChunkMap[chunkMapIndex].end(); i++)
				{
#ifdef _DEBUG
					GeometryMap &gmRef = *i;
#endif
					if (i->first->GetNumChunks() < 1 || i->first->GetNumVertices() < 1)
						continue;

					PGEOMETRY pGeometry = NULL;

					std::list< std::pair<PGEOMETRY, PGEOMETRY> >::iterator o = optimisedGeometry.begin();
					for (; o != optimisedGeometry.end(); o++)
					{
#ifdef _DEBUG
						std::pair<PGEOMETRY, PGEOMETRY> &geomPaiRef = *o;
#endif
						if (o->first == i->first)
						{
							pGeometry = o->second;
							break;
						}
					}

					Chunk *chunks = NULL;

					if (FAILED(i->first->Lock((PVOID*)&chunks, NULL, NULL, NULL)))
						return;

					std::vector< std::vector<int> > effectGroups;
					effectGroups.resize(numEffectGroups + 1);

					for (int e = 0; e < numEffectGroups + 1; e++)
					{
						std::vector< std::pair< std::pair< int, int >, int > > triangleList;
						std::vector< std::pair< std::pair< int, int >, int > > lineList;
						std::vector< std::pair< std::pair< int, int >, int > > pointList;

						//std::vector< std::vector< std::vector< int > > >::iterator c = i->second.begin();
						std::vector< std::vector< int > >::iterator c = i->second.begin();
						for (int f = 0; c != i->second.end(); f++, c++)
						{
#ifdef _DEBUG
							//std::vector< std::vector< int > > &vOfVofIntsRef = *c;
#endif
							if ((*c).size() < 1)
								continue;

							if (chunks[f].effect != e - 1)
								continue;

#ifdef _DEBUG
							std::vector< int >::iterator p = (*c)[0].begin();
							for (; p != (*c)[0].end(); p++)
							{
								int &iVal = *p;
								triangleList.push_back(std::make_pair(std::make_pair(chunks[f].idTexture0, chunks[f].idTexture1), (*p)));
							}

							p = (*c)[1].begin();
							for (; p != (*c)[1].end(); p++)
							{
								int &iVal = *p;
								lineList.push_back(std::make_pair(std::make_pair(chunks[f].idTexture0, chunks[f].idTexture1), (*p)));
							}

							p = (*c)[2].begin();
							for (; p != (*c)[2].end(); p++)
							{
								int &iVal = *p;
								pointList.push_back(std::make_pair(std::make_pair(chunks[f].idTexture0, chunks[f].idTexture1), (*p)));
							}
#else
							std::vector< int >::iterator p = (*c)[0].begin();
							for (; p != (*c)[0].end(); p++)
								triangleList.push_back(std::make_pair(std::make_pair(chunks[f].idTexture0, chunks[f].idTexture1), (*p)));

							p = (*c)[1].begin();
							for (; p != (*c)[1].end(); p++)
								lineList.push_back(std::make_pair(std::make_pair(chunks[f].idTexture0, chunks[f].idTexture1), (*p)));

							p = (*c)[2].begin();
							for (; p != (*c)[2].end(); p++)
								pointList.push_back(std::make_pair(std::make_pair(chunks[f].idTexture0, chunks[f].idTexture1), (*p)));
#endif
						}

						std::vector< std::pair< std::pair< int, int >, std::vector< int > > > sortedTriangleList;
						std::vector< std::pair< std::pair< int, int >, std::vector< int > > > sortedLineList;
						std::vector< std::pair< std::pair< int, int >, std::vector< int > > > sortedPointList;

						// Create Sorted Triangle List
						std::vector< std::pair< std::pair< int, int >, int > >::iterator t = triangleList.begin();
						for (; t != triangleList.end(); t++)
						{
#ifdef _DEBUG
							std::pair< std::pair< int, int >, int > &unsortedTriPairOfPair = *t;
#endif
							std::vector< std::pair< std::pair< int, int >, std::vector< int > > >::iterator s = sortedTriangleList.begin();
							for (; s != sortedTriangleList.end(); s++)
							{
#ifdef _DEBUG
								std::pair< std::pair< int, int >, std::vector< int > > &sortedTriPairOfPair = *s;
#endif
								if ((*t).first.first != (*s).first.first || (*t).first.second != (*s).first.second)
									continue;

								(*s).second.push_back((*t).second);
								break;
							}

							if (s == sortedTriangleList.end())
							{
								std::vector<int> list;
								list.push_back((*t).second);
								sortedTriangleList.push_back(std::make_pair(std::make_pair((*t).first.first, 
															(*t).first.second), list));
							}
						}

						// Create Sorted Line List
						t = lineList.begin();
						for (; t != lineList.end(); t++)
						{
#ifdef _DEBUG
							std::pair< std::pair< int, int >, int > &unsortedLinePairOfPair = *t;
#endif
							std::vector< std::pair< std::pair< int, int >, std::vector< int > > >::iterator s = sortedLineList.begin();
							for (; s != sortedLineList.end(); s++)
							{
#ifdef _DEBUG
								std::pair< std::pair< int, int >, std::vector< int > > &sortedLinePairOfPair = *s;
#endif
								if ((*t).first.first != (*s).first.first || (*t).first.second != (*s).first.second)
									continue;

								(*s).second.push_back((*t).second);
								break;
							}

							if (s == sortedLineList.end())
							{
								std::vector<int> list;
								list.push_back((*t).second);
								sortedLineList.push_back(std::make_pair(std::make_pair((*t).first.first, 
														(*t).first.second), list));
							}
						}

						// Create Sorted Point List
						t = pointList.begin();
						for (; t != pointList.end(); t++)
						{
#ifdef _DEBUG
							std::pair< std::pair< int, int >, int > &unsortedPointPairOfPair = *t;
#endif
							std::vector< std::pair< std::pair< int, int >, std::vector< int > > >::iterator s = sortedPointList.begin();
							for (; s != sortedPointList.end(); s++)
							{
#ifdef _DEBUG
								std::pair< std::pair< int, int >, std::vector< int > > &sortedPointPairOfPair = *s;
#endif
								if ((*t).first.first != (*s).first.first || (*t).first.second != (*s).first.second)
									continue;

								(*s).second.push_back((*t).second);
								break;
							}

							if (s == sortedPointList.end())
							{
								std::vector<int> list;
								list.push_back((*t).second);
								sortedPointList.push_back(std::make_pair(std::make_pair((*t).first.first, 
														 (*t).first.second), list));
							}
						}

						effectGroups[e].push_back(pGeometry->GetNumChunks());

						// Create triangle chunks
						std::vector< std::pair< std::pair< int, int >, std::vector< int > > >::iterator s = sortedTriangleList.begin();
						for (; s != sortedTriangleList.end(); s++)
						{
#ifdef _DEBUG
							std::pair< std::pair< int, int >, std::vector< int > > &sortedTriPairOfPair = *s;
#endif
							int *triangleVertIndices = new int[(*s).second.size()];

							for (unsigned int v = 0; v < (*s).second.size(); v++)
								triangleVertIndices[v] = (*s).second[v];

							Chunk triangleChunk;
							ZeroMemory(&triangleChunk, sizeof(Chunk));
							triangleChunk.startIndex = pGeometry->GetNumVertexIndices();
							triangleChunk.numVerts = (*s).second.size();
							triangleChunk.type = CT_TRIANGLELIST;
							triangleChunk.effect = e - 1;
							triangleChunk.idTexture0 = (*s).first.first;
							triangleChunk.idTexture1 = (*s).first.second;
							triangleChunk.material = -1;

							int triangleChunkIndex = pGeometry->GetNumChunks();

							RenderGroup *group = new RenderGroup(this);
							group->pGeometry = o->first;
							group->startFaceIndex = pGeometry->GetNumChunkIndices();
							group->numFaces = 1;
							group->effectID = e - 1;
							group->hasTransform = false;

							leaf->renderList.push_back(group);

							pGeometry->Insert(1, 0, 1, (*s).second.size(), &triangleChunk, NULL, 
											  &triangleChunkIndex, triangleVertIndices, 0);

							delete[] triangleVertIndices;
						}

						// Create line chunks
						s = sortedLineList.begin();
						for (; s != sortedLineList.end(); s++)
						{
#ifdef _DEBUG
							std::pair< std::pair< int, int >, std::vector< int > > &sortedLinePairOfPair = *s;
#endif
							int *lineVertIndices = new int[(*s).second.size()];

							for (unsigned int v = 0; v < (*s).second.size(); v++)
								lineVertIndices[v] = (*s).second[v];

							Chunk lineChunk;
							ZeroMemory(&lineChunk, sizeof(Chunk));
							lineChunk.startIndex = pGeometry->GetNumVertexIndices();
							lineChunk.numVerts = (*s).second.size();
							lineChunk.type = CT_LINELIST;
							lineChunk.effect = e - 1;
							lineChunk.idTexture0 = (*s).first.first;
							lineChunk.idTexture1 = (*s).first.second;
							lineChunk.material = -1;

							int lineChunkIndex = pGeometry->GetNumChunks();

							RenderGroup *group = new RenderGroup(this);
							group->pGeometry = o->first;
							group->startFaceIndex = pGeometry->GetNumChunkIndices();
							group->numFaces = 1;
							group->effectID = e - 1;
							group->hasTransform = false;

							leaf->renderList.push_back(group);

							pGeometry->Insert(1, 0, 1, (*s).second.size(), &lineChunk, NULL, &lineChunkIndex, lineVertIndices, 0);

							delete[] lineVertIndices;
						}

						// Create point chunks
						s = sortedPointList.begin();
						for (; s != sortedPointList.end(); s++)
						{
#ifdef _DEBUG
							std::pair< std::pair< int, int >, std::vector< int > > &sortedPointPairOfPair = *s;
#endif
							int *pointVertIndices = new int[(*s).second.size()];

							for (unsigned int v = 0; v < (*s).second.size(); v++)
								pointVertIndices[v] = (*s).second[v];

							Chunk pointChunk;
							ZeroMemory(&pointChunk, sizeof(Chunk));
							pointChunk.startIndex = pGeometry->GetNumVertexIndices();
							pointChunk.numVerts = (*s).second.size();
							pointChunk.type = CT_POINTLIST;
							pointChunk.effect = e - 1;
							pointChunk.idTexture0 = (*s).first.first;
							pointChunk.idTexture1 = (*s).first.second;
							pointChunk.material = -1;

							int pointChunkIndex = pGeometry->GetNumChunks();

							RenderGroup *group = new RenderGroup;
							group->pGeometry = o->first;
							group->startFaceIndex = pGeometry->GetNumChunkIndices();
							group->numFaces = 1;
							group->effectID = e - 1;
							group->hasTransform = false;

							leaf->renderList.push_back(group);

							pGeometry->Insert(1, 0, 1, (*s).second.size(), &pointChunk, NULL, &pointChunkIndex, pointVertIndices, 0);

							delete[] pointVertIndices;
						}
					}

					i->first->Unlock();
				}
				}
#endif
			}
			else
			{
				if (chunkMapIndex >= (int)nodeChunkMap.size())
				{
#ifdef _DEBUG
					Logf("Error: chunkMapIndex exceeds nodeChunkMap.size().");
#endif
					return;
				}

				if (nodeChunkMap[chunkMapIndex]->m.empty())
				{
#ifdef _DEBUG
					Logf("Error: nodeChunkMap[chunkMapIndex] is empty.");
#endif
					return;
				}

				NodeChunkMap &ncm = nodeChunkMap[chunkMapIndex]->m;

				NodeChunkMap::iterator i = ncm.begin();
				for (; i != ncm.end(); i++)
				{
					if ((*i)->m.empty())
						continue;

					OrderedGeometryMap &orderedGeomMapRef = (*i)->m;
					OrderedGeometryMap::iterator j = orderedGeomMapRef.begin();
					for (; j != orderedGeomMapRef.end(); j++)
					{
#ifdef DEBUG_DUMP
						char buf[256];
						sprintf(buf, "Sorting chunks at order = %d\n", j->first);
						OutputDebugString(buf);
#endif
						GeometryMap &gm = j->second->m;
						GeometryMap::iterator k = gm.begin();
						for (; k != gm.end(); k++)
						{
							PGEOMETRY pGeometry = k->first;

							if (pGeometry->GetNumChunks() < 1 || pGeometry->GetNumVertices() < 1)
								continue;

							Chunk *chunks = NULL;

							if (FAILED(pGeometry->Lock((PVOID*)&chunks, NULL, NULL, NULL)))
								return;

							std::vector< std::vector<int> > effectGroups;
							effectGroups.resize(numEffectGroups + 1);

							ChunkIndexList::iterator c = k->second->m.begin();

							for (int f = 0; c != k->second->m.end(); f++, c++)
							{
								if (chunks[f].order != j->first)
									continue;

								effectGroups[chunks[f].effect + 1].push_back(f);
							}

							g_pSortingChunks = chunks;

							for (int e = 0; e < numEffectGroups + 1; e++)
							{
								DebugPrintf("\tCreating RenderGroup for effectGroup %d.", e);

								if (effectGroups[e].size() < 1)
								{
									DebugPrintf("\tSkipping.\n");
									continue;
								}

								sort(effectGroups[e].begin(), effectGroups[e].end(), SortByTechniqueID);

								int currentTechniqueId = chunks[*effectGroups[e].begin()].technique;

								RenderGroup *group = new RenderGroup(this);
								group->order = j->first;
								group->pGeometry = pGeometry;
								group->startFaceIndex = pGeometry->GetNumChunkIndices();
								group->numFaces = effectGroups[e].size();
								group->effectID = e - 1;
								//group->techniqueID = 
								group->hasTransform = false;

								DebugPrintf(" numChunks = %d.\n", group->numFaces);

								leaf->renderList.push_back(group);

								int *chunkIndices = new int[effectGroups[e].size()];

								for (unsigned int m = 0; m < group->numFaces; m++)
									chunkIndices[m] = effectGroups[e][m];

								pGeometry->Insert(0, 0, effectGroups[e].size(), 0, NULL, NULL, chunkIndices, NULL, 0);

								delete[] chunkIndices;
							}

							g_pSortingChunks = NULL;
						}
					}
				}
#if 0
				{
				NodeChunkMap &ncm = nodeChunkMap[chunkMapIndex].m;	// typedef std::vector< _OrderedGeometryMap > NodeChunkMap;
				NodeChunkMap::iterator i = ncm.begin();

				for (; i != nodeChunkMap[chunkMapIndex].m.end(); i++)
				{
					OrderedGeometryMap &orderedGeomMapRef = i->m;	// typedef std::map< unsigned int, _GeometryMap > OrderedGeometryMap;
					OrderedGeometryMap::iterator j = orderedGeomMapRef.begin();
					for (; j != orderedGeomMapRef.end(); j++)
					{
						GeometryMap &gm = j->second->m;
						GeometryMap::iterator k = gm.begin();
						for (; k != gm.end(); k++)
						{
							PGEOMETRY pGeometry = k->first;

							if (pGeometry->GetNumChunks() < 1 || pGeometry->GetNumVertices() < 1)
								continue;

							Chunk *chunks = NULL;

							if (FAILED(pGeometry->Lock((PVOID*)&chunks, NULL, NULL, NULL)))
								return;

							std::vector< std::vector<int> > effectGroups;
							effectGroups.resize(numEffectGroups + 1);

							ChunkIndexList::iterator c;// = k->second.m.begin();

							//for (int f = 0; c != k->second.m.end(); f++, c++)
							int f = 0;
							{
								VertexIndexList &viList = c->m;
								if (viList.size() < 1)
									continue;

								effectGroups[chunks[f].effect + 1].push_back(f);
							}

							for (int e = 0; e < numEffectGroups + 1; e++)
							{
								if (effectGroups[e].size() < 1)
									continue;

								RenderGroup *group = new RenderGroup;
								group->pGeometry = pGeometry;
								group->startFaceIndex = pGeometry->GetNumChunkIndices();
								group->numFaces = effectGroups[e].size();
								group->effectID = e - 1;
								group->hasTransform = false;

								leaf->renderList.push_back(group);

								int *chunkIndices = new int[effectGroups[e].size()];

								for (unsigned int m = 0; m < group->numFaces; m++)
									chunkIndices[m] = effectGroups[e][m];

								pGeometry->Insert(0, 0, effectGroups[e].size(), 0, NULL, NULL, chunkIndices, NULL, 0);

								delete[] chunkIndices;
							}
						}
					}
				}
				}
#endif
			}
		}

		if (node->leafIndex > -1 && settings->optimiseChunks)
		{
			float nodeVolume = (node->max.x - node->min.x) * (node->max.y - node->min.y) * (node->max.z - node->min.z);
			float rootVolume = (root->max.x - root->min.x) * (root->max.y - root->min.y) * (root->max.z - root->min.z);

			if (rootVolume > 0)
			{
				optimiseChunksProgress += (nodeVolume / rootVolume) * 100.0f;

				if (progressCallback)
					progressCallback(PC_SCENE_OPTIMISECHUNKS, optimiseChunksProgress, pProgressCallbackPayload);
			}
		}
	}
#endif


//-----------------------------------------------------------------------
	void Scene::UpdateSpacePartitionProgress(TreeNode *node)
	//-------------------------------------------------------------------
	{
		if (!node || !root)
			return;

		if (node->m_leafIndex > -1)
		{
			float nodeVolume = (node->m_max.x - node->m_min.x) * (node->m_max.y - node->m_min.y) * (node->m_max.z - node->m_min.z);
			float rootVolume = (root->m_max.x - root->m_min.x) * (root->m_max.y - root->m_min.y) * (root->m_max.z - root->m_min.z);

			if (rootVolume > 0)
			{
				spacePartitionProgress += (nodeVolume / rootVolume) * 100.0f;

				if (progressCallback)
					progressCallback(PC_SCENE_SPACEPARTITIONING, spacePartitionProgress, pProgressCallbackPayload);
			}
		}
	}


//-----------------------------------------------------------------------
	void Scene::UpdateOptimiseChunksProgress(TreeNode *node)
	//-------------------------------------------------------------------
	{
		if (!node || !root)
			return;

		if (node->m_leafIndex > -1 && settings->optimiseChunks)
		{
			float nodeVolume = (node->m_max.x - node->m_min.x) * (node->m_max.y - node->m_min.y) * (node->m_max.z - node->m_min.z);
			float rootVolume = (root->m_max.x - root->m_min.x) * (root->m_max.y - root->m_min.y) * (root->m_max.z - root->m_min.z);

			if (rootVolume > 0)
			{
				optimiseChunksProgress += (nodeVolume / rootVolume) * 100.0f;

				if (progressCallback)
					progressCallback(PC_SCENE_OPTIMISECHUNKS, optimiseChunksProgress, pProgressCallbackPayload);
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

		root->min = geometry->min;
		root->max = geometry->max;

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Scene::InsertGeometry(int nChunks, int nVerts, void *pChunks, void *pVerts, DWORD flags)
	//-------------------------------------------------------------------
	{
		geometry->Insert(nChunks, nVerts, pChunks, pVerts, flags);

		root->min = geometry->min;
		root->max = geometry->max;

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Scene::InsertGeometry(PGEOMETRY pGeometry, DWORD flags)
	//-------------------------------------------------------------------
	{
		geometry->Insert(pGeometry, flags);

		root->min = geometry->min;
		root->max = geometry->max;

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
		pTree->numTreeNodes = numNodes;
		pTree->numTreeLeafs = leafs.size();

		if (pTree)
		{
			//memcpy(pTree->leafFaces, leafFaceIndices, numLeafFaces * sizeof(int));

			if (root)
			{
				std::vector<TreeNodeOutput> outputNodes;
				RecurseOutputPartition(root, outputNodes);
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

		geometry.push_back(pGeometry);

		if (settings->optimiseChunks)
		{
			Geometry *pOptimisedGeometry = new Geometry(D3DDevice);
			optimisedGeometry.push_back(std::make_pair(pGeometry, pOptimisedGeometry));
		}

		if (geometry.size() == 1)
		{
			root->m_min = pGeometry->m_min;
			root->m_max = pGeometry->m_max;
		}
		else
			Math::SortMinMax(pGeometry->m_min, pGeometry->m_max, root->m_min, root->m_max);

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Scene::RemoveGeometry(PGEOMETRY pGeometry)
	//-------------------------------------------------------------------
	{
		if (!pGeometry)
			return E_FAIL;

		std::list<PGEOMETRY>::iterator i = geometry.begin();
		for (; i != geometry.end();)
		{
			if ((*i) == pGeometry)
				i = geometry.erase(i);
			else
				i++;
		}

		if (settings->optimiseChunks)
		{
			std::list< std::pair<PGEOMETRY, PGEOMETRY> >::iterator o = optimisedGeometry.begin();
			for (; o != optimisedGeometry.end(); )
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
			}
		}

		std::vector<TreeLeaf *>::iterator leaf = leafs.begin();
		for (; leaf != leafs.end(); leaf++)
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
 		if (!pEntity || !sceneGraphRoot)
			return E_FAIL;

		sceneGraphRoot->AttachChild(pEntity);

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Scene::RemoveEntity(PENTITY pEntity)
	//-------------------------------------------------------------------
	{
		if (!pEntity || !sceneGraphRoot)
			return E_FAIL;

		sceneGraphRoot->DetachChild(pEntity);

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Scene::InsertPatch(PPATCH pPatch)
	//-------------------------------------------------------------------
	{
		if (!pPatch)
			return E_FAIL;

		//patches.push_back(pPatch);

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Scene::GetWorldBounds(Vector3 *pMin, Vector3 *pMax)
	//-------------------------------------------------------------------
	{
		if (!root)
			return E_FAIL;

		if (pMin)
			*pMin = root->m_min;

		if (pMax)
			*pMax = root->m_max;

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
		int numCollisions = bodyCollisions.size() + worldCollisions.size();
		*pCollisionInfo = new CollisionInfo[numCollisions];

		for (int i = 0; i < bodyCollisions.size(); i++)
		{
			CollisionInfo info = bodyCollisions[i];
			memcpy((*pCollisionInfo) + sizeof(CollisionInfo) * i, &info, sizeof(CollisionInfo));
		}

		for (; i < numCollisions; i++)
		{
			CollisionInfo info = worldCollisions[i - bodyCollisions.size()];
			memcpy((*pCollisionInfo) + sizeof(CollisionInfo) * i, &info, sizeof(CollisionInfo));
		}

		if (pNumCollisions)
			*pNumCollisions = numCollisions;

		bodyCollisions.clear();
		worldCollisions.clear();
*/
		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Scene::GetGeometry(ConstGeometryIterator *begin, ConstGeometryIterator *end)
	//-------------------------------------------------------------------
	{
		*begin = geometry.begin();
		*end = geometry.end();

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Scene::GetSpacePartitionOutlineGeometry(PGEOMETRY *ppGeometry)
	//-------------------------------------------------------------------
	{
		if (!ppGeometry || !spacePartitionOutline)
			return E_FAIL;

		(*ppGeometry) = spacePartitionOutline;

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Scene::GetSceneGraphRootEntity(PENTITY *ppEntity)
	//-------------------------------------------------------------------
	{
		if (!ppEntity || !sceneGraphRoot)
			return E_FAIL;

		(*ppEntity) = sceneGraphRoot;

		return S_OK;
	}


//-----------------------------------------------------------------------
	int Scene::GetSubdivisionsExecuted()
	//-------------------------------------------------------------------
	{
		return subdivisionsExecuted;
	}


//-----------------------------------------------------------------------
	int Scene::GetNumLeafs()
	//-------------------------------------------------------------------
	{
		return leafs.size();
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
		pOutputScene->settings = *settings;

		pOutputScene->numGeometry = geometry.size();
		pOutputScene->pGeometryIDs = new unsigned int[pOutputScene->numGeometry];

		std::list<PGEOMETRY>::iterator g = geometry.begin();
		for (int n = 0; g != geometry.end(); g++, n++)
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
			if (spacePartitionOutline == i->first)
			{
				pOutputScene->spacePartitionGeometryID = i->second;
				break;
			}
		}

		if (1) // (flags & SPACEPARTITION)
		{
			// Tree Nodes
				pOutputScene->numTreeNodes = numNodes;

				std::vector<OutputTreeNode> outputTreeNodes;

				RecurseOutputPartition(root, outputTreeNodes);

				pOutputScene->nodes = new OutputTreeNode[outputTreeNodes.size()];
				pOutputScene->size += outputTreeNodes.size() * sizeof(OutputTreeNode);

				std::vector<OutputTreeNode>::iterator i = outputTreeNodes.begin();
				for (int n = 0; i != outputTreeNodes.end(); i++, n++)
					pOutputScene->nodes[n] = *i;

			// Tree Leafs
				pOutputScene->numTreeLeafs = leafs.size();

				pOutputScene->leafs = new OutputTreeLeaf[leafs.size()];
				pOutputScene->size += leafs.size() * sizeof(OutputTreeLeaf);

				std::vector<TreeLeaf *>::iterator q = leafs.begin();
				for (int n = 0; q != leafs.end(); q++, n++)
				{
					OutputTreeLeaf leaf;
					leaf.min = (*q)->m_min;
					leaf.max = (*q)->m_max;
					leaf.renderList = new OutputRenderGroup[(*q)->m_renderList.size()];
					leaf.numRenderGroups = (*q)->m_renderList.size();
					leaf.size = (*q)->m_renderList.size() * sizeof(OutputRenderGroup);
					pOutputScene->size += leaf.size;

					std::list<RenderGroup *>::iterator r = (*q)->m_renderList.begin();
					for (int v = 0; r != (*q)->m_renderList.end(); r++)
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
			pOutputScene->numPVSClusters = numClusters;
			pOutputScene->bytesPerPVSCluster = bytesPerCluster;

			pOutputScene->PVS = new byte[numClusters * bytesPerCluster];
			memcpy(pOutputScene->PVS, PVS, numClusters * bytesPerCluster);

			pOutputScene->size += numClusters * bytesPerCluster;
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
					geometry.push_back(g->first);

				if (pOutputScene->spacePartitionGeometryID == g->second)
					spacePartitionOutline = g->first;
			}
		}

		std::vector< std::pair<PENTITY, int> >::iterator e = entityMap.begin();
		for (; e != entityMap.end(); e++)
		{
			if (pOutputScene->sceneGraphRootEntityID == e->second)
				sceneGraphRoot = e->first;
		}

		if (1) // (flags & SPACEPARTITION)
		{
			if (pOutputScene->numTreeNodes)
			{
				if (root)
				{
					delete root;
					root = new TreeNode;

					numNodes = 1;
				}

				root->m_numChildren = pOutputScene->nodes[0].numChildren;
				root->m_leafIndex = pOutputScene->nodes[0].leafIndex;
				root->m_min = pOutputScene->nodes[0].min;
				root->m_max = pOutputScene->nodes[0].max;
				root->m_pParent = NULL;
				root->m_id = 0;

				RecurseInputPartition(root, pOutputScene->nodes, numNodes);
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

					leafs.push_back(leaf);
				}
			}
		}

		if (/*flags & PVS &&*/ pOutputScene->numPVSClusters && pOutputScene->bytesPerPVSCluster)
		{
			numClusters = pOutputScene->numPVSClusters;
			bytesPerCluster = pOutputScene->bytesPerPVSCluster;

			PVS = new byte[numClusters * bytesPerCluster];
			memcpy(PVS, pOutputScene->PVS, numClusters * bytesPerCluster);
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
		else if (leafs.size() > 0)
		{
			if (node->m_leafIndex > -1)
				CreateLeafRenderList(leafs[node->m_leafIndex], renderList);
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
				PVISUAL pVisual = resourceManager->GetVisual(entity->GetVisualID());

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

		if (vps.drawSpacePartition)
		{
			if (spacePartitionOutline && spacePartitionOutline->GetNumVertexIndices())
			{
				if (activeLeafID > -1 && leafs[activeLeafID]->m_pParent)
				{
					RenderGroup outlineList(this);
					outlineList.m_pGeometry = spacePartitionOutline;
					outlineList.m_startFaceIndex = leafs[activeLeafID]->m_pParent->m_partitionOutlineChunkId;
					outlineList.m_effectID = -1;
					outlineList.m_numFaces = 1;

					renderList.push_back(outlineList);
				}
				else
				{
					RenderGroup outlineList(this);
					outlineList.m_pGeometry = spacePartitionOutline;
					outlineList.m_startFaceIndex = 0;
					outlineList.m_effectID = -1;
					outlineList.m_numFaces = spacePartitionOutline->GetNumChunkIndices();

					renderList.push_back(outlineList);
				}
			}
		}

		if (settings->visibilityMode == SS_ENABLED)
		{
			for (unsigned int i = 0; i < leafs.size(); i++)
			{
				if (Math::AABBPointCollision(Math::AABB(leafs[i]->m_min, leafs[i]->m_max), pViewport->GetCamera()->GetPosition()))
				{
					for (unsigned int n = 0; n < leafs.size(); n++)
					{
						if (PVS[i * bytesPerCluster + (n / 8)] & (1 << (n & 7)))
						{
							Vector3 cMin = leafs[n]->m_min;
							Vector3 cMax = leafs[n]->m_max;

							if (!pViewport->GetCamera()->CheckBoxFrustum(cMin, cMax))
								continue;

							CreateLeafRenderList(leafs[n], renderList);
						}
					}

					break;
				}
			}
		}
		else
		{
			if (activeLeafID == -1)
			{
				CreateNodeRenderList(root, renderList, pViewport);
			}
			else if (activeLeafID < (int)leafs.size())
			{
				CreateLeafRenderList(leafs[activeLeafID], renderList);
			}
		}

		CreateEntityRenderList(sceneGraphRoot, renderList, pViewport);

		// TODO: sort render list by depth bias

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Scene::Update(const float deltaTime)
	//-------------------------------------------------------------------
	{
		if (sceneGraphRoot)
			sceneGraphRoot->Update(deltaTime);

		/*
		bodyCollisions.clear();
		worldCollisions.clear();

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

								worldCollisions.push_back(*result);
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

											bodyCollisions.push_back(*result);
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
	void Scene::RecurseDrawPartition(TreeNode *&node, std::vector<Vector3> &verts, std::vector<Chunk> &chunks)
	//-------------------------------------------------------------------
	{
		if (!node)
			return;

		Vector3 a = node->m_min;
		Vector3 b = node->m_max;

		node->m_partitionOutlineChunkId = chunks.size();

		Chunk lines;
		lines.depthBias = 0;
		lines.idTexture0 = -1;
		lines.idTexture1 = -1;
		lines.numVerts = 24;
		lines.startIndex = (int)verts.size();
		lines.effect = -1;
		lines.type = CT_LINELIST;
		chunks.push_back(lines);

		// Bottom face
		verts.push_back(a);
		verts.push_back(Vector3(b.x, a.y, a.z));

		verts.push_back(Vector3(b.x, a.y, a.z));
		verts.push_back(Vector3(b.x, a.y, b.z));

		verts.push_back(Vector3(b.x, a.y, b.z));
		verts.push_back(Vector3(a.x, a.y, b.z));

		verts.push_back(Vector3(a.x, a.y, b.z));
		verts.push_back(a);

		// Top face
		verts.push_back(Vector3(a.x, b.y, a.z));
		verts.push_back(Vector3(b.x, b.y, a.z));

		verts.push_back(Vector3(b.x, b.y, a.z));
		verts.push_back(Vector3(b.x, b.y, b.z));

		verts.push_back(Vector3(b.x, b.y, b.z));
		verts.push_back(Vector3(a.x, b.y, b.z));

		verts.push_back(Vector3(a.x, b.y, b.z));
		verts.push_back(Vector3(a.x, b.y, a.z));

		// side edges
		verts.push_back(Vector3(a.x, a.y, a.z));
		verts.push_back(Vector3(a.x, b.y, a.z));

		verts.push_back(Vector3(b.x, a.y, a.z));
		verts.push_back(Vector3(b.x, b.y, a.z));

		verts.push_back(Vector3(b.x, a.y, b.z));
		verts.push_back(Vector3(b.x, b.y, b.z));

		verts.push_back(Vector3(a.x, a.y, b.z));
		verts.push_back(Vector3(a.x, b.y, b.z));

		for (int i = 0; i < node->m_numChildren; i++)
			RecurseDrawPartition(node->m_pChildren[i], verts, chunks);
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
	void Scene::RecurseInputPartition(TreeNode *node, OutputTreeNode *outputNodes, int &numNodes)
	//-------------------------------------------------------------------
	{
		if (!node || !outputNodes)
			return;

		if (node->m_numChildren)
			node->m_pChildren = new TreeNode *[node->m_numChildren];

		for (int i = 0; i < node->m_numChildren; i++)
		{
			int childID = outputNodes[node->m_id].children[i];

			node->m_pChildren[i] = new TreeNode(outputNodes[childID].min, outputNodes[childID].max, node, numNodes);
			node->m_pChildren[i]->m_leafIndex = outputNodes[childID].leafIndex;
			node->m_pChildren[i]->m_numChildren = outputNodes[childID].numChildren;
			node->m_pChildren[i]->m_min = outputNodes[childID].min;
			node->m_pChildren[i]->m_max = outputNodes[childID].max;
			node->m_pChildren[i]->m_pParent = node;
			node->m_pChildren[i]->m_id = numNodes;

			numNodes++;

			RecurseInputPartition(node->m_pChildren[i], outputNodes, numNodes);
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

				if (settings->spacePartitionAxis == SS_AXIS_X)
					upVector = Vector3(1, 0, 0);
				else if (settings->spacePartitionAxis == SS_AXIS_Y)
					upVector = Vector3(0, 1, 0);
				else if (settings->spacePartitionAxis == SS_AXIS_Z)
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

		if (node->m_leafIndex > -1 && node->m_leafIndex < (int)leafs.size())
		{
			TreeLeaf *pLeaf = leafs[node->m_leafIndex];

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
		if (!root)
			return false;

		bool found = RecurseTestIntersect(root, results, rayPos, rayDir, flags, false);

		return found;
	}


//-----------------------------------------------------------------------
	bool Scene::Intersect(IntersectionResult &result, const Vector3 &rayPos, const Vector3 &rayDir, DWORD flags)
	//-------------------------------------------------------------------
	{
		if (!root)
			return false;

		std::list<IntersectionResult> results;

		bool found = RecurseTestIntersect(root, results, rayPos, rayDir, flags, true);

		if (results.size() < 1)
			return false;

		result = *results.begin();

		return found;
	}

// EOF