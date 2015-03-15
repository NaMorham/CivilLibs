/*! \file */
/*-----------------------------------------------------------------------
    resource.h

    Description: Resource and Memory Management
    Author:         Julian McKinlay
    ---------------------------------------------------------------------*/

#pragma once

struct ResourceManager
{
public:
    ResourceManager();
    ~ResourceManager();

    HRESULT    RegisterMemObject(MemObject *memObject);
    HRESULT    UnregisterMemObject(MemObject *memObject);

    HRESULT RegisterTexture(DWORD type, LPCTSTR filename);
    HRESULT RegisterTexture(LPDIRECT3DBASETEXTURE9 texture);
    HRESULT RegisterVisual(PVISUAL visual);
    HRESULT RegisterEffect(PEFFECT effect);
    HRESULT RegisterEffectTechnique(const String &handle);
    HRESULT RegisterMaterial(D3DMATERIAL9 &material);
    HRESULT RegisterLight(D3DLIGHT9 &light);

    HRESULT DestroyTexture(int id);
    HRESULT DestroyVisual(int id);
    HRESULT DestroyEffect(int id);

    void SetDefaultEffect(PEFFECT effect);
    void SetD3DDevice(LPDIRECT3DDEVICE9 pDevice);

    void CleanUp();

    const DWORD GetUsedMemory() const;

    const LPDIRECT3DBASETEXTURE9 GetTexture(int index) const;
    const PVISUAL GetVisual(int index) const;
    const PEFFECT GetEffect(int index) const;
    const D3DMATERIAL9 *GetMaterial(int index) const;
    const String &GetEffectTechnique(int index) const;
    const int GetEffectTechnique(const String &handle) const;
    const D3DLIGHT9 *GetLight(int index) const;

    D3DLIGHT9 *GetLight(int index);

    const size_t GetNumTextures() const { return m_textures.size(); };
    const size_t GetNumVisuals() const { return m_visuals.size(); };
    const size_t GetNumEffects() const { return m_effects.size(); };
    const size_t GetNumEffectTechniques() const { return m_effectTechniques.size(); };
    const size_t GetNumMaterials() const { return m_materials.size(); };
    const size_t GetNumLights() const { return m_lights.size(); };

private:
    LPDIRECT3DDEVICE9 m_pD3DDevice;

    std::list<MemObject *> m_memObjects;
    std::vector<LPDIRECT3DBASETEXTURE9> m_textures;
    std::vector<PVISUAL> m_visuals;
    std::vector<PEFFECT> m_effects;
    std::map< int, String > m_effectTechniques;
    std::vector<D3DMATERIAL9> m_materials;
    std::vector<D3DLIGHT9> m_lights;

    PEFFECT m_defaultEffect;
    D3DMATERIAL9 m_defaultMaterial;
};

typedef ResourceManager *PRESOURCEMANAGER;

// EOF