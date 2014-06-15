//-----------------------------------------------------------------------------------------
//  Nombre:     RenderContext.h
//
//  Contenido:  Operaciones de renderizado.
//-----------------------------------------------------------------------------------------

#ifndef _RENDER_CONTEXT_H_
#define _RENDER_CONTEXT_H_

#include "ProjectionViewport.h"
#include "DisplayShader.h"

class CDisplayDevice;
class IDisplayMaterial;
struct TVector3;

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

class CRenderContext
{
  public:
    CRenderContext  (): m_pDev(NULL)    { }
    ~CRenderContext ()                  { End(); }

    void  Init      (CDisplayDevice *pDev,
                     int x, int y, int w, int h,
                     float ivpw = 0, float ivph = 2.f,
                     float zn = 1.f, float zf = 1025.f);

    void  Init      (CDisplayDevice *pDev,
                     const TProjectionViewport &vp);

    void  End       ()                                { m_pDev = NULL; }
    bool  IsOk      ()            const { return m_pDev != NULL; }

    uint  GetWidth  () const    { return m_vp.vw;  }
    uint  GetHeight () const    { return m_vp.vh; }
    float GetZNear  () const    { return m_vp.zn;  }
    float GetZFar   () const    { return m_vp.zf;   }

    CDisplayDevice            *GetDevice    ()              const { return m_pDev; }
    const TProjectionViewport &GetViewport  ()              const { return m_vp; }
    void                       SetViewport  (const TProjectionViewport &vp);

    void  Clear     (TColor color = 0xFF000000, float depth = 1.f, int stValue = -1);

    bool        SetupSimpleMaterial       (TColor c, const IDisplayMaterial *pMat = NULL);

    // Quick & dirty primitives for getting stuff up and running.
    void  FillTexRect (float x, float y, float w, float h, TColor c, const IDisplayMaterial *pMat, float iz = 1.f);
    void  FillRect    (float x, float y, float w, float h, TColor c, const IDisplayMaterial *pMat = NULL, float iz = 1.f);
    void  DrawLine2D  (float x0, float y0, float x1, float y1, TColor c, const IDisplayMaterial *pMat = NULL, float iz = 1.f);
    void  DrawLine3D  (const TVector3 &v0, const TVector3 &v1, TColor c, const IDisplayMaterial *pMat = NULL);
    void  DrawTri     (const TVector3 &v0, const TVector3 &v1, const TVector3 &v2, TColor c, const IDisplayMaterial *pMat = NULL);

  private:

    // ------------------

    CDisplayDevice     *m_pDev;
    TProjectionViewport m_vp;

    CDisplayShader      m_Shader;
};

#endif // _RENDER_CONTEXT_H_
