#include "../include/ksr.h"

#include "../include/leakwatcher.h"

#ifdef _DO_MEMORY_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace KSR;

#ifdef _SHOW_KSR_SIZES_

#pragma message("\tPrinting out KSR sizes")

#define varnameKSR(v) #v
#define LOGSIZE(x) Logf(LL_LOWEST, "sizeof(%s)%*s= %d bytes", varnameKSR(x), 27-strlen(varnameKSR(x)), " ", sizeof(x))

void PrintOutSizes()
{
    Logf(LL_LOWEST, "\n============= KSR SIZE DUMP =============\n");

    LOGSIZE(Vector2);
    LOGSIZE(Vector3);
    LOGSIZE(Interface);
    LOGSIZE(CoreSettings);
    LOGSIZE(ViewportSettings);
    LOGSIZE(SceneSettings);
    LOGSIZE(Viewport);
    LOGSIZE(Geometry);
    LOGSIZE(Scene);
    LOGSIZE(KSR::Font);
    LOGSIZE(Entity);
    LOGSIZE(Visual);
    LOGSIZE(RenderGroup);
    LOGSIZE(Effect);
    LOGSIZE(Camera);
    LOGSIZE(MemObject);
    LOGSIZE(OutputEntity);
    LOGSIZE(TreeNode);
    LOGSIZE(TreeLeaf);
    LOGSIZE(Chunk);
    LOGSIZE(Logger);
    LOGSIZE(Math::Sphere);
    LOGSIZE(Math::AABB);
    LOGSIZE(Math::OBB);
    LOGSIZE(Math::Polygon);
    LOGSIZE(Plane);
    LOGSIZE(Light);
    LOGSIZE(PatchBlock);
    LOGSIZE(Patch);
    LOGSIZE(Renderer);
    LOGSIZE(ResourceManager);
    LOGSIZE(Serialiser);
    LOGSIZE(SwapChain);
    LOGSIZE(Timer);
    LOGSIZE(IDirect3DDevice9);
    LOGSIZE(IDirect3D9);
    LOGSIZE(VERTEX_DIFFUSE_TEX1);
    LOGSIZE(VERTEX_NORMAL_DIFFUSE_TEX1);
    LOGSIZE(VERTEX_DIFFUSE);
    LOGSIZE(VERTEX_NORMAL_TEX1);
    LOGSIZE(VERTEX_DIFFUSE_TEX2);
    LOGSIZE(VERTEX_NORMAL_DIFFUSE_TEX2);
    LOGSIZE(VERTEX_NORMAL_TEX2);
    LOGSIZE(VisualSprite);

    Logf(LL_LOWEST, "=========================================\n\n");
}
#undef LOGSIZE
#undef varnameKSR
#else
void PrintOutSizes() {}
#endif

//-----------------------------------------------------------------------
    HRESULT KSR::CreateInterface(HWND hWnd, LPCTSTR logFilename, int maxLogSize, PCORESETTINGS pCoreSettings, PINTERFACE *pKSR)
    //-------------------------------------------------------------------
    {
        if (!pKSR)
            return E_FAIL;

        *pKSR = new Interface(hWnd, pCoreSettings, logFilename, maxLogSize);

        PrintOutSizes();

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT KSR::DestroyInterface(PINTERFACE pKSR)
    //-------------------------------------------------------------------
    {
        if (!pKSR)
            return E_FAIL;

        delete pKSR;

        return S_OK;
    }


//-----------------------------------------------------------------------
    void KSR::DebugPrintf(LPCTSTR fmt, ...)
    //-------------------------------------------------------------------
    {
        va_list args;
        TCHAR buf[1024];

        va_start(args, fmt);
        _vstprintf(buf, fmt, args);
        va_end(args);

        OutputDebugString(buf);
    }

// EOF