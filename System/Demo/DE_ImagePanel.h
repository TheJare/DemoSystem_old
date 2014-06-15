//----------------------------------------------------------------------------
//  Nombre:    DE_ImagePanel.h
//
//  Contenido: Efecto para pintar paneles de texto
//----------------------------------------------------------------------------

#ifndef _DE_IMAGEPANEL_H_
#define _DE_IMAGEPANEL_H_

#include "DE_BasePanel.h"
#include "DisplayShader.h"

class CDisplayTexture;

// ----------------------------------------------------------------

class CDE_ImagePanel: public CDE_BasePanel
{
  typedef CDE_BasePanel inherited;
  public:
            CDE_ImagePanel       () { }
    virtual ~CDE_ImagePanel      () { End(); }

    virtual TError  Init        (CDemoEffectManager *pManager, const char *pszName, const char *pszArg);
    virtual void    End         ();
//    virtual TError  Command     (const char *pszCommand, const char *pszArg);

  protected:

    virtual void    Draw  (CRenderContext &rc, float x, float y, float sx, float sy, dword c) const;

    CDisplayTexture *m_pImage;
    dword            m_color;

    mutable CDisplayShader m_Shader;
};

#endif // _DE_IMAGEPANEL_H_
