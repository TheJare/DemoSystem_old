//-----------------------------------------------------------------------------------------
//  Nombre:     RenderContext.cpp
//
//  Contenido:  Operaciones de renderizado.
//-----------------------------------------------------------------------------------------

#include "GfxPCH.h"

#include "RenderContext.h"
#include "DisplayDevice.h"
#include "StdDisplayMaterials.h"
#include "Vectors.h"
#include "DisplayVertex.h"

//----------------------------------------------------------------------
//----------------------------------------------------------------------
//----------------------------------------------------------------------

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
__forceinline bool CRenderContext::SetupSimpleMaterial(TColor c, const IDisplayMaterial *pMat)
{
  if (pMat)
  {
    pMat->Set(m_pDev);
  }
  else
  {
    int a = Color_A(c);
    if (a == 0)
      return false;
    if (a < 255)
    {
      m_Shader.GetPass(0)->SrcBlend = D3DBLEND_SRCALPHA;
      m_Shader.GetPass(0)->DstBlend = D3DBLEND_INVSRCALPHA;
    }
    else
    {
      m_Shader.GetPass(0)->SrcBlend = D3DBLEND_ONE;
      m_Shader.GetPass(0)->DstBlend = D3DBLEND_ZERO;
    }
    m_Shader.Set(m_pDev);
  }
  return true;
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------
//----------------------------------------------------------------------


//----------------------------------------------------------------------
//  Retorno:        
//  Parametros:     
//  Uso:            
//----------------------------------------------------------------------
void CRenderContext::Init   (CDisplayDevice *pDev,
                             int x, int y, int w, int h,
                             float ivpw, float ivph,
                             float zn, float zf)
{
  TProjectionViewport vp(x, y, w, h, ivpw, ivph, zn, zf);
  Init(pDev, vp);
}

//----------------------------------------------------------------------
//  Retorno:        
//  Parametros:     
//  Uso:            
//----------------------------------------------------------------------
void CRenderContext::Init   (CDisplayDevice *pDev,
                             const TProjectionViewport &vp)
{
  m_pDev = pDev;
  SetViewport(vp);

  m_Shader.Init();
}


// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void CRenderContext::SetViewport  (const TProjectionViewport &vp)
{
  m_vp = vp;
  m_pDev->SetProjectionViewport(&m_vp);
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void CRenderContext::Clear     (TColor color, float depth, int stValue)
{
  ASSERT(IsOk());
//  ASSERT(m_pDev->IsActive());

  m_pDev->Clear(color, depth, stValue);
}

//----------------------------------------------------------------------
//  Retorno:        
//  Parametros:     
//  Uso:            
//----------------------------------------------------------------------
void CRenderContext::FillTexRect(float x, float y, float w, float h, TColor c, const IDisplayMaterial *pMat, float iz)
{
  ASSERT(IsOk());
  ASSERT(pMat);
//  ASSERT(m_pDev->IsActive());

  pMat->Set(m_pDev);

  TTLVertexUV av[4];
  float rhw = iz;
  float z = m_vp.XFormInvZ(iz);

  av[0].Set(x,   y,   z, rhw, c, 0, 0);
  av[1].Set(x+w, y,   z, rhw, c, 1, 0);
  av[2].Set(x,   y+h, z, rhw, c, 0, 1);
  av[3].Set(x+w, y+h, z, rhw, c, 1, 1);

  m_pDev->SetFVFShader(DV_FVF_TLVERTEXUV);
  m_pDev->GetDirect3DDevice()->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, av, sizeof(*av));
}

//----------------------------------------------------------------------
//  Retorno:        
//  Parametros:     
//  Uso:            
//----------------------------------------------------------------------
void CRenderContext::FillRect(float x, float y, float w, float h, TColor c, const IDisplayMaterial *pMat, float iz)
{
  ASSERT(IsOk());
//  ASSERT(m_pDev->IsActive());

//  m_Shader.GetPass(0)->ZFunc  = D3DCMP_ALWAYS;
  m_Shader.GetPass(0)->ZFunc  = D3DCMP_LESSEQUAL;
  if (!SetupSimpleMaterial(c, pMat))
    return;

  TTLVertex av[4];
  float rhw = iz;
  float z = m_vp.XFormInvZ(iz);

  av[0].Set(x,   y,   z, rhw, c);
  av[1].Set(x+w, y,   z, rhw, c);
  av[2].Set(x,   y+h, z, rhw, c);
  av[3].Set(x+w, y+h, z, rhw, c);

  m_pDev->SetFVFShader(DV_FVF_TLVERTEX);
  m_pDev->GetDirect3DDevice()->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, av, sizeof(*av));
}

//----------------------------------------------------------------------
//  Retorno:        
//  Parametros:     
//  Uso:            
//----------------------------------------------------------------------
void CRenderContext::DrawLine2D(float x0, float y0, float x1, float y1, TColor c, const IDisplayMaterial *pMat, float iz)
{
  ASSERT(IsOk());
//  ASSERT(m_pDev->IsActive());

  m_Shader.GetPass(0)->ZFunc  = D3DCMP_ALWAYS;
  if (!SetupSimpleMaterial(c, pMat))
    return;

  TTLVertex av[2];

  float rhw = iz;
  float z = m_vp.XFormInvZ(iz);
  
  av[0].Set(x0, y0, z, rhw, c);
  av[1].Set(x1, y1, z, rhw, c);

  m_pDev->SetFVFShader(DV_FVF_TLVERTEX);
  m_pDev->GetDirect3DDevice()->DrawPrimitiveUP(D3DPT_LINELIST, 1, av, sizeof(*av));
}

//----------------------------------------------------------------------
//  Retorno:        
//  Parametros:     
//  Uso:            
//----------------------------------------------------------------------
void CRenderContext::DrawLine3D(const TVector3 &v0, const TVector3 &v1, TColor c, const IDisplayMaterial *pMat)
{
  ASSERT(IsOk());
//  ASSERT(m_pDev->IsActive());

  m_Shader.GetPass(0)->ZFunc  = D3DCMP_LESSEQUAL;
  if (!SetupSimpleMaterial(c, pMat))
    return;

  TLVertex av[2];
  
  av[0].Set(v0.x, v0.y, v0.z, c);
  av[1].Set(v1.x, v1.y, v1.z, c);
  
  m_pDev->SetFVFShader(DV_FVF_LVERTEX);
  m_pDev->GetDirect3DDevice()->DrawPrimitiveUP(D3DPT_LINELIST, 1, av, sizeof(*av));
//  m_pDev->GetLBatch()->Unlock();
}

//----------------------------------------------------------------------
//  Retorno:        
//  Parametros:     
//  Uso:            
//----------------------------------------------------------------------
void CRenderContext::DrawTri     (const TVector3 &v0, const TVector3 &v1, const TVector3 &v2, TColor c, const IDisplayMaterial *pMat)
{
  ASSERT(IsOk());
//  ASSERT(m_pDev->IsActive());

  m_Shader.GetPass(0)->ZFunc  = D3DCMP_LESSEQUAL;
  if (!SetupSimpleMaterial(c, pMat))
    return;

  TLVertex av[3];
  
  av[0].Set(v0.x, v0.y, v0.z, c);
  av[1].Set(v1.x, v1.y, v1.z, c);
  av[2].Set(v2.x, v2.y, v2.z, c);
  
  m_pDev->SetFVFShader(DV_FVF_LVERTEX);
  m_pDev->GetDirect3DDevice()->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 1, av, sizeof(*av));
}
