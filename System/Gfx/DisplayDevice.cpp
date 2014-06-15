// -------------------------------------------------------------------------------------
// File:        DisplayDevice.cpp
//
// Purpose:     
// -------------------------------------------------------------------------------------

#include "GfxPCH.h"

#include "DisplayDevice.h"
#include "WinWindow.h"
#include "ProjectionViewport.h"
#include "vectors.h"

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

#define MY_DEVTYPE D3DDEVTYPE_HAL
//#define MY_DEVTYPE D3DDEVTYPE_REF

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
// Function:      Init
// Purpose:       Initialize the object
// Parameters:    
// --------------------------------------------------------------------------
TError CDisplayDevice::Init  (CWinWindow *pWindow)
{
  End();
  m_pD3D    = ::Direct3DCreate9(D3D_SDK_VERSION);
  if (!m_pD3D)
    return RET_FAIL;

  m_flags = 0;
  m_pDevice = NULL;
  ZeroMem(&m_mode, sizeof(m_mode));
  ZeroMem(&m_newMode, sizeof(m_newMode));
  m_desktopFormat = FindDesktopFormat();

  m_pWindow = pWindow;
  m_bWorldViewProjDirty = true;
  m_bScreenTransformDirty = true;

  m_ManagerList.Init();

  return RET_OK;
}


