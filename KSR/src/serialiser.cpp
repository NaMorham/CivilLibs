#include "../include/ksr.h"

#include "../include/leakwatcher.h"

#ifdef _DO_MEMORY_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace KSR;

//-----------------------------------------------------------------------
    OutputTreeNode::OutputTreeNode()
    //-------------------------------------------------------------------
    {
        min = max = Vector3(0, 0, 0);

        numChildren = 0;
        leafIndex = 0;

        parent = -1;
        for (int i = 0; i < 8; i++)
            children[i] = -1;
    }


//-----------------------------------------------------------------------
    OutputRenderGroup::OutputRenderGroup()
    //-------------------------------------------------------------------
    {
        geometryID = startFaceIndex = numFaces = 0;

        effectID = -1;
    }


//-----------------------------------------------------------------------
    OutputTreeLeaf::OutputTreeLeaf()
    //-------------------------------------------------------------------
    {
        size = numRenderGroups = 0;

        min = max = Vector3(0, 0, 0);

        renderList = NULL;
    }


//-----------------------------------------------------------------------
    OutputTreeLeaf::~OutputTreeLeaf()
    //-------------------------------------------------------------------
    {
        if (renderList)
            delete[] renderList;
    }


//-----------------------------------------------------------------------
    OutputGeometry::OutputGeometry()
    //-------------------------------------------------------------------
    {
        size = numChunks = numVertices = numChunkIndices = numVertexIndices = numVertexElements =
        vertexLength = indexLength = FVF = indexFormat = 0;

        min = max = Vector3(0, 0, 0);

        useVertexDeclarationFormat = false;

        chunks = vertices = chunkIndices = vertexIndices = NULL;
    }


//-----------------------------------------------------------------------
    OutputGeometry::~OutputGeometry()
    //-------------------------------------------------------------------
    {
        if (chunks)
            free(chunks);

        if (vertices)
            free(vertices);

        if (chunkIndices)
            free(chunkIndices);

        if (vertexIndices)
            free(vertexIndices);
    }


//-----------------------------------------------------------------------
    OutputEntity::OutputEntity()
    //-------------------------------------------------------------------
    {
        size = 0;

        parentID = -1;
        numChildren = 0;
        visualId = -1;

        pChildIDs = NULL;

        position = rotation = Vector3(0, 0, 0);
    }


//-----------------------------------------------------------------------
    OutputEntity::~OutputEntity()
    //-------------------------------------------------------------------
    {
        if (pChildIDs)
            delete[] pChildIDs;
    }


//-----------------------------------------------------------------------
    OutputScene::OutputScene()
    //-------------------------------------------------------------------
    {
        size = numGeometry = numTreeNodes = numTreeLeafs = numPVSClusters = bytesPerPVSCluster = 0;

        nodes = NULL;
        leafs = NULL;
        PVS = NULL;

        pGeometryIDs = NULL;
        spacePartitionGeometryID = -1;
        sceneGraphRootEntityID = -1;
    }


//-----------------------------------------------------------------------
    OutputScene::~OutputScene()
    //-------------------------------------------------------------------
    {
        if (nodes)
            delete[] nodes;

        if (leafs)
            delete[] leafs;

        if (PVS)
            delete[] PVS;

        if (pGeometryIDs)
            delete[] pGeometryIDs;
    }


//-----------------------------------------------------------------------
    Serialiser::Serialiser(LPDIRECT3DDEVICE9 pDevice)
    //-------------------------------------------------------------------
    {
        m_pD3DDevice = pDevice;
    }


//-----------------------------------------------------------------------
    Serialiser::~Serialiser()
    //-------------------------------------------------------------------
    {
        Clear();
    }


