//----------------------------------------------------------------------------
//  Nombre:    DisplayFont.cpp
//
//  Contenido: Fuente de letras
//----------------------------------------------------------------------------

#include "GfxPCH.h"
#include "DisplayFont.h"
#include "DisplayDevice.h"
#include "DisplayTexture.h"
#include "DisplayVertex.h"
#include "RenderContext.h"
#include "FileSystem.h"
#include "DynamicVB.h"

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

struct CDisplayFont::TTexture
{
  CDisplayTexture *pTex;
};

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TError CDisplayFont::Init (const char *pszName, CDynamicVB *pVB, CDisplayDevice *pDev)
{
  End();

  m_paChars = NULL;
  m_paTextures = NULL;
  if (!pszName[0])
    return RET_FAIL;

  int l = strlen(pszName);
  m_pszName = NEW_ARRAY(char, l+1);
  strcpy(m_pszName, pszName);

  CFile *pFile = FileSystem::Open(pszName);
  if (!pFile)
    return RET_FAIL;

  m_Shader.Init();
  m_Shader.GetPass(0)->SrcBlend = D3DBLEND_SRCALPHA;
  m_Shader.GetPass(0)->DstBlend = D3DBLEND_INVSRCALPHA;
  m_Shader.GetPass(0)->ZWrite = false;
  m_Shader.GetPass(0)->ZFunc = D3DCMP_ALWAYS;

  m_pVB = pVB;

  TError ret = RET_OK;

  int sig = pFile->GetLong();
  if (sig != '0GFI')
    ret = RET_FAIL;
  else
  {
    pFile->Read(m_aCharMap, sizeof(m_aCharMap));

    m_nChars = pFile->GetLong();
    m_paChars = NEW_ARRAY(TCharInfo, m_nChars);

    m_fontHeight = pFile->GetLong();

    for (int i = 0; i < m_nChars; i++)
    {
      TCharInfo &ci = m_paChars[i];

      ci.x = pFile->GetLong();
      ci.y = pFile->GetLong();
      ci.w = pFile->GetLong();
      ci.h = pFile->GetLong();
      ci.logWidth  = pFile->GetLong();
      ci.nTex      = pFile->GetLong();
      ci.offsX     = pFile->GetLong();
      ci.offsY     = pFile->GetLong();
    }

    if (pDev)
      ret = LoadTextures(pDev, pFile);
  }
  FileSystem::Close(pFile);

  if (ret != RET_OK)
    End();

  return ret;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void CDisplayFont::End         ()
{
  if (IsOk())
  {
    Unload();
    DISPOSE_ARRAY(m_paChars);
    DISPOSE_ARRAY(m_pszName);
    m_nChars = 0;
  }
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TError CDisplayFont::Load      (CDisplayDevice *pDev)
{
  if (!IsOk())
    return RET_FAIL;

  TError ret = RET_OK;

  if (!IsLoaded())
  {
    CFile *pFile = FileSystem::Open(m_pszName);
    if (!pFile)
      return RET_FAIL;

    int sig = pFile->GetLong();
    if (sig != '0GFI')
      ret = RET_FAIL;
    else
    {
      pFile->Seek( (1 + MAX_CHARS + 2 + 8*m_nChars)*sizeof(int));
      ret = LoadTextures(pDev, pFile);
    }
    FileSystem::Close(pFile);
  }
  return ret;
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void CDisplayFont::Unload ()
{
  if (IsOk())
  {
    for (int i = 0; i < m_nTextures; i++)
      DISPOSE(m_paTextures[i].pTex);
    DISPOSE_ARRAY(m_paTextures);
    m_nTextures = 0;
  }
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
TError CDisplayFont::LoadTextures(CDisplayDevice *pDev, CFile *pFile)
{
  TError ret = RET_OK;

  bool bRGB = (0 != pFile->GetLong());
  m_nTextures = pFile->GetLong();
  m_paTextures = NEW_ARRAY(TTexture, m_nTextures);
  if (!m_paTextures)
    return RET_FAIL;
  ZeroMemory(m_paTextures, sizeof(*m_paTextures)*m_nTextures);

  for (int i = 0; ret == RET_OK && i < m_nTextures; i++)
  {
    TTexture &t = m_paTextures[i];
    int w = pFile->GetLong();
    int h = pFile->GetLong();

    t.pTex = NEW (CDisplayTexture);
    if (RET_OK != t.pTex->Init("", w, h, bRGB? D3DFMT_A8R8G8B8 : D3DFMT_A4R4G4B4) || RET_OK != t.pTex->Load(pDev))
      ret = RET_FAIL;
    else
    {
      D3DLOCKED_RECT d3dlr;
      if (FAILED(t.pTex->GetTexture()->LockRect(0, &d3dlr, NULL, 0 )))
        ret = RET_FAIL;
      else
      {
        for (int j = 0; j < h; j++)
          if (bRGB)
          {
            DWORD* pDst = POINTER_ADD_T(DWORD, d3dlr.pBits, d3dlr.Pitch*j);
            pFile->Read(pDst, sizeof(*pDst)*w);
          }
          else
          {
            WORD* pDst = POINTER_ADD_T(WORD, d3dlr.pBits, d3dlr.Pitch*j);
            for (int k = 0; k < w; k++)
//              *pDst++ = ((pFile->GetChar() & 0xF0) << 12) | 0xFFF;
              *pDst++ = ((pFile->GetChar() & 0xF0) << 8) | 0xFFF;
          }
        t.pTex->GetTexture()->UnlockRect(0);
      }
    }
  }
  return ret;
}


// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------

void CDisplayFont::Printf    (CRenderContext &rc, float x, float y, dword color, const char *pszFmt, ...) const
{
  char buf[1000];
  va_list vl;
  va_start(vl, pszFmt);
  vsprintf(buf, pszFmt, vl);
  Puts(rc, x, y, 1, 1, color, (color & 0xFF000000), buf);
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void CDisplayFont::Printf    (CRenderContext &rc, float x, float y, float sx, float sy, dword color, dword shColor, const char *pszFmt, ...) const
{
  char buf[1000];
  va_list vl;
  va_start(vl, pszFmt);
  vsprintf(buf, pszFmt, vl);
  Puts(rc, x, y, sx, sy, color, shColor, buf);
}

// --------------------------------------------------------------------------
// Function:      
// Purpose:       
// Parameters:    
// --------------------------------------------------------------------------
void CDisplayFont::Puts (CRenderContext &rc, float x, float y, float sx, float sy, dword color, dword shColor, const char *pszStr) const
{
  if (!Color_A(color | shColor) || !IsLoaded())
    return;

  int nVertsPerChar = (Color_A(color)? 6 : 0) + (Color_A(shColor)? 6 : 0);
  float initX = x;

  CDisplayTexture *pPrevTex = NULL;
  bool bPendingSetDevice = true;

  float txf;
  float tyf;

  while (*pszStr)
  {
    unsigned c = (unsigned char)*pszStr++;

    if (c == '\n')
    {
      x = initX;
      y += sy*(m_fontHeight+1);
      continue;
    }

    int l = m_aCharMap[c];

    // Get char data, and skip char if not printable.
    if (l == -1)
      continue;
    const TCharInfo &i = m_paChars[l];
    int cw = i.logWidth;
    if (!cw)
      continue;
    ASSERT(i.nTex < m_nTextures);
    
    CDisplayTexture *pTex = m_paTextures[i.nTex].pTex;
    if (!pTex->IsLoaded())
      continue;

    int vstart = 0;
    TTLVertexUV *pv = (TTLVertexUV *)m_pVB->Lock(nVertsPerChar, &vstart);
    if (pv)
    {
      if (pPrevTex != pTex)
      {
        m_Shader.GetStage(0, 0)->pTex = pTex;
        pPrevTex = pTex;
        m_Shader.Set(rc.GetDevice());

        txf = 1.f/pTex->GetWidth ();
        tyf = 1.f/pTex->GetHeight();
      }

      float ox = x + sx*float(i.offsX)+.0f;
      float oy = y + sy*float(i.offsY)+.0f;
      float oxw = ox + sx*i.w;
      float oyh = oy + sy*i.h;
      float o2x = ox + sx*1;
      float o2y = oy + sy*1;
      float o2xw = oxw + sx*1;
      float o2yh = oyh + sy*1;

      float tx0 = txf*(.5f+i.x    );
      float tx1 = txf*(.5f+i.x+i.w);
      float ty0 = tyf*(.5f+i.y    );
      float ty1 = tyf*(.5f+i.y+i.h);

      if (Color_A(shColor))
      {
        pv->Set(o2x , o2y , 0, 1, shColor, tx0, ty0);   pv++;
        pv->Set(o2xw, o2y , 0, 1, shColor, tx1, ty0);   pv++;
        pv->Set(o2x , o2yh, 0, 1, shColor, tx0, ty1);   pv++;
        *pv = pv[-1];   pv++;
        *pv = pv[-3];   pv++;
        pv->Set(o2xw, o2yh, 0, 1, shColor, tx1, ty1);   pv++;
      }

      if (Color_A(color))
      {
        pv->Set(ox , oy , 0, 1, color, tx0, ty0);   pv++;
        pv->Set(oxw, oy , 0, 1, color, tx1, ty0);   pv++;
        pv->Set(ox , oyh, 0, 1, color, tx0, ty1);   pv++;
        *pv = pv[-1];  pv++;
        *pv = pv[-3];  pv++;
        pv->Set(oxw, oyh, 0, 1, color, tx1, ty1);   pv++;
      }

      m_pVB->Unlock();

      if (bPendingSetDevice)
      {
        rc.GetDevice()->SetFVFShader(m_pVB->GetFVF());
        rc.GetDevice()->GetDirect3DDevice()->SetStreamSource(0, m_pVB->GetVB(), 0, sizeof(*pv));
        bPendingSetDevice  = false;
      }
      rc.GetDevice()->GetDirect3DDevice()->DrawPrimitive(D3DPT_TRIANGLELIST, vstart, nVertsPerChar/3);

      x += sx*(cw+1);
    }
  }
}