// --------------------------------------------------------------------------
// Function:      End
// Purpose:       Release all resources and deinitialize the object
// Parameters:    
// --------------------------------------------------------------------------
void CDisplayDevice::End  ()
{
  if (IsOk())
  {
    Shutdown();
    m_ManagerList.RemoveAll();
    m_pD3D->Release();
    m_pWindow = NULL;
  }
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TError CDisplayDevice::SetDisplayMode    (int w, int h, int bpp, bool bFullscreen, unsigned wantFlags)
{
  if (!IsAvailable())
    return RET_FAIL;

  if (bFullscreen)
    wantFlags |= F_WANT_FULLSCREEN;
  else
    wantFlags &= ~F_WANT_FULLSCREEN;

  m_newMode.w = w;
  m_newMode.h = h;
  m_newMode.bpp = bpp;
  m_newMode.flags = wantFlags;

  if (RET_OK != Restore())
    return RET_FAIL;

  return RET_OK;
}


// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void CDisplayDevice::Shutdown          ()
{
  if (IsAvailable())
  {
    if (m_pDevice)
    {
      ShutdownManagers();
      m_pDevice->Release();
      m_pDevice = NULL;
    }
  }
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TError CDisplayDevice::Restore           ()
{
  if (!IsAvailable())
    return RET_FAIL;

  // If the window is gone, bail out
  if (!m_pWindow->IsOk())
    return RET_FAIL;

  // If the device is not ours to play with, don't even try.
  if (m_pDevice && m_pDevice->TestCooperativeLevel() == D3DERR_DEVICELOST)
    return RET_FAIL;

  // Find out the most up-to-date desktop format in case we're going windowed.
  // Only do so if we're not ok or we're windowed already.
  if (!m_pDevice || !(m_mode.IsFS()))
    m_desktopFormat = FindDesktopFormat();

  m_mode = m_newMode;

  D3DPRESENT_PARAMETERS pp;

  pp.BackBufferWidth  = m_newMode.w;
  pp.BackBufferHeight = m_newMode.h;
  pp.BackBufferFormat = D3DFORMAT(m_newMode.IsFS()? FindBackbufferFormat(m_newMode.bpp) : m_desktopFormat);
  pp.BackBufferCount  = 1;

  pp.SwapEffect       = m_newMode.IsNoDiscard()? D3DSWAPEFFECT_COPY : D3DSWAPEFFECT_DISCARD;

  pp.hDeviceWindow    = 0;
  pp.Windowed         = !m_newMode.IsFS();
  
  pp.EnableAutoDepthStencil = m_newMode.IsDB();
  pp.AutoDepthStencilFormat = D3DFORMAT(m_newMode.IsDB()? FindDepthbufferFormat(pp.BackBufferFormat, m_newMode.flags) : D3DFMT_UNKNOWN);
  
  pp.Flags = 0;
  pp.FullScreen_RefreshRateInHz       = D3DPRESENT_RATE_DEFAULT;
  pp.PresentationInterval  = m_newMode.IsFS()? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_DEFAULT ;

  bool bMultisample = m_newMode.IsFSAA();
  if (bMultisample)
    bMultisample = SUCCEEDED(m_pD3D->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, MY_DEVTYPE,
                             pp.BackBufferFormat, pp.Windowed, D3DMULTISAMPLE_2_SAMPLES, NULL));
  if (bMultisample && pp.EnableAutoDepthStencil)
    bMultisample = SUCCEEDED(m_pD3D->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, MY_DEVTYPE,
                             pp.AutoDepthStencilFormat, pp.Windowed, D3DMULTISAMPLE_2_SAMPLES, NULL));
  if (bMultisample)
    pp.MultiSampleType  = D3DMULTISAMPLE_4_SAMPLES;
  else
  {
    pp.MultiSampleType  = D3DMULTISAMPLE_NONE;
    m_mode.flags &= ~F_WANT_FSAA;
  }
  pp.MultiSampleQuality = 0;

  HRESULT hr;

  if (m_pDevice)
    ShutdownManagers();

  // Perform the Display (re)Initializatiom
  m_flags |= F_INRESET;
  if (!m_pDevice)
    hr = m_pD3D->CreateDevice(D3DADAPTER_DEFAULT, MY_DEVTYPE, (HWND)m_pWindow->GetHwnd(), D3DCREATE_MIXED_VERTEXPROCESSING, &pp, &m_pDevice);
//    hr = m_pD3D->CreateDevice(D3DADAPTER_DEFAULT, MY_DEVTYPE, (HWND)m_pWindow->GetHwnd(), D3DCREATE_SOFTWARE_VERTEXPROCESSING, &pp, &m_pDevice);
  else
    hr = m_pDevice->Reset(&pp);

  // Reset the window size if success and windowed
  D3DCAPS9 caps;
  if (SUCCEEDED(hr))
    hr = m_pDevice->GetDeviceCaps(&caps);
  if (SUCCEEDED(hr))
  {
    m_maxTextureW = caps.MaxTextureWidth;
    m_maxTextureH = caps.MaxTextureHeight;
    m_flags &= ~FS_MASK;
    if (caps.Caps2 & D3DCAPS2_DYNAMICTEXTURES)
      m_flags |= FS_DYNAMIC_TEXTURES;
    if (m_pWindow->IsOk() && pp.Windowed)
    {
      HWND hwnd = (HWND)m_pWindow->GetHwnd();
      DWORD style = ::GetWindowLong(hwnd, GWL_STYLE);
      RECT r = { 0, 0, m_newMode.w, m_newMode.h };
      ::AdjustWindowRect(&r, style, (::GetMenu(hwnd) != NULL));
      ::SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, r.right - r.left, r.bottom - r.top,
                     SWP_DRAWFRAME | SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOCOPYBITS | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_SHOWWINDOW);
    }
  }
  m_flags &= ~F_INRESET;

  // Maybe we failed.
  if (FAILED(hr))
  {
    GLOG(("ERROR: Restore 0x%08X\n", hr));
    if (m_pDevice)
      m_pDevice->Release();
    m_pDevice = NULL;
    return RET_FAIL;
  }

  // Everything went well, notify the managers we're back up.
  ResetMaterial();
  RestoreManagers();

  return RET_OK;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
bool CDisplayDevice::IsDeviceReady     () const
{
  return IsAvailable() && m_pDevice && m_pDevice->TestCooperativeLevel() == D3D_OK;
}

