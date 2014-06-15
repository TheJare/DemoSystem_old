// -------------------------------------------------------------------------------------
// File:        DynamicVB.h
//
// Purpose:     
//   Modified from the original by:
//    D. Sim Dietrich Jr.
//    sim.dietrich@nvidia.com
// -------------------------------------------------------------------------------------

#ifndef _DYNAMICVB_H_
#define _DYNAMICVB_H_

#include "DXNames.h"

class CDisplayDevice;

class CDynamicVB 
{
	public :
    CDynamicVB  (): m_pVB(NULL)   { }
    ~CDynamicVB ()                { End(); }

    TError    Init        (CDisplayDevice *pDev, unsigned fvf, int count, int stride);
    void      End         ();
    bool      IsOk        ()                  const { return (m_pVB != NULL); }

    unsigned  GetFVF      ()                  const { return m_fvf; }
    int       GetPos      ()                  const { return m_pos; }
    int       GetStride   ()                  const { return m_stride; }

    // Query this to find out if you should flush the buffer before adding "count" new vertices.
    bool      MustFlush   (int count)         const { return 	( m_bFlush || ( ( count + m_pos) > m_count ) ); }

    DXN_D3DVertexBuffer *
              GetVB       ()                  const { return m_pVB; }

    // Mark the buffer as "pending to be flushed".
    void      SetFlush    ()                        { m_bFlush = true; }
    void     *Lock        (int count, int *pStart);
    void      Unlock      ();

	private :
		DXN_D3DVertexBuffer   *m_pVB;

		int                   m_count;
		int                   m_pos;
    int                   m_stride;
    unsigned              m_fvf;

		bool		              m_bLocked;
		bool		              m_bFlush;
};

#endif  _DYNAMICVB_H_
