#include <ksr.h>
#include "..\include\ksrx.h"

using namespace KSR;

//-----------------------------------------------------------------------
	KSRX::SimpleEntity::SimpleEntity(PINTERFACE pKSRInterface, PGEOMETRY pGeometry, PVISUAL pVisual, 
									 int visualId, bool redraw, bool update)
	//-------------------------------------------------------------------
	{
		m_pKSR = pKSRInterface;
		m_pGeometry = pGeometry;
		m_pVisual = pVisual;
		m_visualId = visualId;

		m_pVisual->InsertGeometry(m_pGeometry);
		SetVisualID(m_visualId);
		SetRedraw(redraw ? RT_LOOP : RT_NONE);
		SetUpdate(update ? RT_LOOP : RT_NONE);
	}


//-----------------------------------------------------------------------
	KSRX::SimpleEntity::~SimpleEntity()
	//-------------------------------------------------------------------
	{
		// TODO: this isn't really a leak since KSR will clean it up eventually,
		// but a safe way should be found to Release m_pGeometry and Destroy m_pVisual.
	}


//-----------------------------------------------------------------------
	KSRX::SimpleEntity *KSRX::SimpleEntity::Create(PINTERFACE pKSRInterface, bool redraw, bool update)
	//-------------------------------------------------------------------
	{
		if (!pKSRInterface)
			return NULL;

		KSR::PGEOMETRY pGeometry;
		KSR::PVISUAL pVisual;
		int visualId;

		if (FAILED(pKSRInterface->CreateGeometry(&pGeometry)))
			return NULL;

        if (FAILED(pKSRInterface->CreateVisual(&pVisual, &visualId)))
			return NULL;

		SimpleEntity *pNewEntity = new SimpleEntity(pKSRInterface, pGeometry, pVisual, visualId,  redraw, update);

		return pNewEntity;
	}


//-----------------------------------------------------------------------
	void KSRX::SimpleEntity::UpdateGeometry()
	//-------------------------------------------------------------------
	{
		if (m_pVisual)
			m_pVisual->UpdateGeometry(m_pGeometry);
	}