// ------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
unsigned CDisplayDevice::FindDesktopFormat      () const
{
  D3DDISPLAYMODE dm;
  m_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &dm);
  return dm.Format;
}


// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
static struct
{
  D3DFORMAT fmt;
  int       bpp;
}
s_aBBFormats[] =
{
  { D3DFMT_X1R5G5B5,    16 },
  { D3DFMT_R5G6B5,      16 },
  { D3DFMT_X8R8G8B8,    32 },
  { D3DFMT_A8R8G8B8,    32 },
};

unsigned CDisplayDevice::FindBackbufferFormat   (int bpp) const
{
  int bestBpp = 0;
  D3DFORMAT bestFmt = D3DFMT_UNKNOWN;

  for (int i = 0; bestFmt == D3DFMT_UNKNOWN && i < ARRAY_LEN(s_aBBFormats); i++)
  {
    if (s_aBBFormats[i].bpp == bpp
        && !FAILED(m_pD3D->CheckDeviceType(D3DADAPTER_DEFAULT,
                                           MY_DEVTYPE,
                                           s_aBBFormats[i].fmt, s_aBBFormats[i].fmt,
                                           FALSE)))
    {
      bestFmt = s_aBBFormats[i].fmt;
      bestBpp = s_aBBFormats[i].bpp;
    }
  }
       
  GLOG(("FindBackbufferFormat found format %d\n", bestFmt));
  return bestFmt;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
static struct
{
  D3DFORMAT fmt;
  int       bpp;
  bool      bStencil;
}
s_aDBFormats[] =
{
  { D3DFMT_D15S1,       16, true  },
  { D3DFMT_D16,         16, false },
  { D3DFMT_D16_LOCKABLE,16, false },

  { D3DFMT_D24S8,       32, true  },
  { D3DFMT_D24X4S4,     32, true  },
  { D3DFMT_D32,         32, false },
  { D3DFMT_D24X8,       32, false},
};

unsigned CDisplayDevice::FindDepthbufferFormat  (unsigned backbufferFormat, unsigned flags) const
{
  int bestBpp = 0;
  D3DFORMAT bestFmt = D3DFMT_UNKNOWN;
  int displayBpp = FindFormatBpp(backbufferFormat);

  for (int i = 0; i < ARRAY_LEN(s_aDBFormats); i++)
  {
    // Verify that the depth format exists.
    if (!(flags & F_WANT_STENCIL) || s_aDBFormats[i].bStencil)
    {
      HRESULT hr = m_pD3D->CheckDeviceFormat( D3DADAPTER_DEFAULT,
                                              MY_DEVTYPE,
                                              (D3DFORMAT)backbufferFormat,
                                              D3DUSAGE_DEPTHSTENCIL,
                                              D3DRTYPE_SURFACE,
                                              s_aDBFormats[i].fmt);

      if (!FAILED(hr))
      {
        // Verify that the depth format is compatible.
        hr = m_pD3D->CheckDepthStencilMatch( D3DADAPTER_DEFAULT,
                                             MY_DEVTYPE,
                                             (D3DFORMAT)backbufferFormat,
                                             (D3DFORMAT)backbufferFormat,
                                             s_aDBFormats[i].fmt);
        if (!FAILED(hr))
        {
          if (bestBpp != displayBpp && bestBpp != s_aDBFormats[i].bpp)
          {
            bestBpp = s_aDBFormats[i].bpp;
            bestFmt = s_aDBFormats[i].fmt;
          }
        }
      }
    }
  }

  GLOG(("FindDepthbufferFormat found format %d\n", bestFmt));
  return bestFmt;
}

unsigned CDisplayDevice::FindFormatBpp (unsigned fmt) const
{
  for (int i = 0; i < ARRAY_LEN(s_aDBFormats); i++)
    if (s_aDBFormats[i].fmt == (D3DFORMAT)fmt)
      return s_aDBFormats[i].bpp;
  for (int i = 0; i < ARRAY_LEN(s_aBBFormats); i++)
    if (s_aBBFormats[i].fmt == (D3DFORMAT)fmt)
      return s_aBBFormats[i].bpp;
    return D3DFMT_UNKNOWN;
}

//----------------------------------------------------------------------
//  Retorno:        
//  Parametros:     
//  Uso:            
//----------------------------------------------------------------------
void CDisplayDevice::SetViewport(int x, int y, int w, int h)
{
  if (IsAvailable() && m_pDevice != NULL)
  {
    m_bScreenTransformDirty = true;
    // Set up the viewport data parameters
    m_ViewportPos.Set(float(x), float(y));
    m_ViewportSize.Set(float(w), float(h));
    DXN_D3DVIEWPORT vp;
    vp.X          = x;
    vp.Y          = y;
    vp.Width      = w;
    vp.Height     = h;
    vp.MinZ       = 0.0f;
    vp.MaxZ       = 1.0f;
    m_pDevice->SetViewport(&vp);
  }
}

//----------------------------------------------------------------------
//  Retorno:        
//  Parametros:     
//  Uso:            
//----------------------------------------------------------------------
void CDisplayDevice::SetProjection(float ivpw, float ivph, float Q, float zn)
{
  if (IsAvailable() && m_pDevice != NULL)
  {
    m_bWorldViewProjDirty = true;
    memset(&m_ProjTransform, 0, sizeof(m_ProjTransform));
    m_ProjTransform._11 = ivpw;
    m_ProjTransform._22 = ivph;
    m_ProjTransform._33 = Q;
    m_ProjTransform._43 = -zn*Q;
    m_ProjTransform._34 = 1.f;

    m_pDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)&m_ProjTransform);
  }
}

