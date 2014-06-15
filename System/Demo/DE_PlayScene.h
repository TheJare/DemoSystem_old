// -------------------------------------------------------------------------------------
// File:        DE_PlayScene.h
//
// Purpose:     
// -------------------------------------------------------------------------------------

#ifndef _DE_PLAYSCENE_H_
#define _DE_PLAYSCENE_H_

#include "DemoEffect.h"
#include "DisplayScene.h"

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

class CDE_PlayScene: public CDemoEffect
{
    typedef CDemoEffect inherited;
  public:
    CDE_PlayScene      ()                  { }
    ~CDE_PlayScene     ()                  { End(); }

    TError      Init        (CDemoEffectManager *pManager, const char *pszName, const char *pszArgs);
    void        End         ();

    void        Run         (uint time);
    void        Draw        (CRenderContext &rc) const;

  private:

    CDisplayScene     m_Scene;
};

#endif // _DE_PLAYSCENE_H_