#if 0
//-----------------------------------------------------------------------
	HRESULT KSRX::CreateHeightfieldFromTexture(int heightTextureID, int numDivisions, DWORD filterType, Heightfield *field,
											   PGEOMETRY pGeometry, PPATCH pPatch, PINTERFACE pKSR)
	//-------------------------------------------------------------------
	{
		if (heightTextureID < 0 || !pKSR)
			return E_FAIL;

		int width = field->width,
			widthVerts = width + 1,
			numChunks = numDivisions * numDivisions,
			numVerts = widthVerts * widthVerts,
			numCells = width * width,
			divisionWidth = (int)ceil((float)width / numDivisions),
			divisionWidthVerts = (int)ceil((float)widthVerts / numDivisions),
			numDivisionVerts = divisionWidthVerts * divisionWidthVerts,
			numDivisionCells = divisionWidth * divisionWidth;

		int numIndices,
			numDivisionIndices;

		if (field->type == CT_TRIANGLELIST)
		{
			numIndices = 6 * numCells;
			numDivisionIndices = 6 * numDivisionCells;
		}
		else
		{
			numIndices = numVerts;
			numDivisionIndices = numDivisionVerts;
		}

		float uvX = field->uvScale.x / width;
		float uvY = field->uvScale.y / width;

		int *textureBits = NULL;
		int textureWidth = 0, 
			textureHeight = 0, 
			texturePitch = 0;

		if (FAILED(pKSR->LockTexture(heightTextureID, &textureWidth, &textureHeight, &texturePitch, (PVOID*)&textureBits)))
			return E_FAIL;

		float *map = new float[textureWidth * textureWidth];

		int *texelStart = textureBits;
		for (int y = 0; y < textureWidth; y++)
		{
			int *texel = texelStart;
			for (int x = 0; x < textureWidth; x++)
			{
				int t = *texel++;
				int r = (t & 0x00ff0000) >> 16;
				int g = (t & 0x0000ff00) >> 8;
				int b = (t & 0x000000ff);

				map[y * textureWidth + x] = (r + g + b) / (3.0f * 255);
			}

			texelStart = (int*)((char*)texelStart + texturePitch);
		}

		if (FAILED(pKSR->UnlockTexture(heightTextureID)))
			return E_FAIL;

		Chunk *chunks = new Chunk[numChunks];
		VERTEX_NORMAL_TEX1 *vertices = new VERTEX_NORMAL_TEX1[numVerts];
		int *indices = new int[numIndices];
		//short int *indices = new short int[numIndices];

		for (int i = 0; i < numChunks; i++)
		{
			chunks[i].depthBias = 0;
			chunks[i].idTexture0 = field->idTexture0;
			chunks[i].idTexture1 = field->idTexture1;
			chunks[i].numVerts = numIndices;
			chunks[i].startIndex = 0;
			chunks[i].type = field->type;
		}

		for (y = 0; y < widthVerts; y++)
			for (int x = 0; x < widthVerts; x++)
			{
				float h = 0;

				float dx = (float)x / widthVerts;
				float dy = (float)y / widthVerts;

				if (filterType == FT_NEARESTPOINT)
				{
					int tx = Math::Round((float)textureWidth * dx);
					int ty = Math::Round((float)textureWidth * dy);

					h = map[ty * textureWidth + tx];
				}
				else if (filterType == FT_BILINEAR)
				{
					int numSteps = textureWidth / widthVerts;

					int tx = (int)ceil((float)textureWidth * dx);
					int ty = (int)ceil((float)textureWidth * dy);

					if (x > 0 && y > 0)
					{
						for (int fy = ty - numSteps; fy < ty; fy++)
							for (int fx = tx - numSteps; fx < tx; fx++)
								h += map[fy * textureWidth + fx];
					}

					if (x < width && y > 0)
					{
						for (int fy = ty - numSteps; fy < ty; fy++)
							for (int fx = tx; fx < tx + numSteps; fx++)
								h += map[fy * textureWidth + fx];
					}

					if (x > 0 && y < width)
					{
						for (int fy = ty; fy < ty + numSteps; fy++)
							for (int fx = tx - numSteps; fx < tx; fx++)
								h += map[fy * textureWidth + fx];
					}

					if (x < width && y < width)
					{
						for (int fy = ty; fy < ty + numSteps; fy++)
							for (int fx = tx; fx < tx + numSteps; fx++)
								h += map[fy * textureWidth + fx];
					}

					h /= numSteps * numSteps * 4;
				}

				int id = y * widthVerts + x;
				vertices[id].normal = Vector3(0, 1, 0);
				vertices[id].position = Vector3(x * field->scale.x, h * field->scale.y, y * field->scale.z);
				vertices[id].uv = Vector2(x * uvX, y * uvY);
			}

		if (field->type == CT_TRIANGLELIST)
		{
			int v = 0;
			for (y = 0; y < width; y++)
				for (int x = 0; x < width; x++)
				{
					int id = y * widthVerts + x;
					indices[v + 0] = id;
					indices[v + 1] = id + widthVerts;
					indices[v + 2] = id + widthVerts + 1;

					indices[v + 3] = id;
					indices[v + 4] = id + widthVerts + 1;
					indices[v + 5] = id + 1;

					v += 6;
				}
		}
		else if (field->type == CT_PATCH)
		{
			for (i = 0; i < numIndices; i++)
				indices[i] = i;
		}

		if (pGeometry)
		{
			pGeometry->Init(sizeof(VERTEX_NORMAL_TEX1), FVF_NORMAL_TEX1, IT_32);
			pGeometry->Insert(numChunks, numVerts, 0, numIndices, chunks, vertices, NULL, indices, 0);
		}

		if (pPatch && field->type == CT_PATCH)
		{
			//pPatch->InitGeometry(sizeof(VERTEX_NORMAL_TEX1), FVF_NORMAL_TEX1, IT_32);
			//pPatch->InsertGeometry(numChunks, numVerts, 0, numIndices, chunks, vertices, NULL, indices, 0);

			if (pGeometry)
				pPatch->SetGeometry(pGeometry);
			else
			{
				Geometry *pPatchGeometry;
				pKSR->CreateGeometry(&pPatchGeometry);

				pPatchGeometry->Init(sizeof(VERTEX_NORMAL_TEX1), FVF_NORMAL_TEX1, IT_32);
				//pPatchGeometry->Insert(numChunks, numVerts, 0, numIndices, chunks, vertices, NULL, indices, 0);
				pPatchGeometry->Insert(numChunks, numVerts, 0, 0, chunks, vertices, NULL, NULL, 0);

				pPatch->SetGeometry(pPatchGeometry);
			}
		}

		delete[] chunks;
		delete[] vertices;
		delete[] indices;

		return S_OK;
	}
#endif