//----------------------------------------------------------------------
//  Retorno:        
//  Parametros:     
//  Uso:            
//----------------------------------------------------------------------
void CDisplayDevice::SetProjectionViewport(const TProjectionViewport *vp)
{
  SetViewport(vp->vx, vp->vy, vp->vw, vp->vh);
  SetProjection(vp->ivpw, vp->ivph, vp->Kz, vp->zn);
}

//----------------------------------------------------------------------
//  Retorno:        
//  Parametros:     
//  Uso:            
//----------------------------------------------------------------------
void CDisplayDevice::SetWorldTransform (const TMatrix3 &m)
{
  if (IsAvailable() && m_pDevice != NULL)
  {
    m_bWorldViewProjDirty = true;
    m_bScreenTransformDirty = true;
    m_WorldTransform = m;
    TMatrix4 m4;
    m.ToMatrix4(m4);
    m_pDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&m4);
  }
}

//----------------------------------------------------------------------
//  Retorno:        
//  Parametros:     
//  Uso:            
//----------------------------------------------------------------------
void CDisplayDevice::SetViewTransform  (const TMatrix3 &m)
{
  if (IsAvailable() && m_pDevice != NULL)
  {
    m_bWorldViewProjDirty = true;
    m_bScreenTransformDirty = true;
    m_ViewTransform = m;
    TMatrix4 m4;
    m.ToMatrix4(m4);
    m_pDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX*)&m4);
  }
}

//----------------------------------------------------------------------
//  Retorno:        
//  Parametros:     
//  Uso:            
//----------------------------------------------------------------------
void CDisplayDevice::SetCameraTransform  (const TMatrix3 &m)
{
  if (IsAvailable() && m_pDevice != NULL)
  {
    m_bWorldViewProjDirty = true;
    m_bScreenTransformDirty = true;
    TMatrix3 im = m.Inverse();
    m_ViewTransform = im;
    TMatrix4 m4;
    im.ToMatrix4(m4);
    m_pDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX*)&m4);
  }
}

