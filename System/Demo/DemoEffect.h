//----------------------------------------------------------------------------
//  Nombre:    DemoEffect.h
//
//  Contenido: Base para un efecto de la demo.
//----------------------------------------------------------------------------

#ifndef _DEMO_EFFECT_H_
#define _DEMO_EFFECT_H_

#include "LinkedList.h"

class CRenderContext;
class CDemoEffectManager;

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

class CDemoEffect
{
  public:
            CDemoEffect     (): m_pManager(NULL)  { m_szName[0] = '\0'; }
    virtual ~CDemoEffect    ()                    { End(); }

            TError  Init        (CDemoEffectManager *pManager, const char *pszName, const char *pszArgs);
    virtual void    End         ();
            bool    IsOk        () const                                            { return (m_pManager != NULL); }

            bool    IsName      (const char *pszName) const                         { return (stricmp(m_szName, pszName) == 0); }
    virtual TError  Command     (const char *pszCommand, const char *pszArg)        { return RET_OK; }

    virtual void    Run         (uint msThisFrame)          { m_ticks += msThisFrame; }
    virtual void    Draw        (CRenderContext &rc)  const { };
    virtual void    PreDraw     ()                          { }

            void    Play        (bool bPlay)          { m_flags = bPlay? (m_flags | FLAG_PLAYING) : (m_flags & ~FLAG_PLAYING); }
            void    FillScreen  (bool bFill)          { m_flags = bFill? (m_flags | FLAG_FILLSCREEN) : (m_flags & ~FLAG_FILLSCREEN); }

            bool    FillsScreen () const              { return IsOk() && (m_flags & (FLAG_PLAYING | FLAG_FILLSCREEN)) == (FLAG_PLAYING | FLAG_FILLSCREEN); }
            bool    IsPlaying   () const              { return IsOk() && (m_flags & FLAG_PLAYING) != 0; }

            void          AddSubEffect          (CDemoEffect *pEffect);
            void          RemoveSubEffect       (CDemoEffect *pEffect);
            CDemoEffect*  GetParentEffect       ()                        const { return m_pParentEffect; }

  protected:
    enum
    {
      FLAG_PLAYING    = 0x0001,
      FLAG_FILLSCREEN = 0x0002,
    };

    CDemoEffectManager  *m_pManager;
    char                m_szName[300];

    uint                m_flags;
    uint                m_ticks;

    CDemoEffect*              m_pParentEffect;
    CLinkedList<CDemoEffect*> m_SubEffects;

    void    SetParentEffect   (CDemoEffect *pEffect)          { m_pParentEffect = pEffect; }

};

// ----------------------------------------------------
// Utilities for declaring and creating an effect class list.


#define EFFECTCLASS_FNDEFINE(c)  extern CDemoEffect *MACRO_PASTE(Create_,c) (CDemoEffectManager *pMgr, const char *pszName, const char *pszArg) \
{ \
  MACRO_PASTE(CDE_,c) *pEffect = new MACRO_PASTE(CDE_,c); \
  if (pEffect != NULL) \
  { \
    if (RET_OK != pEffect->Init(pMgr, pszName, pszArg)) \
    { \
      delete pEffect; \
      pEffect = NULL; \
    } \
  } \
  return pEffect; \
}


#endif // _DEMO_EFFECT_H_
