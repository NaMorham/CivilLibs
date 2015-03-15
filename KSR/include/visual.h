/*! \file */
/*-----------------------------------------------------------------------
    visual.h

    Description: Visual object declaration
    Author:         Julian McKinlay
    ---------------------------------------------------------------------*/

#pragma once

struct ResourceManager;
typedef ResourceManager *PRESOURCEMANAGER;

struct VisualSprite
{
    LPDIRECT3DVERTEXBUFFER9 m_pVertexBuffer;
    int m_textureId;
    bool m_fixedX, m_fixedY, m_fixedZ;
};

//! \struct Visual
struct Visual : public MemObject
{
public:
    //typedef GeometryList::const_iterator ConstGeometryIterator;
    typedef GeometryList::const_iterator ConstGeometryIterator;

    Visual(LPDIRECT3DDEVICE9 pDevice, PRESOURCEMANAGER pResourceManager);
    ~Visual();

    //! \brief Clears this Visual of Geometry and Sprites
    /*! Returns S_OK on success and E_FAIL on failure.*/
    HRESULT Clear();

    //! \brief Inserts a sprite
        /*! Returns S_OK on success and E_FAIL on failure.
            \param min Minimum point of the sprite, in model space
            \param max Maximum point of the sprite, in model space
            \param textureId texture ID for this sprite */
    HRESULT InsertSprite(Vector2 min, Vector2 max, bool fixedX, bool fixedY, bool fixedZ, int textureId);

    //! \brief Inserts a geometry object
        /*! Returns S_OK on success and E_FAIL on failure.
            \param pGeometry Pointer to a KSRGeometry structure, representing the geometry to insert. */
    HRESULT InsertGeometry(PGEOMETRY pGeometry);

    //! \brief Removes a geometry object
        /*! Returns S_OK on success and E_FAIL on failure.
            \param pGeometry Pointer to a KSRGeometry structure, representing the geometry to remove. */
    HRESULT RemoveGeometry(PGEOMETRY pGeometry);

    //! \brief Updates a modified geometry object
        /*! Returns S_OK on success and E_FAIL on failure.
            \param pGeometry Pointer to a KSRGeometry structure, representing the geometry to update. */
    HRESULT UpdateGeometry(PGEOMETRY pGeometry);

    HRESULT GetGeometry(ConstGeometryIterator *begin, ConstGeometryIterator *end) const;

    //! \brief Inserts geometry data
        /*! Returns S_OK on success and E_FAIL on failure.
            \param pGeometry Pointer to a KSRGeometry structure, representing the geometry to insert. */
    HRESULT CreateRenderList(std::vector<RenderGroup> &renderListToAdd, const Matrix &transform, bool renderOutline);

    //! \brief Gets this Visual's unique ID.
    /*! Returns an int representing the Visual's unique ID. */
    inline const int GetID() const { return m_id; }

    HRESULT SetID(int visualID);

    inline const Vector3 &GetMin() const { return m_min; }
    inline const Vector3 &GetMax() const { return m_max; }

private:
    int m_id;

    Vector3 m_min,
            m_max;

    GeometryList m_geometry;
    std::list<RenderGroup> m_renderList;
    std::list<VisualSprite *> m_sprites;

    PGEOMETRY m_pOutline;

    LPDIRECT3DDEVICE9 m_pD3DDevice;
    PRESOURCEMANAGER m_pResourceManager;
};

//! \typedef Visual *PVISUAL
typedef Visual *PVISUAL;

// EOF