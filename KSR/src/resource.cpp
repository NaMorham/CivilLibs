#include "../include/ksr.h"

#include "../include/leakwatcher.h"

#ifdef _DO_MEMORY_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace KSR;

PRESOURCEMANAGER g_pResourceManager = NULL;

//-----------------------------------------------------------------------
	MemObject::MemObject()
	//-------------------------------------------------------------------
	:	m_refCount(1), m_usedMemory(0)
	//-------------------------------------------------------------------
	{
		g_pResourceManager->RegisterMemObject(this);

		AddUsedMemory(sizeof(MemObject), "MemObject::MemObject()");
	}


//-----------------------------------------------------------------------
	MemObject::~MemObject()
	//-------------------------------------------------------------------
	{
		FreeUsedMemory(sizeof(MemObject), "MemObject::~MemObject()");
	}


//-----------------------------------------------------------------------
	void MemObject::AddUsedMemory(const DWORD numBytes, const String &name)
	//-------------------------------------------------------------------
	{ 
		m_usedMemory += numBytes; 

#ifdef _DEBUG		
		Logf(LL_HIGHEST, _T("Usage [%s] at 0x%8.8x increased by %d to %d bytes"), name.c_str(), this, numBytes, m_usedMemory); 
#endif
	}
 

//-----------------------------------------------------------------------
	void MemObject::FreeUsedMemory(const DWORD numBytes, const String &name)
	//-------------------------------------------------------------------
	{ 
		m_usedMemory -= numBytes; 

#ifdef _DEBUG		
		Logf(LL_HIGHEST, _T("Usage [%s] at 0x%8.8x decreased by %d to %d bytes"), name.c_str(), this, numBytes, m_usedMemory); 
#endif
	}


//-----------------------------------------------------------------------
	void MemObject::AddRef()
	//-------------------------------------------------------------------
	{
		m_refCount++;
	}


//-----------------------------------------------------------------------
	void MemObject::Release()
	//-------------------------------------------------------------------
	{
		if (m_refCount > 0)
		{
			m_refCount--;
		}

		if (m_refCount < 1)
		{
			g_pResourceManager->UnregisterMemObject(this);

			delete this;
		}
	}


//-----------------------------------------------------------------------
	ResourceManager::ResourceManager()
	//-------------------------------------------------------------------
		:	m_pD3DDevice(NULL), m_defaultEffect(NULL)
	//-------------------------------------------------------------------
	{
		g_pResourceManager = this;

		ZeroMemory(&m_defaultMaterial, sizeof(D3DMATERIAL9));
		m_defaultMaterial.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
		m_defaultMaterial.Ambient = D3DXCOLOR(0.2f, 0.2f, 0.2f, 1.0f);
		m_defaultMaterial.Emissive = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);
		m_defaultMaterial.Specular = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);
		m_defaultMaterial.Power = 0.0f;
	}


//-----------------------------------------------------------------------
	void ResourceManager::SetD3DDevice(LPDIRECT3DDEVICE9 pDevice)
	//-------------------------------------------------------------------
	{
		m_pD3DDevice = pDevice;
	}


//-----------------------------------------------------------------------
	void ResourceManager::SetDefaultEffect(PEFFECT effect)
	//-------------------------------------------------------------------
	{
		m_defaultEffect = effect;
	}


//-----------------------------------------------------------------------
	ResourceManager::~ResourceManager()
	//-------------------------------------------------------------------
	{
		CleanUp();
	}


