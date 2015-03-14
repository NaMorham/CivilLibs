/*! \file */
/*-----------------------------------------------------------------------
	ksrxmesh.h

	Description: Keays Simulation & Rendering Extensions API mesh header
	Author:		 Julian McKinlay
	---------------------------------------------------------------------*/

#pragma once

/*-----------------------------------------------------------------------
	Enumerations
	---------------------------------------------------------------------*/
enum { FT_NEARESTPOINT, FT_BILINEAR };

/*-----------------------------------------------------------------------
	Structures
	---------------------------------------------------------------------*/
	struct Heightfield
	{
		int idTexture0,
			idTexture1,
			width;

		KSR::Vector3 scale;
		KSR::Vector2 uvScale;

		KSR::CHUNKTYPE type;
	};

	struct SimpleEntity : public KSR::Entity
	{
	private:
		SimpleEntity(KSR::PINTERFACE pKSRInterface, KSR::PGEOMETRY pGeometry, KSR::PVISUAL pVisual, int visualId,
					 bool redraw = true, bool update = true);

	public:
		~SimpleEntity();

		static SimpleEntity *Create(KSR::PINTERFACE pKSRInterface, bool redraw = true, bool update = true);

		void UpdateGeometry();

		KSR::PGEOMETRY GetGeometry() { return m_pGeometry; }
		KSR::PVISUAL GetVisual() { return m_pVisual; }
		int GetVisualID() const { return m_visualId; }

	private:
		KSR::PINTERFACE m_pKSR;
		KSR::PGEOMETRY m_pGeometry;
		KSR::PVISUAL m_pVisual;
		int m_visualId;
	};

	typedef SimpleEntity *PSIMPLEENTITY;


/*-----------------------------------------------------------------------
	Function Declarations
	---------------------------------------------------------------------*/
	//! \brief Loads a Keays UT File
	/*! Returns S_OK on success and E_FAIL on failure.
		\param pMesh [in] Pointer to a valid ID3DXMesh interface, representing the X Mesh to be converted.
		\param pGeometry [out] Pointer to a KSRGeometry structure to be filled with the converted geometry.*/
	HRESULT CovertXMesh(LPD3DXMESH pMesh, KSR::PGEOMETRY pGeometry);

#if 0
	//! \brief Creates a heightfield
	/*! Returns S_OK on success and E_FAIL on failure.
		\param heightTextureID Index of the managed texture representing heightmap data.
		\param textureID0 Index of the managed texture the first texture.
		\param textureID1 Index of the managed texture the second texture.
		\param width Width of the heightfield, in number of cells.
		\param scale Width of one cell.
		\param pScene	 [in] Pointer to a valid KSRInterface. Can be NULL if pGeometry is not NULL.
		\param pGeometry [in] Pointer to a KSRGeometry structure to be filled with the loaded geometry. Can be NULL. 
		\param pPatch [in] Pointer to a KSRPatch structure to be filled with the loaded geometry. Can be NULL.
		\param pKSR [in] Pointer to a KSRInterface. Cannot be NULL.*/
	HRESULT CreateHeightfieldFromTexture(int heightTextureID, int numDivisions, DWORD filterType, Heightfield *field,
										 KSR::PGEOMETRY pGeometry, KSR::PPATCH pPatch, KSR::PINTERFACE pKSR);
#endif

	//! \brief Creates a heightfield
	/*! Returns S_OK on success and E_FAIL on failure.
		\param heightTextureID Index of the managed texture representing heightmap data.
		\param textureID0 Index of the managed texture the first texture.
		\param textureID1 Index of the managed texture the second texture.
		\param width Width of the heightfield, in number of cells.
		\param scale Width of one cell.
		\param pScene	 [in] Pointer to a valid KSRInterface. Can be NULL if pGeometry is not NULL.
		\param pGeometry [in] Pointer to a KSRGeometry structure to be filled with the loaded geometry. Can be NULL. 
		\param pKSR [in] Pointer to a KSRInterface. Cannot be NULL.*/
	HRESULT CreateHeightfieldFromTexture(int heightTextureID, int numDivisions, DWORD filterType, Heightfield *field,
										 KSR::PGEOMETRY pGeometry, KSR::PINTERFACE pKSR);

// EOF