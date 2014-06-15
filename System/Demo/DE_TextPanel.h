//----------------------------------------------------------------------------
//  Nombre:    DE_TextPanel.h
//
//  Contenido: Efecto para pintar paneles de texto
//----------------------------------------------------------------------------

#ifndef _DE_TEXTPANEL_H_
#define _DE_TEXTPANEL_H_

#include "DE_BasePanel.h"

class CDisplayFont;

// ----------------------------------------------------------------

class CDE_TextPanel: public CDE_BasePanel
{
  typedef CDE_BasePanel inherited;
  public:
            CDE_TextPanel       () { }
    virtual ~CDE_TextPanel      () { End(); }

    virtual TError  Init        (CDemoEffectManager *pManager, const char *pszName, const char *pszArg);
    virtual void    End         ();
//    virtual TError  Command     (const char *pszCommand, const char *pszArg);

  protected:

    virtual void    Draw  (CRenderContext &rc, float x, float y, float sx, float sy, dword c) const;

    CDisplayFont    *m_pFont;
    char            *m_pszText;
    dword            m_color;
    dword            m_shColor;
};

#endif // _DE_TEXTPANEL_H_