//----------------------------------------------------------------------
//  Retorno:        
//  Parametros:     
//  Uso:            
//----------------------------------------------------------------------
void CDisplayDevice::SetTexture        (int stage, DXN_Direct3DTexture *pTex)
{
  if (!IsAvailable() || m_pDevice == NULL)
    return;
  m_pDevice->SetTexture(stage, pTex);
}

//----------------------------------------------------------------------
//  Retorno:        
//  Parametros:     
//  Uso:            
//----------------------------------------------------------------------
void CDisplayDevice::InternalSetRS           (unsigned rs, unsigned val)
{
  m_pDevice->SetRenderState((D3DRENDERSTATETYPE)rs, val);
  m_aRenderState[rs] = val;
}

//----------------------------------------------------------------------
//  Retorno:        
//  Parametros:     
//  Uso:            
//----------------------------------------------------------------------
void CDisplayDevice::InternalSetTSS          (int stage, unsigned tss, unsigned val)
{
  m_pDevice->SetTextureStageState(stage, (D3DTEXTURESTAGESTATETYPE)tss, val);
  m_aTextureStageState[stage][tss] = val;
}

//----------------------------------------------------------------------
//  Retorno:        
//  Parametros:     
//  Uso:            
//----------------------------------------------------------------------
void CDisplayDevice::InternalSetSS           (int stage, unsigned ss, unsigned val)
{
  m_pDevice->SetSamplerState(stage, (D3DSAMPLERSTATETYPE)ss, val);
  m_aSamplerState[stage][ss] = val;
}

