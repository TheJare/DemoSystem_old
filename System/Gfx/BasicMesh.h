// -------------------------------------------------------------------------------------
// File:        BasicMesh.h
//
// Purpose:     
// -------------------------------------------------------------------------------------

#ifndef _BASICMESH_H_
#define _BASICMESH_H_

#include "DXNames.h"

class CDisplayDevice;
class CRenderContext;

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

class CBasicMesh
{
  public:
    CBasicMesh       (): m_pVBuffer(NULL)   { }
    ~CBasicMesh      ()                     { End(); }

    struct TSection
    {
      dword data;
      int startV;
      int nVerts;
      int startF;
      int nFaces;
    };

    TError      Init        (CDisplayDevice *pDev, unsigned fvf, int nVerts, int nFaces, int stride, int nSections = 1, bool bSysmem = false);
    void        End         ();
    bool        IsOk        ()                              const { return (m_pVBuffer != NULL); }

    void       *LockVB      (int start = 0, int len = -1);
    void        UnlockVB    ();
    word       *LockIB      (int start = 0, int len = -1);
    void        UnlockIB    ();

    int         GetNVerts       ()                          const { return m_nVerts;    }
    int         GetNFaces       ()                          const { return m_nFaces;    }
    int         GetFVF          ()                          const { return m_fvf;       }
    int         GetStride       ()                          const { return m_stride;    }
    int         GetNumSections  ()                          const { return m_nSections; }

    const TSection &
                GetSection      (int n)                         const { ASSERT(n >= 0 && n < m_nSections); return m_paSections[n]; }
    void        SetSection      (int n, int startF, int nFaces, int startV = 0, int nVerts = -1);

    dword       GetSectionData  (int n)                         const { ASSERT(n >= 0 && n < m_nSections); return m_paSections[n].data; }
    void        SetSectionData  (int n, dword data)                   { ASSERT(n >= 0 && n < m_nSections); m_paSections[n].data = data; }

    void        Draw        (CDisplayDevice *pDev, int nSection = 0) const;

  private:
    DXN_D3DVertexBuffer     *m_pVBuffer;
    DXN_D3DIndexBuffer      *m_pIBuffer;

    int       m_nVerts;
    int       m_nFaces;
    unsigned  m_fvf;
    int       m_stride;
    int       m_nSections;
    TSection  *m_paSections;
};

#endif // _BASICMESH_H_
