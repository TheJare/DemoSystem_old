//----------------------------------------------------------------------------
//  Nombre:    DemoEffect.cpp
//
//  Contenido: Base para un efecto de la demo.
//----------------------------------------------------------------------------

#include "DemoPCH.h"
#include "DemoEffect.h"

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TError  CDemoEffect::Init  (CDemoEffectManager *pManager, const char *pszName, const char *pszArgs)
{
  m_pManager = pManager;
  strcpy(m_szName, pszName);
  m_flags = 0;
  m_ticks = 0;
  m_pParentEffect = NULL;
  m_SubEffects.Init();
  return RET_OK;
}
// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void CDemoEffect::End   ()
{
  if (IsOk())
  {
    if (GetParentEffect())
      GetParentEffect()->RemoveSubEffect(this);
    TLinkedListNode<CDemoEffect*> *pEffect = m_SubEffects.GetHead();
    while (pEffect != NULL)
    {
      pEffect->t->SetParentEffect(NULL);
      pEffect = pEffect->pNext;
    }
    m_SubEffects.End();
    m_pManager = NULL;
    m_szName[0] = '\0';
  }
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void CDemoEffect::AddSubEffect          (CDemoEffect *pEffect)
{
  if (pEffect->GetParentEffect() && pEffect->GetParentEffect() != this)
    pEffect->GetParentEffect()->RemoveSubEffect(pEffect);

  m_SubEffects.AddEnd(pEffect);
  pEffect->SetParentEffect(this);
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void CDemoEffect::RemoveSubEffect       (CDemoEffect *pEffect)
{
  if (m_SubEffects.Remove(pEffect))
    pEffect->SetParentEffect(NULL);
}
