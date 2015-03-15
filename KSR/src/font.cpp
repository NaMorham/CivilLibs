#include "../include/ksr.h"

#include "../include/leakwatcher.h"

#ifdef _DO_MEMORY_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace KSR;

//-----------------------------------------------------------------------
    Font::Font(LPDIRECT3DDEVICE9 pDevice)
        :    m_pDevice(pDevice), m_pD3DFont(NULL), m_pOrientedFontTexture(NULL),
            m_width(6), m_height(12), m_weight(FW_NORMAL), m_fontOrientation(0)
    //-------------------------------------------------------------------
    {
        assert(pDevice != NULL);
        AddUsedMemory(sizeof(Font), _T("Font::Font()"));
    }


//-----------------------------------------------------------------------
    Font::~Font()
    //-------------------------------------------------------------------
    {
        FreeUsedMemory(sizeof(Font), _T("Font::~Font()"));

        if (m_pD3DFont)
            m_pD3DFont->Release();
    }


//-----------------------------------------------------------------------
    void Font::Create(int height, int width, int weight, /*int orientation,*/ bool italic, DWORD charSet, DWORD pitchAndFamily, LPCTSTR pFaceName)
    //-------------------------------------------------------------------
    {
        if (!pFaceName)
            return;

        m_width = width;
        m_height = height;
        m_weight = weight;
        //m_fontOrientation = orientation;

        D3DXCreateFont(m_pDevice, height, width, weight, 0, italic, charSet, OUT_TT_ONLY_PRECIS, 0, pitchAndFamily, pFaceName, &m_pD3DFont);

        if (m_fontOrientation != 0)
        {
            D3DXCreateTexture(m_pDevice, width, height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &m_pOrientedFontTexture);
        }
    }


//-----------------------------------------------------------------------
    void Font::Draw(const String &text, LPRECT pRect, DWORD format, DWORD textColour)
    //-------------------------------------------------------------------
    {
        if (!m_pD3DFont)
            return;

        if (m_fontOrientation != 0)
        {
            if (!m_pOrientedFontTexture)
                return;

            D3DXMATRIX currentWorld,
                       currentView,
                       currentProjection,
                       world,
                       view,
                       projection;

            m_pDevice->GetTransform(D3DTS_WORLD, &currentWorld);
            m_pDevice->GetTransform(D3DTS_VIEW, &currentView);
            m_pDevice->GetTransform(D3DTS_PROJECTION, &currentProjection);

            D3DXMatrixIdentity(&world);
            D3DXMatrixLookAtLH(&view, &D3DXVECTOR3(0, 0, -10), &D3DXVECTOR3(0, 0, 0), &D3DXVECTOR3(0, 0, 1));
            D3DXMatrixOrthoLH(&projection, (float)m_width, (float)m_height, 1, 100);

            m_pDevice->SetTransform(D3DTS_WORLD, &world);
            m_pDevice->SetTransform(D3DTS_VIEW, &view);
            m_pDevice->SetTransform(D3DTS_PROJECTION, &projection);

            m_pD3DFont->DrawText(NULL, text.c_str(), text.size(), pRect, format, textColour);

            m_pDevice->SetTransform(D3DTS_WORLD, &currentWorld);
            m_pDevice->SetTransform(D3DTS_VIEW, &currentView);
            m_pDevice->SetTransform(D3DTS_PROJECTION, &currentProjection);
        }
        else
        {
            m_pD3DFont->DrawText(NULL, text.c_str(), text.size(), pRect, format, textColour);
        }
    }


//-----------------------------------------------------------------------
    HRESULT Font::GetLogfont(LOGFONT *pLogfont) const
    //-------------------------------------------------------------------
    {
        if (!m_pD3DFont || !pLogfont)
            return E_FAIL;

        D3DXFONT_DESC desc;

        if (FAILED(m_pD3DFont->GetDesc(&desc)))
            return E_FAIL;

        ZeroMemory(pLogfont, sizeof(LOGFONT));
        pLogfont->lfHeight = desc.Height;
        pLogfont->lfWidth = desc.Width;
        pLogfont->lfEscapement = 0;
        pLogfont->lfOrientation = 0;
        pLogfont->lfWeight = desc.Weight;
        pLogfont->lfItalic = desc.Italic;
        pLogfont->lfUnderline = FALSE;
        pLogfont->lfStrikeOut = FALSE;
        pLogfont->lfCharSet = desc.CharSet;
        pLogfont->lfOutPrecision = desc.OutputPrecision;
        pLogfont->lfClipPrecision = CLIP_DEFAULT_PRECIS;
        pLogfont->lfQuality = desc.Quality;
        pLogfont->lfPitchAndFamily = desc.PitchAndFamily;
        _tcscpy(pLogfont->lfFaceName, desc.FaceName);

        return S_OK;
    }


// EOF