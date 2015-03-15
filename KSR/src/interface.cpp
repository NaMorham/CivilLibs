#include "../include/ksr.h"

#define INITGUID
#include <initguid.h>
#include <dxdiag.h>
#include <string>

#include "../include/leakwatcher.h"

#ifdef _DO_MEMORY_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const TCHAR *cDefaultEffectStr = _T(
    "texture texture0 = NULL,"
    "         texture1 = NULL;"
    "technique Default"
    "{"
    "    pass P0"
    "    {"
    "        FillMode = SOLID;"
    "        CullMode = NONE;"
    "        ZEnable = TRUE;"
    "        ZWriteEnable = TRUE;"
    "        Lighting = FALSE;"
    "        ShadeMode = GOURAUD;"
    "        Texture[0] = <texture0>;"
    "        Texture[1] = <texture1>;"
    "        VertexShader = NULL;"
    "        PixelShader = NULL;"
    "    }"
    "}");

using namespace KSR;

void LogDiagnosticContainer(WCHAR *parentName, IDxDiagContainer *pContainer);

//-----------------------------------------------------------------------
    Interface::Interface(HWND hWnd, PCORESETTINGS pCoreSettings, LPCTSTR logFilename, int maxLogSize)
        :    m_pRenderer(NULL),
            m_pResourceManager(NULL),
            m_pFnProgressCallback(NULL)
    //-------------------------------------------------------------------
    {
        if (!pCoreSettings)
            return;

        m_pSettings = new CoreSettings;
        SetSettings(pCoreSettings);

        if (logFilename)
            m_logger.SetFile(true, logFilename);
        else
            m_logger.SetFile(false, NULL);

        m_logger.SetMaxSize(maxLogSize);
        m_logger.SetMinLevel(m_pSettings->minLogLevel);
        m_logger.SetMaxLevel(m_pSettings->maxLogLevel);

        m_pResourceManager = new ResourceManager;

        if (hWnd)
        {
            m_pRenderer = new Renderer(m_pResourceManager);

            if (FAILED(m_pRenderer->Create(hWnd, m_pSettings)))
                Logf("Renderer::Create() failed");

            m_pResourceManager->SetD3DDevice(m_pRenderer->GetD3DDevice());
        }
        else
        {
            Logf(LL_HIGHEST, "No Parent Window specified.");

            m_pResourceManager->SetD3DDevice(NULL);
        }

        m_scenes.clear();

        m_pTimer = new Timer;

        if (m_pSettings->createDefaultResources)
        {
            // Create default white texture
                int whiteBit = 0xffffffff;
                int redBit = 0xffff0000;
                int greenBit = 0xff00ff00;
                int blueBit = 0xff0000ff;
                CreateTexture(1, 1, &whiteBit, TT_ARGB32, NULL);
                CreateTexture(1, 1, &redBit, TT_ARGB32, NULL);
                CreateTexture(1, 1, &greenBit, TT_ARGB32, NULL);
                CreateTexture(1, 1, &blueBit, TT_ARGB32, NULL);

            // Create default materials
                CreateMaterial(0xffffffff, 0xff222222, 0xff000000, 0xff000000, 0, NULL);
                CreateMaterial(0xffff0000, 0xff220000, 0xff000000, 0xff000000, 0, NULL);
                CreateMaterial(0xff00ff00, 0xff002200, 0xff000000, 0xff000000, 0, NULL);
                CreateMaterial(0xff0000ff, 0xff000022, 0xff000000, 0xff000000, 0, NULL);
        }

        // Create default effect (regardless of createDefaultResources)
            PEFFECT defaultEffect = new Effect(m_pRenderer->GetD3DDevice(),
                                               m_pSettings->vertexShaderDebugging,
                                               m_pSettings->pixelShaderDebugging);

            if (SUCCEEDED(defaultEffect->CompileFromString(cDefaultEffectStr)))
                m_pResourceManager->SetDefaultEffect(defaultEffect);
    }


//-----------------------------------------------------------------------
    Interface::~Interface()
    //-------------------------------------------------------------------
    {
        Logf("Destroying Interface...");

        delete m_pSettings;
        delete m_pResourceManager;
    }


