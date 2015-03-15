/*! \file */
/*-----------------------------------------------------------------------
    font.h

    Description: KSRFont class declaration
    Author:         Julian McKinlay
    ---------------------------------------------------------------------*/

#pragma once

//! \struct Font
struct Font : public MemObject
{
    Font(LPDIRECT3DDEVICE9 pDevice);
    ~Font();

    void Create(int height, int width, int weight, /*int orientation,*/ bool italic, DWORD charSet, DWORD pitchAndFamily, LPCTSTR pFaceName);
    void Draw(const String &text, LPRECT pRect, DWORD format, DWORD textColour);
    HRESULT GetLogfont(LOGFONT *pLogfont) const;

private:
    LPDIRECT3DDEVICE9 m_pDevice;
    LPD3DXFONT m_pD3DFont;

    int m_width, m_height, m_weight, m_fontOrientation;
    LPDIRECT3DTEXTURE9 m_pOrientedFontTexture;
};

typedef Font *PFONT;

// EOF