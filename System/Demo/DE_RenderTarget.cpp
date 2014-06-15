//----------------------------------------------------------------------------
//  Nombre:    DE_RenderTarget.cpp
//
//  Contenido: Base de efecto que agrupa otros efectos.
//----------------------------------------------------------------------------

#include "DemoPCH.h"

#include "DE_RenderTarget.h"
#include "DemoEffectManager.h"
#include "DisplayTexture.h"
#include "RenderContext.h"
#include "StrUtil.h"

EFFECTCLASS_FNDEFINE(RenderTarget)

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TError  CDE_RenderTarget::Init   (CDemoEffectManager *pManager, const char *pszName, const char *pszArg)
{
  End();

  char token[500];
  int w, h, bpp;
  if (!StrUtil::GetToken(token, sizeof(token), pszArg))
    return RET_FAIL;

  w = StrUtil::GetInt(pszArg, 512);
  h = StrUtil::GetInt(pszArg, 256);
  bpp = StrUtil::GetInt(pszArg, 16);
  m_nTargets = Clamp(StrUtil::GetInt(pszArg), 1, (int)MAX_TARGETS);

  m_bForceNoClear = false;
  m_vpx = 0;
  m_vpy = 0;
  m_vpw = w;
  m_vph = h;
  m_fillColor = 0xFF000000;

  if (RET_OK != inherited::Init(pManager, pszName, pszArg))
    return RET_FAIL;

  DXN_Direct3DSurface  *pzb = NULL;
  for (int i = 0; i < m_nTargets; i++)
  {
    char buf[500];
    if (i == 0)
      strcpy(buf, token);
    else
      sprintf(buf, "%s_%d", token, i);

    if (RET_OK != m_aRenderTargets[i].Init(pManager->GetTextureManager(), w, h, bpp, buf, true, false, NULL, pzb))
      return RET_FAIL;
    pzb = m_aRenderTargets[i].GetZBuffer();
  }

  return RET_OK;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void    CDE_RenderTarget::End    ()
{
  if (IsOk())
  {
    for (int i = 0; i < m_nTargets; i++)
      m_aRenderTargets[i].End();
    inherited::End();
  }
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TError  CDE_RenderTarget::Command     (const char *pszCommand, const char *pszArg)
{
  if (stricmp(pszCommand, "SetFillColor") == 0)
  {
    byte r, g, b, a;
    r = StrUtil::GetInt(pszArg);
    g = StrUtil::GetInt(pszArg);
    b = StrUtil::GetInt(pszArg);
    a = StrUtil::GetInt(pszArg);
    m_fillColor = MakeColor(r,g,b,a);
    return RET_OK;
  }
  else if (stricmp(pszCommand, "ForceNoClear") == 0)
  {
    m_bForceNoClear = (StrUtil::GetInt(pszArg) != 0);
    return RET_OK;
  }
  else if (stricmp(pszCommand, "SetViewport") == 0)
  {
    int x = StrUtil::GetInt(pszArg);
    int y = StrUtil::GetInt(pszArg);
    int w = StrUtil::GetInt(pszArg);
    int h = StrUtil::GetInt(pszArg);

    int dw = m_aRenderTargets[0].GetWidth();
    int dh = m_aRenderTargets[0].GetWidth();
    if (w == 0) w = dw;
    if (h == 0) h = dh;
    if (x+w > dw) w = dw-x;
    if (y+h > dh) h = dh-y;
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }

    if (w > 0 && h > 0)
    {
      m_vpx = x;
      m_vpy = y;
      m_vpw = w;
      m_vph = h;
    }
    return RET_OK;
  }
  else
    return RET_FAIL;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void    CDE_RenderTarget::PreDraw     ()
{
  // ------------
  // Rotate rendertarget chain
  // ------------------------
  DXN_Direct3DTexture *pPrevTex = m_aRenderTargets[0].GetBackbuffer()->GetTexture();
  for (int i = m_nTargets-1; i >= 0; i--)
    pPrevTex = m_aRenderTargets[i].GetBackbuffer()->SetTexture(pPrevTex);
  //LEAK - SHOULD DO?
  SAFE_RELEASE(pPrevTex);

  // ---------------------------
  // Busca cual es el ultimo nodo que rellena la pantalla
  bool bFills = false;

  // Si ningun efecto borra, entonces empieza por el primero.
  TLinkedListNode<CDemoEffect*> *pFirstEffect = m_SubEffects.GetHead();
  TLinkedListNode<CDemoEffect*> *pEffect = m_SubEffects.GetTail();

  while (!bFills && pEffect != NULL)
  {
    if (pEffect->t->FillsScreen() && !pEffect->t->GetParentEffect())
    {
      bFills = true;
      pFirstEffect = pEffect;
    }
    else
      pEffect = pEffect->pPrev;
  }

  // Predraw de los efectos.
  // De TODOS los efectos, no solo de los que se van a pintar.
  pEffect = m_SubEffects.GetHead();
  while (pEffect != NULL)
  {
    TLinkedListNode<CDemoEffect*> *pNext = pEffect->pNext;

    if (pEffect->t->IsPlaying() && pEffect->t->GetParentEffect() == this)
      pEffect->t->PreDraw();
    pEffect = pNext;
  }

  // ------------
  // Save previous rendertarget
  if (RET_OK == m_aRenderTargets[0].BeginFrame(m_pManager->GetDisplayDevice()))
  {
    // Prepare RenderContext
    // ------------
    TProjectionViewport vp(m_vpx, m_vpy, m_vpw, m_vph);
    vp.SetFOV(RAD_45);
    CRenderContext rc;
    rc.Init(m_pManager->GetDisplayDevice(), vp);

    // Si ningun efecto rellena la pantalla, y tampoco forzamos a que no se borre, entonces se borra.
    if (!bFills && !m_bForceNoClear)
    {
      if (Color_A(m_fillColor) == 0)
        ;
      else if (Color_A(m_fillColor) < 255)
        rc.FillRect(float(m_vpx), float(m_vpy), float(m_vpw), float(m_vph), m_fillColor, NULL, .001f);//.001f);
      else
        rc.Clear(m_fillColor, 1.f);
    }
    else
    {
      // Borra solo el zbuffer
      rc.Clear(0, 1.f);
    }

    TLinkedListNode<CDemoEffect*> *pEffect = m_SubEffects.GetHead();
    while (pEffect != NULL)
    {
      if (pEffect->t->IsPlaying() && pEffect->t->GetParentEffect() == this)
        pEffect->t->Draw(rc);
      pEffect = pEffect->pNext;
    }

    // EndScene
    // ------------
    m_aRenderTargets[0].EndFrame(m_pManager->GetDisplayDevice());
  }
}
