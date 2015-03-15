#include "../include/ksr.h"

#include "../include/leakwatcher.h"

#ifdef _DO_MEMORY_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace KSR;

//-----------------------------------------------------------------------
    Viewport::Viewport()
    //-------------------------------------------------------------------
        :    MemObject(), m_pStateBlock(NULL), m_pFont(NULL), m_pSwapChain(NULL),
            m_redraw(RT_LOOP), m_updateStates(false), m_renderPaused(false),
            m_textFormat(0), m_textColour(0),
            m_pFnPostDrawDCCallback(NULL), m_pFnPreDrawDCCallback(NULL),
            m_pPostDrawData(NULL), m_pPreDrawData(NULL)
    //-------------------------------------------------------------------
    {
        AddUsedMemory(sizeof(Viewport), "Viewport::Viewport()");

        m_pSettings = new ViewportSettings;
        AddUsedMemory(sizeof(ViewportSettings), "Viewport::Viewport() - ViewportSettings");

        m_pCamera = new Camera;

        m_textRect.left = m_textRect.top = m_textRect.right = m_textRect.bottom = 0;

        m_textInstances.clear();

        m_projections.clear();
    }


//-----------------------------------------------------------------------
    Viewport::~Viewport()
    //-------------------------------------------------------------------
    {
        Logf("Destroying Viewport...");

        if (m_pSettings)
        {
            delete m_pSettings;
            FreeUsedMemory(sizeof(ViewportSettings), "Viewport::~Viewport() - ViewportSettings");
        }

        if (m_pCamera)
            m_pCamera->Release();

        m_pFont = NULL;
        m_textInstances.clear();
    }


//-----------------------------------------------------------------------
    HRESULT Viewport::ZoomExtents(const KSR::Math::AABB &bounds, float bufferRatio)
    //-------------------------------------------------------------------
    {
        if (!bounds.IsValid() || bufferRatio == 0.0f || !m_pSettings)
            return E_FAIL;

        if (m_pSettings->aspect == 0.0f)
            m_pSettings->aspect = 1.0f;

        float worldWidth  = bounds.GetDeltaX();
        float worldHeight = bounds.GetDeltaY();
        float newWorldWidth = worldWidth;
        float newWorldHeight = worldHeight;

        if (worldWidth <= 0)
        {
            worldWidth = 1.0f;
        }

        if (worldHeight <= 0)
        {
            worldHeight = 1.0f;
        }

        if ((worldWidth / m_pSettings->width) < (worldHeight / m_pSettings->height))
        {
            m_pSettings->viewHeight = worldHeight * bufferRatio;
            m_pSettings->viewWidth = m_pSettings->viewHeight * m_pSettings->aspect;
        }
        else
        {
            m_pSettings->viewWidth = worldWidth * bufferRatio;
            m_pSettings->viewHeight = m_pSettings->viewWidth / m_pSettings->aspect;
        }

        m_updateStates = true;

        return S_OK;
    }


//-----------------------------------------------------------------------
    void Viewport::SetFont(PFONT pFont)
    //-------------------------------------------------------------------
    {
        m_pFont = pFont;
    }


//-----------------------------------------------------------------------
    void Viewport::SetFontFormat(PRECT pRect, DWORD format, DWORD colour)
    //-------------------------------------------------------------------
    {
        m_textFormat = format;
        m_textColour = colour;

        if (pRect)
            m_textRect = *pRect;
    }


//-----------------------------------------------------------------------
    void Viewport::Drawtext(const String &text, PRECT pRect, DWORD format, DWORD textColour)
    //-------------------------------------------------------------------
    {
        if (!m_pFont)
        {
            OutputDebugString(_T("Warning: No font, so no text.\n"));
            return;
        }

        TextInstance textInstance;
        textInstance.m_text   = text;
        textInstance.m_rect   = pRect ? *pRect : m_textRect;
        textInstance.m_colour = textColour;
        textInstance.m_format = format;
        textInstance.m_pFont   = m_pFont;

        m_textInstances.push_back(textInstance);
    }


