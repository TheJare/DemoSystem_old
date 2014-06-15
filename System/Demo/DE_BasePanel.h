//----------------------------------------------------------------------------
//  Nombre:    DE_BasePanel.h
//
//  Contenido: Base de efecto para pintar paneles (texto o img)
//----------------------------------------------------------------------------

#ifndef _DE_BASEPANEL_H_
#define _DE_BASEPANEL_H_

#include "DemoEffect.h"

class CDE_BasePanel: public CDemoEffect
{
  typedef CDemoEffect inherited;
  public:
            CDE_BasePanel       () { }
    virtual ~CDE_BasePanel      () { End(); }

    virtual TError  Init        (CDemoEffectManager *pManager, const char *pszName, const char *pszArg);
    virtual void    End         ();
    virtual TError  Command     (const char *pszCommand, const char *pszArg);

    virtual void    Run         (uint msThisFrame);
    virtual void    Draw        (CRenderContext &rc) const;

  protected:
    struct TData
    {
      float  x, y;
      float  sx, sy;
      float  a, r, g, b;
    };

    virtual void    Draw  (CRenderContext &rc, float x, float y, float sx, float sy, dword c) const = 0;

    TData             m_actual;
    TData             m_from;
    TData             m_to;
    TData             m_vibrato;
    TData             m_speed;
    TData             m_accel;
    unsigned          m_total;
    unsigned          m_pos;
    bool              m_bMonoVib;
    bool              m_bAdd;
    unsigned          m_offMask;
};

#endif // _DE_BASEPANEL_H_