//-----------------------------------------------------------------------
	HRESULT KSRX::CreateHeightfieldFromTexture(int heightTextureID, int numDivisions, DWORD filterType, Heightfield *field,
											   PGEOMETRY pGeometry, PINTERFACE pKSR)
	//-------------------------------------------------------------------
	{
		if (heightTextureID < 0 || !pKSR)
			return E_FAIL;

		int width = field->width,
			widthVerts = width + 1,
			numChunks = numDivisions * numDivisions,
			numVerts = widthVerts * widthVerts,
			numCells = width * width,
			divisionWidth = (int)ceil((float)width / numDivisions),
			divisionWidthVerts = (int)ceil((float)widthVerts / numDivisions),
			numDivisionVerts = divisionWidthVerts * divisionWidthVerts,
			numDivisionCells = divisionWidth * divisionWidth;

		int numIndices,
			numDivisionIndices;

		if (field->type == CT_TRIANGLELIST)
		{
			numIndices = 6 * numCells;
			numDivisionIndices = 6 * numDivisionCells;
		}
		else
		{
			numIndices = numVerts;
			numDivisionIndices = numDivisionVerts;
		}

		float uvX = field->uvScale.x / width;
		float uvY = field->uvScale.y / width;

		int *textureBits = NULL;
		int textureWidth = 0, 
			textureHeight = 0, 
			texturePitch = 0;

		if (FAILED(pKSR->LockTexture(heightTextureID, &textureWidth, &textureHeight, &texturePitch, (PVOID*)&textureBits)))
			return E_FAIL;

		float *map = new float[textureWidth * textureWidth];

		int *texelStart = textureBits;
		for (int y = 0; y < textureWidth; y++)
		{
			int *texel = texelStart;
			for (int x = 0; x < textureWidth; x++)
			{
				int t = *texel++;
				int r = (t & 0x00ff0000) >> 16;
				int g = (t & 0x0000ff00) >> 8;
				int b = (t & 0x000000ff);

				map[y * textureWidth + x] = (r + g + b) / (3.0f * 255);
			}

			texelStart = (int*)((char*)texelStart + texturePitch);
		}

		if (FAILED(pKSR->UnlockTexture(heightTextureID)))
			return E_FAIL;

		Chunk *chunks = new Chunk[numChunks];
		VERTEX_NORMAL_TEX1 *vertices = new VERTEX_NORMAL_TEX1[numVerts];
		int *indices = new int[numIndices];
		//short int *indices = new short int[numIndices];

		for (int i = 0; i < numChunks; i++)
		{
			chunks[i].depthBias = 0;
			chunks[i].idTexture0 = field->idTexture0;
			chunks[i].idTexture1 = field->idTexture1;
			chunks[i].numVerts = numIndices;
			chunks[i].startIndex = 0;
			chunks[i].type = field->type;
		}

		for (y = 0; y < widthVerts; y++)
			for (int x = 0; x < widthVerts; x++)
			{
				float h = 0;

				float dx = (float)x / widthVerts;
				float dy = (float)y / widthVerts;

				if (filterType == FT_NEARESTPOINT)
				{
					int tx = Math::Round((float)textureWidth * dx);
					int ty = Math::Round((float)textureWidth * dy);

					h = map[ty * textureWidth + tx];
				}
				else if (filterType == FT_BILINEAR)
				{
					int numSteps = textureWidth / widthVerts;

					int tx = (int)ceil((float)textureWidth * dx);
					int ty = (int)ceil((float)textureWidth * dy);

					if (x > 0 && y > 0)
					{
						for (int fy = ty - numSteps; fy < ty; fy++)
							for (int fx = tx - numSteps; fx < tx; fx++)
								h += map[fy * textureWidth + fx];
					}

					if (x < width && y > 0)
					{
						for (int fy = ty - numSteps; fy < ty; fy++)
							for (int fx = tx; fx < tx + numSteps; fx++)
								h += map[fy * textureWidth + fx];
					}

					if (x > 0 && y < width)
					{
						for (int fy = ty; fy < ty + numSteps; fy++)
							for (int fx = tx - numSteps; fx < tx; fx++)
								h += map[fy * textureWidth + fx];
					}

					if (x < width && y < width)
					{
						for (int fy = ty; fy < ty + numSteps; fy++)
							for (int fx = tx; fx < tx + numSteps; fx++)
								h += map[fy * textureWidth + fx];
					}

					h /= numSteps * numSteps * 4;
				}

				int id = y * widthVerts + x;
				vertices[id].normal = Vector3(0, 1, 0);
				vertices[id].position = Vector3(x * field->scale.x, h * field->scale.y, y * field->scale.z);
				vertices[id].uv = Vector2(x * uvX, y * uvY);
			}

		if (field->type == CT_TRIANGLELIST)
		{
			int v = 0;
			for (y = 0; y < width; y++)
				for (int x = 0; x < width; x++)
				{
					int id = y * widthVerts + x;
					indices[v + 0] = id;
					indices[v + 1] = id + widthVerts;
					indices[v + 2] = id + widthVerts + 1;

					indices[v + 3] = id;
					indices[v + 4] = id + widthVerts + 1;
					indices[v + 5] = id + 1;

					v += 6;
				}
		}
		else if (field->type == CT_PATCH)
		{
			for (i = 0; i < numIndices; i++)
				indices[i] = i;
		}

		if (pGeometry)
		{
			pGeometry->Init(sizeof(VERTEX_NORMAL_TEX1), FVF_NORMAL_TEX1, IT_32);
			pGeometry->Insert(numChunks, numVerts, 0, numIndices, chunks, vertices, NULL, indices, 0);
		}

		delete[] chunks;
		delete[] vertices;
		delete[] indices;

		return S_OK;
	}

// EOF