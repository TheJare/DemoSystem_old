//----------------------------------------------------------------------------
//  Nombre:    DemoEffectManager.cpp
//
//  Contenido: Gestor de efectos de la demo
//----------------------------------------------------------------------------

#include "DemoPCH.h"
#include "DemoEffectManager.h"
#include "DemoEffect.h"

#include "TextureManager.h"
#include "FontManager.h"
#include "DisplayFont.h"

#include "MusicPlayer.h"
#include "StrUtil.h"
#include "WinWindow.h"
#include "FileSystem.h"
#include <ctype.h>

// -----------------------------------
// Lista de clases de efectos.
// -----------------------------------

#define EFFECTENTRY(c) EFFECTCLASS_FNDECLARE(c)
#include "StandardEffectClassList.h"
#undef EFFECTENTRY

static TEffectClassDesc s_aStandardEffectClass[] = 
{
#define EFFECTENTRY(c) EFFECTCLASS_TBLENTRY(c)
#include "StandardEffectClassList.h"
#undef EFFECTENTRY
};

static const TEffectClassDesc *FindClass(const char *pszClass, const TEffectClassDesc *paList, int nEntries)
{
  for (int i = 0; i < nEntries; i++)
    if (stricmp(paList[i].pszClassName, pszClass) == 0)
      return paList + i;
  return NULL;
}

// ----------------------------------------------------------------------
// Comandos de sistema guardados como efectos.
// ----------------------------------------------------------------------

// -----------------------------------
// -----------------------------------
extern CDemoEffect *Create_LoadMusic(CDemoEffectManager *pMgr, const char *pszName, const char *pszArg)
{
  char token[300];
  if (StrUtil::GetToken(token, sizeof(token), pszArg))
  {
    if (!pMgr->GetMusicPlayer()->IsOk())
      pMgr->GetMusicPlayer()->Init(pMgr->GetWindow()->GetHwnd());
    pMgr->GetMusicPlayer()->LoadModule(token);
  }
  return NULL;
}

// -----------------------------------
// -----------------------------------
extern CDemoEffect *Create_SetBlitMode(CDemoEffectManager *pMgr, const char *pszName, const char *pszArg)
{
  int b = StrUtil::GetInt(pszArg);
  pMgr->SetBlitMode(b != 0);
  return NULL;
}

// -----------------------------------
// -----------------------------------
extern CDemoEffect *Create_SetFillColor(CDemoEffectManager *pMgr, const char *pszName, const char *pszArg)
{
  byte r, g, b, a;
  r = StrUtil::GetInt(pszArg);
  g = StrUtil::GetInt(pszArg);
  b = StrUtil::GetInt(pszArg);
  a = StrUtil::GetInt(pszArg);
  pMgr->SetFillColor(MakeColor(r,g,b,a));
  return NULL;
}

// -----------------------------------
// -----------------------------------
extern CDemoEffect *Create_ForceNoClear(CDemoEffectManager *pMgr, const char *pszName, const char *pszArg)
{
  int b = StrUtil::GetInt(pszArg);
  pMgr->SetForceNoClear(b != 0);
  return NULL;
}

// -----------------------------------
// -----------------------------------
extern CDemoEffect *Create_RetraceWait(CDemoEffectManager *pMgr, const char *pszName, const char *pszArg)
{
  int b = StrUtil::GetInt(pszArg);
  pMgr->SetRetraceWait(b != 0);
  return NULL;
}

// -----------------------------------
// -----------------------------------
extern CDemoEffect *Create_SetMode(CDemoEffectManager *pMgr, const char *pszName, const char *pszArg)
{
  int w = StrUtil::GetInt(pszArg);
  int h = StrUtil::GetInt(pszArg);
  int bpp = StrUtil::GetInt(pszArg);
  if (bpp == 0)
    bpp = 32;
  if (w > 0 && h > 0)
    pMgr->GetDisplayDevice()->SetDisplayMode(w, h, bpp, pMgr->GetDisplayDevice()->IsFullScreen());
  return NULL;
}

