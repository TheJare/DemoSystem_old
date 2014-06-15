//----------------------------------------------------------------------------
//  Nombre:    FontManager.cpp
//
//  Contenido: Agrupacion de fonts
//----------------------------------------------------------------------------

#include "GfxPCH.h"
#include "FontManager.h"
#include "DisplayFont.h"
#include "DisplayVertex.h"

struct CFontManager::TFont
{
  int          refCount;
  CDisplayFont Font;
};

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TError    CFontManager::Init        (CDisplayDevice *pDevice)
{
  End();

  m_Fonts.Init();
  m_pDev = pDevice;
  m_pDev->RegisterManager(this);
  return RET_OK;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void      CFontManager::End         ()
{
  if (IsOk())
  {
    Shutdown(m_pDev);

    m_VB.End();
    m_pDev->UnregisterManager(this);
    TLinkedListNode<TFont*> *pNode = m_Fonts.GetHead();
    while (pNode)
    {
      DISPOSE(pNode->t);
      pNode = pNode->pNext;
    }
  }
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
CDisplayFont *CFontManager::InitFont(const char *pszName, uint wantMatFlags)
{
  TLinkedListNode<TFont *> *pFontNode = FindFont(pszName);
  if (pFontNode)
  {
    pFontNode->t->refCount++;
    return &pFontNode->t->Font;
  }

  TFont *pFont = NEW(TFont);
  if (RET_OK != pFont->Font.Init(pszName, &m_VB, m_pDev))
    DISPOSE(pFont);
  else
  {
    pFont->refCount = 1;
    m_Fonts.AddFirst(pFont);
  }
  return pFont? &pFont->Font : NULL;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
bool CFontManager::ReleaseFont(const char *pszName)
{
  TLinkedListNode<TFont*> *pFontNode = FindFont(pszName);
  if (pFontNode)
  {
    pFontNode->t->refCount--;
    if (pFontNode->t->refCount > 0)
      return true;    // Font still exists in this manager

    DISPOSE(pFontNode->t);
    m_Fonts.Remove(pFontNode);
  }
  return false;   // Font doesn't exist (anymore) in this manager.
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
bool CFontManager::ReleaseFont(CDisplayFont *pFont)
{
  TLinkedListNode<TFont*> *pFontNode = FindFont(pFont);
  if (pFontNode)
  {
    pFontNode->t->refCount--;
    if (pFontNode->t->refCount == 0)
    {
      DISPOSE(pFontNode->t);
      m_Fonts.Remove(pFontNode);
    }
    return false;
  }
  return true;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
CDisplayFont *CFontManager::GetFont (const char *pszName) const
{
  ASSERT(IsOk());

  TLinkedListNode<TFont*> *pNode = FindFont(pszName);
  if (!pNode)
    return NULL;
  return &pNode->t->Font;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
bool CFontManager::Shutdown  (CDisplayDevice *pDevice)
{
  ASSERT(IsOk());

  m_VB.End();

  TLinkedListNode<TFont*> *pNode = m_Fonts.GetHead();
  while (pNode)
  {
    pNode->t->Font.Unload();
    pNode = pNode->pNext;
  }
  return true;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
bool CFontManager::Restore (CDisplayDevice *pDevice)
{
  ASSERT(IsOk());

  if (RET_OK != m_VB.Init(pDevice, DV_FVF_TLVERTEXUV, 600, sizeof(TTLVertexUV)))
    return false;

  TLinkedListNode<TFont*> *pNode = m_Fonts.GetHead();
  while (pNode)
  {
    TLinkedListNode<TFont*> *pNext = pNode->pNext;

    if (RET_OK != pNode->t->Font.Load(pDevice))
      m_Fonts.Remove(pNode);
    pNode = pNext;
  }
  return true;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TLinkedListNode<CFontManager::TFont*> *CFontManager::FindFont (const char *pszName) const
{
  TLinkedListNode<TFont*> *pNode = m_Fonts.GetHead();
  while (pNode)
  {
    if (stricmp(pszName, pNode->t->Font.GetName()) == 0)
      return pNode;
    pNode = pNode->pNext;
  }
  return NULL;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TLinkedListNode<CFontManager::TFont*> *CFontManager::FindFont (CDisplayFont *pFont) const
{
  TLinkedListNode<TFont*> *pNode = m_Fonts.GetHead();
  while (pNode)
  {
    if (pFont == &pNode->t->Font)
      return pNode;
    pNode = pNode->pNext;
  }
  return NULL;
}
