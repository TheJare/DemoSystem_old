//----------------------------------------------------------------------------
//  Nombre:    DE_RenderTarget.h
//
//  Contenido: Base de efecto que agrupa otros efectos.
//----------------------------------------------------------------------------

#ifndef _DE_RENDERTARGET_H_
#define _DE_RENDERTARGET_H_

#include "DemoEffect.h"
#include "RenderTarget.h"
#include "DXNames.h"

class CRenderTarget;

class CDE_RenderTarget: public CDemoEffect
{
  typedef CDemoEffect inherited;
  public:
            CDE_RenderTarget      () { }
    virtual ~CDE_RenderTarget     () { End(); }

    virtual TError  Init        (CDemoEffectManager *pManager, const char *pszName, const char *pszArg);
    virtual void    End         ();

    virtual void    PreDraw     ();
    virtual TError  Command     (const char *pszCommand, const char *pszArg);

  protected:
    enum { MAX_TARGETS = 4 };
    int                   m_nTargets;
    CRenderTarget         m_aRenderTargets[MAX_TARGETS];

    bool m_bForceNoClear;

    int m_vpx, m_vpy, m_vpw, m_vph;

    TColor m_fillColor;

};

#endif // _DE_RENDERTARGET_H_
