#include "../include/ksr.h"

#include "../include/leakwatcher.h"

#ifdef _DO_MEMORY_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace KSR;

//-----------------------------------------------------------------------
	Visual::Visual(PDIRECT3DDEVICE9 pDevice, PRESOURCEMANAGER pResourceManager)
	//-------------------------------------------------------------------
	:	m_pD3DDevice(pDevice), m_pResourceManager(pResourceManager)
	//-------------------------------------------------------------------
	{
		AddUsedMemory(sizeof(Visual), "Visual::Visual()");

		m_geometry.clear();
		m_renderList.clear();
		m_sprites.clear();

		m_pOutline = new Geometry(m_pD3DDevice);
		AddUsedMemory(sizeof(Geometry), "Visual::Visual() - Geometry");
	}


//-----------------------------------------------------------------------
	Visual::~Visual()
	//-------------------------------------------------------------------
	{
		Logf("Destroying Visual...");

		std::list<VisualSprite *>::iterator i = m_sprites.begin();
		for (; i != m_sprites.end(); i++)
		{
			if (!(*i))
				continue;

			if ((*i)->m_pVertexBuffer)
				(*i)->m_pVertexBuffer->Release();

			delete (*i);
			(*i) = NULL;
			FreeUsedMemory(sizeof(VisualSprite), "Visual::~Visual() - VisualSprite");
		}

		m_sprites.clear();
		m_geometry.clear();
	}