//-----------------------------------------------------------------------
    HRESULT Serialiser::InsertGeometry(PGEOMETRY pGeometry)
    //-------------------------------------------------------------------
    {
        if (!pGeometry)
            return E_FAIL;

        m_geometries.push_back(pGeometry);

        int id = m_geometryIDs.size();

        m_geometryIDs.push_back(std::make_pair(pGeometry, id));

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Serialiser::InsertScene(PSCENE pScene)
    //-------------------------------------------------------------------
    {
        if (!pScene)
            return E_FAIL;

        PGEOMETRY pSpacePartitionOutlineGeometry = NULL;
        PENTITY pSceneGraphRootEntity = NULL;

        if (FAILED(pScene->GetSpacePartitionOutlineGeometry(&pSpacePartitionOutlineGeometry)))
            return E_FAIL;

        if (FAILED(InsertGeometry(pSpacePartitionOutlineGeometry)))
            return E_FAIL;

        if (FAILED(pScene->GetSceneGraphRootEntity(&pSceneGraphRootEntity)))
            return E_FAIL;

        if (FAILED(InsertEntity(pSceneGraphRootEntity)))
            return E_FAIL;

        m_scenes.push_back(pScene);

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Serialiser::InsertEntity(PENTITY pEntity)
    //-------------------------------------------------------------------
    {
        if (!pEntity)
            return E_FAIL;

        m_entities.push_back(pEntity);

        int id = m_entityIDs.size();

        m_entityIDs.push_back(std::make_pair(pEntity, id));

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Serialiser::GetGeometry(ConstGeometryIterator *begin, ConstGeometryIterator *end)
    //-------------------------------------------------------------------
    {
        if (!begin || !end)
            return E_FAIL;

        *begin = m_geometries.begin();
        *end = m_geometries.end();

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Serialiser::GetEntities(ConstEntityIterator *begin, ConstEntityIterator *end)
    //-------------------------------------------------------------------
    {
        if (!begin || !end)
            return E_FAIL;

        *begin = m_entities.begin();
        *end = m_entities.end();

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Serialiser::GetScenes(ConstSceneIterator *begin, ConstSceneIterator *end)
    //-------------------------------------------------------------------
    {
        if (!begin || !end)
            return E_FAIL;

        *begin = m_scenes.begin();
        *end = m_scenes.end();

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Serialiser::Serialise(DWORD flags)
    //-------------------------------------------------------------------
    {
        std::vector<PGEOMETRY>::iterator g = m_geometries.begin();
        for (; g != m_geometries.end(); g++)
        {
            OutputGeometry *outputGeometry = new OutputGeometry;

            (*g)->Serialise(outputGeometry);

            m_serialisedGeometry.push_back(outputGeometry);
        }

        std::vector<PENTITY>::iterator e = m_entities.begin();
        for (; e != m_entities.end(); e++)
        {
            OutputEntity *outputEntity = new OutputEntity;

            (*e)->Serialise(outputEntity, m_entityIDs);

            m_serialisedEntities.push_back(outputEntity);
        }

        std::vector<PSCENE>::iterator s = m_scenes.begin();
        for (; s != m_scenes.end(); s++)
        {
            OutputScene *outputScene = new OutputScene;

            (*s)->Serialise(outputScene, m_geometryIDs, m_entityIDs, NULL);

            m_serialisedScenes.push_back(outputScene);
        }

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Serialiser::Reassemble(DWORD flags, Interface *pKSRInterface)
    //-------------------------------------------------------------------
    {
        // Reassemble Geometry
            std::vector<OutputGeometry *>::iterator sg = m_serialisedGeometry.begin();
            for (int id = 0; sg != m_serialisedGeometry.end(); sg++, id++)
            {
                PGEOMETRY pGeometry = NULL;
                pKSRInterface->CreateGeometry(&pGeometry);

                m_geometryIDs.push_back(std::make_pair(pGeometry, id));

                m_geometries.push_back(pGeometry);
            }

            std::vector<PGEOMETRY>::iterator g = m_geometries.begin();
            for (sg = m_serialisedGeometry.begin(); g != m_geometries.end(); g++, sg++)
            {
                if (FAILED((*g)->Reassemble(*sg)))
                    return E_FAIL;
            }

        // Reassemble Entities
            std::vector<OutputEntity *>::iterator se = m_serialisedEntities.begin();
            for (id = 0; se != m_serialisedEntities.end(); se++, id++)
            {
                PENTITY pEntity = NULL;
                pKSRInterface->CreateEntity(&pEntity);

                m_entityIDs.push_back(std::make_pair(pEntity, id));

                m_entities.push_back(pEntity);
            }

            std::vector<PENTITY>::iterator e = m_entities.begin();
            for (se = m_serialisedEntities.begin(); e != m_entities.end(); e++, se++)
            {
                if (FAILED((*e)->Reassemble((*se), m_entityIDs)))
                    return E_FAIL;
            }
/*
        std::vector<Entity *>::iterator i = entities.begin();
        for (e = serialisedEntities.begin(); i != entities.end() && e != serialisedEntities.end(); i++, e++)
        {
            (*i)->children.resize((*e)->numChildren);

            std::vector< std::pair<PENTITY, int> >::iterator id = entityIDs.begin();
            for (; id != entityIDs.end(); id++)
            {
                if (id->second == (*e)->parentID)
                    (*i)->parent = id->first;

                for (int c = 0; c < (*e)->numChildren; c++)
                {
                    if (id->second == (*e)->pChildIDs[c])
                        (*i)->children.push_back(id->first);

                    break;
                }
            }

            // visualID
        }
*/
        // Reassemble Scenes
            std::vector<OutputScene *>::iterator ss = m_serialisedScenes.begin();
            for (; ss != m_serialisedScenes.end(); ss++)
            {
                PSCENE pScene = NULL;
                pKSRInterface->CreateScene(&(*ss)->settings, &pScene);

                if (FAILED(pScene->Reassemble((*ss), m_geometryIDs, m_entityIDs, flags)))
                    return E_FAIL;

                m_scenes.push_back(pScene);
            }

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Serialiser::Clear()
    //-------------------------------------------------------------------
    {
        std::vector<OutputGeometry *>::iterator g = m_serialisedGeometry.begin();
        for (; g != m_serialisedGeometry.end(); g++)
        {
            if (*g)
                delete (*g);
        }

        std::vector<OutputEntity *>::iterator e = m_serialisedEntities.begin();
        for (; e != m_serialisedEntities.end(); e++)
        {
            if (*e)
                delete (*e);
        }

        std::vector<OutputScene *>::iterator s = m_serialisedScenes.begin();
        for (; s != m_serialisedScenes.end(); s++)
        {
            if (*s)
                delete (*s);
        }

        m_geometries.clear();
        m_entities.clear();
        m_scenes.clear();

        m_serialisedGeometry.clear();
        m_serialisedEntities.clear();
        m_serialisedScenes.clear();

        m_geometryIDs.clear();
        m_entityIDs.clear();

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Serialiser::Save(const String &filename, DWORD flags)
    //-------------------------------------------------------------------
    {
        if (filename.size() < 1)
            return E_FAIL;

        if (FAILED(Serialise(flags)))
            return E_FAIL;

        FILE *file = fopen(filename.c_str(), _T("wb"));

        if (!file)
            return E_FAIL;

        unsigned int versionNumber = VERSION;
        size_t numGeometry = m_serialisedGeometry.size();
        size_t numEntities = m_serialisedEntities.size();
        size_t numScenes = m_serialisedScenes.size();

        // Write header
        fwrite(&versionNumber, sizeof(unsigned int), 1, file);
        fwrite(&numGeometry, sizeof(size_t), 1, file);
        fwrite(&numEntities, sizeof(size_t), 1, file);
        fwrite(&numScenes, sizeof(size_t), 1, file);

        size_t offset = sizeof(size_t) * 4;

        for (size_t i = 0; i < numGeometry; i++)
        {
            FileTableEntry tableEntry;
            tableEntry.length = m_serialisedGeometry[i]->size;
            tableEntry.offset = offset;

            fwrite(&tableEntry, sizeof(FileTableEntry), 1, file);
            offset += tableEntry.length;
        }

        for (i = 0; i < numEntities; i++)
        {
            FileTableEntry tableEntry;
            tableEntry.length = m_serialisedEntities[i]->size;
            tableEntry.offset = offset;

            fwrite(&tableEntry, sizeof(FileTableEntry), 1, file);
            offset += tableEntry.length;
        }

        for (i = 0; i < numScenes; i++)
        {
            FileTableEntry tableEntry;
            tableEntry.length = m_serialisedScenes[i]->size;
            tableEntry.offset = offset;

            fwrite(&tableEntry, sizeof(FileTableEntry), 1, file);
            offset += tableEntry.length;
        }

        // Write Geometry
        std::vector<OutputGeometry *>::iterator g = m_serialisedGeometry.begin();
        for (; g != m_serialisedGeometry.end(); g++)
        {
            fwrite(&(*g)->size, sizeof(unsigned int), 1, file);
            fwrite(&(*g)->numChunks, sizeof(unsigned int), 1, file);
            fwrite(&(*g)->numVertices, sizeof(unsigned int), 1, file);
            fwrite(&(*g)->numChunkIndices, sizeof(unsigned int), 1, file);
            fwrite(&(*g)->numVertexIndices, sizeof(unsigned int), 1, file);
            fwrite(&(*g)->vertexLength, sizeof(unsigned int), 1, file);
            fwrite(&(*g)->indexLength, sizeof(unsigned int), 1, file);
            fwrite(&(*g)->FVF, sizeof(unsigned int), 1, file);
            fwrite(&(*g)->indexFormat, sizeof(unsigned int), 1, file);

            fwrite(&(*g)->min, sizeof(Vector3), 1, file);
            fwrite(&(*g)->max, sizeof(Vector3), 1, file);

            if ((*g)->numChunks)
                fwrite((*g)->chunks, sizeof(Chunk), (*g)->numChunks, file);

            if ((*g)->numVertices)
                fwrite((*g)->vertices, (*g)->vertexLength, (*g)->numVertices, file);

            if ((*g)->numChunkIndices)
                fwrite((*g)->chunkIndices, (*g)->indexLength, (*g)->numChunkIndices, file);

            if ((*g)->numVertexIndices)
                fwrite((*g)->vertexIndices, (*g)->indexLength, (*g)->numVertexIndices, file);
        }

        // Write entities
        std::vector<OutputEntity *>::iterator e = m_serialisedEntities.begin();
        for (; e != m_serialisedEntities.end(); e++)
        {
            fwrite(&(*e)->size, sizeof(unsigned int), 1, file);
            fwrite(&(*e)->parentID, sizeof(int), 1, file);
            fwrite(&(*e)->numChildren, sizeof(int), 1, file);
            fwrite(&(*e)->visualId, sizeof(int), 1, file);

            if ((*e)->numChildren)
                fwrite((*e)->pChildIDs, sizeof(int), (*e)->numChildren, file);

            fwrite(&(*e)->position, sizeof(Vector3), 1, file);
            fwrite(&(*e)->rotation, sizeof(Vector3), 1, file);
        }

        // Write scenes
        std::vector<OutputScene *>::iterator s = m_serialisedScenes.begin();
        for (; s != m_serialisedScenes.end(); s++)
        {
            fwrite(&(*s)->size, sizeof(unsigned int), 1, file);
            fwrite(&(*s)->numGeometry, sizeof(unsigned int), 1, file);
            fwrite(&(*s)->numTreeNodes, sizeof(unsigned int), 1, file);
            fwrite(&(*s)->numTreeLeafs, sizeof(unsigned int), 1, file);
            fwrite(&(*s)->numPVSClusters, sizeof(unsigned int), 1, file);
            fwrite(&(*s)->bytesPerPVSCluster, sizeof(unsigned int), 1, file);

            fwrite(&(*s)->settings, sizeof(SceneSettings), 1, file);

            if ((*s)->numTreeNodes)
                fwrite((*s)->nodes, sizeof(OutputTreeNode), (*s)->numTreeNodes, file);

            if ((*s)->numTreeLeafs)
            {
                for (i = 0; i < (*s)->numTreeLeafs; i++)
                {
                    OutputTreeLeaf &outputLeaf = (*s)->leafs[i];

                    fwrite(&(*s)->leafs[i].size, sizeof(unsigned int), 1, file);
                    fwrite(&(*s)->leafs[i].numRenderGroups, sizeof(size_t), 1, file);
                    fwrite(&(*s)->leafs[i].min, sizeof(Vector3), 1, file);
                    fwrite(&(*s)->leafs[i].max, sizeof(Vector3), 1, file);

                    for (size_t r = 0; r < (*s)->leafs[i].numRenderGroups; r++)
                    {
                        fwrite(&(*s)->leafs[i].renderList[r].geometryID, sizeof(unsigned int), 1, file);
                        fwrite(&(*s)->leafs[i].renderList[r].startFaceIndex, sizeof(unsigned int), 1, file);
                        fwrite(&(*s)->leafs[i].renderList[r].numFaces, sizeof(unsigned int), 1, file);
                        fwrite(&(*s)->leafs[i].renderList[r].effectID, sizeof(unsigned int), 1, file);
                    }
                }
            }

            if ((*s)->numPVSClusters && (*s)->bytesPerPVSCluster)
                fwrite((*s)->PVS, (*s)->bytesPerPVSCluster, (*s)->numPVSClusters, file);

            if ((*s)->numGeometry)
                fwrite((*s)->pGeometryIDs, sizeof(unsigned int), (*s)->numGeometry, file);

            fwrite(&(*s)->spacePartitionGeometryID, sizeof(unsigned int), 1, file);
            fwrite(&(*s)->sceneGraphRootEntityID, sizeof(int), 1, file);
        }

        fclose(file);

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Serialiser::Load(const String &filename, DWORD flags, Interface *pKSRInterface)
    //-------------------------------------------------------------------
    {
        if (filename.size() < 1 || !pKSRInterface)
            return E_FAIL;

        FILE *file = fopen(filename.c_str(), "rb");

        if (!file)
            return E_FAIL;

        unsigned int versionNumber = 0;
        size_t numGeometry = 0;
        size_t numEntities = 0;
        size_t numScenes = 0;

        FileTableEntry *geometryTable = NULL;
        FileTableEntry *entityTable = NULL;
        FileTableEntry *sceneTable = NULL;

        // Read Header
        fread(&versionNumber, sizeof(unsigned int), 1, file);
        fread(&numGeometry, sizeof(size_t), 1, file);
        fread(&numEntities, sizeof(size_t), 1, file);
        fread(&numScenes, sizeof(size_t), 1, file);

        if (versionNumber != VERSION)
            return E_FAIL;

        if (numGeometry)
        {
            geometryTable = new FileTableEntry[numGeometry];
            fread(geometryTable, sizeof(FileTableEntry), numGeometry, file);
        }

        if (numEntities)
        {
            entityTable = new FileTableEntry[numEntities];
            fread(entityTable, sizeof(FileTableEntry), numEntities, file);
        }

        if (numScenes)
        {
            sceneTable = new FileTableEntry[numScenes];
            fread(sceneTable, sizeof(FileTableEntry), numScenes, file);
        }

        // Read Geometry
        for (size_t i = 0; i < numGeometry; i++)
        {
            OutputGeometry *outputGeometry = new OutputGeometry;

            fread(&outputGeometry->size, sizeof(unsigned int), 1, file);
            fread(&outputGeometry->numChunks, sizeof(unsigned int), 1, file);
            fread(&outputGeometry->numVertices, sizeof(unsigned int), 1, file);
            fread(&outputGeometry->numChunkIndices, sizeof(unsigned int), 1, file);
            fread(&outputGeometry->numVertexIndices, sizeof(unsigned int), 1, file);
            fread(&outputGeometry->vertexLength, sizeof(unsigned int), 1, file);
            fread(&outputGeometry->indexLength, sizeof(unsigned int), 1, file);
            fread(&outputGeometry->FVF, sizeof(unsigned int), 1, file);
            fread(&outputGeometry->indexFormat, sizeof(unsigned int), 1, file);

            fread(&outputGeometry->min, sizeof(Vector3), 1, file);
            fread(&outputGeometry->max, sizeof(Vector3), 1, file);

            if (outputGeometry->numChunks)
            {
                outputGeometry->chunks = malloc(sizeof(Chunk) * outputGeometry->numChunks);
                fread(outputGeometry->chunks, sizeof(Chunk), outputGeometry->numChunks, file);
            }
            else
                outputGeometry->chunks = NULL;

            if (outputGeometry->numVertices)
            {
                outputGeometry->vertices = malloc(outputGeometry->vertexLength * outputGeometry->numVertices);
                fread(outputGeometry->vertices, outputGeometry->vertexLength, outputGeometry->numVertices, file);
            }
            else
                outputGeometry->vertices = NULL;

            if (outputGeometry->numChunkIndices)
            {
                outputGeometry->chunkIndices = malloc(outputGeometry->indexLength * outputGeometry->numChunkIndices);
                fread(outputGeometry->chunkIndices, outputGeometry->indexLength, outputGeometry->numChunkIndices, file);
            }
            else
                outputGeometry->chunkIndices = NULL;

            if (outputGeometry->numVertexIndices)
            {
                outputGeometry->vertexIndices = malloc(outputGeometry->indexLength * outputGeometry->numVertexIndices);
                fread(outputGeometry->vertexIndices, outputGeometry->indexLength, outputGeometry->numVertexIndices, file);
            }
            else
                outputGeometry->vertexIndices = NULL;

            m_serialisedGeometry.push_back(outputGeometry);
        }

        // Read Entities
        for (i = 0; i < numEntities; i++)
        {
            OutputEntity *outputEntity = new OutputEntity;

            fread(&outputEntity->size, sizeof(unsigned int), 1, file);
            fread(&outputEntity->parentID, sizeof(int), 1, file);
            fread(&outputEntity->numChildren, sizeof(int), 1, file);
            fread(&outputEntity->visualId, sizeof(int), 1, file);

            if (outputEntity->numChildren)
            {
                outputEntity->pChildIDs = new int[outputEntity->numChildren];
                fread(outputEntity->pChildIDs, sizeof(int), outputEntity->numChildren, file);
            }
            else
                outputEntity->pChildIDs = NULL;

            fread(&outputEntity->position, sizeof(Vector3), 1, file);
            fread(&outputEntity->rotation, sizeof(Vector3), 1, file);

            m_serialisedEntities.push_back(outputEntity);
        }

        // Read Scenes
        for (i = 0; i < numScenes; i++)
        {
            OutputScene *outputScene = new OutputScene;

            fread(&outputScene->size, sizeof(unsigned int), 1, file);
            fread(&outputScene->numGeometry, sizeof(unsigned int), 1, file);
            fread(&outputScene->numTreeNodes, sizeof(unsigned int), 1, file);
            fread(&outputScene->numTreeLeafs, sizeof(unsigned int), 1, file);
            fread(&outputScene->numPVSClusters, sizeof(unsigned int), 1, file);
            fread(&outputScene->bytesPerPVSCluster, sizeof(unsigned int), 1, file);

            fread(&outputScene->settings, sizeof(SceneSettings), 1, file);

            if (outputScene->numTreeNodes)
            {
                outputScene->nodes = new OutputTreeNode[outputScene->numTreeNodes];
                fread(outputScene->nodes, sizeof(OutputTreeNode), outputScene->numTreeNodes, file);
            }
            else
                outputScene->nodes = NULL;

            if (outputScene->numTreeLeafs)
            {
                outputScene->leafs = new OutputTreeLeaf[outputScene->numTreeLeafs];

                for (unsigned int i = 0; i < outputScene->numTreeLeafs; i++)
                {
                    fread(&outputScene->leafs[i].size, sizeof(unsigned int), 1, file);
                    fread(&outputScene->leafs[i].numRenderGroups, sizeof(unsigned int), 1, file);
                    fread(&outputScene->leafs[i].min, sizeof(Vector3), 1, file);
                    fread(&outputScene->leafs[i].max, sizeof(Vector3), 1, file);

                    outputScene->leafs[i].renderList = new OutputRenderGroup[outputScene->leafs[i].numRenderGroups];

                    for (unsigned int r = 0; r < outputScene->leafs[i].numRenderGroups; r++)
                    {
                        fread(&outputScene->leafs[i].renderList[r].geometryID, sizeof(unsigned int), 1, file);
                        fread(&outputScene->leafs[i].renderList[r].startFaceIndex, sizeof(unsigned int), 1, file);
                        fread(&outputScene->leafs[i].renderList[r].numFaces, sizeof(unsigned int), 1, file);
                        fread(&outputScene->leafs[i].renderList[r].effectID, sizeof(unsigned int), 1, file);
                    }
                }
            }
            else
                outputScene->leafs = NULL;

            if (outputScene->numPVSClusters && outputScene->bytesPerPVSCluster)
            {
                outputScene->PVS = new byte[outputScene->numPVSClusters * outputScene->bytesPerPVSCluster];
                fread(outputScene->PVS, outputScene->bytesPerPVSCluster, outputScene->numPVSClusters, file);
            }
            else
                outputScene->PVS = NULL;

            if (outputScene->numGeometry)
            {
                outputScene->pGeometryIDs = new unsigned int[outputScene->numGeometry];
                fread(outputScene->pGeometryIDs, sizeof(unsigned int), outputScene->numGeometry, file);
            }
            else
                outputScene->pGeometryIDs = NULL;

            fread(&outputScene->spacePartitionGeometryID, sizeof(unsigned int), 1, file);
            fread(&outputScene->sceneGraphRootEntityID, sizeof(int), 1, file);

            m_serialisedScenes.push_back(outputScene);
        }

        fclose(file);

        if (geometryTable)
            delete[] geometryTable;

        if (entityTable)
            delete[] entityTable;

        if (sceneTable)
            delete[] sceneTable;

        if (FAILED(Reassemble(flags, pKSRInterface)))
            return E_FAIL;

        return S_OK;
    }

// EOF