//-----------------------------------------------------------------------
	HRESULT ResourceManager::RegisterMemObject(MemObject *memObject)
	//-------------------------------------------------------------------
	{
		if (!memObject)
			return E_FAIL;

		m_memObjects.push_back(memObject);

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT ResourceManager::UnregisterMemObject(MemObject *memObject)
	//-------------------------------------------------------------------
	{
		if (!memObject)
			return E_FAIL;

		m_memObjects.remove(memObject);

		return S_OK;
	}

//-----------------------------------------------------------------------
	HRESULT ResourceManager::RegisterTexture(DWORD type, LPCTSTR filename)
	//-------------------------------------------------------------------
	{
		if (!m_pD3DDevice)
			return E_FAIL;

		LPDIRECT3DBASETEXTURE9 pBaseTexture = NULL;

		if (type == TT_TEXTURE)
		{
			LPDIRECT3DTEXTURE9 texture;
			if (FAILED(D3DXCreateTextureFromFileEx(m_pD3DDevice, 
												   filename,
												   D3DX_DEFAULT,
												   D3DX_DEFAULT,
												   1,
												   0,
												   D3DFMT_A8R8G8B8,
												   D3DPOOL_MANAGED,
												   D3DX_DEFAULT,
												   D3DX_DEFAULT,
												   0,
												   NULL,
												   NULL,
												   &texture)))
				return E_FAIL;

			pBaseTexture = texture;
		}
		else if (type == TT_CUBE)
		{
			LPDIRECT3DCUBETEXTURE9 texture;
			if (FAILED(D3DXCreateCubeTextureFromFileEx(m_pD3DDevice, 
												   filename,
												   D3DX_DEFAULT,
												   1,
												   0,
												   D3DFMT_A8R8G8B8,
												   D3DPOOL_MANAGED,
												   D3DX_DEFAULT,
												   D3DX_DEFAULT,
												   0,
												   NULL,
												   NULL,
												   &texture)))
				return E_FAIL;

			pBaseTexture = texture;
		}
		else if (type == TT_VOLUME)
		{
			LPDIRECT3DVOLUMETEXTURE9 texture;
			if (FAILED(D3DXCreateVolumeTextureFromFileEx(m_pD3DDevice, 
												   filename,
												   D3DX_DEFAULT,
												   D3DX_DEFAULT,
												   D3DX_DEFAULT,
												   1,
												   0,
												   D3DFMT_A8R8G8B8,
												   D3DPOOL_MANAGED,
												   D3DX_DEFAULT,
												   D3DX_DEFAULT,
												   0,
												   NULL,
												   NULL,
												   &texture)))
				return E_FAIL;

			pBaseTexture = texture;
		}
		else 
			return E_FAIL;

		m_textures.push_back(pBaseTexture);

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT ResourceManager::RegisterTexture(LPDIRECT3DBASETEXTURE9 texture)
	//-------------------------------------------------------------------
	{
		if (!texture)
			return E_FAIL;

		m_textures.push_back(texture);

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT ResourceManager::RegisterVisual(PVISUAL visual)
	//-------------------------------------------------------------------
	{
		if (!visual)
			return E_FAIL;

		m_visuals.push_back(visual);

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT ResourceManager::RegisterEffect(PEFFECT effect)
	//-------------------------------------------------------------------
	{
		if (!effect)
			return E_FAIL;

		m_effects.push_back(effect);

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT ResourceManager::RegisterMaterial(D3DMATERIAL9 &material)
	//-------------------------------------------------------------------
	{
		m_materials.push_back(material);

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT ResourceManager::RegisterLight(D3DLIGHT9 &light)
	//-------------------------------------------------------------------
	{
		m_lights.push_back(light);

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT ResourceManager::RegisterEffectTechnique(const String &handle)
	//-------------------------------------------------------------------
	{
		if (!handle.c_str())
			return E_FAIL;

		m_effectTechniques.insert(std::make_pair< int, String >(m_effectTechniques.size(), handle));

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT ResourceManager::DestroyTexture(int id)
	//-------------------------------------------------------------------
	{
		if (id >= (int)m_textures.size())
			return E_FAIL;

		if (!m_textures[id])
			return E_FAIL;

		m_textures[id]->Release();
		m_textures[id] = NULL;

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT ResourceManager::DestroyVisual(int id)
	//-------------------------------------------------------------------
	{
		if (id >= (int)m_visuals.size())
			return E_FAIL;

		if (!m_visuals[id])
			return E_FAIL;

		m_visuals[id]->Release();
		m_visuals[id] = NULL;

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT ResourceManager::DestroyEffect(int id)
	//-------------------------------------------------------------------
	{
		if (id >= (int)m_effects.size())
			return E_FAIL;

		if (!m_effects[id])
			return E_FAIL;

		m_effects[id]->Release();
		m_effects[id] = NULL;

		return S_OK;
	}


//-----------------------------------------------------------------------
	void ResourceManager::CleanUp()
	//-------------------------------------------------------------------
	{
		DWORD usedMemory = GetUsedMemory();

		/*while ((*memObjects.begin())->GetRefCount() > 0)
		{
			(*memObjects.begin())->Release();

			if (memObjects.empty())
				break;
		}*/

		while (!m_memObjects.empty())
		{
			delete *m_memObjects.begin();

			m_memObjects.erase(m_memObjects.begin());
		}

		std::vector<LPDIRECT3DBASETEXTURE9>::iterator t = m_textures.begin();
		for (; t != m_textures.end(); t++)
		{
			if (!(*t))
				continue;

			(*t)->Release();
			(*t) = NULL;
		}

		m_memObjects.clear();
		m_textures.clear();
		m_visuals.clear();
		m_materials.clear();
		m_effects.clear();

		DWORD freedMemory = usedMemory - GetUsedMemory();

		if (freedMemory > 0)
			Logf(LL_HIGHEST, _T("Cleaning up resources: Freed %d of %d bytes \n"), freedMemory, usedMemory);
	}


//-----------------------------------------------------------------------
	const DWORD ResourceManager::GetUsedMemory() const
	//-------------------------------------------------------------------
	{
		DWORD usedMemory = 0;

		std::list<MemObject *>::const_iterator i = m_memObjects.begin();
		for (; i != m_memObjects.end(); i++)
		{
			if (!(*i))
				continue;

			usedMemory += (*i)->GetUsedMemory();
		}

		return usedMemory;
	}


//-----------------------------------------------------------------------
	const LPDIRECT3DBASETEXTURE9 ResourceManager::GetTexture(int index) const
	//-------------------------------------------------------------------
	{
		if (index < 0 || index > (int)m_textures.size() - 1 || m_textures.empty())
			return NULL;

		return m_textures[index];
	}


//-----------------------------------------------------------------------
	const PVISUAL ResourceManager::GetVisual(int index) const
	//-------------------------------------------------------------------
	{
		if (index < 0 || index > (int)m_visuals.size() - 1 || m_visuals.empty())
			return NULL;

		return m_visuals[index];
	}


//-----------------------------------------------------------------------
	const PEFFECT ResourceManager::GetEffect(int index) const
	//-------------------------------------------------------------------
	{
		if (index < 0 || index > (int)m_effects.size() - 1 || m_effects.empty())
			return m_defaultEffect;

		return m_effects[index];
	}


/*
//-----------------------------------------------------------------------
	const String &ResourceManager::GetEffectTechnique(int index)
	//-------------------------------------------------------------------
	{
		std::map< int, String >::const_iterator i = m_effectTechniques.find(index);

		if (i == m_effectTechniques.end())
			return defaultEffectTechnique;

		return i->second;
	}
*/

//-----------------------------------------------------------------------
	const int ResourceManager::GetEffectTechnique(const String &handle) const
	//-------------------------------------------------------------------
	{
		std::map< int, String >::const_iterator i = m_effectTechniques.begin();

		for (int n = 0; i != m_effectTechniques.end(); i++)
		{
			if (i->second == handle)	// TODO: Determine if this should be case sensitive
				return n;
		}

		return -1;
	}


//-----------------------------------------------------------------------
	const D3DMATERIAL9 *ResourceManager::GetMaterial(int index) const
	//-------------------------------------------------------------------
	{
		if (index < 0 || index > (int)m_materials.size() - 1 || m_materials.empty())
			return &m_defaultMaterial;

		return &m_materials[index];
	}


//-----------------------------------------------------------------------
	const D3DLIGHT9 *ResourceManager::GetLight(int index) const
	//-------------------------------------------------------------------
	{
		if (index < 0 || index > (int)m_lights.size() - 1 || m_lights.empty())
			return NULL;

		return &m_lights[index];
	}


//-----------------------------------------------------------------------
	D3DLIGHT9 *ResourceManager::GetLight(int index)
	//-------------------------------------------------------------------
	{
		if (index < 0 || index > (int)m_lights.size() - 1 || m_lights.empty())
			return NULL;

		return &m_lights[index];
	}

// EOF