//----------------------------------------------------------------------
//  Retorno:        
//  Parametros:     
//  Uso:            
//----------------------------------------------------------------------
void CDisplayDevice::SetFVFShader      (unsigned fvf)
{
  if (fvf != m_FVFShader)
  {
    if (fvf != (unsigned)-1)
      m_pDevice->SetFVF(fvf);
    m_FVFShader = fvf;
  }
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void CDisplayDevice::RecomputeWorldViewProj() const
{
  TMatrix4 wm;
  m_WorldTransform.ToMatrix4(wm);
  TMatrix4 vm;
  m_ViewTransform.ToMatrix4(vm);
  m_TransWorldViewProj = Transpose(wm * vm * m_ProjTransform);
  m_bWorldViewProjDirty = false;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void CDisplayDevice::RecomputeScreenTransform() const
{
/*  // If we want world / view transforms to affect the screen transforms, we might as well provide a SetOrthoProj and be done with it
  TMatrix4 wm;
  m_WorldTransform.ToMatrix4(wm);
  TMatrix4 vm;
  m_ViewTransform.ToMatrix4(vm);
  TMatrix4 om; // Ortho projection
  memset(&om, 0, sizeof(om));
  om._11 = 1.f/(m_ViewportSize.x*.5f);
  om._22 = -1.f/(m_ViewportSize.y*.5f);
  om._41 = -1.f + m_ViewportPos.x/m_ViewportSize.x;
  om._42 =  1.f - m_ViewportPos.y/m_ViewportSize.y;
  om._33 = 1.f;
  om._44 = 1.f;
  m_TransScreenTransform = Transpose(wm * vm * m_ProjTransform);
*/
  TMatrix4 &om = m_TransScreenTransform;
  memset(&om, 0, sizeof(om));
  om._11 = 1.f/(m_ViewportSize.x*.5f);
  om._22 = -1.f/(m_ViewportSize.y*.5f);
  om._14 = -1.f + m_ViewportPos.x/m_ViewportSize.x;
  om._24 =  1.f - m_ViewportPos.y/m_ViewportSize.y;
  om._33 = 1.f;
  om._44 = 1.f;
  m_bScreenTransformDirty = false;
}

// ------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void CDisplayDevice::ShutdownManagers        ()
{
  TLinkedListNode<IManager*> *pNode = m_ManagerList.GetHead();

  while (pNode)
  {
    if (pNode->GetData()->Shutdown(this))
      pNode = pNode->pNext;
    else
    {
      TLinkedListNode<IManager*> *pNext = pNode->pNext;
      m_ManagerList.Remove(pNode);
      pNode = pNext;
    }
  }
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void CDisplayDevice::RestoreManagers         ()
{
  TLinkedListNode<IManager*> *pNode = m_ManagerList.GetHead();

  while (pNode)
  {
    if (pNode->GetData()->Restore(this))
      pNode = pNode->pNext;
    else
    {
      TLinkedListNode<IManager*> *pNext = pNode->pNext;
      m_ManagerList.Remove(pNode);
      pNode = pNext;
    }
  }
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void CDisplayDevice::RegisterManager   (IManager *pMgr)
{
  ASSERT(IsOk());
  if (!m_ManagerList.FindNode(pMgr))
    m_ManagerList.Add(pMgr);
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void CDisplayDevice::UnregisterManager (IManager *pMgr)
{
  ASSERT(IsOk());
  m_ManagerList.Remove(pMgr);
}

//----------------------------------------------------------------------
//  Retorno:        
//  Parametros:     
//  Uso:            
//----------------------------------------------------------------------
void CDisplayDevice::ResetMaterial ()
{
  if (!IsAvailable() || m_pDevice == NULL)
    return;
  memset(m_aRenderState, -1, sizeof(m_aRenderState));
  memset(m_aTextureStageState, -1, sizeof(m_aTextureStageState));
  memset(m_aSamplerState, -1, sizeof(m_aSamplerState));
  m_FVFShader = -1;

  // Limpiar stagestates.
  for (int i = 0; i < MAXTEXTURESTAGES; i++)
  {
    InternalSetTSS(i, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    InternalSetTSS(i, D3DTSS_COLOROP, D3DTOP_DISABLE);
    InternalSetSS(i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    InternalSetSS(i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    InternalSetSS(i, D3DSAMP_ADDRESSU,   D3DTADDRESS_WRAP);
    InternalSetSS(i, D3DSAMP_ADDRESSV,   D3DTADDRESS_WRAP);
    InternalSetSS(i, D3DSAMP_ADDRESSW,   D3DTADDRESS_WRAP);
    m_pDevice->SetTexture   (i, NULL);
  }
  m_pDevice->SetTextureStageState(MAXTEXTURESTAGES+1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
  m_pDevice->SetTextureStageState(MAXTEXTURESTAGES+1, D3DTSS_COLOROP, D3DTOP_DISABLE);

  // Estados que no van a cambiar.
  InternalSetTSS(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
  InternalSetTSS(0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE);
  InternalSetSS(0, D3DSAMP_ADDRESSU,  D3DTADDRESS_WRAP);
  InternalSetSS(0, D3DSAMP_ADDRESSV,  D3DTADDRESS_WRAP);
  InternalSetSS(0, D3DSAMP_ADDRESSW,  D3DTADDRESS_WRAP);
  InternalSetRS (D3DRS_DITHERENABLE,       TRUE);
  InternalSetRS (D3DRS_ALPHAREF,           0x80);
  InternalSetRS (D3DRS_ALPHAFUNC,          D3DCMP_GREATEREQUAL);
  InternalSetRS (D3DRS_ZFUNC,              D3DCMP_LESSEQUAL);

  // Estados de material que queremos tener por defecto.
  m_pDevice->SetTexture          (0, NULL);
  InternalSetTSS(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
  InternalSetTSS(0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
  InternalSetSS(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
  InternalSetSS(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
  InternalSetRS (D3DRS_SRCBLEND,           D3DBLEND_SRCALPHA);
  InternalSetRS (D3DRS_DESTBLEND,          D3DBLEND_INVSRCALPHA);
  InternalSetRS (D3DRS_SHADEMODE,          D3DSHADE_GOURAUD);
  InternalSetRS (D3DRS_SPECULARENABLE,     TRUE);
  InternalSetRS (D3DRS_ALPHATESTENABLE,    FALSE);
  InternalSetRS (D3DRS_ALPHABLENDENABLE,   FALSE);
  InternalSetRS (D3DRS_LIGHTING,           FALSE);

  InternalSetRS (D3DRS_COLORVERTEX , FALSE);
  InternalSetRS (D3DRS_AMBIENT , 0xFF303030);
  InternalSetRS (D3DRS_DIFFUSEMATERIALSOURCE,  D3DMCS_MATERIAL);
  InternalSetRS (D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL);
  InternalSetRS (D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL);
  InternalSetRS (D3DRS_AMBIENTMATERIALSOURCE,  D3DMCS_MATERIAL);
  for (int i = 0; i < 8; i++)
    m_pDevice->LightEnable(i, FALSE);

  m_bUsingSoftwareVP = false;
  m_pDevice->SetSoftwareVertexProcessing(m_bUsingSoftwareVP);

/* 
  DWORD nPasses = (uint)-1;
  HRESULT hRet = m_pDevice->ValidateDevice(&nPasses);
  if (FAILED(hRet))
    GLOG(("D3D: ValidateDevice() retorna 0x%X (%s) en %d pasos.\n", hRet, DXGetErrorString8(hRet), nPasses));
*/
}

// ------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------
//  Retorno:        
//  Parametros:     
//  Uso:            
//----------------------------------------------------------------------
void CDisplayDevice::Clear(TColor color, float depth, int stValue)
{
  if (IsAvailable())
  {
    bool bColor   = (color >> 24) > 0;
    bool bZBuffer = (depth >= 0) && UsingZBuffer();
    bool bStencil = (stValue >= 0) && UsingStencil();
    if (bColor || bZBuffer || bStencil)
    {
      unsigned flags = 0;
      if (bColor)   flags |= D3DCLEAR_TARGET;
      if (bZBuffer) flags |= D3DCLEAR_ZBUFFER;
      if (bStencil) flags |= D3DCLEAR_STENCIL;
      GetDirect3DDevice()->Clear(0, NULL, flags, color, depth, stValue);
    }
  }
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TError CDisplayDevice::BeginFrame        ()
{
  if (!IsAvailable() || !m_pDevice)
    return RET_FAIL;

  // If the device is not ours to play with, don't even try.

  if (m_pDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
    Restore();

  if (!IsDeviceReady())
    return RET_FAIL;

  HRESULT hr = m_pDevice->BeginScene();

  return (hr == D3D_OK)? RET_OK : RET_FAIL;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void CDisplayDevice::EndFrame          ()
{
  if (IsDeviceReady())
  {
    Flush();
    m_pDevice->EndScene();
  }
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void CDisplayDevice::Update            ()
{
  if (IsDeviceReady())
  {
//    GLOG(("---- Present ----\n"));
    if (FAILED(m_pDevice->Present(NULL, NULL, NULL, NULL)))
    {
      GLOG(("Present Failed!?!?!\n"));
    }
  }
}

//----------------------------------------------------------------------
//  Retorno:        
//  Parametros:     
//  Uso:            
//----------------------------------------------------------------------
void CDisplayDevice::Flush             ()
{
}

//----------------------------------------------------------------------
//  Retorno:        
//  Parametros:     
//  Uso:            
//----------------------------------------------------------------------
bool CDisplayDevice::UsingHWTnL        () const
{
  return !m_bUsingSoftwareVP;
}

//----------------------------------------------------------------------
//  Retorno:        
//  Parametros:     
//  Uso:            
//----------------------------------------------------------------------
void CDisplayDevice::EnableHWTnL       (bool bEnable)
{
  if (UsingHWTnL() ^ bEnable)
  {
    Flush();
    m_bUsingSoftwareVP = !bEnable;
    m_pDevice->SetSoftwareVertexProcessing(m_bUsingSoftwareVP);
  }
}
