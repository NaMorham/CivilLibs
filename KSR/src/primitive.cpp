#include "base.h"
#include "primitive.h"

//-----------------------------------------------------------------------
	KSRGeometry::KSRGeometry()
	//-------------------------------------------------------------------
	{
		primitiveType = PT_TRIANGLELIST;
		vertexLength = indexLength = 0;

		redraw = RT_NODRAW;
		vertexStride = 0;
		FVF = 0;

		verticesLocked = indicesLocked = false;

		vertexBuffer = NULL;
		indexBuffer = NULL;
	}


//-----------------------------------------------------------------------
	HRESULT KSRGeometry::Lock(LPVOID *pVertices, LPVOID *pIndices)
	//-------------------------------------------------------------------
	{
		if (vertexBuffer && pVertices)
		{
			if (FAILED(vertexBuffer->Lock(0, 0, pVertices, 0)))
				return E_FAIL;

			verticesLocked = true;
		}
		else
			pVertices = NULL;

		if (indexBuffer && pIndices)
		{
			if (FAILED(indexBuffer->Lock(0, 0, pIndices, 0)))
				return E_FAIL;

			indicesLocked = true;
		}
		else
			pIndices = NULL;

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT KSRGeometry::Unlock()
	//-------------------------------------------------------------------
	{
		if (vertexBuffer && verticesLocked)
		{
			if (FAILED(vertexBuffer->Unlock()))
				return E_FAIL;
			
			verticesLocked = false;
		}

		if (indexBuffer && indicesLocked)
		{
			if (FAILED(indexBuffer->Unlock()))
				return E_FAIL;

			indicesLocked = false;
		}

		return S_OK;
	}

// EOF