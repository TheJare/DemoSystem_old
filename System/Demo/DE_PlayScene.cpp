// -------------------------------------------------------------------------------------
// File:        DE_PlayScene.cpp
//
// Purpose:     
// -------------------------------------------------------------------------------------

#include "DemoPCH.h"
#include "DE_PlayScene.h"
#include "DemoEffectManager.h"

EFFECTCLASS_FNDEFINE(PlayScene)

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TError CDE_PlayScene::Init        (CDemoEffectManager *pManager, const char *pszName, const char *pszArgs)
{
  End();

  if (RET_OK != inherited::Init(pManager, pszName, pszArgs))
    return RET_FAIL;

  return m_Scene.Init(pManager->GetDisplayDevice(), pszArgs, pManager->GetTextureManager());
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void CDE_PlayScene::End         ()
{
  if (IsOk())
  {
    m_Scene.End();
    inherited::End();
  }
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void CDE_PlayScene::Run         (uint time)
{
  inherited::Run(time);

  m_Scene.SetCurTime(float(m_ticks)*.001f);
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void CDE_PlayScene::Draw        (CRenderContext &rc) const
{
  ASSERT(IsOk());

  m_Scene.Draw(rc);
}
