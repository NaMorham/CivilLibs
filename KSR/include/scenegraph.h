/*-----------------------------------------------------------------------
	scenegraph.h

	Description: Scene Graph and related class declarations
	Author:		 Julian McKinlay
	---------------------------------------------------------------------*/

#pragma once

struct SceneGraph : public MemObject
{
public:
	SceneGraph(LPDIRECT3DDEVICE9 pDevice, LPRESOURCEMANAGER pResourceManager);
	~SceneGraph();

	void SetPartitionType(DWORD type, DWORD visibility, DWORD colour, int maxSubdivisions, int numPolys, float fixedSize);
	void SetCollisionEfficiency(float tolerance, float precision);

	int GetSubdivisionsExecuted();
	int GetNumLeafs();

	HRESULT Lock(void **pChunks, void **pVerts, void **pIndices);
	HRESULT Unlock();

	HRESULT Reset();
	HRESULT Clear();

	HRESULT InitGeometry(int vertexSize, DWORD vertexFormat, DWORD indexFormat);
	HRESULT InsertGeometry(int nChunks, int nVerts, int nIndices, void *pChunks, void *pVerts, void *pIndices);
	HRESULT InsertGeometry(LPKSRGEOMETRY pGeometry);
	HRESULT OutputGeometry(LPKSRGEOMETRY pGeometry, LPKSRPARTITIONTREE pTree);

	HRESULT InsertPatch(LPKSRPATCH pPatch);
	HRESULT GeneratePatch(LPKSRPATCH pPatch);

	HRESULT InsertEntity(LPKSRENTITY pEntity);
	HRESULT RemoveEntity(LPKSRENTITY pEntity);

	HRESULT SetBounds(Vector3 min, Vector3 max);
	HRESULT GetBounds(Vector3 *pMin, Vector3 *pMax);
	HRESULT Partition();
	HRESULT CreateDrawPartition();
	HRESULT Render(LPKSRVIEWPORT pViewport);

	HRESULT Update(float deltaTime);

	HRESULT GetCollisionInfo(KSRCollisionInfo **pCollisionInfo, int *pNumCollisions);

private:
	void SubdivideNode(TreeNode *node, int numSubdivisions);
	void RenderNode(TreeNode *node, LPKSRVIEWPORT pViewport);
	void RenderLeaf(TreeLeaf *leaf, LPKSRVIEWPORT pViewport);
	void RenderChunk(KSRChunk *chunk);
	void GenerateLeafPatches(TreeLeaf *leaf, LPKSRVIEWPORT pViewport);
	HRESULT GenerateLeafPatch(KSRChunk *source, LPKSRVIEWPORT pViewport);

	void RecurseDrawPartition(TreeNode *&node, std::vector<Vector3> &verts);
	void RecurseOutputPartition(TreeNode *node, std::vector<TreeNodeOutput> &outputNodes);
	void RecurseInputPartition(TreeNode *node, TreeNodeOutput *outputNodes, int &numNodes);
	void RecurseSphereCollisionPartition(TreeNode *node, Math::Sphere sphere1, Math::Sphere sphere2, std::vector<TreeLeaf *> &leafs,
										 std::vector<TreeLeaf *> &collidingLeafs);

	// Partitioning
	DWORD partitionType,
		  visibilityType,
		  partitionColour;

	float polyDepth,
		  sizeDepth,
		  subdivisionDepth;
	int subdivisionsExecuted;

	int numNodes;
	TreeNode *root;
	std::vector<TreeLeaf *> leafs;

	int numClusters,
		bytesPerCluster;
	byte *PVS;

	LPKSRGEOMETRY outline;
	int numOutlineVerts;

	// Geometry
	std::vector<LPKSRPATCH> patches;
	LPKSRGEOMETRY geometry;
	LPKSRGEOMETRY patchGeometry;

	int numLeafFaces;
	int	*leafFaceIndices;

	float currentDepthBias;

	bool vertexBufferLocked,
		 indexBufferLocked;

	std::vector<int> leafsDrawn;
	std::vector<int> facesDrawn;

	// Entities
	std::list<KSREntity *> entities;
	std::vector<KSRCollisionInfo> bodyCollisions;
	std::vector<KSRCollisionInfo> worldCollisions;

	float collisionTolerance,
		  collisionPrecision;

	// General
	int frameNumber;
	LPDIRECT3DDEVICE9 D3DDevice;
	LPRESOURCEMANAGER resourceManager;
};

typedef struct SceneGraph *LPSCENEGRAPH;

// EOF