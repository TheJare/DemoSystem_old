//----------------------------------------------------------------------------
//  Nombre:    TextureManager.cpp
//
//  Contenido: Agrupacion de texturas.
//----------------------------------------------------------------------------

#include "GfxPCH.h"
#include "TextureManager.h"
#include "DisplayTexture.h"

struct CTextureManager::TTexture
{
  int             refCount;
  CDisplayTexture Tex;
};

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TError    CTextureManager::Init        (CDisplayDevice *pDevice)
{
  End();

  m_Textures.Init();
  m_pDev = pDevice;
  m_pDev->RegisterManager(this);
  return RET_OK;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void      CTextureManager::End         ()
{
  if (IsOk())
  {
    Shutdown(m_pDev);
    m_pDev->UnregisterManager(this);
    TLinkedListNode<TTexture*> *pNode = m_Textures.GetHead();
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
CDisplayTexture *CTextureManager::InitMaterial(const char *pszName, uint wantMatFlags)
{
  TLinkedListNode<TTexture *> *pTexNode = FindMaterial(pszName);
  if (pTexNode)
  {
    pTexNode->t->refCount++;
    return &pTexNode->t->Tex;
  }

  TTexture *pTex = NEW(TTexture);
  pTex->Tex.Init(pszName, wantMatFlags);
  pTex->refCount = 1;
  m_Textures.AddFirst(pTex);
  return &pTex->Tex;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
bool CTextureManager::ReleaseMaterial(const char *pszName)
{
  TLinkedListNode<TTexture*> *pTexNode = FindMaterial(pszName);
  if (pTexNode)
  {
    pTexNode->t->refCount--;
    if (pTexNode->t->refCount > 0)
      return true;    // Material still exists in this manager

    DISPOSE(pTexNode->t);
    m_Textures.Remove(pTexNode);
  }
  return false;   // Material doesn't exist (anymore) in this manager.
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
bool CTextureManager::ReleaseMaterial(CDisplayTexture *pTex)
{
  TLinkedListNode<TTexture*> *pTexNode = FindMaterial(pTex);
  if (pTexNode)
  {
    pTexNode->t->refCount--;
    if (pTexNode->t->refCount == 0)
    {
      DISPOSE(pTexNode->t);
      m_Textures.Remove(pTexNode);
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
CDisplayTexture *CTextureManager::GetMaterial (const char *pszName) const
{
  ASSERT(IsOk());

  TLinkedListNode<TTexture*> *pNode = FindMaterial(pszName);
  if (!pNode)
    return NULL;
  return &pNode->t->Tex;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
bool CTextureManager::Shutdown  (CDisplayDevice *pDevice)
{
  ASSERT(IsOk());

  TLinkedListNode<TTexture*> *pNode = m_Textures.GetHead();
  while (pNode)
  {
    pNode->t->Tex.Unload();
    pNode = pNode->pNext;
  }
  return true;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
bool CTextureManager::Restore (CDisplayDevice *pDevice)
{
  ASSERT(IsOk());

  TLinkedListNode<TTexture*> *pNode = m_Textures.GetHead();
  while (pNode)
  {
    pNode->t->Tex.Load(pDevice);
    pNode = pNode->pNext;
  }
  return true;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TLinkedListNode<CTextureManager::TTexture*> *CTextureManager::FindMaterial (const char *pszName) const
{
  TLinkedListNode<TTexture*> *pNode = m_Textures.GetHead();
  while (pNode)
  {
    if (stricmp(pszName, pNode->t->Tex.GetName()) == 0)
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
TLinkedListNode<CTextureManager::TTexture*> *CTextureManager::FindMaterial (CDisplayTexture *pTex) const
{
  TLinkedListNode<TTexture*> *pNode = m_Textures.GetHead();
  while (pNode)
  {
    if (pTex == &pNode->t->Tex)
      return pNode;
    pNode = pNode->pNext;
  }
  return NULL;
}