//-----------------------------------------------------------------------
    void Viewport::Drawtext(const String &text)
    //-------------------------------------------------------------------
    {
        if (!m_pFont)
        {
            OutputDebugString("Warning: No font, so no text.\n");
            return;
        }

        TextInstance textInstance;
        textInstance.m_text   = text;
        textInstance.m_rect   = m_textRect;
        textInstance.m_colour = m_textColour;
        textInstance.m_format = m_textFormat;
        textInstance.m_pFont  = m_pFont;

        m_textInstances.push_back(textInstance);
    }


//-----------------------------------------------------------------------
    void Viewport::FlushText()
    //-------------------------------------------------------------------
    {
        std::vector<TextInstance>::iterator i = m_textInstances.begin();
        for (; i != m_textInstances.end(); i++)
            i->m_pFont->Draw(i->m_text, &i->m_rect, i->m_format, i->m_colour);

        m_textInstances.clear();
    }


//-----------------------------------------------------------------------
    void Viewport::SetSwapChain(PSWAPCHAIN pSwapChain)
    //-------------------------------------------------------------------
    {
        m_pSwapChain = pSwapChain;
    }


//-----------------------------------------------------------------------
    void Viewport::SetSettings(PVIEWPORTSETTINGS pSettings)
    //-------------------------------------------------------------------
    {
        memcpy(m_pSettings, pSettings, sizeof(ViewportSettings));
        m_updateStates = true;
    }


//-----------------------------------------------------------------------
    HRESULT Viewport::GetSettings(PVIEWPORTSETTINGS pSettings)
    //-------------------------------------------------------------------
    {
        if (!m_pSettings || !pSettings)
            return E_FAIL;

        memcpy(pSettings, m_pSettings, sizeof(ViewportSettings));

        return S_OK;
    }


//-----------------------------------------------------------------------
    void Viewport::SetCameraCoordSystem(const Vector3 &up, const Vector3 &forward)
    //-------------------------------------------------------------------
    {
        if (!m_pSettings)
            return;

        m_pCamera->SetCoordSystemMode(m_pSettings->coordSystemMode, up, forward);
    }


//-----------------------------------------------------------------------
    void Viewport::SetCameraPosition(const Vector3 &pos)
    //-------------------------------------------------------------------
    {
        m_pCamera->SetPosition(pos);
    }


//-----------------------------------------------------------------------
    void Viewport::SetCameraRotation(const Vector3 &rot)
    //-------------------------------------------------------------------
    {
        m_pCamera->SetRotation(rot);
    }


//-----------------------------------------------------------------------
    void Viewport::SetCameraTarget(const Vector3 &target)
    //-------------------------------------------------------------------
    {
        m_pCamera->SetTarget(target);
    }


//-----------------------------------------------------------------------
    void Viewport::SetCameraMode(DWORD transformMode, DWORD translationMode, DWORD rotationMode, DWORD orbitMode)
    //-------------------------------------------------------------------
    {
        m_pCamera->SetTransformMode(transformMode);
        m_pCamera->SetTranslationMode(translationMode);
        m_pCamera->SetRotationMode(rotationMode);
        m_pCamera->SetOrbitMode(orbitMode);
    }


//-----------------------------------------------------------------------
    void Viewport::SetProjectionMatrix(int id, const Matrix &matrix)
    //-------------------------------------------------------------------
    {
        if (m_projections.size() < (size_t)id + 1)
            m_projections.resize(id + 1);

        m_projections[id] = matrix;
    }


//-----------------------------------------------------------------------
    void Viewport::SetPreDrawData(void *pData)
    //-------------------------------------------------------------------
    {
        m_pPreDrawData = pData;
    }


//-----------------------------------------------------------------------
    void Viewport::SetPostDrawData(void *pData)
    //-------------------------------------------------------------------
    {
        m_pPostDrawData = pData;
    }


//-----------------------------------------------------------------------
    void Viewport::SetPreDrawCallback(DrawDCCallback pFnDrawDCCallback)
    //-------------------------------------------------------------------
    {
        m_pFnPreDrawDCCallback = pFnDrawDCCallback;
    }



//-----------------------------------------------------------------------
    void Viewport::SetPostDrawCallback(DrawDCCallback pFnDrawDCCallback)
    //-------------------------------------------------------------------
    {
        m_pFnPostDrawDCCallback = pFnDrawDCCallback;
    }


//-----------------------------------------------------------------------
    void *Viewport::GetPreDrawData() const
    //-------------------------------------------------------------------
    {
        return m_pPreDrawData;
    }