//-----------------------------------------------------------------------
    HRESULT Interface::CreateViewport(HWND hWnd, PVIEWPORTSETTINGS pSettings, PVIEWPORT *pViewport)
    //-------------------------------------------------------------------
    {
        if (!m_pRenderer)
            return E_FAIL;

        if (FAILED(m_pRenderer->CreateViewport(hWnd, pSettings, pViewport)))
            return E_FAIL;

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Interface::CreateScene(PSCENESETTINGS pSettings, PSCENE *pScene)
    //-------------------------------------------------------------------
    {
        if (!pScene || !m_pRenderer)
            return E_FAIL;

        *pScene = new Scene(pSettings, m_pRenderer->GetD3DDevice(), m_pResourceManager);

        if (!(*pScene))
            return E_FAIL;

        //(*pScene)->SetProgressCallback(progressCallback, NULL);

        m_scenes.push_back(*pScene);

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Interface::CreateVisual(PVISUAL *pVisual, int *id)
    //-------------------------------------------------------------------
    {
        if (!pVisual || !m_pRenderer)
            return E_FAIL;

        *pVisual = new Visual(m_pRenderer->GetD3DDevice(), m_pResourceManager);

        if (!(*pVisual))
            return E_FAIL;

        m_pResourceManager->RegisterVisual(*pVisual);

        int visualID = m_pResourceManager->GetNumVisuals() - 1;

        if (id)
            *id = visualID;

        (*pVisual)->SetID(visualID);

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Interface::CreateEntity(PENTITY *pEntity)
    //-------------------------------------------------------------------
    {
        if (!pEntity)
            return E_FAIL;

        *pEntity = new Entity;

        if (!(*pEntity))
            return E_FAIL;

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Interface::CreateTexture(int width, int height, int *pBits, DWORD flags, int *pID)
    //-------------------------------------------------------------------
    {
        if (!m_pRenderer)
            return E_FAIL;

        if (flags & TT_RENDERTARGET && pBits)
            return E_FAIL;

        DWORD usage = 0;
        D3DPOOL pool = D3DPOOL_MANAGED;
        D3DFORMAT colourFormat = D3DFMT_UNKNOWN;

        if (flags & TT_RENDERTARGET)
        {
            usage = D3DUSAGE_RENDERTARGET; // | D3DUSAGE_WRITEONLY;
            pool = D3DPOOL_DEFAULT;
        }

        if (flags & TT_ARGB32)
            colourFormat = D3DFMT_A8R8G8B8;

        LPDIRECT3DTEXTURE9 texture = NULL;
        if (FAILED(m_pRenderer->GetD3DDevice()->CreateTexture(width, height, 1, usage, colourFormat, pool, &texture, 0)))
            return E_FAIL;

        if (!(usage & D3DUSAGE_RENDERTARGET) && pBits)
        {
            D3DLOCKED_RECT rect;
            if (FAILED(texture->LockRect(0, &rect, 0, 0)))
                return E_FAIL;

            int *texelStart = (int*)((char*)rect.pBits);

            for (int y = 0; y < height; y++)
            {
                int *texel = texelStart;

                for (int x = 0; x < width; x++)
                    *texel++ = pBits[y * width + x];

                texelStart = (int*)((char*)texelStart + rect.Pitch);
            }

            if (FAILED(texture->UnlockRect(0)))
                return E_FAIL;
        }

        m_pResourceManager->RegisterTexture(texture);

        if (pID)
            *pID = m_pResourceManager->GetNumTextures() - 1;

        return S_OK;
    }

/*
//-----------------------------------------------------------------------
    HRESULT Interface::CreateCubeTexture(int width, int *pBits, DWORD flags, int *pID)
    //-------------------------------------------------------------------
    {
        if (!renderer)
            return E_FAIL;

        if (flags & TT_RENDERTARGET && pBits)
            return E_FAIL;

        DWORD usage = 0;
        D3DPOOL pool = D3DPOOL_MANAGED;
        D3DFORMAT colourFormat = D3DFMT_UNKNOWN;

        if (flags & TT_RENDERTARGET)
        {
            usage |= D3DUSAGE_RENDERTARGET;
            pool = D3DPOOL_DEFAULT;
        }

        if (flags & TT_ARGB32)
            colourFormat = D3DFMT_A8R8G8B8;

        LPDIRECT3DCUBETEXTURE9 texture = NULL;
        if (FAILED(renderer->GetD3DDevice()->CreateCubeTexture(width, 1, usage, colourFormat, pool, &texture, 0)))
            return E_FAIL;

        if (!(usage & D3DUSAGE_RENDERTARGET) && pBits)
        {
            for (int i = 0; i < 6; i++)
            {
                D3DLOCKED_RECT rect;
                if (FAILED(texture->LockRect((D3DCUBEMAP_FACES)i, 0, &rect, 0, 0)))
                    return E_FAIL;

                int *texelStart = (int*)((char*)rect.pBits);

                for (int y = 0; y < width; y++)
                {
                    int *texel = texelStart;

                    for (int x = 0; x < width; x++)
                        *texel++ = pBits[i * y * width + x];

                    texelStart = (int*)((char*)texelStart + rect.Pitch);
                }

                if (FAILED(texture->UnlockRect((D3DCUBEMAP_FACES)i, 0)))
                    return E_FAIL;
            }
        }

        resourceManager->RegisterTexture(texture);

        if (pID)
            *pID = resourceManager->GetNumTextures() - 1;

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Interface::CreateVolumeTexture(int width, int height, int depth, int *pBits, DWORD flags, int *pID)
    //-------------------------------------------------------------------
    {
        if (!renderer)
            return E_FAIL;

        if (flags & TT_RENDERTARGET && pBits)
            return E_FAIL;

        DWORD usage = 0;
        D3DPOOL pool = D3DPOOL_MANAGED;
        D3DFORMAT colourFormat = D3DFMT_UNKNOWN;

        if (flags & TT_RENDERTARGET)
        {
            usage |= D3DUSAGE_RENDERTARGET;
            pool = D3DPOOL_DEFAULT;
        }

        if (flags & TT_ARGB32)
            colourFormat = D3DFMT_A8R8G8B8;

        LPDIRECT3DVOLUMETEXTURE9 texture = NULL;
        if (FAILED(renderer->GetD3DDevice()->CreateVolumeTexture(width, height, depth, 1, usage, colourFormat, pool, &texture, 0)))
            return E_FAIL;

        if (!(usage & D3DUSAGE_RENDERTARGET) && pBits)
        {
            D3DLOCKED_BOX box;
            if (FAILED(texture->LockBox(0, &box, 0, 0)))
                return E_FAIL;

            int *texelStart = (int*)((char*)box.pBits);

            for (int y = 0; y < height; y++)
            {
                int *texel = texelStart;

                for (int x = 0; x < width; x++)
                    *texel++ = pBits[y * width + x];

                texelStart = (int*)((char*)texelStart + box.Pitch);
            }

            if (FAILED(texture->UnlockBox(0)))
                return E_FAIL;
        }

        resourceManager->RegisterTexture(texture);

        if (pID)
            *pID = resourceManager->GetNumTextures() - 1;

        return S_OK;
    }*/


#if 0
//-----------------------------------------------------------------------
    HRESULT Interface::CreatePatch(PPATCH *pPatch)
    //-------------------------------------------------------------------
    {
        if (!pPatch || !m_pRenderer)
            return E_FAIL;

        *pPatch = new Patch(m_pRenderer->GetD3DDevice());

        if (!(*pPatch))
            return E_FAIL;

        return S_OK;
    }
#endif


//-----------------------------------------------------------------------
    HRESULT Interface::CreateSerialiser(PSERIALISER *pSerialiser)
    //-------------------------------------------------------------------
    {
        if (!pSerialiser || !m_pRenderer)
            return E_FAIL;

        *pSerialiser = new Serialiser(m_pRenderer->GetD3DDevice());

        if (!(*pSerialiser))
            return E_FAIL;

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Interface::CreateEffect(LPCTSTR pFXFile, LPCTSTR pFXCode, int codeLength, int *pID,
                                    LPCTSTR pDefaultTechnique, String *pErrorBuf)
    //-------------------------------------------------------------------
    {
        if (!m_pRenderer || (!pFXFile && !pFXCode))
            return E_FAIL;

        PEFFECT effect = new Effect(m_pRenderer->GetD3DDevice(),
                                    m_pSettings->vertexShaderDebugging,
                                    m_pSettings->pixelShaderDebugging);

        if (pFXFile)
        {
            if (FAILED(effect->CompileFromFile(pFXFile, pDefaultTechnique, pErrorBuf)))
                return E_FAIL;
        }
        else if (pFXCode)
        {
            if (FAILED(effect->CompileFromString(pFXCode, pDefaultTechnique, pErrorBuf)))
                return E_FAIL;
        }

        m_pResourceManager->RegisterEffect(effect);

        if (pID)
            *pID = m_pResourceManager->GetNumEffects() - 1;

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Interface::CreateMaterial(DWORD diffuse, DWORD ambient, DWORD specular, DWORD emissive, float specularPower, int *pID)
    //-------------------------------------------------------------------
    {
        D3DMATERIAL9 material;
        ZeroMemory(&material, sizeof(D3DMATERIAL9));

        material.Diffuse = D3DXCOLOR(diffuse);
        material.Ambient = D3DXCOLOR(ambient);
        material.Specular = D3DXCOLOR(specular);
        material.Emissive = D3DXCOLOR(emissive);
        material.Power = specularPower;

        m_pResourceManager->RegisterMaterial(material);

        if (pID)
            *pID = m_pResourceManager->GetNumMaterials() - 1;

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Interface::CreateGeometry(PGEOMETRY *pGeometry)
    //-------------------------------------------------------------------
    {
        if (!pGeometry || !m_pRenderer)
            return E_FAIL;

        (*pGeometry) = new Geometry(m_pRenderer->GetD3DDevice());

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Interface::CreateFont(int height, int width, int weight, bool italic,
                                  DWORD charSet, DWORD pitchAndFamily, LPCTSTR pFaceName, PFONT *pFont)
    //-------------------------------------------------------------------
    {
        if (!m_pRenderer)
            return E_FAIL;

        (*pFont) = new Font(m_pRenderer->GetD3DDevice());
        (*pFont)->Create(height, width, weight, italic, charSet, pitchAndFamily, pFaceName);

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Interface::InsertLight(const Light &light, int *pID)
    //-------------------------------------------------------------------
    {
        if (!m_pResourceManager)
            return E_FAIL;

        m_pResourceManager->RegisterLight((D3DLIGHT9)light);

        if (pID)
            *pID = m_pResourceManager->GetNumLights() - 1;

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Interface::DestroyScene(PSCENE pScene)
    //-------------------------------------------------------------------
    {
        if (!pScene)
            return E_FAIL;

        SceneList::iterator i = m_scenes.begin();
        for (; i != m_scenes.end();)
        {
            if ((*i) == pScene)
            {
                i = m_scenes.erase(i);

                pScene->Release();
            }
            else
            {
                i++;
            }

            break;
        }

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Interface::DestroyViewport(PVIEWPORT pViewport)
    //-------------------------------------------------------------------
    {
        if (!pViewport || !m_pRenderer)
            return E_FAIL;

        if (FAILED(m_pRenderer->DestroyViewport(pViewport)))
            return E_FAIL;

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Interface::DestroyTexture(int textureID)
    //-------------------------------------------------------------------
    {
        if (!m_pResourceManager)
            return E_FAIL;

        m_pResourceManager->DestroyTexture(textureID);

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Interface::DestroyVisual(int visualID)
    //-------------------------------------------------------------------
    {
        if (!m_pResourceManager)
            return E_FAIL;

        m_pResourceManager->DestroyVisual(visualID);

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Interface::DestroyEffect(int effectID)
    //-------------------------------------------------------------------
    {
        if (!m_pResourceManager)
            return E_FAIL;

        m_pResourceManager->DestroyEffect(effectID);

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Interface::SetEffectTechnique(int effectID, const String &handle)
    //-------------------------------------------------------------------
    {
        if (!m_pResourceManager)
            return E_FAIL;

#ifdef _DEBUG
        KSR::Logf(LL_LOWEST, _T("Setting effect technique \"%s\" on effect %d"), handle.c_str(), effectID);
#endif

        PEFFECT effect = m_pResourceManager->GetEffect(effectID);

        if (!effect)
            return E_FAIL;

        if (FAILED(effect->SetTechnique(handle.c_str())))
            return E_FAIL;

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Interface::SetEffectValue(int effectID, const String &handle, LPVOID pData, int size)
    //-------------------------------------------------------------------
    {
        if (!m_pResourceManager)
            return E_FAIL;

        PEFFECT effect = m_pResourceManager->GetEffect(effectID);

        if (!effect)
            return E_FAIL;

        if (FAILED(effect->SetValue(handle.c_str(), pData, size)))
            return E_FAIL;

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Interface::SetEffectLight(int effectID, int lightID, PLIGHT pLight)
    //-------------------------------------------------------------------
    {
        /*
        if (!resourceManager || !pLight)
            return E_FAIL;

        LPD3DXEFFECT effect = resourceManager->GetEffect(effectID);

        if (!effect)
            return E_FAIL;

        D3DXEFFECT_DESC desc;
        effect->GetDesc(&desc);

        char indexBuf[16];
        sprintf(indexBuf, "%d", lightID);

        std::string lightDirection = "lightDirection";
        std::string lightDiffuse   = "lightDiffuse";
        std::string lightAmbient   = "lightAmbient";
        std::string lightSpecular  = "lightSpecular";
        std::string lightRange       = "lightRange";

        std::string lightDirectionIndexed = lightDirection + indexBuf;
        std::string lightDiffuseIndexed = lightDiffuse + indexBuf;
        std::string lightAmbientIndexed = lightAmbient + indexBuf;
        std::string lightSpecularIndexed = lightSpecular + indexBuf;
        std::string lightRangeIndexed = lightRange + indexBuf;

        D3DXCOLOR diffuse  = D3DXCOLOR(pLight->diffuseColour);
        D3DXCOLOR ambient  = D3DXCOLOR(pLight->ambientColour);
        D3DXCOLOR specular = D3DXCOLOR(pLight->specularColour);

        for (int p = 0; p < desc.Parameters; p++)
        {
            D3DXHANDLE param = effect->GetParameter(NULL, p);

            D3DXPARAMETER_DESC paramDesc;
            effect->GetParameterDesc(param, &paramDesc);

            if (strcmp(paramDesc.Name, lightDirectionIndexed.c_str()) == 0)
                effect->SetVector(lightDirectionIndexed.c_str(), (Vector4*)&pLight->direction);
            else if (strcmp(paramDesc.Name, lightDiffuseIndexed.c_str()) == 0)
                effect->SetVector(lightDiffuseIndexed.c_str(), (Vector4*)&diffuse);
            else if (strcmp(paramDesc.Name, lightAmbientIndexed.c_str()) == 0)
                effect->SetVector(lightAmbientIndexed.c_str(), (Vector4*)&ambient);
            else if (strcmp(paramDesc.Name, lightSpecularIndexed.c_str()) == 0)
                effect->SetVector(lightSpecularIndexed.c_str(), (Vector4*)&specular);
            else if (strcmp(paramDesc.Name, lightRangeIndexed.c_str()) == 0)
                effect->SetFloat(lightRangeIndexed.c_str(), pLight->range);
        }
*/
        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Interface::SetEffectMaterial(int effectID, int materialID)
    //-------------------------------------------------------------------
    {
        /*
        if (!resourceManager)
            return E_FAIL;

        D3DMATERIAL9 material;

        ZeroMemory(&material, sizeof(D3DMATERIAL9));

        if (materialID > -1)
            material = *resourceManager->GetMaterial(materialID);

        LPD3DXEFFECT effect = resourceManager->GetEffect(effectID);

        if (!effect)
            return E_FAIL;

        D3DXEFFECT_DESC desc;
        effect->GetDesc(&desc);

        std::string materialDiffuse     = "materialDiffuse";
        std::string materialAmbient     = "materialAmbient";
        std::string materialSpecular = "materialSpecular";
        std::string materialEmissive = "materialEmissive";
        std::string materialPower     = "materialPower";

        for (int p = 0; p < desc.Parameters; p++)
        {
            D3DXHANDLE param = effect->GetParameter(NULL, p);

            D3DXPARAMETER_DESC paramDesc;
            effect->GetParameterDesc(param, &paramDesc);

            if (strcmp(paramDesc.Name, materialDiffuse.c_str()) == 0)
                effect->SetVector(materialDiffuse.c_str(), (Vector4*)&material.Diffuse);
            else if (strcmp(paramDesc.Name, materialAmbient.c_str()) == 0)
                effect->SetVector(materialAmbient.c_str(), (Vector4*)&material.Ambient);
            else if (strcmp(paramDesc.Name, materialSpecular.c_str()) == 0)
                effect->SetVector(materialSpecular.c_str(), (Vector4*)&material.Specular);
            else if (strcmp(paramDesc.Name, materialEmissive.c_str()) == 0)
                effect->SetVector(materialEmissive.c_str(), (Vector4*)&material.Emissive);
            else if (strcmp(paramDesc.Name, materialPower.c_str()) == 0)
                effect->SetFloat(materialPower.c_str(), material.Power);
        }
*/
        return S_OK;
    }


//-----------------------------------------------------------------------
    const StringArray *Interface::GetEffectTechniqueNames(int effectID)
    //-------------------------------------------------------------------
    {
        if (!m_pResourceManager)
            return NULL;

        Effect *pEffect = m_pResourceManager->GetEffect(effectID);

        if (!pEffect)
            return NULL;

        return pEffect->GetTechniqueNames();
    }


//-----------------------------------------------------------------------
    HRESULT Interface::SetSettings(PCORESETTINGS pSettings)
    //-------------------------------------------------------------------
    {
        memcpy(m_pSettings, pSettings, sizeof(CoreSettings));

        return S_OK;
    }


//-----------------------------------------------------------------------
    DWORD Interface::GetNumScenes()
    //-------------------------------------------------------------------
    {
        return m_scenes.size();
    }


//-----------------------------------------------------------------------
    DWORD Interface::GetNumVisuals()
    //-------------------------------------------------------------------
    {
        if (!m_pResourceManager)
            return 0;

        return m_pResourceManager->GetNumVisuals();
    }


//-----------------------------------------------------------------------
    DWORD Interface::GetNumTextures()
    //-------------------------------------------------------------------
    {
        if (!m_pResourceManager)
            return 0;

        return m_pResourceManager->GetNumTextures();
    }


//-----------------------------------------------------------------------
    DWORD Interface::GetNumMaterials()
    //-------------------------------------------------------------------
    {
        if (!m_pResourceManager)
            return 0;

        return m_pResourceManager->GetNumMaterials();
    }


//-----------------------------------------------------------------------
    DWORD Interface::GetNumEffects()
    //-------------------------------------------------------------------
    {
        if (!m_pResourceManager)
            return 0;

        return m_pResourceManager->GetNumEffects();
    }

//-----------------------------------------------------------------------
    HRESULT Interface::StartSimulation()
    //-------------------------------------------------------------------
    {
        if (!m_pTimer)
            return E_FAIL;

        m_pTimer->Start();

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Interface::StopSimulation()
    //-------------------------------------------------------------------
    {
        if (!m_pTimer)
            return E_FAIL;

        m_pTimer->Stop();

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Interface::StepSimulation(float timeDelta)
    //-------------------------------------------------------------------
    {
        if (!m_pTimer)
            return E_FAIL;

        m_pTimer->Step(timeDelta);

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Interface::QueryTime(float *pTime)
    //-------------------------------------------------------------------
    {
        if (!m_pTimer || !pTime)
            return E_FAIL;

        *pTime = m_pTimer->GetTime();

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Interface::QueryTimeDelta(float *pTimeDelta)
    //-------------------------------------------------------------------
    {
        if (!m_pTimer || !pTimeDelta)
            return E_FAIL;

        *pTimeDelta = m_pTimer->GetDeltaTime();

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Interface::QueryFramesPerSecond(float *pFPS)
    //-------------------------------------------------------------------
    {
        if (!m_pTimer || !pFPS)
            return E_FAIL;

        *pFPS = m_pTimer->GetFramesPerSecond();

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Interface::GetLog(StringList::iterator *pBegin, StringList::iterator *pEnd)
    //-------------------------------------------------------------------
    {
        m_logger.GetLog(pBegin, pEnd);

        return S_OK;
    }


//-----------------------------------------------------------------------
    D3DLIGHT9 *Interface::GetLight(int index)
    //-------------------------------------------------------------------
    {
        if (!m_pResourceManager)
            return NULL;

        return m_pResourceManager->GetLight(index);
    }


//-----------------------------------------------------------------------
    HRESULT Interface::LockTexture(int id, int *pWidth, int *pHeight, int *pPitch, void **pBits)
    //-------------------------------------------------------------------
    {
        if (id < 0)
            return E_FAIL;

        LPDIRECT3DBASETEXTURE9 pBaseTexture = m_pResourceManager->GetTexture(id);

        if (pBaseTexture->GetType() != D3DRTYPE_TEXTURE)
            return E_FAIL;

        LPDIRECT3DTEXTURE9 pTexture = (LPDIRECT3DTEXTURE9)pBaseTexture;

        if (!pTexture)
            return E_FAIL;

        D3DLOCKED_RECT rect;
        pTexture->LockRect(0, &rect, NULL, 0);
        *pPitch = rect.Pitch;
        *pBits = rect.pBits;

        D3DSURFACE_DESC desc;
        pTexture->GetLevelDesc(0, &desc);
        *pWidth = desc.Width;
        *pHeight = desc.Height;

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Interface::UnlockTexture(int id)
    //-------------------------------------------------------------------
    {
        if (id < 0)
            return E_FAIL;

        LPDIRECT3DBASETEXTURE9 pBaseTexture = m_pResourceManager->GetTexture(id);

        if (pBaseTexture->GetType() != D3DRTYPE_TEXTURE)
            return E_FAIL;

        LPDIRECT3DTEXTURE9 pTexture = (LPDIRECT3DTEXTURE9)pBaseTexture;

        if (!pTexture)
            return E_FAIL;

        pTexture->UnlockRect(0);

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Interface::LoadTextureFromFile(LPCTSTR filename, int *id)
    //-------------------------------------------------------------------
    {
        if (FAILED(m_pResourceManager->RegisterTexture(TT_TEXTURE, filename)))
            return E_FAIL;

        if (id)
            *id = m_pResourceManager->GetNumTextures() - 1;

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Interface::LoadCubeTextureFromFile(LPCTSTR filename, int *id)
    //-------------------------------------------------------------------
    {
        if (FAILED(m_pResourceManager->RegisterTexture(TT_CUBE, filename)))
            return E_FAIL;

        if (id)
            *id = m_pResourceManager->GetNumTextures() - 1;

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Interface::LoadVolumeTextureFromFile(LPCTSTR filename, int *id)
    //-------------------------------------------------------------------
    {
        if (FAILED(m_pResourceManager->RegisterTexture(TT_VOLUME, filename)))
            return E_FAIL;

        if (id)
            *id = m_pResourceManager->GetNumTextures() - 1;

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Interface::ResizeViewport(PVIEWPORT pViewport)
    //-------------------------------------------------------------------
    {
        if (!m_pRenderer)
            return E_FAIL;

        if (FAILED(m_pRenderer->ResizeViewport(pViewport)))
            return E_FAIL;

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Interface::ClearViewport(PVIEWPORT pViewport)
    //-------------------------------------------------------------------
    {
        if (!m_pRenderer)
            return E_FAIL;

        if (FAILED(m_pRenderer->ClearViewport(pViewport)))
            return E_FAIL;

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Interface::PrepareViewport(PVIEWPORT pViewport)
    //-------------------------------------------------------------------
    {
        if (!m_pRenderer)
            return E_FAIL;

        if (FAILED(m_pRenderer->PrepareViewport(pViewport)))
            return E_FAIL;

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Interface::PauseViewport(PVIEWPORT pViewport, bool paused)
    //-------------------------------------------------------------------
    {
        if (!m_pRenderer)
            return E_FAIL;

        if (FAILED(m_pRenderer->PauseViewport(pViewport, paused)))
            return E_FAIL;

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Interface::Update()
    //-------------------------------------------------------------------
    {
        m_pTimer->Update();

        SceneList::iterator i = m_scenes.begin();
        for (; i != m_scenes.end(); i++)
        {
            if ((*i)->GetUpdate())
            {
                if ((*i)->GetUpdate() == RT_ONCE)
                    (*i)->SetUpdate(RT_NONE);

                if (FAILED((*i)->Update(m_pTimer->GetDeltaTime())))
                    return E_FAIL;
            }
        }

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Interface::SelectSourceViewport(PVIEWPORT pViewport)
    //-------------------------------------------------------------------
    {
        if (!m_pRenderer)
            return E_FAIL;

        m_pRenderer->SelectSourceViewport(pViewport);

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Interface::BeginRender(PVIEWPORT pViewport)
    //-------------------------------------------------------------------
    {
        if (!m_pRenderer)
            return E_FAIL;

        m_pRenderer->Begin(pViewport);

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Interface::EndRender()
    //-------------------------------------------------------------------
    {
        if (!m_pRenderer)
            return E_FAIL;

        m_pRenderer->End();

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Interface::Render(RenderList &renderList)
    //-------------------------------------------------------------------
    {
        if (renderList.size() < 1)
            return S_OK;

        if (!m_pRenderer)
            return E_FAIL;

        if (FAILED(m_pRenderer->Render(renderList)))
            return E_FAIL;

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Interface::Render()
    //-------------------------------------------------------------------
    {
        if (!m_pRenderer)
            return E_FAIL;

        if (FAILED(m_pRenderer->Render(m_scenes)))
            return E_FAIL;

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Interface::Render(PSCENE pScene, PVIEWPORT pViewport)
    //-------------------------------------------------------------------
    {
        if (!m_pRenderer)
            return E_FAIL;

        if (FAILED(m_pRenderer->Render(pScene, pViewport)))
            return E_FAIL;

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Interface::Render(PSCENE pScene)
    //-------------------------------------------------------------------
    {
        if (!m_pRenderer)
            return E_FAIL;

        if (FAILED(m_pRenderer->Render(pScene)))
            return E_FAIL;

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Interface::Render(int numScenes, PSCENE *ppScenes, PVIEWPORT pViewport)
    //-------------------------------------------------------------------
    {
        if (!m_pRenderer)
            return E_FAIL;

        if (!ppScenes || !numScenes)
            return m_pRenderer->Render(m_scenes, pViewport);

        SceneList scenesToRender;

        for (int n = 0; n < numScenes; n++)
        {
            SceneList::iterator i = m_scenes.begin();
            for (; i != m_scenes.end(); i++)
            {
                if ((*i) == ppScenes[n])
                    scenesToRender.push_back(*i);
            }
        }

        if (FAILED(m_pRenderer->Render(scenesToRender, pViewport)))
            return E_FAIL;

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Interface::SetFullscreen(PVIEWPORT pViewport)
    //-------------------------------------------------------------------
    {
        if (FAILED(m_pRenderer->SetFullscreen(pViewport)))
            return E_FAIL;

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Interface::SetRenderTarget(int textureID)
    //-------------------------------------------------------------------
    {
        if (!m_pRenderer)
            return E_FAIL;

        if (FAILED(m_pRenderer->SetRenderTarget(textureID)))
            return E_FAIL;

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Interface::SetProgressCallback(ProgressCallbackFunc progressCallbackFunc)
    //-------------------------------------------------------------------
    {
        if (!progressCallbackFunc)
            return E_FAIL;

        m_pFnProgressCallback = progressCallbackFunc;

        return S_OK;
    }


//-----------------------------------------------------------------------
    DWORD Interface::TestRenderingDevice()
    //-------------------------------------------------------------------
    {
        if (!m_pRenderer)
            return 0;

        return m_pRenderer->TestDevice();
    }


//-----------------------------------------------------------------------
    HRESULT Interface::ResetRenderingDevice()
    //-------------------------------------------------------------------
    {
        if (!m_pRenderer)
            return E_FAIL;

        if (FAILED(m_pRenderer->ResetDevice()))
            return E_FAIL;

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Interface::LogDiagnostic()
    //-------------------------------------------------------------------
    {
        Logf(" Diagnostic");

        CoInitialize(NULL);

        // Run DirectX Diagnostic
            IDxDiagProvider *dxDiagProvider = NULL;
            IDxDiagContainer *dxDiagRoot = NULL;

            if (FAILED(CoCreateInstance(CLSID_DxDiagProvider, NULL, CLSCTX_INPROC_SERVER,
                                        IID_IDxDiagProvider, (PVOID*)&dxDiagProvider)))
                return E_FAIL;

            DXDIAG_INIT_PARAMS dxDiagInitParams;
            ZeroMemory(&dxDiagInitParams, sizeof(DXDIAG_INIT_PARAMS));

            dxDiagInitParams.dwSize = sizeof(DXDIAG_INIT_PARAMS);
            dxDiagInitParams.dwDxDiagHeaderVersion = DXDIAG_DX9_SDK_VERSION;
            dxDiagInitParams.bAllowWHQLChecks = FALSE;
            dxDiagInitParams.pReserved = NULL;

            if (SUCCEEDED(dxDiagProvider->Initialize(&dxDiagInitParams)))
            {
                if (SUCCEEDED(dxDiagProvider->GetRootContainer(&dxDiagRoot)))
                    LogDiagnosticContainer(NULL, dxDiagRoot);
            }

            if (dxDiagProvider)
                dxDiagProvider->Release();

            if (dxDiagRoot)
                dxDiagRoot->Release();

        CoUninitialize();

        return S_OK;
    }


//-----------------------------------------------------------------------
    void LogDiagnosticContainer(WCHAR *parentName, IDxDiagContainer *pContainer)
    //-------------------------------------------------------------------
    {
        DWORD numProps = 0;
        VARIANT variant;

        VariantInit(&variant);

        if (SUCCEEDED(pContainer->GetNumberOfProps(&numProps)))
        {
            for (DWORD i = 0; i < numProps; i++)
            {
                WCHAR propName[256];
                char propValue[256];

                if (FAILED(pContainer->EnumPropNames(i, propName, 256)))
                    continue;

                if (FAILED(pContainer->GetProp(propName, &variant)))
                    continue;

                switch (variant.vt)
                {
                case VT_UI4:
                    sprintf(propValue, "%ud", variant.ulVal);
                    break;

                case VT_I4:
                    sprintf(propValue, "%d", variant.lVal);
                    break;

                case VT_BOOL:
                    sprintf(propValue, "%s", variant.boolVal ? "TRUE" : "FALSE");
                    break;

                case VT_BSTR:
                    sprintf(propValue, "%s", variant.bstrVal);
                    WideCharToMultiByte(CP_ACP, 0, variant.bstrVal, -1, propValue, 256, NULL, NULL);
                    propValue[255] = 0;
                    break;
                }

                char buf[1024];
                char mbPropName[256];
                char mbParentName[256];

                WideCharToMultiByte(CP_ACP, 0, propName, -1, mbPropName, 256, NULL, NULL);

                if (parentName)
                {
                    WideCharToMultiByte(CP_ACP, 0, parentName, -1, mbParentName, 256, NULL, NULL);
                    sprintf(buf, "%s.%s = %s", mbParentName, mbPropName, propValue);
                }
                else
                    sprintf(buf, "%s = %s", mbPropName, propValue);

                Logf(LL_HIGHEST, buf);

                VariantClear(&variant);
            }
        }

        DWORD numChildren = 0;

        if (SUCCEEDED(pContainer->GetNumberOfChildContainers(&numChildren)))
        {
            for (DWORD i = 0; i < numChildren; i++)
            {
                WCHAR childName[256];
                IDxDiagContainer *childContainer;

                if (FAILED(pContainer->EnumChildContainerNames(i, childName, 256)))
                    continue;

                if (wcscmp(childName, L"DxDiag_DirectSound") == 0    ||
                    wcscmp(childName, L"DxDiag_DirectMusic") == 0    ||
                    wcscmp(childName, L"DxDiag_DirectPlay")  == 0    ||
                    wcscmp(childName, L"DxDiag_DirectInput") == 0    ||
                    wcscmp(childName, L"DxDiag_DirectShowFilters") == 0    ||
                    wcscmp(childName, L"DxDiag_LogicalDisks") == 0)
                    continue;

                if (FAILED(pContainer->GetChildContainer(childName, &childContainer)))
                    continue;

                WCHAR fullChildName[512];

                if (parentName)
                    swprintf(fullChildName, L"%s.%s", parentName, childName);
                else
                    swprintf(fullChildName, L"%s", childName);

                LogDiagnosticContainer(fullChildName, childContainer);

                if (childContainer)
                    childContainer->Release();
            }
        }
    }

// EOF