#include "ksr.h"

#include "leakwatcher.h"

#ifdef _DO_MEMORY_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace KSR;

//-----------------------------------------------------------------------
	PatchBlock::PatchBlock()
	//-------------------------------------------------------------------
	{
		x = y = size = 0;
		deltaMax = 0;

		stride = 0;

		visible = false;

		splitChildren = false;

		childUpdateCount = 0;

		parent = NULL;

		children = new PatchBlock *[4];
		children[0] = NULL;
		children[1] = NULL;
		children[2] = NULL;
		children[3] = NULL;
	}


//-----------------------------------------------------------------------
	Patch::Patch(LPDIRECT3DDEVICE9 pDevice)
	//-------------------------------------------------------------------
	{
		AddUsedMemory(sizeof(Patch), "Patch::Patch()");

		D3DDevice = pDevice;

		controlGeometry = NULL;
		renderGeometry = new Geometry(D3DDevice);

		quality = blockWidth = vertexWidth = controlVertexWidth = 0;

		divisionTree = NULL;

		D3DXMatrixIdentity(&frustum);

		root = NULL;

		blocks.clear();
		activeBlocks.clear();

		numStaticBlocks = 0;
	}


//-----------------------------------------------------------------------
	Patch::~Patch()
	//-------------------------------------------------------------------
	{
		Logf("Destroying Patch...");

		FreeUsedMemory(sizeof(Patch), "Patch::~Patch()");
	}


