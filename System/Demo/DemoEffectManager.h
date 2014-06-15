//----------------------------------------------------------------------------
//  Nombre:    DemoEffectManager.h
//
//  Contenido: Gestor de efectos de la demo
//----------------------------------------------------------------------------

#ifndef _DEMO_EFFECT_MANAGER_H_
#define _DEMO_EFFECT_MANAGER_H_

#include "LinkedList.h"

class CMusicPlayer;
class CWinWindow;
class CDisplayDevice;
class CRenderContext;
class CTextureManager;
class CFontManager;

class CDemoEffect;
class CDemoEffectManager;

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

struct TEffectClassDesc
{
  const char    *pszClassName;
  CDemoEffect *(*pCreateFn)(CDemoEffectManager *pMgr, const char *pszName, const char *pszArg);
};

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

class CDemoEffectManager
{
  public:
    CDemoEffectManager      (): m_pCurEffect(NULL)  { }
    ~CDemoEffectManager     ()                      { End(); }

    TError  Init        (CMusicPlayer *pMusicPlayer, CDisplayDevice *pDevice, const char *pszScriptName, const TEffectClassDesc *paEffects = NULL, int nEffects = 0);
    void    End         ();
    bool    IsOk        () const          { return !m_EffectScript.IsEmpty(); }

    void    Start       ();
    void    Run         (uint msThisFrame);
    void    Draw        ();

    void    EndDemo     ()                  { m_pCurEffect = NULL; }
    bool    IsAtEnd     ()            const { return !IsOk() || (m_pCurEffect == NULL); }

    bool    GetBlitMode ()            const { return m_bWantBlit; }
    void    SetBlitMode (bool mode)         { m_bWantBlit = mode; }

    bool    GetForceNoClear()         const { return m_bForceNoClear; }
    void    SetForceNoClear(bool mode)      { m_bForceNoClear = mode; }

    bool    GetRetraceWait()          const { return m_bRetraceWait; }
    void    SetRetraceWait(bool mode)       { m_bRetraceWait = mode; }

    void    SetFillColor  (TColor c)        { m_fillColor = c; }

    void    SetViewport   (int x, int y, int w, int h) { m_vpx = x; m_vpy = y; m_vpw = w; m_vph = h; }

    // ----------------------

    void              GetMusicPos         (int *pPos, int *pRow) const { *pPos = m_musicPos; *pRow = m_musicRow; }

    CMusicPlayer      *GetMusicPlayer     () const                     { return m_pMusicPlayer; }
    CDisplayDevice    *GetDisplayDevice   () const                     { return m_pDevice; }
    CWinWindow        *GetWindow          () const;
    CTextureManager   *GetTextureManager  () const                     { return m_pTextures; }
    CFontManager      *GetFontManager     () const                     { return m_pFonts; }

  private:

    TLinkedListNode<CDemoEffect*>     *FindEffectByName (const char *pszName);

    struct TEffect
    {
      enum { NAMELEN = 32 };
      int     pos;
      int     row;
      char    szName[NAMELEN];      // Name/identifier of the effect.
      char    szCommand[NAMELEN];
      char    *pszArgs;

      TEffect(): pszArgs(NULL) { }
      ~TEffect() { free(pszArgs); }
    };

    CLinkedList<TEffect*>             m_EffectScript;
    const TLinkedListNode<TEffect *>  *m_pCurEffect;

    int         m_musicPos;
    int         m_musicRow;

    CMusicPlayer      *m_pMusicPlayer;
    CDisplayDevice    *m_pDevice;
    CTextureManager   *m_pTextures;
    CFontManager      *m_pFonts;

    const TEffectClassDesc *m_paUserEffects;
    int                     m_nUserEffects;

    CLinkedList<CDemoEffect*>   m_EffectList;

    bool m_bWantBlit;
    bool m_bForceNoClear;
    bool m_bRetraceWait;

    int m_vpx, m_vpy, m_vpw, m_vph;

    TColor m_fillColor;
};

// ----------------------------------------------------
// Utilities for declaring and creating an effect class list.

#define EFFECTCLASS_FNDECLARE(c) extern CDemoEffect *MACRO_PASTE(Create_,c) (CDemoEffectManager *pMgr, const char *pszName, const char *pszArg);
#define EFFECTCLASS_TBLENTRY(c)  { #c, MACRO_PASTE(Create_,c) },

#endif // _DEMO_EFFECT_MANAGER_H_
