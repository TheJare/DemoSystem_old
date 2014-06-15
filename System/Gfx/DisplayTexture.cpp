// -------------------------------------------------------------------------------------
// File:        DisplayTexture.cpp
//
// Purpose:     
// -------------------------------------------------------------------------------------

#include "GfxPCH.h"

#include "DisplayTexture.h"
#include "DisplayDevice.h"

#ifndef DISPLAY_TEXTURE_LIGHTWEIGHT
#include "FileSystem.h"
#endif

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TError CDisplayTexture::Init      (const char *pszName, unsigned dwFlags)
{
  End();

  if (!pszName[0])
    return RET_FAIL;

  int l = strlen(pszName);
  m_pszName = NEW_ARRAY(char, l+1);
  strcpy(m_pszName, pszName);
  m_pTex = NULL;

  m_w   = 0;
  m_h   = 0;
  m_fmt = 0;
  m_dwFlags = dwFlags | F_FROMFILE;

  return RET_OK;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TError CDisplayTexture::Init      (const char *pszName, int w, int h, dword fmt, unsigned dwFlags)
{
  End();

  if (w <= 0 || h <= 0)
    return RET_FAIL;

  int l = strlen(pszName);
  m_pszName = NEW_ARRAY(char, l+1);
  strcpy(m_pszName, pszName);
  m_pTex = NULL;

  m_w   = w;
  m_h   = h;
  m_fmt = fmt;
  m_dwFlags = dwFlags;

  return RET_OK;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void        CDisplayTexture::End       ()
{
  if (IsOk())
  {
    Unload();
    DISPOSE_ARRAY(m_pszName);
  }
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TError      CDisplayTexture::Load      (CDisplayDevice *pDev)
{
  if (!IsOk())
    return RET_FAIL;

  if (!m_pTex)
  {
    if (!pDev->HasDynamicTextures())
      m_dwFlags &= ~F_DYNAMIC;
    DWORD usage  = IsRenderTarget()? D3DUSAGE_RENDERTARGET : (IsDynamic()? D3DUSAGE_DYNAMIC : 0);
    D3DPOOL pool = IsRenderTarget()? D3DPOOL_DEFAULT : D3DPOOL_MANAGED;

#ifdef DISPLAY_TEXTURE_LIGHTWEIGHT
    if (FAILED(pDev->GetDirect3DDevice()->CreateTexture(m_w, m_h, 1, usage, D3DFORMAT(m_fmt), pool, &m_pTex)))
      GLOG(("********ERROR!!!********** Device->CreateTexture(%d x %d, fmt = %d, flags = 0x%04X);\n", m_w, m_h, m_fmt, m_dwFlags));
#else //DISPLAY_TEXTURE_LIGHTWEIGHT

    if (IsFromFile())         // Create a texture from a disk file.
    {
      GLOG(("D3DXCreateTextureFromFileInMemoryEx(\"%s\");\n", m_pszName));

      int textureSize = 0;
      const void *pTextureMem = FileSystem::ReadFile(&textureSize, m_pszName);
      if (!pTextureMem)
        GLOG(("********ERROR!!!********** Loading texture file (\"%s\");\n", m_pszName));
      {
        if (FAILED(D3DXCreateTextureFromFileInMemoryEx(
                    pDev->GetDirect3DDevice(),
                    pTextureMem,
                    textureSize,
                    D3DX_DEFAULT, D3DX_DEFAULT,
                    D3DX_DEFAULT,
                    usage,
                    D3DFMT_UNKNOWN,//D3DFMT_R5G6B5,
                    pool,
                    D3DX_FILTER_LINEAR,
                    D3DX_FILTER_BOX,
                    0,
                    NULL,
                    NULL,
                    &m_pTex)))
          GLOG(("********ERROR!!!********** D3DXCreateTextureFromFileInMemoryEx(\"%s\");\n", m_pszName));
        FileSystem::FreeFile(pTextureMem);
      }
    }
    else                // Create an empty texture
    {
      GLOG(("D3DXCreateTexture(%d x %d, fmt = %d, flags = 0x%04X);\n", m_w, m_h, m_fmt, m_dwFlags));
      if (FAILED(D3DXCheckTextureRequirements(pDev->GetDirect3DDevice(), &m_w, &m_h, NULL, usage, (D3DFORMAT*)&m_fmt, pool)))
        GLOG(("********ERROR!!!********** D3DXCheckTextureRequirements(%d x %d, fmt = %d, flags = 0x%04X);\n", m_w, m_h, m_fmt, m_dwFlags));
      else
      {
        if (FAILED(D3DXCreateTexture(pDev->GetDirect3DDevice(), m_w, m_h, 1, usage, D3DFORMAT(m_fmt), pool, &m_pTex)))
          GLOG(("********ERROR!!!********** D3DXCreateTexture(%d x %d, fmt = %d, flags = 0x%04X);\n", m_w, m_h, m_fmt, m_dwFlags));
      }
    }
#endif // !DISPLAY_TEXTURE_LIGHTWEIGHT

    if (m_pTex)
    {
      D3DSURFACE_DESC desc;
      m_pTex->GetLevelDesc(0, &desc);
      m_w = desc.Width;
      m_h = desc.Height;
      m_fmt = desc.Format;
    }
  }

  return m_pTex? RET_OK : RET_FAIL;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void CDisplayTexture::Unload    ()
{
  if (!IsOk() || !m_pTex)
    return;

  int nRef = m_pTex->Release();
  m_pTex = NULL;
  GLOG(("Texture \"%s\" References: %d\n", m_pszName, nRef));

  if (IsFromFile())
  {
    m_w = 0;
    m_h = 0;
    m_fmt = 0;
  }
}


// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TError  CDisplayTexture::Lock					(byte **ppDest, int *pPitch)
{
  if (!IsOk() || !m_pTex)
    return RET_FAIL;

  D3DLOCKED_RECT lr = { 0, 0 };
  if (FAILED(m_pTex->LockRect(0, &lr, NULL, IsDynamic()? D3DLOCK_DISCARD : 0)))
    return RET_FAIL;

  if (ppDest)
    *ppDest = (byte *)lr.pBits;
  if (pPitch)
    *pPitch = lr.Pitch;
  return RET_OK;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void  CDisplayTexture::Unlock				()
{
  if (!IsOk() || !m_pTex)
    return;
  m_pTex->UnlockRect(0);
}
