// -------------------------------------------------------------------------------------
// File:        DynamicVB.cpp
//
// Purpose:     
//   Modified from the original by:
//    D. Sim Dietrich Jr.
//    sim.dietrich@nvidia.com
// -------------------------------------------------------------------------------------

#include "GfxPCH.h"

#include "DynamicVB.h"
#include "DisplayDevice.h"

enum LOCK_FLAGS
{
	LOCKFLAGS_FLUSH  = D3DLOCK_NOSYSLOCK | D3DLOCK_DISCARD,
	LOCKFLAGS_APPEND = D3DLOCK_NOSYSLOCK | D3DLOCK_NOOVERWRITE
};


// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TError    CDynamicVB::Init        (CDisplayDevice *pDev, unsigned fvf, int count, int stride)
{
  End();

	m_pos = 0;
	m_bFlush = true;
	m_bLocked = false;
  m_stride = stride;
  m_count = count;
  m_fvf   = fvf;

  HRESULT hr = pDev->GetDirect3DDevice()->CreateVertexBuffer(
                                         count * stride,
				                                 D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC,
											                   fvf,
											                   D3DPOOL_DEFAULT,
  											                 &m_pVB, NULL);
  if (FAILED(hr))
  {
    m_pVB = NULL;
    return RET_FAIL;
  }
  return RET_OK;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void      CDynamicVB::End         ()
{
  if (IsOk())
  {
    Unlock();
    m_pVB->Release();
    m_pVB = NULL;
  }
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void     *CDynamicVB::Lock        (int count, int *pStart)
{
  *pStart = 0;
  if (!IsOk() || m_bLocked)
    return NULL;

  int start = 0;
	void* pData = NULL;

	// Ensure there is enough space in the VB for this data
	if (count > m_count)
    return NULL;

	DWORD dwFlags = LOCKFLAGS_APPEND;

	// If either user forced us to flush,
	//  or there is not enough space for the vertex data,
	//  then flush the buffer contents
	if (MustFlush(count))
	{
		m_bFlush = false;
		m_pos = 0;
		dwFlags = LOCKFLAGS_FLUSH;
	}

	HRESULT hr = m_pVB->Lock(m_pos * m_stride, 
							             count * m_stride, 
							             &pData, 
							             dwFlags );

	if (!FAILED(hr))
	{
		ASSERT(pData);
		m_bLocked = true;
		start = m_pos;
		m_pos += count;
	}

	*pStart = start;
	return pData;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void      CDynamicVB::Unlock      ()
{
  if (IsOk() && m_bLocked)
  {
    m_pVB->Unlock();
    m_bLocked = false;
  }
}