// -----------------------------------
// -----------------------------------
extern CDemoEffect *Create_SetViewport(CDemoEffectManager *pMgr, const char *pszName, const char *pszArg)
{
  int x = StrUtil::GetInt(pszArg);
  int y = StrUtil::GetInt(pszArg);
  int w = StrUtil::GetInt(pszArg);
  int h = StrUtil::GetInt(pszArg);

  int dw = pMgr->GetDisplayDevice()->GetWidth();
  int dh = pMgr->GetDisplayDevice()->GetHeight();
  if (w == 0) w = dw;
  if (h == 0) h = dh;
  if (x+w > dw) w = dw-x;
  if (y+h > dh) h = dh-y;
  if (x < 0) { w += x; x = 0; }
  if (y < 0) { h += y; y = 0; }

  if (w > 0 && h > 0)
    pMgr->SetViewport(x, y, w, h);
  return NULL;
}

// -----------------------------------
// -----------------------------------
extern CDemoEffect *Create_PlayMusic(CDemoEffectManager *pMgr, const char *pszName, const char *pszArg)
{
  char token[300];
  bool bLoop = false;

  if (StrUtil::GetToken(token, sizeof(token), pszArg))
    bLoop = atoi(token) != 0;
  pMgr->GetMusicPlayer()->Play(bLoop);
  return NULL;
}

// -----------------------------------
// -----------------------------------
extern CDemoEffect *Create_SetMusicPos(CDemoEffectManager *pMgr, const char *pszName, const char *pszArg)
{
  int pos = StrUtil::GetInt(pszArg);
  pMgr->GetMusicPlayer()->SetPosition(pos);
  return NULL;
}

// -----------------------------------
// -----------------------------------
extern CDemoEffect *Create_SetMusicVol(CDemoEffectManager *pMgr, const char *pszName, const char *pszArg)
{
  float pos = StrUtil::GetFloat(pszArg);
  pMgr->GetMusicPlayer()->SetVolume(pos);
  return NULL;
}

// -----------------------------------
// -----------------------------------
extern CDemoEffect *Create_DemoEnd(CDemoEffectManager *pMgr, const char *pszName, const char *)
{
  pMgr->EndDemo();
  return NULL;
}

// -----------------------------------

