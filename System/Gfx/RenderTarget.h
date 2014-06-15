//----------------------------------------------------------------------------
//  Nombre:    RenderTarget.h
//
//  Contenido: Gestion de rendertarget
//----------------------------------------------------------------------------

#ifndef _RENDER_TARGET_H_
#define _RENDER_TARGET_H_

#include "DXNames.h"

class CDisplayTexture;
class CTextureManager;
class CDisplayDevice;

class CRenderTarget
{
  public:
    CRenderTarget       (): m_pBackBuffer(NULL)   { }
    ~CRenderTarget      ()                        { End(); }

    TError  Init        (CTextureManager *pMgr, int w, int h, int bpp, const char *pszName, bool bZBuf = true, bool bAlpha = false, CDisplayTexture *pTex = NULL, DXN_Direct3DSurface *pZBuf = NULL);
    TError  Init        (CTextureManager *pMgr, CDisplayTexture *pTex = NULL, DXN_Direct3DSurface *pZBuf = NULL)
                                                              { return Init(pMgr, 0, 0, 0, "", true, true, pTex, pZBuf); }
    void    End         ();
    bool    IsOk        ()                              const { return m_pBackBuffer != NULL; }

    int     GetWidth      () const;
    int     GetHeight     () const;

    CDisplayTexture      *GetBackbuffer ()              const { return m_pBackBuffer; }
    DXN_Direct3DSurface  *GetZBuffer    ()              const { return m_pZBuffer; }

    TError                BeginFrame    (CDisplayDevice *pDev);
    void                  EndFrame      (CDisplayDevice *pDev);

  protected:
    CDisplayTexture     *m_pBackBuffer;
    DXN_Direct3DSurface *m_pZBuffer;
    bool                m_bOwnBackBuffer;
    bool                m_bOwnZBuffer;
    CTextureManager     *m_pMgr;

    DXN_Direct3DSurface *m_pOldZBuffer;
    DXN_Direct3DSurface *m_pOldBackBuffer;
};

#endif // _RENDER_TARGET_H_
