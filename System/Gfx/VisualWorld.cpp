// -------------------------------------------------------------------------------------
// File:        VisualWorld.cpp
//
// Purpose:     General-purpose Scenegraph
// -------------------------------------------------------------------------------------

#include "GfxPCH.h"

#include "VisualWorld.h"
#include "DisplayDevice.h"
#include "RenderContext.h"

static const CRenderContext *s_pRC;     // !!!!!!!!

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
static int  __cdecl CompareDepth(const void *e1, const void *e2)
{
  union
  {
    float f;
    int i;
  } a;
  a.f = (*(CVisualWorld::IRenderable**)e1)->GetRenderDepth(*s_pRC) - (*(CVisualWorld::IRenderable**)e2)->GetRenderDepth(*s_pRC);
  return a.i;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
static int  __cdecl CompareMaterial(const void *e1, const void *e2)
{
  CVisualWorld::TSolidSection *p1 = *(CVisualWorld::TSolidSection**)e1;
  CVisualWorld::TSolidSection *p2 = *(CVisualWorld::TSolidSection**)e2;
  return p1->pRenderable->GetMaterialRef(p1->nSection) - p2->pRenderable->GetMaterialRef(p2->nSection);
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TError CVisualWorld::Init      (CDisplayDevice *pDevice, int nBuckets)
{
  ASSERT(pDevice);
  ASSERT(nBuckets > 0);

  if (!IsOk())
  {
    m_aTransparentNodes.Init(10);
    m_aSolidNodes.Init(10);
  }
  m_aBuckets.Init(nBuckets);
  m_pDevice = pDevice;
  return RET_OK;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void CVisualWorld::End       ()
{
  if (IsOk())
  {
    m_aSolidNodes.End();
    m_aTransparentNodes.End();
    m_aBuckets.End();
    m_pDevice = NULL;
  }
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void CVisualWorld::AddNode     (IRenderable *pRenderable, int iBucket)
{
  ASSERT(IsOk());
  ASSERT(pRenderable);
  ASSERT(iBucket >= 0);
  ASSERT(-1 == FindNode(pRenderable));

  if ((unsigned)iBucket >= m_aBuckets.GetCount())
    m_aBuckets.SetCount(iBucket+1);
  m_aBuckets[iBucket].flags = 0;
  m_aBuckets[iBucket].aNodes.Add(pRenderable);
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void CVisualWorld::SetBucketFlags  (int iBucket, dword flags)
{
  ASSERT(IsOk());
  ASSERT(iBucket >= 0);

  if ((unsigned)iBucket >= m_aBuckets.GetCount())
    m_aBuckets.SetCount(iBucket+1);
  m_aBuckets[iBucket].flags = flags;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void CVisualWorld::RemoveNode  (IRenderable *pRenderable)
{
  ASSERT(IsOk());
  ASSERT(pRenderable);

  for (unsigned i = 0; i < m_aBuckets.GetCount(); ++i)
  {
    if (m_aBuckets[i].aNodes.RemoveOrdered(pRenderable))
      break;
  }
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
int  CVisualWorld::FindNode    (IRenderable *pRenderable) const     // -1 if not found
{
  ASSERT(IsOk());
  ASSERT(pRenderable);

  for (unsigned i = 0; i < m_aBuckets.GetCount(); ++i)
  {
    const CDynArray<IRenderable*> &Nodes = m_aBuckets[i].aNodes;
    if (Nodes.Find(pRenderable) < Nodes.GetCount())
      return i;
  }
  return -1;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void CVisualWorld::PreDraw           (CRenderContext &rc)
{
  ASSERT(IsOk());

  for (unsigned i = 0; i < m_aBuckets.GetCount(); ++i)
  {
    CDynArray<IRenderable*> &Nodes = m_aBuckets[i].aNodes;
    for (unsigned j = 0; j < Nodes.GetCount(); ++j)
      Nodes[j]->PreDraw(rc);
  }
}

// --------------------------------------------------------------------------
// Function:      DrawBucket
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void CVisualWorld::DrawBucket      (CRenderContext &rc, unsigned nBucket) const
{
  ASSERT(IsOk());
  ASSERT(nBucket < m_aBuckets.GetCount());

  if (nBucket >= m_aBuckets.GetCount())
    return;

  const CDynArray<IRenderable*> &Nodes = m_aBuckets[nBucket].aNodes;
  m_aTransparentNodes.SetCount(0);
  m_aSolidNodes.SetCount(0);

  bool bSortByMaterial = (m_aBuckets[nBucket].flags & TBucket::F_SORTBYMATERIAL) != 0;
  bool bTransparent = (m_aBuckets[nBucket].flags & TBucket::F_TRANSPARENT) != 0;

  // Process nodes in the bucket.
  for (unsigned j = 0; j < Nodes.GetCount(); ++j)
  {
    IRenderable *pNode = Nodes[j];

    // Check visibility.
    if (!pNode->IsHidden() && pNode->IsVisible(rc))
    {
      // Store in transparent list if necessary
      if (bTransparent || pNode->HasTransparency())
        m_aTransparentNodes.Add(pNode);      // Store in transparency list

      // If has solid sections
      if (!bTransparent && pNode->HasSolid())
      {
        if (bSortByMaterial)                    // Must sort
        {
          int ns = pNode->GetNumSections();
          for (int k = 0; k < ns; k++)
          {
            if (!pNode->IsTransparent(k))
            {
              TSolidSection s = { pNode, pNode->GetMaterialRef(k) };
              m_aSolidNodes.Add(s);            // Store for material sorting
            }
          }
        }
        else
          Nodes[j]->Draw(rc, IRenderable::DRAW_SOLID);            // Draw directly
      }
    }
  }

  // Sort by material, and draw the nodes, if bucket says so.
  if (bSortByMaterial && m_aSolidNodes.GetCount())
  {
    qsort(&m_aSolidNodes[0], m_aSolidNodes.GetCount(), sizeof(m_aSolidNodes[0]), CompareMaterial);

    for (unsigned j = 0; j < m_aSolidNodes.GetCount(); ++j)
      m_aSolidNodes[j].pRenderable->Draw(rc, m_aSolidNodes[j].nSection);
  }

  // Sort transparent nodes is appropriate.
  if (m_aTransparentNodes.GetCount())
  {
    if (!(m_aBuckets[nBucket].flags & TBucket::F_PRESORTED))
    {
      s_pRC = &rc;
      qsort(&m_aTransparentNodes[0], m_aTransparentNodes.GetCount(), sizeof(m_aTransparentNodes[0]), CompareDepth);
    }

    for (unsigned j = 0; j < m_aTransparentNodes.GetCount(); ++j)
      m_aTransparentNodes[j]->Draw(rc, IRenderable::DRAW_TRANSPARENT);
  }
}
// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void CVisualWorld::Draw              (CRenderContext &rc) const
{
  ASSERT(IsOk());

  for (unsigned i = 0; i < m_aBuckets.GetCount(); ++i)
    DrawBucket(rc, i);
}