//----------------------------------------------------------------------
//  Uso:            
//  Retorno:        
//  Parametros:     
//----------------------------------------------------------------------
TError CDemoEffectManager::Init(CMusicPlayer *pMusicPlayer, CDisplayDevice *pDevice, const char *pszScriptName, const TEffectClassDesc *paEffects, int nEffects)
{
  End();

  // Clear stuff that may be destroyed in case of init error.
  m_pTextures     = NULL;

  CFile *f = FileSystem::Open(pszScriptName, "rt");
  int nline = 0;
  bool bError = false;

  if (f == NULL)
    return RET_FAIL;

  int inComment = 0;
  int pos = 0;
  int row = 0;
  while (!f->Eof())
  {
    nline++;

    char line[1000];
    char token[1000];

    if (NULL == f->Gets(line, 1000))
      break;

    StrUtil::CleanLine(line, line);
    if (line[0] == '\0')
      continue;

    // Comentarios de bloque
    if (line[0] == '{')
    {
      inComment++;
      continue;
    }
    if (line[0] == '}')
    {
      inComment--;
      continue;
    }
    if (inComment > 0)
      continue;

    // Nos vale la linea
    //strupr(line);

    const char *p = line;
    TEffect *pEffect = new TEffect;
    if (pEffect == NULL)
      break;

    bError = true;
    if (isdigit(p[0]))
    {
      pos = StrUtil::GetInt(p);
      row = StrUtil::GetInt(p);
    }
    pEffect->pos = pos;
    pEffect->row = row;

    if (StrUtil::GetToken(token, sizeof(token), p) && strlen(token) < TEffect::NAMELEN)
    {
      if (token[0] == '-')
        bError = false;       // Unnamed effect means line may be a plain sync point, without command
      else
        strcpy(pEffect->szName, token);

      if (StrUtil::GetToken(token, sizeof(token), p) && strlen(token) < TEffect::NAMELEN)
      {
        strcpy(pEffect->szCommand, token);
        strupr(pEffect->szCommand);
        strupr(pEffect->szName);
        pEffect->pszArgs = strdup(p);
        bError = false;
        m_EffectScript.AddEnd(pEffect);
      }
    }
    if (bError)
      break;
  }
  FileSystem::Close(f);

  if (bError)
  {
    m_EffectScript.DeleteAll();
    GLOG(("Error en la linea %d parseando el script %s\n", nline, pszScriptName));
    return RET_FAIL;
  }

  m_pMusicPlayer  = pMusicPlayer;
  m_pDevice       = pDevice;
  m_pTextures     = NEW(CTextureManager);
  m_pTextures->Init(pDevice);
  m_pFonts        = NEW(CFontManager);
  m_pFonts->Init(pDevice);

  m_paUserEffects = paEffects;
  m_nUserEffects  = nEffects;
  m_pCurEffect    = m_EffectScript.GetHead();
  m_bWantBlit     = false;
  m_bForceNoClear = false;
  m_bRetraceWait  = false;

  m_vpx = 0;
  m_vpy = 0;
  m_vpw = 640;
  m_vph = 480;

  m_fillColor = MakeColor(0, 0, 0, 255);

  return RET_OK;
}

//----------------------------------------------------------------------
//  Uso:            
//  Retorno:        
//  Parametros:     
//----------------------------------------------------------------------
void CDemoEffectManager::End()
{
  if (!IsOk())
    return;

  m_EffectList.DeleteAll();
  m_EffectScript.DeleteAll();
  DISPOSE(m_pFonts);
  DISPOSE(m_pTextures);
}

//----------------------------------------------------------------------
//  Uso:            
//  Retorno:        
//  Parametros:     
//----------------------------------------------------------------------
void CDemoEffectManager::Start()
{
  if (!IsOk())
    return;
  m_vpw = m_pDevice->GetWidth();
  m_vph = m_pDevice->GetHeight();
}

//----------------------------------------------------------------------
//  Uso:            
//  Retorno:        
//  Parametros:     
//----------------------------------------------------------------------
CWinWindow *CDemoEffectManager::GetWindow() const
{
  if (!IsOk())
    return NULL;
  return m_pDevice->GetWindow();
}


//----------------------------------------------------------------------
//  Uso:            
//  Retorno:        
//  Parametros:     
//----------------------------------------------------------------------
TLinkedListNode<CDemoEffect*> *CDemoEffectManager::FindEffectByName (const char *pszName)
{
  TLinkedListNode<CDemoEffect*> *pEffect = m_EffectList.GetHead();
  while (pEffect != NULL)
  {
    if (pEffect->t->IsName(pszName))
      break;
    else
      pEffect = pEffect->pNext;
  }
  return pEffect;
}

