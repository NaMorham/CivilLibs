/*! \file */
/*-----------------------------------------------------------------------
	serialiser.h

	Description: Serialised versions of KSR data structures
	Author:		 Julian McKinlay
	---------------------------------------------------------------------*/

#pragma once

struct OutputTreeNode
{
public:
	OutputTreeNode();

	Vector3 min, max;

	unsigned int numChildren;
	int leafIndex;

	int parent;
	int children[8];
};


struct OutputRenderGroup
{
public:
	OutputRenderGroup();

	unsigned int geometryID,
				 startFaceIndex,
				 numFaces;

	int effectID;
};


struct OutputTreeLeaf
{
public:
	OutputTreeLeaf();
	~OutputTreeLeaf();

	unsigned int size;
	unsigned int numRenderGroups;

	Vector3 min, max;

	OutputRenderGroup *renderList;
};


struct OutputGeometry
{
public:
	OutputGeometry();
	~OutputGeometry();

	unsigned int size,
				 numChunks, 
				 numVertices,
				 numChunkIndices,
				 numVertexIndices,
				 numVertexElements,
				 vertexLength,
				 indexLength,
				 FVF,
				 indexFormat;

	Vector3 min, max;

	bool useVertexDeclarationFormat;

	PVOID chunks,
		  vertices,
		  chunkIndices, 
		  vertexIndices,
		  vertexElements;
};


struct OutputEntity
{
public:
	OutputEntity();
	~OutputEntity();

	unsigned int size;

	int parentID,
		numChildren,
		visualId;

	int *pChildIDs;

	Vector3 position,
			rotation;
};


struct OutputScene
{
public:
	OutputScene();
	~OutputScene();

	unsigned int size,
				 numGeometry,
				 numTreeNodes, 
				 numTreeLeafs,
				 numPVSClusters, 
				 bytesPerPVSCluster;

	SceneSettings settings;

	OutputTreeNode *nodes;
	OutputTreeLeaf *leafs;
	byte *PVS;

	unsigned int *pGeometryIDs;
	unsigned int spacePartitionGeometryID;
	int sceneGraphRootEntityID;
};


struct FileTableEntry
{
	unsigned int offset,
				 length;
};


struct Interface;


struct Serialiser : public MemObject
{
public:
	typedef std::vector<PGEOMETRY>::const_iterator ConstGeometryIterator;
	typedef std::vector<PENTITY>::const_iterator ConstEntityIterator;
	typedef std::vector<PSCENE>::const_iterator ConstSceneIterator;

	Serialiser(LPDIRECT3DDEVICE9 pDevice);
	~Serialiser();

	HRESULT InsertGeometry(PGEOMETRY pGeometry);
	HRESULT InsertEntity(PENTITY pEntity);
	HRESULT InsertScene(PSCENE pScene);

	HRESULT GetGeometry(ConstGeometryIterator *begin, ConstGeometryIterator *end);
	HRESULT GetEntities(ConstEntityIterator *begin, ConstEntityIterator *end);
	HRESULT GetScenes(ConstSceneIterator *begin, ConstSceneIterator *end);

	HRESULT Clear();

	HRESULT Save(const String &filename, DWORD flags);
	HRESULT Load(const String &filename, DWORD flags, Interface *pKSRInterface);

private:
	HRESULT Serialise(DWORD flags);
	HRESULT Reassemble(DWORD flags, Interface *pKSRInterface);

	std::vector<PGEOMETRY> m_geometries;
	std::vector<PENTITY> m_entities;
	std::vector<PSCENE> m_scenes;

	std::vector<OutputGeometry *> m_serialisedGeometry;
	std::vector<OutputScene *> m_serialisedScenes;
	std::vector<OutputEntity *> m_serialisedEntities;

	std::vector< std::pair<PGEOMETRY, int> > m_geometryIDs;
	std::vector< std::pair<PENTITY, int> > m_entityIDs;

	LPDIRECT3DDEVICE9 m_pD3DDevice;
};

//! \typedef Serialiser *PSERIALISER
typedef Serialiser *PSERIALISER;

// EOF