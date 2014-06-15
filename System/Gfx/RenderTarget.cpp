//----------------------------------------------------------------------------
//  Nombre:    RenderTarget.cpp
//
//  Contenido: Gestion de rendertarget
//----------------------------------------------------------------------------

#include "GfxPCH.h"

#include "RenderTarget.h"
#include "DisplayDevice.h"
#include "DisplayTexture.h"
#include "TextureManager.h"

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TError  CRenderTarget::Init  (CTextureManager *pMgr, int w, int h, int bpp, const char *pszName, bool bZBuf, bool bAlpha, CDisplayTexture *pTex, DXN_Direct3DSurface *pZBuf)
{
  End();

  TError ret = RET_OK;
  m_pMgr = pMgr;
  m_pZBuffer = NULL;
  m_pOldZBuffer = NULL;
  m_pOldBackBuffer = NULL;
  m_bOwnZBuffer = false;

  if (pTex)
  {
    ASSERT(pTex->IsOk());

    if (pTex->GetFormat() == (dword)D3DFMT_X1R5G5B5 || pTex->GetFormat() == (dword)D3DFMT_R5G6B5)
      bpp = 16;
    else      //D3DFMT_X8R8G8B8, and D3DFMT_A8R8G8B8
      bpp = 32;
    m_pBackBuffer = pTex;
    w = pTex->GetWidth();
    h = pTex->GetHeight();
    m_pBackBuffer = pTex;
    m_bOwnBackBuffer = false;
  }
  else
  {
    m_pBackBuffer = pMgr->InitMaterial(pszName, 0);
    D3DFORMAT fmt = (bpp == 16)? (bAlpha? D3DFMT_A1R5G5B5 : D3DFMT_R5G6B5) : (bAlpha? D3DFMT_A8R8G8B8 : D3DFMT_X8R8G8B8);
    if (RET_OK != m_pBackBuffer->Init(pszName, w, h, fmt, CDisplayTexture::F_RENDERTARGET))
    {
      fmt = (bpp == 16)? (bAlpha? D3DFMT_R5G6B5 : D3DFMT_A1R5G5B5) : (bAlpha? D3DFMT_X8R8G8B8 : D3DFMT_A8R8G8B8);
      ret = m_pBackBuffer->Init(pszName, w, h, fmt, CDisplayTexture::F_RENDERTARGET);
    }
  }
  if (RET_OK == ret)
    ret = m_pBackBuffer->Load(m_pMgr->GetDevice());

  if (pZBuf || !bZBuf)
  {
    m_pZBuffer = pZBuf;
    m_bOwnZBuffer = false;
  }
  else
  {
    D3DFORMAT zbFmt = (bpp == 16)? D3DFMT_D16 : D3DFMT_D24S8;
    if (FAILED(m_pMgr->GetDevice()->GetDirect3DDevice()->CreateDepthStencilSurface(w, h, zbFmt, D3DMULTISAMPLE_NONE, 0, TRUE, &m_pZBuffer, NULL)))
    {
      zbFmt = (bpp == 16)? D3DFMT_D16_LOCKABLE : D3DFMT_D32;
      if (FAILED(m_pMgr->GetDevice()->GetDirect3DDevice()->CreateDepthStencilSurface(w, h, zbFmt, D3DMULTISAMPLE_NONE, 0, TRUE, &m_pZBuffer, NULL)))
      {
        zbFmt = (bpp == 16)? D3DFMT_D15S1: D3DFMT_D24X8;
        if (FAILED(m_pMgr->GetDevice()->GetDirect3DDevice()->CreateDepthStencilSurface(w, h, zbFmt, D3DMULTISAMPLE_NONE, 0, TRUE, &m_pZBuffer, NULL)))
          ret = RET_FAIL;
      }
    }
    m_bOwnZBuffer = true;
  }
  if (ret != RET_OK)
    End();
  else
  {
/*
  Rendertargets can't be locked, thus another way of clearing them must be found.
    // Clear the texture.
    byte *pPix;
    int pitch;
    if (RET_OK == m_pBackBuffer->Lock(&pPix, &pitch))
    {
      for (int i = 0; i < h; i++)
        memset(pPix + i*pitch, 0, w*(bpp/8));
      m_pBackBuffer->Unlock();
    }
*/
    // At this point we're sure the object is properly initialized, albeit
    if (RET_OK == BeginFrame(pMgr->GetDevice()))
    {
      pMgr->GetDevice()->Clear();
      EndFrame(pMgr->GetDevice());
    }
  }

  return ret;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void    CRenderTarget::End         ()
{
  if (IsOk())
  {
    ASSERT(!m_pOldBackBuffer);

    if (m_bOwnZBuffer)
      SAFE_RELEASE(m_pZBuffer);
    if (m_bOwnBackBuffer)
      m_pMgr->ReleaseMaterial(m_pBackBuffer);
    m_pBackBuffer = NULL;
  }
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
int     CRenderTarget::GetWidth      () const
{
  ASSERT(IsOk());
  return m_pBackBuffer->GetWidth();
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
int     CRenderTarget::GetHeight     () const
{
  ASSERT(IsOk());
  return m_pBackBuffer->GetHeight();
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TError  CRenderTarget::BeginFrame    (CDisplayDevice *pDev)
{
  if (!IsOk() || !pDev->IsDeviceReady())
    return RET_FAIL;

  EndFrame(pDev);

  TError ret = RET_FAIL;

  pDev->GetDirect3DDevice()->GetRenderTarget(0, &m_pOldBackBuffer);
  pDev->GetDirect3DDevice()->GetDepthStencilSurface(&m_pOldZBuffer);

  if (!m_pOldBackBuffer)
    GLOG(("******* ERROR: Can't get Current RenderTarget\n"));
  else
  {
    // ------------
    // Prepare rendertarget
    DXN_Direct3DSurface *pRTSurface = NULL;
    m_pBackBuffer->GetTexture()->GetSurfaceLevel(0, &pRTSurface);

    if (pRTSurface && !FAILED(pDev->GetDirect3DDevice()->SetRenderTarget(0, pRTSurface)))
      ret = RET_OK;
    else
      GLOG(("******* ERROR: Can't set New RenderTarget\n"));
    if (RET_OK == ret && m_pZBuffer && !FAILED(pDev->GetDirect3DDevice()->SetDepthStencilSurface(m_pZBuffer)))
      ret = RET_OK;
    else
      GLOG(("******* ERROR: Can't set New DepthStencil\n"));
//    SAFE_RELEASE(pRTSurface);
  }

  if (ret == RET_OK)
    ret = pDev->BeginFrame();
  if (ret != RET_OK)
  {
    SAFE_RELEASE(m_pOldBackBuffer);
    SAFE_RELEASE(m_pOldZBuffer);
  }

  return ret;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void    CRenderTarget::EndFrame      (CDisplayDevice *pDev)
{
  if (!IsOk() || !m_pOldBackBuffer)
    return;
  pDev->EndFrame();
  if (FAILED(pDev->GetDirect3DDevice()->SetRenderTarget(0, m_pOldBackBuffer)))
    GLOG(("******* ERROR: Can't restore Old RenderTarget\n"));
  if (FAILED(pDev->GetDirect3DDevice()->SetDepthStencilSurface(m_pOldZBuffer)))
    GLOG(("******* ERROR: Can't restore Old DepthStencil\n"));
  SAFE_RELEASE(m_pOldBackBuffer);
  SAFE_RELEASE(m_pOldZBuffer);
}
