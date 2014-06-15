// -------------------------------------------------------------------------------------
// File:        BasicMesh.cpp
//
// Purpose:     
// -------------------------------------------------------------------------------------

#include "GfxPCH.h"

#include "BasicMesh.h"
#include "DisplayDevice.h"

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
// Function:      Init
// Purpose:       Initialize the object
// Parameters:    
// --------------------------------------------------------------------------
TError CBasicMesh::Init  (CDisplayDevice *pDev, unsigned fvf, int nVerts, int nFaces, int stride, int nSections, bool bSysmem)
{
  ASSERT(nVerts > 0);
  ASSERT(nFaces > 0);
  ASSERT(stride > 0);
  ASSERT(nSections > 0);

  End();

  m_paSections = NEW_ARRAY(TSection, nSections);
  if (!m_paSections)
    return RET_FAIL;
  ZeroMemory(m_paSections, nSections*sizeof(*m_paSections));
  m_nSections = nSections;
  m_paSections[0].nFaces = nFaces;
  m_paSections[0].nVerts = nVerts;
  
  HRESULT hr;
  D3DPOOL pool  = bSysmem? D3DPOOL_SYSTEMMEM : D3DPOOL_MANAGED;
  DWORD   usage = bSysmem? D3DUSAGE_SOFTWAREPROCESSING : D3DUSAGE_WRITEONLY;
  hr = pDev->GetDirect3DDevice()->CreateVertexBuffer(nVerts*stride, usage, fvf, pool, &m_pVBuffer, NULL);
  if (!FAILED(hr))
    hr = pDev->GetDirect3DDevice()->CreateIndexBuffer(nFaces*3*sizeof(word), usage, D3DFMT_INDEX16, pool, &m_pIBuffer, NULL);

  if (FAILED(hr))
  {
    DISPOSE_ARRAY(m_paSections);
    End();
    return RET_FAIL;
  }
  m_nVerts = nVerts;
  m_nFaces = nFaces;
  m_stride = stride;
  m_fvf = fvf;

  return RET_OK;
}

// --------------------------------------------------------------------------
// Function:      End
// Purpose:       Release all resources and deinitialize the object
// Parameters:    
// --------------------------------------------------------------------------
void CBasicMesh::End  ()
{
  if (IsOk())
  {
    SAFE_RELEASE(m_pVBuffer);
    SAFE_RELEASE(m_pIBuffer);
    DISPOSE_ARRAY(m_paSections);
  }
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void *CBasicMesh::LockVB      (int start, int len)
{
  if (IsOk())
  {
    void *pv;
    if (!FAILED(m_pVBuffer->Lock(start*m_stride, (len > 0)? len*m_stride : 0, &pv, 0)))//D3DLOCK_DISCARD)))
      return pv;
  }
  return NULL;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void CBasicMesh::UnlockVB    ()
{
  if (IsOk())
    m_pVBuffer->Unlock();
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
word *CBasicMesh::LockIB      (int start, int len)
{
  if (IsOk())
  {
    word *pi;
    if (!FAILED(m_pIBuffer->Lock(start*3*sizeof(*pi), (len > 0)? len*3*sizeof(*pi) : 0, (VOID**)&pi, 0)))//D3DLOCK_DISCARD)))
      return pi;
  }
  return NULL;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void CBasicMesh::UnlockIB    ()
{
  if (IsOk())
    m_pIBuffer->Unlock();
}


// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void  CBasicMesh::SetSection  (int n, int startF, int nFaces, int startV, int nVerts)
{
  if (IsOk())
  {
    ASSERT(n >= 0 && n < m_nSections);
    m_paSections[n].startV = startV;
    m_paSections[n].nVerts = (nVerts < 0)? (m_nVerts - startV) : nVerts;
    m_paSections[n].startF = startF;
    m_paSections[n].nFaces = nFaces;
  }
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void  CBasicMesh::Draw     (CDisplayDevice *pDev, int nSection) const
{
  if (IsOk())
  {
    ASSERT(nSection >= 0 && nSection < m_nSections);
    if (m_paSections[nSection].nFaces)
    {
      pDev->SetFVFShader(m_fvf);
      pDev->GetDirect3DDevice()->SetStreamSource(0, m_pVBuffer, 0, m_stride);
      pDev->GetDirect3DDevice()->SetIndices(m_pIBuffer);
      pDev->GetDirect3DDevice()->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,
                                                      0,
                                                      m_paSections[nSection].startV,
                                                      m_paSections[nSection].nVerts,
                                                      m_paSections[nSection].startF*3,
                                                      m_paSections[nSection].nFaces);
    }
  }
}