//-----------------------------------------------------------------------
    void *Viewport::GetPostDrawData() const
    //-------------------------------------------------------------------
    {
        return m_pPostDrawData;
    }


//-----------------------------------------------------------------------
    Viewport::DrawDCCallback Viewport::GetPreDrawCallback() const
    //-------------------------------------------------------------------
    {
        return m_pFnPreDrawDCCallback;
    }


//-----------------------------------------------------------------------
    Viewport::DrawDCCallback Viewport::GetPostDrawCallback() const
    //-------------------------------------------------------------------
    {
        return m_pFnPostDrawDCCallback;
    }


//-----------------------------------------------------------------------
    void Viewport::AdjustFOV(float fov)
    //-------------------------------------------------------------------
    {
        m_pSettings->fov += fov;
    }


//-----------------------------------------------------------------------
    void Viewport::AdjustViewVolume(const Vector2 &viewSize)
    //-------------------------------------------------------------------
    {
        m_pSettings->viewWidth += viewSize.x;
        m_pSettings->viewHeight += viewSize.y;
    }


//-----------------------------------------------------------------------
    void Viewport::CameraLookAt(const Vector3 &v)
    //-------------------------------------------------------------------
    {
        m_pCamera->LookAt(v);
    }


//-----------------------------------------------------------------------
    void Viewport::CameraTransform(int type, const Vector3 &transform)
    //-------------------------------------------------------------------
    {
        m_pCamera->Transform(type, transform);
    }


//-----------------------------------------------------------------------
    void Viewport::SetCameraFrustum(const Matrix &cameraFrustum)
    //-------------------------------------------------------------------
    {
        if (m_pCamera)
            m_pCamera->SetFrustum(cameraFrustum);
    }


//-----------------------------------------------------------------------
    bool Viewport::CheckPointCameraFrustum(const Vector3 &point)
    //-------------------------------------------------------------------
    {
        if (m_pCamera)
            return m_pCamera->CheckPointFrustum(point);

        return false;
    }


//-----------------------------------------------------------------------
    bool Viewport::CheckSphereCameraFrustum(const Vector3 &position, float radius)
    //-------------------------------------------------------------------
    {
        if (m_pCamera)
            return m_pCamera->CheckSphereFrustum(position, radius);

        return false;
    }


//-----------------------------------------------------------------------
    bool Viewport::CheckBoxCameraFrustum(const Vector3 &min, const Vector3 &max)
    //-------------------------------------------------------------------
    {
        if (m_pCamera)
            return m_pCamera->CheckBoxFrustum(min, max);

        return false;
    }


//-----------------------------------------------------------------------
    DWORD Viewport::GetCameraTransformMode()
    //-------------------------------------------------------------------
    {
        return m_pCamera->GetTransformMode();
    }


//-----------------------------------------------------------------------
    const Vector3 &Viewport::GetCameraPosition()
    //-------------------------------------------------------------------
    {
        return m_pCamera->GetPosition();
    }


//-----------------------------------------------------------------------
    const Vector3 Viewport::GetCameraDirection()
    //-------------------------------------------------------------------
    {
        return m_pCamera->GetDirection();
    }


//-----------------------------------------------------------------------
    const Vector3 &Viewport::GetCameraTarget()
    //-------------------------------------------------------------------
    {
        return m_pCamera->GetTarget();
    }


//-----------------------------------------------------------------------
    Matrix Viewport::GetCameraOrientation()
    //-------------------------------------------------------------------
    {
        return m_pCamera->GetOrientation();
    }


//-----------------------------------------------------------------------
    const Vector3 &Viewport::GetCameraForwardVector()
    //-------------------------------------------------------------------
    {
        return m_pCamera->GetForwardVector();
    }


//-----------------------------------------------------------------------
    const Vector3 &Viewport::GetCameraUpVector()
    //-------------------------------------------------------------------
    {
        return m_pCamera->GetUpVector();
    }


//-----------------------------------------------------------------------
    Matrix Viewport::GetCameraFrustum()
    //-------------------------------------------------------------------
    {
        return m_pCamera->GetFrustum();
    }


//-----------------------------------------------------------------------
    Matrix Viewport::GetProjectionMatrix(int id)
    //-------------------------------------------------------------------
    {
        return m_projections[id];
    }

// EOF