//-----------------------------------------------------------------------
	HRESULT Patch::SetGeometry(PGEOMETRY pGeometry)
	//-------------------------------------------------------------------
	{
		controlGeometry = pGeometry;

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Patch::Generate(int patchQuality)
	//-------------------------------------------------------------------
	{
/*
		if (!controlGeometry || patchQuality < 2)
			return E_FAIL;

		quality = patchQuality;

		blockWidth = pow(2, quality) + 1;
		vertexWidth = 3 + ((blockWidth - 1) * 2);

		controlVertexWidth = sqrt((float)controlGeometry->GetNumVertices());
		controlBlockWidth  = controlVertexWidth - 1;

		divisionTree = new int[blockWidth * blockWidth];

		// Create root block
			root = new PatchBlock;
			root->x = controlGeometry->GetMin().x;
			root->y = controlGeometry->GetMin().y;
			root->stride = blockWidth;
			numStaticBlocks = 1;

			float dx = controlGeometry->GetMax().x - controlGeometry->GetMin().x;
			float dy = controlGeometry->GetMax().y - controlGeometry->GetMin().y;

			if (dx > dy)
				root->size = dx;
			else
				root->size = dy;

			LPVOID pVertsLocked = NULL;
			LPVOID pIndicesLocked = NULL;

			controlGeometry->Lock(NULL, &pVertsLocked, NULL, &pIndicesLocked);

			RecurseGenerateBlocks(root, pVertsLocked, pIndicesLocked);

			controlGeometry->Unlock();

			controlGeometry->GenerateIndexBuffers(0);

			//blocks.push_back(root);

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Patch::Update(PVIEWPORT pViewport)
	//-------------------------------------------------------------------
	{
		if (!pViewport)
			return E_FAIL;

		if (frustum == pViewport->GetCameraFrustum())
			return S_OK;

		frustum = pViewport->GetCameraFrustum();

		ViewportSettings vps;
		pViewport->GetSettings(&vps);

//		// Build activeBlocks
//			activeBlocks.clear();
//
//			std::list<PatchBlock *>::iterator i = blocks.begin();
//			for (; i != blocks.end(); i++)
//			{
//				if (1) // if active
//					activeBlocks.push_back(*i);
//			}

		// Update active blocks
			//CalculateBlockDeltas(vps.width, vps.height);
			//UpdateBlocks();

		activeBlocks.clear();

		RecurseUpdateBlocks(root, (float)vps.width, (float)vps.height);
		RecurseGetActiveBlocks(root);

		// Temp introspection
			RECT rect;
			rect.top = 0;
			rect.left = 0;
			rect.bottom = vps.height / 12;
			rect.right = vps.width;

			char buf[128];
			sprintf(buf, "numStaticBlocks = %d, numActiveBlocks = %d", numStaticBlocks, activeBlocks.size());
			pViewport->SetFontFormat(&rect, DT_CENTER | DT_NOCLIP | DT_VCENTER, 0xffff00ff);
			pViewport->Drawtext(buf);
//*/
		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Patch::CreateRenderList(std::vector<RenderGroup> &renderList)
	//-------------------------------------------------------------------
	{
		/*std::list<PatchBlock *>::iterator i = activeBlocks.begin();
		for (; i != activeBlocks.end(); i++)
		{
			RenderGroup group;
			group.pGeometry = controlGeometry;
			group.startFaceIndex = 0;
			group.numFaces = effectGroups[i].size();
			group.effectID = i - 1;
			group.hasTransform = false;
		}*/

		RenderGroup group(NULL);
		group.m_pGeometry = controlGeometry;
		group.m_startFaceIndex = 0;
		group.m_numFaces = controlGeometry->GetNumChunkIndices();
		group.m_effectID = -1;
		group.m_hasTransform = false;
		renderList.push_back(group);

		//RenderGeometry();
/*
		if (viewport == pViewport && frustum == pViewport->GetCameraFrustum())
		{
			RenderGeometry();
		}
		else
		{
			viewport = pViewport;
			frustum = pViewport->GetCameraFrustum();

			CalculateBlockDeltas();
			UpdateBlocks();

			RenderGeometry();
		}
*/
		/*
		std::vector<PatchBlock *>::iterator i = activeBlocks.begin();
		for (; i != activeBlocks.end(); i++)
			RenderBlock((*i));*/

		// Temp Introspection
		/*
			ViewportSettings vps;
			pViewport->GetSettings(&vps);

			RECT rect;
			rect.top = 0;
			rect.left = 0;
			rect.bottom = vps.height / 12;
			rect.right = vps.width;

			char buf[128];
			sprintf(buf, "numTotalBlocks = %d, numActiveBlocks = %d", blocks.size(), activeBlocks.size());
			pViewport->SetFontFormat(&rect, DT_CENTER | DT_NOCLIP | DT_VCENTER, 0xffff00ff);
			pViewport->DrawText(buf);*/

		return S_OK;
	}


//-----------------------------------------------------------------------
	void Patch::RenderGeometry()
	//-------------------------------------------------------------------
	{
		/*
		D3DDevice->SetStreamSource(0, renderGeometry->GetVertexBuffer(), 0, renderGeometry->vertexLength);
		D3DDevice->SetIndices(renderGeometry->GetIndexBuffer());
		D3DDevice->SetFVF(renderGeometry->FVF);

		for (int i = 0; i < renderGeometry->numChunks; i++)
		{
			Chunk chunk = renderGeometry->chunks[i];
			D3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, 0, 0, chunk.numVerts, chunk.startIndex, chunk.numVerts - 2);
		}*/
	}


//-----------------------------------------------------------------------
	void Patch::CalculateBlockDeltas(float width, float height)
	//-------------------------------------------------------------------
	{
		std::vector<PatchBlock *>::iterator i = activeBlocks.begin();
		for (; i != activeBlocks.end(); i++)
		{
			(*i)->deltaMax = CalculateBlockDelta((*i), width, height);

			float lowerBound = 0;
			float upperBound = 1;

			if ((*i)->deltaMax < lowerBound)
				(*i)->childUpdateCount++;
			else if ((*i)->deltaMax > upperBound)
				(*i)->splitChildren = true;
		}
	}


//-----------------------------------------------------------------------
	void Patch::UpdateBlocks()
	//-------------------------------------------------------------------
	{
		//RecurseUpdateBlocks(root, width, height);
		//RecurseGetActiveBlocks(root);
/*
		std::vector<PatchBlock *>::iterator i = activeBlocks.begin();
		for (; i != activeBlocks.end(); i++)
		{
			if ((*i)->splitChildren)
			{
				// Create 4 child blocks

				(*i)->visible = false;
			}
			else if ((*i)->childUpdateCount >= 4)
			{
				// Destroy child blocks

				(*i)->visible = true;
			}

			(*i)->splitChildren = false;
			(*i)->childUpdateCount = 0;
		}*/
	}


//-----------------------------------------------------------------------
	float Patch::CalculateBlockDelta(PatchBlock *block, float width, float height)
	//-------------------------------------------------------------------
	{
		if (!block)
			return 0;

		int x = (int)block->x;
		int y = (int)block->y;
		int size = (int)block->size;

		int s2 = size - 1;
		int s1 = s2 / 2;
/*
		int n0 = (y + s1) * vertexWidth + (x + s1);
		int n1 = y * vertexWidth + x;
		int n2 = y * vertexWidth + (x + s1);
		int n3 = y * vertexWidth + (x + s2);
		int n4 = (y + s1) * vertexWidth + (x + s2);
		int n5 = (y + s2) * vertexWidth + (x + s2);
		int n6 = (y + s2) * vertexWidth + (x + s1);
		int n7 = (y + s2) * vertexWidth + x;
		int n8 = (y + s1) * vertexWidth + x;

		Vector3 v0 = geometry->GetVertexPosition(vertices, n0);
		Vector3 v1 = geometry->GetVertexPosition(vertices, n1);
		Vector3 v2 = geometry->GetVertexPosition(vertices, n2);
		Vector3 v3 = geometry->GetVertexPosition(vertices, n3);
		Vector3 v4 = geometry->GetVertexPosition(vertices, n4);
		Vector3 v5 = geometry->GetVertexPosition(vertices, n5);
		Vector3 v6 = geometry->GetVertexPosition(vertices, n6);
		Vector3 v7 = geometry->GetVertexPosition(vertices, n7);
		Vector3 v8 = geometry->GetVertexPosition(vertices, n8);

		float delta[5];
		delta[0] = CalculateVertexDelta(v0, (v1 + v3 + v5 + v7) * 0.25f);
		delta[1] = CalculateVertexDelta(v2, (v1 + v3) * 0.5f); 
		delta[2] = CalculateVertexDelta(v4, (v3 + v5) * 0.5f);
		delta[3] = CalculateVertexDelta(v6, (v5 + v7) * 0.5f);
		delta[4] = CalculateVertexDelta(v8, (v7 + v1) * 0.5f);

		float dMax = delta[0];

		for (int i = 1; i < 5; i++)
		{
			if (delta[i] > dMax)
				dMax = delta[i];
		}
*/
		//return dMax;

		return 1;
	}


//-----------------------------------------------------------------------
	float Patch::CalculateVertexDelta(Vector3 p, Vector3 neighboursAvg, float width, float height)
	//-------------------------------------------------------------------
	{
		Vector3 vProj3, vNeighboursProj3;
		D3DXVec3TransformCoord(&vProj3, &p, &frustum);
		D3DXVec3TransformCoord(&vNeighboursProj3, &neighboursAvg, &frustum);

		//ViewportSettings vps;
		//viewport->GetSettings(&vps);

		float w = width * 0.5f;
		float h = height * 0.5f;

		Vector2 vProj2, vNeighboursProj2;
		vProj2.x = w + vProj3.x / vProj3.z * w;
		vProj2.y = h + vProj3.y / vProj3.z * h;
		vNeighboursProj2.x = w + vNeighboursProj3.x / vNeighboursProj3.z * w;
		vNeighboursProj2.y = h + vNeighboursProj3.y / vNeighboursProj3.z * h;

		return Math::Length(vProj2 - vNeighboursProj2);
	}


//-----------------------------------------------------------------------
	void Patch::RecurseGenerateBlocks(PatchBlock *block, LPVOID pVertsLocked, LPVOID pIndicesLocked)
	//-------------------------------------------------------------------
	{
/*
		if (!block)
			return;

		float d = (float)blockWidth / (float)controlBlockWidth;

		Chunk chunk;
		chunk.idTexture0 = -1;
		chunk.idTexture1 = -1;
		chunk.numVerts = 6;
		chunk.startIndex = controlGeometry->GetNumVertexIndices();
		chunk.type = CT_POLYGON;

		// Index control geometry
			int *chunkIndices = new int[6];

			int dBlockX = block->x * d;
			int dBlockY = block->y * d;
			int dStride = block->stride * d;

			chunkIndices[0] = (dBlockY + dStride * 0.5f) * controlVertexWidth + (dBlockX + dStride * 0.5f);
			chunkIndices[1] = dBlockY * controlVertexWidth + dBlockX;
			chunkIndices[2] = dBlockY * controlVertexWidth + (dBlockX + dStride);
			chunkIndices[3] = (dBlockY + dStride) * controlVertexWidth + (dBlockX + dStride);
			chunkIndices[4] = (dBlockY + dStride) * controlVertexWidth + dBlockX;
			chunkIndices[5] = (dBlockY + dStride * 0.5f) * controlVertexWidth + (dBlockX + dStride * 0.5f);

			controlGeometry->Insert(0, 0, 6, 0, NULL, NULL, chunkIndices, NULL, 0);

			delete[] chunkIndices;

		// Create children
		if (dStride <= 2)
			return;

		int childSize = block->size * 0.5f;
		int childStride = block->stride * 0.5f;

        block->children[0] = new PatchBlock;
		block->children[0]->x = block->x;
		block->children[0]->y = block->y;
		block->children[0]->size = childSize;
		block->children[0]->stride = childStride;

		block->children[1] = new PatchBlock;
		block->children[1]->x = block->x + childSize;
		block->children[1]->y = block->y;
		block->children[1]->size = childSize;
		block->children[1]->stride = childStride;

		block->children[2] = new PatchBlock;
		block->children[2]->x = block->x;
		block->children[2]->y = block->y + childSize;
		block->children[2]->size = childSize;
		block->children[2]->stride = childStride;

		block->children[3] = new PatchBlock;
		block->children[3]->x = block->x + childSize;
		block->children[3]->y = block->y + childSize;
		block->children[3]->size = childSize;
		block->children[3]->stride = childStride;

		numStaticBlocks += 4;

		RecurseGenerateBlocks(block->children[0], pVertsLocked, pIndicesLocked);
		RecurseGenerateBlocks(block->children[1], pVertsLocked, pIndicesLocked);
		RecurseGenerateBlocks(block->children[2], pVertsLocked, pIndicesLocked);
		RecurseGenerateBlocks(block->children[3], pVertsLocked, pIndicesLocked);
//*/
	}


//-----------------------------------------------------------------------
	void Patch::RecurseUpdateBlocks(PatchBlock *block, float width, float height)
	//-------------------------------------------------------------------
	{
		if (!block)
			return;

		block->deltaMax = CalculateBlockDelta(block, width, height);

		float lowerBound = 0;
		float upperBound = 1;
/*
		if (block->deltaMax < lowerBound)
			block->childUpdateCount++;
		else if (block->deltaMax > upperBound)
			block->splitChildren = true;*/

		if (block->deltaMax > upperBound)
		{
			if (block->stride > 1)
			{
				block->visible = false;

				block->children[0]->visible = true;
				block->children[1]->visible = true;
				block->children[2]->visible = true;
				block->children[3]->visible = true;

				RecurseUpdateBlocks(block->children[0], width, height);
				RecurseUpdateBlocks(block->children[1], width, height);
				RecurseUpdateBlocks(block->children[2], width, height);
				RecurseUpdateBlocks(block->children[3], width, height);
			}
			else
				block->visible = true;
		}
		else if (block->deltaMax < lowerBound)
		{
			block->visible = true;
		}
		else
		{
			block->visible = true;
		}
	}


//-----------------------------------------------------------------------
	void Patch::RecurseGetActiveBlocks(PatchBlock *block)
	//-------------------------------------------------------------------
	{
		if (!block)
			return;

        if (block->visible)
		{
			activeBlocks.push_back(block);
			return;
		}

		RecurseGetActiveBlocks(block->children[0]);
		RecurseGetActiveBlocks(block->children[1]);
		RecurseGetActiveBlocks(block->children[2]);
		RecurseGetActiveBlocks(block->children[3]);
	}

/*
//-----------------------------------------------------------------------
	Patch::Patch(LPDIRECT3DDEVICE9 pDevice)
	//-------------------------------------------------------------------
	{
		AddUsedMemory(sizeof(Patch));

		D3DDevice = pDevice;

		divisionTree = NULL;
		morphTree = NULL;
		roughTree = NULL;

		quality = 1;
		width = 0;
		threshold = 1;

		renderGeometry = new Geometry(D3DDevice);
	}


//-----------------------------------------------------------------------
	Patch::~Patch()
	//-------------------------------------------------------------------
	{
		Logf("Destroying Patch...");

		FreeUsedMemory(sizeof(Patch));

		delete[] divisionTree;
		delete[] morphTree;
		delete[] roughTree;
	}


//-----------------------------------------------------------------------
	HRESULT Patch::SetErrorThreshold(float errorThreshold)
	//-------------------------------------------------------------------
	{
		if (errorThreshold <= 0)
			return E_FAIL;

		threshold = errorThreshold;

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Patch::Create(int q)
	//-------------------------------------------------------------------
	{
		if (quality < 1)
			return E_FAIL;

		quality = q;

		delete[] divisionTree;
		delete[] morphTree;
		delete[] roughTree;

		width = pow(2, quality) + 1;
		int numDivisionTreeVerts = width * width;
		divisionTree = new int[numDivisionTreeVerts];
		morphTree = new float[numDivisionTreeVerts];
		roughTree = new float[numDivisionTreeVerts];

		for (int i = 0; i < numDivisionTreeVerts; i++)
		{
			divisionTree[i] = -1;
			morphTree[i] = 0;
			roughTree[i] = 0;
		}

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Patch::Render(Chunk *pChunk, LPGEOMETRY pGeometry, LPVIEWPORT pViewport)
	//-------------------------------------------------------------------
	{
		if (!pChunk || !pGeometry || !pViewport)
			return E_FAIL;

		// Reset, prepare for new render
			renderGeometry->Clear();
			renderGeometry->Init(sizeof(VERTEX_NORMAL_TEX1), FVF_NORMAL_TEX1, IT_32);

			for (int i = 0; i < width * width; i++)
			{
				divisionTree[i] = -1;
				morphTree[i] = 0;
				roughTree[i] = 0;
			}

		chunk = pChunk;
		geometry = pGeometry;
		viewport = pViewport;
		frustum = viewport->GetCamera()->GetFrustum();

		vertexWidth = sqrt(pChunk->numVerts);

		if (FAILED(geometry->Lock(NULL, &vertices, &indices)))
			return E_FAIL;

		RecurseCreateDivisionTree(0, 0, width);
		RecurseRenderPatch(0, 0, width);

		if (FAILED(geometry->Unlock()))
			return E_FAIL;

		for (i = 0; i < renderGeometry->numChunks; i++)
			RenderChunk(&renderGeometry->chunks[i]);

		return S_OK;
	}


//-----------------------------------------------------------------------
	float Patch::CalculateBlockDelta(int x, int y, int size)
	//-------------------------------------------------------------------
	{
		int s2 = size - 1;
		int s1 = s2 * 0.5f;

		int n0 = (y + s1) * vertexWidth + (x + s1);
		int n1 = y * vertexWidth + x;
		int n2 = y * vertexWidth + (x + s1);
		int n3 = y * vertexWidth + (x + s2);
		int n4 = (y + s1) * vertexWidth + (x + s2);
		int n5 = (y + s2) * vertexWidth + (x + s2);
		int n6 = (y + s2) * vertexWidth + (x + s1);
		int n7 = (y + s2) * vertexWidth + x;
		int n8 = (y + s1) * vertexWidth + x;

		Vector3 v0 = geometry->GetVertexPosition(vertices, n0);
		Vector3 v1 = geometry->GetVertexPosition(vertices, n1);
		Vector3 v2 = geometry->GetVertexPosition(vertices, n2);
		Vector3 v3 = geometry->GetVertexPosition(vertices, n3);
		Vector3 v4 = geometry->GetVertexPosition(vertices, n4);
		Vector3 v5 = geometry->GetVertexPosition(vertices, n5);
		Vector3 v6 = geometry->GetVertexPosition(vertices, n6);
		Vector3 v7 = geometry->GetVertexPosition(vertices, n7);
		Vector3 v8 = geometry->GetVertexPosition(vertices, n8);

		float delta[5];
		delta[0] = CalculateVertexDelta(v0, (v1 + v3 + v5 + v7) * 0.25f);
		delta[1] = CalculateVertexDelta(v2, (v1 + v3) * 0.5f);
		delta[2] = CalculateVertexDelta(v4, (v3 + v5) * 0.5f);
		delta[3] = CalculateVertexDelta(v6, (v5 + v7) * 0.5f);
		delta[4] = CalculateVertexDelta(v8, (v7 + v1) * 0.5f);

		float dMax = delta[0];

		for (int i = 1; i < 5; i++)
		{
			if (delta[i] > dMax)
				dMax = delta[i];
		}

		return dMax;
	}


//-----------------------------------------------------------------------
	float Patch::CalculateVertexDelta(Vector3 p, Vector3 neighboursAvg)
	//-------------------------------------------------------------------
	{
		// project
		Vector3 vProj3, vNeighboursProj3;
		D3DXVec3TransformCoord(&vProj3, &p, &frustum);
		D3DXVec3TransformCoord(&vNeighboursProj3, &neighboursAvg, &frustum);

		ViewportSettings vps;
		viewport->GetSettings(&vps);

		float w = vps.width * 0.5f;
		float h = vps.height * 0.5f;

		Vector2 vProj2, vNeighboursProj2;
		vProj2.x = w + vProj3.x / vProj3.z * w;
		vProj2.y = h + vProj3.y / vProj3.z * h;
		vNeighboursProj2.x = w + vNeighboursProj3.x / vNeighboursProj3.z * w;
		vNeighboursProj2.y = h + vNeighboursProj3.y / vNeighboursProj3.z * h;

		return Length(vProj2 - vNeighboursProj2);
	}


//-----------------------------------------------------------------------
	void Patch::RecurseCreateDivisionTree(int x, int y, int size)
	//-------------------------------------------------------------------
	{
		float blockDelta = CalculateBlockDelta(0, 0, vertexWidth);

		int s = size * 0.5f;

		if (blockDelta > threshold)
		{
			divisionTree[(y + s) * width + (x + s)] = 0;
		}
		else
		{
			if (s >= 10000)
			{
				divisionTree[(y + s) * width + (x + s)] = 1;

				RecurseCreateDivisionTree(x, y, s);
				RecurseCreateDivisionTree(x + s, y, s);
				RecurseCreateDivisionTree(x, y + s, s);
				RecurseCreateDivisionTree(x + s, y + s, s);
			}
			else
				divisionTree[(y + s) * width + (x + s)] = 0;
		}
	}


//-----------------------------------------------------------------------
	void Patch::RecurseRenderPatch(int x, int y, int size)
	//-------------------------------------------------------------------
	{
		int s = size * 0.5f;

		// If patch is to be drawn at this level
		if (divisionTree[(y + s) * width + (x + s)] == 0)
		{
			VERTEX_NORMAL_TEX1 verts[5];
			VERTEX_NORMAL_TEX1 *pVerts = (VERTEX_NORMAL_TEX1*)vertices;
			int *pIndices = (int*)indices;

			float dw = (float)vertexWidth / width;
			int dx = x * dw;
			int dy = y * dw;
			int ds = s * dw;
			int dsize = size * dw;
			int dxs = dx + ds;
			int dys = dy + ds;
			int dxsize = dx + dsize;
			int dysize = dy + dsize;

			verts[0] = pVerts[pIndices[chunk->startIndex + (dys * vertexWidth + dxs)]];
			verts[1] = pVerts[pIndices[chunk->startIndex + (dy * vertexWidth + dx)]];
			verts[2] = pVerts[pIndices[chunk->startIndex + (dy * vertexWidth + dxsize)]];
			verts[3] = pVerts[pIndices[chunk->startIndex + (dysize * vertexWidth + dxsize)]];
			verts[4] = pVerts[pIndices[chunk->startIndex + (dysize * vertexWidth + dx)]];

			Chunk renderChunk;
			renderChunk.depthBias = 0;
			renderChunk.idTexture0 = chunk->idTexture0;
			renderChunk.idTexture1 = chunk->idTexture1;
			renderChunk.numVerts = 5;
			renderChunk.startIndex = 0;
			renderChunk.type = FT_LINELIST;
			renderChunk.material = chunk->material;

			renderGeometry->Insert(1, renderChunk.numVerts, 0, &renderChunk, &verts, NULL, 0);
		}
		else
		{
			if (s >= 3)
			{
				RecurseRenderPatch(x, y, s);
				RecurseRenderPatch(x + s, y, s);
				RecurseRenderPatch(x, y + s, s);
				RecurseRenderPatch(x + s, y + s, s);
			}
		}
	}


//-----------------------------------------------------------------------
	void Patch::RenderChunk(Chunk *pChunk)
	//-------------------------------------------------------------------
	{
		if (!chunk)
			return;

		D3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, 0, 0, pChunk->numVerts, pChunk->startIndex, pChunk->numVerts - 2);
	}
*/

// EOF