//----------------------------------------------------------------------
//  Uso:            
//  Retorno:        
//  Parametros:     
//----------------------------------------------------------------------
void CDemoEffectManager::Run(uint msThisFrame)
{
  if (!IsOk())
    return;

  // Procesa el script
  bool bForceFrameSync = false;
  m_pMusicPlayer->GetPosition(&m_musicPos, &m_musicRow);
  while (!bForceFrameSync     &&
         m_pCurEffect != NULL &&
         (m_pCurEffect->t->pos < m_musicPos || 
          (m_pCurEffect->t->pos == m_musicPos && m_pCurEffect->t->row <= m_musicRow)
         )
        )
  {
    if (strcmp(m_pCurEffect->t->szCommand, "NEW") == 0)
    {
      const char *p = m_pCurEffect->t->pszArgs;
      char token[300];

      if (StrUtil::GetToken(token, sizeof(token), p))
      {
        const TEffectClassDesc *pClass = FindClass(token, s_aStandardEffectClass, SIZE_ARRAY(s_aStandardEffectClass));
        if (pClass == NULL)
          pClass = FindClass(token, m_paUserEffects, m_nUserEffects);
        if (pClass != NULL)
        {
          CDemoEffect *pDemoEffect = pClass->pCreateFn(this, m_pCurEffect->t->szName, p);
          if (pDemoEffect != NULL)
            m_EffectList.AddEnd(pDemoEffect);
        }
        else
          GLOG(("ERRRRRRROR: Efecto %s no encontrado\n", token));
      }
    }
    else if (strcmp(m_pCurEffect->t->szCommand, "DELETE") == 0)
    {
      TLinkedListNode<CDemoEffect*> *pEffect = FindEffectByName(m_pCurEffect->t->szName);
      if (pEffect != NULL)
      {
        NUKE_PTR(pEffect->t);
        m_EffectList.Remove(pEffect);
      }
    }
    else if (strcmp(m_pCurEffect->t->szCommand, "TOFRONT") == 0)
    {
      TLinkedListNode<CDemoEffect*> *pEffect = FindEffectByName(m_pCurEffect->t->szName);
      if (pEffect != NULL)
      {
        CDemoEffect *pt = pEffect->t;
        m_EffectList.Remove(pEffect);
        m_EffectList.AddEnd(pt);
      }
    }
    else if (strcmp(m_pCurEffect->t->szCommand, "PLAY") == 0)
    {
      TLinkedListNode<CDemoEffect*> *pEffect = FindEffectByName(m_pCurEffect->t->szName);
      if (pEffect != NULL)
        pEffect->t->Play(true);
    }
    else if (strcmp(m_pCurEffect->t->szCommand, "PAUSE") == 0)
    {
      TLinkedListNode<CDemoEffect*> *pEffect = FindEffectByName(m_pCurEffect->t->szName);
      if (pEffect != NULL)
        pEffect->t->Play(false);
    }
    else if (strcmp(m_pCurEffect->t->szCommand, "FRAME") == 0)
      bForceFrameSync = true;
    else if (strcmp(m_pCurEffect->t->szCommand, "MOVEOUTSIDE") == 0)
    {
      const char *p = m_pCurEffect->t->pszArgs;
      char token[300];

      while (StrUtil::GetToken(token, sizeof(token), p))
      {
        TLinkedListNode<CDemoEffect*> *pEffect = FindEffectByName(token);
        if (pEffect != NULL && pEffect->t->GetParentEffect())
          pEffect->t->GetParentEffect()->AddSubEffect(pEffect->t);
      }
    }
    else if (strcmp(m_pCurEffect->t->szCommand, "MOVEINSIDE") == 0)
    {
      const char *p = m_pCurEffect->t->pszArgs;
      char token[300];

      TLinkedListNode<CDemoEffect*> *pParent = FindEffectByName(m_pCurEffect->t->szName);
      if (!pParent)
        GLOG(("ERRROR: Parent effect \"%s\" not found!!\n", m_pCurEffect->t->szName));
      else
      {
        while (StrUtil::GetToken(token, sizeof(token), p))
        {
          TLinkedListNode<CDemoEffect*> *pEffect = FindEffectByName(token);
          if (pEffect != NULL && pEffect->t != pParent->t)
            pParent->t->AddSubEffect(pEffect->t);
        }
      }
    }
    else
    {
      // Comando especifico del efecto.
      TLinkedListNode<CDemoEffect*> *pEffect = FindEffectByName(m_pCurEffect->t->szName);
      if (pEffect != NULL)
        pEffect->t->Command(m_pCurEffect->t->szCommand, m_pCurEffect->t->pszArgs);
    }

    if (m_pCurEffect != NULL)
      m_pCurEffect = m_pCurEffect->pNext;
  }

  // Procesa los efectos.
  TLinkedListNode<CDemoEffect*> *pEffect = m_EffectList.GetHead();
  while (pEffect != NULL)
  {
    TLinkedListNode<CDemoEffect*> *pNext = pEffect->pNext;
    if (pEffect->t->IsPlaying())
      pEffect->t->Run(msThisFrame);
    if (!pEffect->t->IsOk())
      m_EffectList.Remove(pEffect);
    pEffect = pNext;
  }
}

