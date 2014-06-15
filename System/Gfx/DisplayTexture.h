// -------------------------------------------------------------------------------------
// File:        DisplayTexture.h
//
// Purpose:     
// -------------------------------------------------------------------------------------

#ifndef _DISPLAY_TEXTURE_H_
#define _DISPLAY_TEXTURE_H_

#include "DXNames.h"

class CDisplayDevice;

class CDisplayTexture
{
  public:
    enum
    {
      F_RENDERTARGET    = 0x0001,
      F_DYNAMIC         = 0x0002,
    };
    CDisplayTexture(): m_pszName(NULL)      { }
    ~CDisplayTexture()                      { End(); }

    TError      Init      (const char *pszName, unsigned dwFlags = 0);
    TError      Init      (const char *pszName, int w, int h, dword fmt, unsigned dwFlags = 0);

    void        End       ();
    bool        IsOk      ()          const { return (m_pszName != NULL); }

    bool        IsLoaded  ()          const { return IsOk() && (m_pTex != NULL); }
    TError      Load      (CDisplayDevice *pDev);
    void        Unload    ();

    int         GetWidth  ()          const { return m_w; }
    int         GetHeight ()          const { return m_h; }
    dword       GetFormat ()          const { return m_fmt; }
    const char *GetName   ()          const { return m_pszName; }
    DXN_Direct3DTexture *
                GetTexture()          const { return m_pTex; }

    bool        IsDynamic     ()      const { return (m_dwFlags & F_DYNAMIC) != 0; }
    bool        IsRenderTarget()      const { return (m_dwFlags & F_RENDERTARGET) != 0; }
    bool        IsFromFile    ()      const { return (m_dwFlags & F_FROMFILE) != 0; }

    // USE WITH CAUTION
    // In case you want to mess with the internals of the D3D Texture pointer
    // being referenced by this CDisplayTexture
    DXN_Direct3DTexture *
                SetTexture    (DXN_Direct3DTexture *pNewTex)    { DXN_Direct3DTexture *pt = m_pTex; m_pTex = pNewTex; return pt; }

		TError			Lock					(byte **ppDest, int *pPitch);
		void				Unlock				();

  private:
    enum
    {
      F_FROMFILE = 0x8000,
    };
    char                *m_pszName;
    DXN_Direct3DTexture *m_pTex;

    unsigned  m_w;
    unsigned  m_h;
    unsigned  m_fmt;
    unsigned  m_dwFlags;
};


#endif //_DISPLAY_TEXTURE_H_