//-----------------------------------------------------------------------
	HRESULT Visual::Clear()
	//-------------------------------------------------------------------
	{
		m_sprites.clear();
		m_geometry.clear();
		m_renderList.clear();

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Visual::InsertSprite(Vector2 min, Vector2 max, bool fixedX, bool fixedY, bool fixedZ, int textureId)
	//-------------------------------------------------------------------
	{
		if (textureId < 0)
			return E_FAIL;

		VisualSprite *sprite = new VisualSprite;
		AddUsedMemory(sizeof(VisualSprite), "Visual::InsertSprite() - VisualSprite");

		sprite->m_fixedX = fixedX;
		sprite->m_fixedY = fixedY;
		sprite->m_fixedZ = fixedZ;

		m_pD3DDevice->CreateVertexBuffer(4 * sizeof(VERTEX_DIFFUSE_TEX1),
										 0,
										 FVF_DIFFUSE_TEX1,
										 D3DPOOL_MANAGED,
										 &sprite->m_pVertexBuffer,
										 NULL);

		VERTEX_DIFFUSE_TEX1 *pVerts = NULL;
		if (FAILED(sprite->m_pVertexBuffer->Lock(0, 0, (PVOID*)&pVerts, 0)))
			return E_FAIL;

		pVerts[0].position = Vector3(min.x, min.y, 0);
		pVerts[1].position = Vector3(min.x, max.y, 0);
		pVerts[2].position = Vector3(max.x, min.y, 0);
		pVerts[3].position = Vector3(max.x, max.y, 0);

		pVerts[0].color = 0xffffffff;
		pVerts[1].color = 0xffffffff;
		pVerts[2].color = 0xffffffff;
		pVerts[3].color = 0xffffffff;

		pVerts[0].uv = Vector2(0, 0);
		pVerts[1].uv = Vector2(0, 1);
		pVerts[2].uv = Vector2(1, 0);
		pVerts[3].uv = Vector2(1, 1);

		sprite->m_pVertexBuffer->Unlock();

		sprite->m_textureId = textureId;

		m_sprites.push_back(sprite);

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Visual::InsertGeometry(PGEOMETRY pGeometry)
	//-------------------------------------------------------------------
	{
		if (!pGeometry)
			return E_FAIL;

		Vector3 oldMin = m_min;
		Vector3 oldMax = m_max;

		if (m_geometry.size() < 1)
		{
			m_min = pGeometry->GetMin();
			m_max = pGeometry->GetMax();
		}
		else
		{
			Math::SortMinMax(pGeometry->GetMin(), pGeometry->GetMax(), m_min, m_max);
		}

		if (oldMin != m_min || oldMax != m_max)
		{
			VERTEX_DIFFUSE vertices[24];

			vertices[0].position = m_min;
			vertices[1].position = Vector3(m_max.x, m_min.y, m_min.z);

			vertices[2].position = Vector3(m_max.x, m_min.y, m_min.z);
			vertices[3].position = Vector3(m_max.x, m_min.y, m_max.z);

			vertices[4].position = Vector3(m_max.x, m_min.y, m_max.z);
			vertices[5].position = Vector3(m_min.x, m_min.y, m_max.z);

			vertices[6].position = Vector3(m_min.x, m_min.y, m_max.z);
			vertices[7].position = m_min;

			// Top face
			vertices[8].position = Vector3(m_min.x, m_max.y, m_min.z);
			vertices[9].position = Vector3(m_max.x, m_max.y, m_min.z);

			vertices[10].position = Vector3(m_max.x, m_max.y, m_min.z);
			vertices[11].position = Vector3(m_max.x, m_max.y, m_max.z);

			vertices[12].position = Vector3(m_max.x, m_max.y, m_max.z);
			vertices[13].position = Vector3(m_min.x, m_max.y, m_max.z);

			vertices[14].position = Vector3(m_min.x, m_max.y, m_max.z);
			vertices[15].position = Vector3(m_min.x, m_max.y, m_min.z);

			// side edges
			vertices[16].position = Vector3(m_min.x, m_min.y, m_min.z);
			vertices[17].position = Vector3(m_min.x, m_max.y, m_min.z);

			vertices[18].position = Vector3(m_max.x, m_min.y, m_min.z);
			vertices[19].position = Vector3(m_max.x, m_max.y, m_min.z);

			vertices[20].position = Vector3(m_max.x, m_min.y, m_max.z);
			vertices[21].position = Vector3(m_max.x, m_max.y, m_max.z);

			vertices[22].position = Vector3(m_min.x, m_min.y, m_max.z);
			vertices[23].position = Vector3(m_min.x, m_max.y, m_max.z);

			Chunk lines;
			lines.depthBias = 0;
			lines.idTexture0 = -1;
			lines.idTexture1 = -1;
			lines.numVerts = 24;
			lines.startIndex = 0;
			lines.effect = -1;
			lines.type = CT_LINELIST;

			if (FAILED(m_pOutline->Init(sizeof(VERTEX_DIFFUSE), FVF_DIFFUSE, IT_32)))
				return E_FAIL;

			if (FAILED(m_pOutline->Insert(1, 24, &lines, vertices, 0)))
				return E_FAIL;

			if (FAILED(m_pOutline->GenerateIndexBuffers(0)))
				return E_FAIL;
		}

		pGeometry->GenerateIndexBuffers(GF_ALLOWZEROCHUNKINDICES);
		m_geometry.push_back(pGeometry);

		unsigned int numEffectGroups = m_pResourceManager->GetNumEffects();
		std::vector< std::vector<int> > effectGroups;
		effectGroups.resize(numEffectGroups + 1);

		for (unsigned int i = 0; i < pGeometry->GetNumChunks(); i++)
		{
			const Chunk &chunk = (*pGeometry)[i];
			effectGroups[chunk.effect + 1].push_back(i);
		}

		for (i = 0; i < numEffectGroups + 1; i++)
		{
			if (effectGroups[i].size() < 1)
				continue;

			RenderGroup group(NULL);
			group.m_pGeometry = pGeometry;
			group.m_startFaceIndex = pGeometry->GetNumChunkIndices();
			group.m_numFaces = effectGroups[i].size();
			group.m_effectID = i - 1;
			group.m_techniqueID = -1;
			group.m_materialID = -1;
			group.m_order = 0;
			group.m_hasTransform = true;
			m_renderList.push_back(group);

			int *chunkIndices = new int[effectGroups[i].size()];

			for (size_t j = 0; j < effectGroups[i].size(); j++)
				chunkIndices[j] = effectGroups[i][j];

			pGeometry->Insert(0, 0, effectGroups[i].size(), 0, NULL, NULL, chunkIndices, NULL, 0);

			delete[] chunkIndices;
		}

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Visual::RemoveGeometry(PGEOMETRY pGeometry)
	//-------------------------------------------------------------------
	{
		if (!pGeometry)
			return E_FAIL;

		GeometryList::iterator i = m_geometry.begin();
		for (; i != m_geometry.end();)
		{
			if ((*i) == pGeometry)
				i = m_geometry.erase(i);
			else
				i++;
		}

		std::list<RenderGroup>::iterator r = m_renderList.begin();
		for (; r != m_renderList.end();)
		{
			if (r->m_pGeometry == pGeometry)
				r = m_renderList.erase(r);
			else
				r++;
		}

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Visual::UpdateGeometry(PGEOMETRY pGeometry)
	//-------------------------------------------------------------------
	{
		if (!pGeometry)
			return E_FAIL;

		RemoveGeometry(pGeometry);
		InsertGeometry(pGeometry);

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Visual::GetGeometry(ConstGeometryIterator *begin, ConstGeometryIterator *end) const
	//-------------------------------------------------------------------
	{
		*begin = m_geometry.begin();
		*end = m_geometry.end();

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Visual::CreateRenderList(std::vector<RenderGroup> &renderListToAdd, const Matrix &transform, 
									 bool renderOutline)
	//-------------------------------------------------------------------
	{
		std::list<RenderGroup>::iterator i = m_renderList.begin();
		for (; i != m_renderList.end(); i++)
		{
			i->m_transform = transform;
			renderListToAdd.push_back(*i);

			RenderGroup &rg = *i;
		}

		if (renderOutline)
		{
			RenderGroup outlineList(NULL);
			outlineList.m_pGeometry = m_pOutline;
			outlineList.m_startFaceIndex = 0;
			outlineList.m_numFaces = m_pOutline->GetNumChunkIndices();
			outlineList.m_materialID = -1;
			outlineList.m_effectID = -1;
			outlineList.m_techniqueID = -1;
			outlineList.m_order = 0;
			outlineList.m_hasTransform = true;
			outlineList.m_transform = transform;

			renderListToAdd.push_back(outlineList);
		}

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Visual::SetID(int visualID)
	//-------------------------------------------------------------------
	{
		if (visualID < 0)
			return E_FAIL;

		m_id = visualID;

		return S_OK;
	}


// EOF