//----------------------------------------------------------------------
//  Uso:            
//  Retorno:        
//  Parametros:     
//----------------------------------------------------------------------
void CDemoEffectManager::Draw()
{
  if (!IsOk())
    return;

  CRenderContext rc;

  // Busca cual es el ultimo nodo que rellena la pantalla
  bool bFills = false;
  TLinkedListNode<CDemoEffect*> *pEffect = m_EffectList.GetTail();
  while (!bFills && pEffect != NULL)
  {
    if (pEffect->t->FillsScreen() && !pEffect->t->GetParentEffect())
      bFills = true;
    else
      pEffect = pEffect->pPrev;
  }

  // Si ningun efecto borra, entonces empieza por el primero.
  if (!bFills)
    pEffect = m_EffectList.GetHead();

  TLinkedListNode<CDemoEffect*> *pFirstEffect = pEffect;

  // Predraw de los efectos.
  // De TODOS los efectos, no solo de los que se van a pintar.
  pEffect = m_EffectList.GetHead();
  while (pEffect != NULL)
  {
    TLinkedListNode<CDemoEffect*> *pNext = pEffect->pNext;

    if (pEffect->t->IsPlaying() && !pEffect->t->GetParentEffect())
      pEffect->t->PreDraw();
    pEffect = pNext;
  }

  // Prepara el render context para los efectos.
  TProjectionViewport vp(m_vpx, m_vpy, m_vpw, m_vph);
  vp.SetFOV(RAD_45);
  rc.Init(m_pDevice, vp);

  // Si ningun efecto rellena la pantalla, y tampoco forzamos a que no se borre, entonces se borra.
  bool bPendingFill = false;
  if (!bFills && !m_bForceNoClear)
  {
    if (Color_A(m_fillColor) == 0)
      ;
    else if (Color_A(m_fillColor) < 255)
    {
      bPendingFill = true;
    }
    else
      rc.Clear(m_fillColor, 1.f);
  }
  else
  {
    // Borra solo el zbuffer
    rc.Clear(0, 1.f);
  }

  if (RET_OK == m_pDevice->BeginFrame())
  {
    if (bPendingFill)
      rc.FillRect(float(m_vpx), float(m_vpy), float(m_vpw), float(m_vph), m_fillColor, NULL, .001f);//.001f);

    pEffect = pFirstEffect;
    // Dibuja los efectos.
    while (pEffect != NULL)
    {
      TLinkedListNode<CDemoEffect*> *pNext = pEffect->pNext;

      if (pEffect->t->IsPlaying() && !pEffect->t->GetParentEffect())
        pEffect->t->Draw(rc);
      pEffect = pNext;
    }
#ifndef VERSIONFINAL
    {
      CDisplayFont *pFont = m_pFonts->GetFont("Fonts\\Arial.igf");
      if (!pFont)
        pFont = m_pFonts->InitFont("Fonts\\Arial.igf", 0);
      if (pFont)
        pFont->Printf(rc, 0, 10, 0xFFFFFFFF, "%d:%d", m_musicPos, m_musicRow);
    }
#endif

    m_pDevice->EndFrame();
  }
  m_pDevice->Update();
}
